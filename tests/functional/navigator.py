from enum import auto, Enum
from functools import partial
from ragger.navigator import NavInsID
from ragger.navigator.navigator import Navigator
from time import sleep

from .app import StaxScreen


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
            # has to be defined for Ragger Navigator internals
            NavInsID.WAIT: sleep,
            CustomNavInsID.WAIT: sleep,
            CustomNavInsID.TOUCH: backend.finger_touch,
            CustomNavInsID.HOME_TO_SETTINGS: self.screen.home.settings,
            CustomNavInsID.HOME_TO_QUIT: self.screen.home.quit,
            CustomNavInsID.HOME_TO_CHECK: self.screen.home.action,
            CustomNavInsID.SETTINGS_TO_HOME: self.screen.settings.single_page_exit,
            CustomNavInsID.LENGTH_CHOOSE_24: partial(self.screen.choice_list.choose, 1),
            CustomNavInsID.LENGTH_CHOOSE_18: partial(self.screen.choice_list.choose, 2),
            CustomNavInsID.LENGTH_CHOOSE_12: partial(self.screen.choice_list.choose, 3),
            CustomNavInsID.LENGTH_TO_PREVIOUS: self.screen.navigation.tap,
            CustomNavInsID.KEYBOARD_TO_PREVIOUS: self.screen.navigation.tap,
            CustomNavInsID.KEYBOARD_WRITE: self._write,
            CustomNavInsID.KEYBOARD_SELECT_SUGGESTION: self.screen.suggestions.choose,
            CustomNavInsID.RESULT_TO_HOME: self.screen.dismiss.tap
        }
        super().__init__(backend, firmware, callbacks) #, golden_run=True)

    def _write(self, characters: str):
        # keyboard write is not an exact science on Ragger for now. The instruction together with
        # the `wait_for_screen_change` function can get messy, as the write performs multiple
        # `finger_touch` and the screen change several time, so `navigate_and_compare` could think
        # the screen can be compared, when it has not reached its last state yet.
        # Adding extra time after writing to have a better chance to get the expected screen
        self.screen.keyboard.write(characters)
        sleep(1)
