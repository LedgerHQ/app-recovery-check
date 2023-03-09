from enum import auto, Enum
from functools import partial
from ragger.navigator.navigator import Navigator
from time import sleep

from .app import StaxScreen


class NavInsID(Enum):
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
    # option with a keyboard (enter a word of the passphrase)
    KEYBOARD_TO_PREVIOUS = auto()
    KEYBOARD_WRITE = auto()
    KEYBOARD_SELECT_SUGGESTION = auto()
    # result screen, one action: going back to home screen
    RESULT_TO_HOME = auto()


class StaxNavigator(Navigator):

    def __init__(self, backend, firmware):
        self.screen = StaxScreen(backend, firmware)

        callbacks = {
            NavInsID.WAIT: sleep,
            NavInsID.TOUCH: backend.finger_touch,
            NavInsID.HOME_TO_SETTINGS: self.screen.home.settings,
            NavInsID.HOME_TO_QUIT: self.screen.home.quit,
            NavInsID.HOME_TO_CHECK: self.screen.home.action,
            NavInsID.SETTINGS_TO_HOME: self.screen.settings.single_page_exit,
            NavInsID.LENGTH_CHOOSE_24: partial(self.screen.choice_list.choose, 1),
            NavInsID.LENGTH_CHOOSE_18: partial(self.screen.choice_list.choose, 2),
            NavInsID.LENGTH_CHOOSE_12: partial(self.screen.choice_list.choose, 3),
            NavInsID.LENGTH_TO_PREVIOUS: self.screen.navigation.tap,
            NavInsID.KEYBOARD_TO_PREVIOUS: self.screen.navigation.tap,
            NavInsID.KEYBOARD_WRITE: self._write,
            NavInsID.KEYBOARD_SELECT_SUGGESTION: self.screen.suggestions.choose,
            NavInsID.RESULT_TO_HOME: self.screen.dismiss.tap
        }
        super().__init__(backend, firmware, callbacks) #, golden_run=True)

    def _write(self, characters: str):
        # keyboard write is not an exact science on Ragger for now. The instruction together with
        # the `wait_for_screen_change` function can get messy, as the write performs multiple
        # `finger_touch` and the screen change several time, so `navigate_and_compare` could think
        # the screen can be compared, when it has not reached its last state yet.
        # Adding extra time after writing to have a better chance to get the expected screen
        self.screen.keyboard.write(characters)
        sleep(0.3)
