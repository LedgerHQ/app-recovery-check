from ragger.navigator import NavIns

from .navigator import NavInsID, StaxNavigator
from .utils import format_instructions


SPECULOS_MNEMONIC = "glory promote mansion idle axis finger extra " \
    "february uncover one trip resource lawn turtle enact monster " \
    "seven myth punch hobby comfort wild raise skin"


def test_nominal_full_passphrase_check_ok(navigator: StaxNavigator, functional_test_directory: str):
    # instructions to go the the keyboard
    instructions = [
        NavInsID.HOME_TO_CHECK,
        NavInsID.LENGTH_CHOOSE_24,
        NavIns(NavInsID.WAIT, args=(0.5, ))
    ]
    # instruction to write the words
    for word in SPECULOS_MNEMONIC.split():
        instructions += [
            NavIns(NavInsID.KEYBOARD_WRITE, args=(word[:4], )),
            NavIns(NavInsID.KEYBOARD_SELECT_SUGGESTION, args=(1, )),
        ]
    instructions.append(NavIns(NavInsID.WAIT, args=(1, )))
    instructions = format_instructions(instructions)
    # running the instruction to go to result screen
    navigator.navigate(instructions)

    # now that the 24 words have been written, we check the resulting screen
    # should be correct

    instructions = format_instructions([
        NavInsID.RESULT_TO_HOME,
    ])

    navigator.navigate_and_compare(functional_test_directory,
                                   "nominal_full_passphrase_check_ok",
                                   instructions,
                                   screen_change_before_first_instruction=True,
                                   screen_change_after_last_instruction=False)


def test_nominal_full_passphrase_check_error_wrong_passphrase(navigator: StaxNavigator, functional_test_directory: str):
    # instructions to go the the keyboard
    instructions = [
        NavInsID.HOME_TO_CHECK,
        NavInsID.LENGTH_CHOOSE_12,
        NavIns(NavInsID.WAIT, args=(0.5, ))
    ]
    # instruction to write the words
    for word in SPECULOS_MNEMONIC.split()[:12]:
        instructions += [
            NavIns(NavInsID.KEYBOARD_WRITE, args=(word[:4], )),
            NavIns(NavInsID.KEYBOARD_SELECT_SUGGESTION, args=(1, )),
        ]
    instructions.append(NavIns(NavInsID.WAIT, args=(1, )))
    instructions = format_instructions(instructions)
    # running the instruction to go to result screen
    navigator.navigate(instructions)

    # now that the 12 words have been written, we check the resulting screen
    # should be incorrect

    instructions = format_instructions([
        NavInsID.RESULT_TO_HOME,
    ])

    navigator.navigate_and_compare(functional_test_directory,
                                   "nominal_full_passphrase_check_error_wrong_passphrase",
                                   instructions,
                                   screen_change_before_first_instruction=True,
                                   screen_change_after_last_instruction=False)
