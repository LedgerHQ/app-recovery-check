from enum import auto, Enum
from functools import partial
from time import sleep

from ledgered.devices import Device
from mnemonic import Mnemonic

from ragger.backend import BackendInterface
from ragger.navigator import NavInsID
from ragger.navigator.navigator import Navigator

from .app import TouchScreen

# BIP39 English word list, used to compute the filtered alphabet at each keyboard step.
_BIP39_WORDS: list[str] = sorted(Mnemonic("english").wordlist)

# When the input buffer is empty the firmware applies no mask: all 26 letters are available.
_FULL_ALPHABET = "abcdefghijklmnopqrstuvwxyz"

# nbgl_layout.h — else branch (non-touch / nano)
_NB_MAX_SUGGESTION_BUTTONS_NANO = 8


def _available_letters(prefix: str) -> str:
    """Return sorted string of letters available on the nano keyboard for the given prefix.

    With an empty prefix the firmware mask is 0 (all letters enabled), so the full alphabet
    is returned.  Once one or more letters have been typed, only letters that extend the prefix
    into a valid BIP39 word are present on the keyboard.
    """
    if not prefix:
        return _FULL_ALPHABET
    letters = sorted({w[len(prefix)] for w in _BIP39_WORDS
                      if w.startswith(prefix) and len(w) > len(prefix)})
    return "".join(letters)


def _triggers_suggestion_mode(prefix: str) -> bool:
    """Return True if the given prefix causes the nano firmware to switch to suggestion mode.

    The firmware calls displaySuggestionSelection() as soon as the whole candidate list fits,
    i.e. when the number of matching BIP39 words is between 1 and NB_MAX_SUGGESTION_BUTTONS
    (1-8 on nano). The upper bound is inclusive so that a fully-typed word which is also the
    prefix of other words (e.g. "can", or "tu" → 8 words) can still be selected.
    """
    if len(prefix) < 2:
        return False
    matches = sum(1 for w in _BIP39_WORDS if w.startswith(prefix))
    return 0 < matches <= _NB_MAX_SUGGESTION_BUTTONS_NANO


class CustomNavInsID(Enum):
    # generic instructions
    WAIT = auto()
    TOUCH = auto()
    # home screen
    HOME_TO_SETTINGS = auto()
    HOME_TO_QUIT = auto()
    HOME_TO_CHECK = auto()
    # settings
    SETTINGS_TO_HOME = auto()
    # Recovery phrase length choice
    LENGTH_CHOOSE_24 = auto()
    LENGTH_CHOOSE_18 = auto()
    LENGTH_CHOOSE_12 = auto()
    LENGTH_TO_PREVIOUS = auto()
    # length selection page navigation (nano only: navigate bar by bar)
    LENGTH_NEXT = auto()
    LENGTH_SELECT = auto()
    # option with a keyboard (enter a word of the passphrase)
    KEYBOARD_TO_PREVIOUS = auto()
    KEYBOARD_WRITE = auto()
    KEYBOARD_SELECT_SUGGESTION = auto()
    # result screen, one action: going back to home screen
    RESULT_TO_HOME = auto()


class TouchNavigator(Navigator):

    def __init__(self, backend: BackendInterface, device: Device, golden_run: bool = False):
        self.screen = TouchScreen(backend, device)

        callbacks = {
            # has to be defined for Ragger Navigator internals
            NavInsID.WAIT: sleep,
            CustomNavInsID.WAIT: sleep,
            CustomNavInsID.TOUCH: backend.finger_touch,
            CustomNavInsID.HOME_TO_SETTINGS: self.screen.home.settings,
            CustomNavInsID.HOME_TO_QUIT: self.screen.home.quit,
            CustomNavInsID.HOME_TO_CHECK: self.screen.home.action,
            CustomNavInsID.SETTINGS_TO_HOME: self.screen.settings.single_page_exit,
            CustomNavInsID.LENGTH_CHOOSE_12: partial(self.screen.choice_list.choose, 1, device),
            CustomNavInsID.LENGTH_CHOOSE_18: partial(self.screen.choice_list.choose, 2, device),
            CustomNavInsID.LENGTH_CHOOSE_24: partial(self.screen.choice_list.choose, 3, device),
            CustomNavInsID.LENGTH_TO_PREVIOUS: self.screen.navigation.tap,
            CustomNavInsID.KEYBOARD_TO_PREVIOUS: self.screen.navigation.tap,
            CustomNavInsID.KEYBOARD_WRITE: self._write,
            CustomNavInsID.KEYBOARD_SELECT_SUGGESTION: self.screen.suggestions.choose,
            CustomNavInsID.RESULT_TO_HOME: self.screen.dismiss.tap
        }
        super().__init__(backend, device, callbacks, golden_run=golden_run)

    def _write(self, characters: str):
        # keyboard write is not an exact science on Ragger for now. The instruction together with
        # the `wait_for_screen_change` function can get messy, as the write performs multiple
        # `finger_touch` and the screen change several time, so `navigate_and_compare` could think
        # the screen can be compared, when it has not reached its last state yet.
        # Adding extra time after writing to have a better chance to get the expected screen
        self.screen.keyboard.write(characters)
        sleep(1)


