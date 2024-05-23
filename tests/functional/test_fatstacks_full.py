from ragger.navigator import NavIns

from .navigator import CustomNavInsID, StaxNavigator
from .utils import format_instructions


SPECULOS_MNEMONIC = "glory promote mansion idle axis finger extra " \
    "february uncover one trip resource lawn turtle enact monster " \
    "seven myth punch hobby comfort wild raise skin"


def test_nominal_full_passphrase_check_ok(navigator: StaxNavigator, functional_test_directory: str):
    # instructions to go the the keyboard
    instructions = [
        CustomNavInsID.HOME_TO_CHECK,
        CustomNavInsID.LENGTH_CHOOSE_24,
    ]
    # instruction to write the words
    for word in SPECULOS_MNEMONIC.split():
        instructions += [
            NavIns(CustomNavInsID.KEYBOARD_WRITE, args=(word[:4], )),
            NavIns(CustomNavInsID.KEYBOARD_SELECT_SUGGESTION, args=(1, )),
        ]
    instructions = format_instructions(instructions)
    # running the instruction to go to result screen
    navigator.navigate(instructions,
                       screen_change_before_first_instruction=False,
                       screen_change_after_last_instruction=False)

    # now that the 24 words have been written, we check the resulting screen
    # should be correct

    instructions = format_instructions([
        CustomNavInsID.RESULT_TO_HOME,
    ])

    navigator.navigate_and_compare(functional_test_directory,
                                   "nominal_full_passphrase_check_ok",
                                   instructions,
                                   screen_change_before_first_instruction=True,
                                   screen_change_after_last_instruction=False)


def test_nominal_full_passphrase_check_error_wrong_passphrase(navigator: StaxNavigator, functional_test_directory: str):
    # instructions to go the the keyboard
    instructions = [
        CustomNavInsID.HOME_TO_CHECK,
        CustomNavInsID.LENGTH_CHOOSE_12,
    ]
    # instruction to write the words
    for word in SPECULOS_MNEMONIC.split()[:12]:
        instructions += [
            # test is designed to fail, so the first character is enough to select a word
            NavIns(CustomNavInsID.KEYBOARD_WRITE, args=(word[0], )),
            NavIns(CustomNavInsID.KEYBOARD_SELECT_SUGGESTION, args=(1, )),
        ]
    instructions = format_instructions(instructions)
    # running the instruction to go to result screen
    navigator.navigate(instructions,
                       screen_change_before_first_instruction=False,
                       screen_change_after_last_instruction=False)

    # now that the 12 words have been written, we check the resulting screen
    # should be incorrect

    instructions = format_instructions([
        CustomNavInsID.RESULT_TO_HOME,
    ])

    navigator.navigate_and_compare(functional_test_directory,
                                   "nominal_full_passphrase_check_error_wrong_passphrase",
                                   instructions,
                                   screen_change_before_first_instruction=True,
                                   screen_change_after_last_instruction=False)
