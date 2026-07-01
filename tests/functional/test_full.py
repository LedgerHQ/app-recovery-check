from pathlib import Path

from ragger.navigator import NavIns
from ragger.bip.seed import SPECULOS_MNEMONIC
from ragger.navigator.navigator import Navigator

from .navigator import CustomNavInsID
from .utils import format_instructions


PLAUSIBLE_MNEMONIC = (
    "feature trigger apart fold answer lend enrich blind foam deny match ecology "
    "reform again snow stadium vibrant brain hungry already sadness verify team speed"
)


def test_nominal_full_passphrase_check_ok(
    navigator: Navigator, default_screenshot_path: Path
):
    # instructions to go the the keyboard
    instructions = [
        CustomNavInsID.HOME_TO_CHECK,
        CustomNavInsID.LENGTH_CHOOSE_24,
    ]
    # instruction to write the words
    for word in SPECULOS_MNEMONIC.split():
        instructions += [
            NavIns(CustomNavInsID.KEYBOARD_WRITE, args=(word[:4],)),
            NavIns(CustomNavInsID.KEYBOARD_SELECT_SUGGESTION, args=(1,)),
        ]
    instructions = format_instructions(instructions)
    # running the instruction to go to result screen
    navigator.navigate(
        instructions,
        screen_change_before_first_instruction=False,
        screen_change_after_last_instruction=False,
    )

    # now that the 24 words have been written, we check the resulting screen
    # should be correct

    instructions = format_instructions(
        [
            CustomNavInsID.RESULT_TO_HOME,
        ]
    )

    navigator.navigate_and_compare(
        default_screenshot_path,
        "nominal_full_passphrase_check_ok",
        instructions,
        screen_change_before_first_instruction=True,
        screen_change_after_last_instruction=False,
    )


def test_nominal_full_passphrase_check_plausible_but_wrong(
    navigator: Navigator, default_screenshot_path: Path
):
    # instructions to go the the keyboard
    instructions = [
        CustomNavInsID.HOME_TO_CHECK,
        CustomNavInsID.LENGTH_CHOOSE_24,
    ]
    # instruction to write the words
    for word in PLAUSIBLE_MNEMONIC.split():
        instructions += [
            NavIns(CustomNavInsID.KEYBOARD_WRITE, args=(word[:4],)),
            NavIns(CustomNavInsID.KEYBOARD_SELECT_SUGGESTION, args=(1,)),
        ]
    instructions = format_instructions(instructions)
    # running the instruction to go to result screen
    navigator.navigate(
        instructions,
        screen_change_before_first_instruction=False,
        screen_change_after_last_instruction=False,
    )

    # now that the 24 words have been written, we check the resulting screen
    # should be correct

    instructions = format_instructions(
        [
            CustomNavInsID.RESULT_TO_HOME,
        ]
    )

    navigator.navigate_and_compare(
        default_screenshot_path,
        "nominal_full_passphrase_check_incorrect",
        instructions,
        screen_change_before_first_instruction=True,
        screen_change_after_last_instruction=False,
    )


def test_nominal_full_passphrase_check_error_wrong_passphrase(
    navigator: Navigator, default_screenshot_path: Path
):
    # instructions to go the the keyboard
    instructions = [
        CustomNavInsID.HOME_TO_CHECK,
        CustomNavInsID.LENGTH_CHOOSE_12,
    ]
    # instruction to write the words
    for word in SPECULOS_MNEMONIC.split()[:12]:
        instructions += [
            # test is designed to fail, so the first character is enough to select a word
            NavIns(CustomNavInsID.KEYBOARD_WRITE, args=(word[0:2],)),
            NavIns(CustomNavInsID.KEYBOARD_SELECT_SUGGESTION, args=(1,)),
        ]
    instructions = format_instructions(instructions)
    # running the instruction to go to result screen
    navigator.navigate(
        instructions,
        screen_change_before_first_instruction=False,
        screen_change_after_last_instruction=False,
    )

    # now that the 12 words have been written, we check the resulting screen
    # should be incorrect

    instructions = format_instructions(
        [
            CustomNavInsID.RESULT_TO_HOME,
        ]
    )

    navigator.navigate_and_compare(
        default_screenshot_path,
        "nominal_full_passphrase_check_incorrect",
        instructions,
        screen_change_before_first_instruction=True,
        screen_change_after_last_instruction=False,
    )
