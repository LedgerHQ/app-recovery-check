from ragger.navigator import NavIns

from .navigator import CustomNavInsID, StaxNavigator
from .utils import format_instructions


SPECULOS_MNEMONIC = "glory promote mansion idle axis finger extra " \
    "february uncover one trip resource lawn turtle enact monster " \
    "seven myth punch hobby comfort wild raise skin"


def test_check_info_then_leave(navigator: StaxNavigator, functional_test_directory: str):
    instructions = format_instructions([
        CustomNavInsID.HOME_TO_SETTINGS,
        CustomNavInsID.SETTINGS_TO_HOME
    ])
    navigator.navigate_and_compare(functional_test_directory,
                                   "check_info_then_leave",
                                   instructions,
                                   screen_change_before_first_instruction=False,
                                   screen_change_after_last_instruction=True)


def test_check_all_passphrase_lengths(navigator: StaxNavigator, functional_test_directory: str):
    instructions = format_instructions([
        CustomNavInsID.HOME_TO_CHECK,
        CustomNavInsID.LENGTH_CHOOSE_24,
        CustomNavInsID.LENGTH_TO_PREVIOUS,
        CustomNavInsID.LENGTH_CHOOSE_18,
        CustomNavInsID.LENGTH_TO_PREVIOUS,
        CustomNavInsID.LENGTH_CHOOSE_12,
        CustomNavInsID.LENGTH_TO_PREVIOUS
    ])
    navigator.navigate_and_compare(functional_test_directory,
                                   "check_all_passphrase_lengths",
                                   instructions,
                                   screen_change_before_first_instruction=False,
                                   screen_change_after_last_instruction=True)


def test_check_previous_word(navigator: StaxNavigator, functional_test_directory: str):
    instructions = format_instructions([
        CustomNavInsID.HOME_TO_CHECK,
        CustomNavInsID.LENGTH_CHOOSE_24,
        NavIns(CustomNavInsID.KEYBOARD_WRITE, args=("rand", )),
        NavIns(CustomNavInsID.KEYBOARD_SELECT_SUGGESTION, args=(1, )),
        NavIns(CustomNavInsID.KEYBOARD_WRITE, args=("ok", )),
        NavIns(CustomNavInsID.KEYBOARD_SELECT_SUGGESTION, args=(1, )),
        CustomNavInsID.KEYBOARD_TO_PREVIOUS,
        CustomNavInsID.KEYBOARD_TO_PREVIOUS,
        CustomNavInsID.KEYBOARD_TO_PREVIOUS,
        CustomNavInsID.LENGTH_TO_PREVIOUS
    ])
    navigator.navigate_and_compare(functional_test_directory,
                                   "check_previous_word",
                                   instructions,
                                   screen_change_before_first_instruction=False,
                                   screen_change_after_last_instruction=True)