class NanoNavigator(Navigator):

    def __init__(self, backend: BackendInterface, device: Device, golden_run: bool = False):
        self._backend = backend
        self._suggestion_prefix: str | None = None   # prefix at which suggestion mode triggered
        self._intended_chars: str | None = None       # characters passed to _write
        self._on_length_page = False  # True when the length-selection page is active
        self._word_count = 0          # words confirmed so far in the current keyboard session

        callbacks = {
            NavInsID.WAIT: sleep,
            CustomNavInsID.WAIT: sleep,
            # home screen (nbgl_useCaseHomeAndSettings): pages are Home → Action → Settings → Info → Quit
            CustomNavInsID.HOME_TO_CHECK: self._home_to_check,
            CustomNavInsID.HOME_TO_SETTINGS: self._home_to_settings,
            CustomNavInsID.HOME_TO_QUIT: self._home_to_quit,
            # settings screen
            CustomNavInsID.SETTINGS_TO_HOME: self._settings_to_home,
            # length selection (nbgl_useCaseNavigableContent BARS_LIST): each bar is a page,
            # last page is "Back"; bars are in order: 12, 18, 24 (pages 0, 1, 2)
            CustomNavInsID.LENGTH_NEXT: self._length_next,
            CustomNavInsID.LENGTH_SELECT: self._length_select,
            CustomNavInsID.LENGTH_CHOOSE_12: self._length_choose_12,
            CustomNavInsID.LENGTH_CHOOSE_18: self._length_choose_18,
            CustomNavInsID.LENGTH_CHOOSE_24: self._length_choose_24,
            CustomNavInsID.LENGTH_TO_PREVIOUS: self._length_to_previous,
            # keyboard (nbgl_useCaseKeyboard): RIGHT_CLICK = next letter, LEFT_CLICK = prev letter,
            # BOTH_CLICK = select; BACKSPACE reachable by LEFT_CLICK from 'a' (wrap-around)
            CustomNavInsID.KEYBOARD_TO_PREVIOUS: self._keyboard_to_previous,
            CustomNavInsID.KEYBOARD_WRITE: self._write,
            CustomNavInsID.KEYBOARD_SELECT_SUGGESTION: self._select_suggestion,
            # result screen (nbgl_useCaseAction): BOTH_CLICK returns to home
            CustomNavInsID.RESULT_TO_HOME: self._result_to_home,
        }
        super().__init__(backend, device, callbacks, golden_run=golden_run)

    def _enter_keyboard(self) -> None:
        """Update state whenever we transition to the keyboard page."""
        self._on_length_page = False
        self._word_count = 0

    def _keyboard_back(self) -> None:
        """Press BACKSPACE on the nano keyboard (LEFT wraps from 'a' to BACKSPACE_KEY)."""
        self._backend.left_click()
        self._backend.both_click()

    def _home_to_check(self):
        # Action page is one RIGHT from the home page
        self._backend.right_click()
        self._backend.both_click()
        self._on_length_page = True

    def _home_to_settings(self):
        # Settings page is two RIGHTs from the home page
        self._backend.right_click()
        self._backend.right_click()
        self._backend.both_click()

    def _home_to_quit(self):
        # Quit page is the last one; navigate until we reach it
        for _ in range(5):
            try:
                self._backend.wait_for_text_on_screen("Quit", timeout=0.5)
                break
            except Exception:
                self._backend.right_click()
        self._backend.both_click()

    def _settings_to_home(self):
        # Navigate to "Back" page in settings and confirm
        for _ in range(10):
            try:
                self._backend.wait_for_text_on_screen("Back", timeout=0.5)
                break
            except Exception:
                self._backend.right_click()
        self._backend.both_click()

    def _length_next(self):
        # Advance one bar forward on the length-selection page (RIGHT_CLICK = next page).
        self._backend.right_click()

    def _length_select(self):
        # Confirm the currently displayed bar and enter the keyboard.
        self._backend.both_click()
        self._enter_keyboard()

    def _length_choose_12(self):
        # "12 words" is page 0, already displayed on entry
        self._backend.both_click()
        self._enter_keyboard()

    def _length_choose_18(self):
        # "18 words" is page 1
        self._backend.right_click()
        self._backend.both_click()
        self._enter_keyboard()

    def _length_choose_24(self):
        # "24 words" is page 2
        self._backend.right_click()
        self._backend.right_click()
        self._backend.both_click()
        self._enter_keyboard()

    def _length_to_previous(self):
        if not self._on_length_page:
            # Called right after LENGTH_CHOOSE_*: we are on the keyboard page with an empty
            # buffer, so BACKSPACE calls keyboard_close() which goes back to the length page.
            self._keyboard_back()
            self._on_length_page = True
        else:
            # Called after all KEYBOARD_TO_PREVIOUS have drained the buffer: we are on the
            # length-selection page. The "Back" entry is the last of 4 pages (0-2 are bars,
            # 3 is Back). Navigate there from page 0 and confirm.
            for _ in range(3):
                self._backend.right_click()
            self._backend.both_click()
            self._on_length_page = False

    def _keyboard_to_previous(self):
        # BACKSPACE either removes the last word (stays on keyboard) or, when the buffer is
        # empty, calls keyboard_close() which goes back to the length-selection page.
        if self._word_count > 0:
            self._word_count -= 1
        else:
            self._on_length_page = True
        self._keyboard_back()

    def _navigate_to_letter(self, letter: str, prefix: str) -> None:
        # The keyboard starts at the first available letter after each selection.
        # With BIP39 filtering the available letters shrink as the prefix grows; the position
        # of the target letter in the filtered set is the number of RIGHT_CLICKs needed.
        available = _available_letters(prefix)
        position = available.index(letter.lower())
        for _ in range(position):
            self._backend.right_click()
            sleep(0.1)

    def _write(self, characters: str) -> None:
        self._suggestion_prefix = None
        self._intended_chars = characters.lower()
        prefix = ""

        def _type_char(char: str) -> bool:
            """Type one character and return True if suggestion mode triggered."""
            nonlocal prefix
            self._navigate_to_letter(char, prefix)
            self._backend.both_click()
            prefix += char
            sleep(0.5)
            if _triggers_suggestion_mode(prefix):
                self._suggestion_prefix = prefix
                return True
            return False

        # Type the characters given by the caller
        for char in characters.lower():
            if _type_char(char):
                return

        # Suggestion mode not yet triggered (prefix has >= NB_MAX_SUGGESTION_BUTTONS matches).
        # Extend with the first available letter at each step until it triggers.  This ensures
        # the device is in suggestion-selection mode before KEYBOARD_SELECT_SUGGESTION runs,
        # regardless of how many characters the caller provided.
        for _ in range(8):
            available = _available_letters(prefix)
            if not available or _type_char(available[0]):
                return

    def _select_suggestion(self, index: int) -> None:
        # When _write stopped early due to suggestion mode, recompute the 1-based position of
        # the target word in the BIP39-ordered suggestion list instead of using the passed index.
        # The caller may always pass args=(1,) and the correct word will still be selected.
        real_index = index
        if self._suggestion_prefix is not None and self._intended_chars is not None:
            suggestions = [w for w in _BIP39_WORDS if w.startswith(self._suggestion_prefix)]
            targets = [w for w in suggestions if w.startswith(self._intended_chars)]
            if len(targets) == 1:
                real_index = suggestions.index(targets[0]) + 1  # 1-based
            self._suggestion_prefix = None
            self._intended_chars = None

        for _ in range(real_index - 1):
            self._backend.right_click()
        self._backend.both_click()
        self._word_count += 1

    def _result_to_home(self) -> None:
        # nbgl_useCaseAction: BOTH_CLICK triggers display_home_page
        self._backend.both_click()
