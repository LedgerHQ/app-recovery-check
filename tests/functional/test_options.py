from pathlib import Path

from ledgered.devices import Device
from ragger.navigator import NavIns
from ragger.navigator.navigator import Navigator

from .navigator import CustomNavInsID
from .utils import format_instructions


def test_check_info_then_leave(navigator: Navigator, default_screenshot_path: Path):
    instructions = format_instructions([CustomNavInsID.HOME_TO_SETTINGS, CustomNavInsID.SETTINGS_TO_HOME])
    navigator.navigate_and_compare(
        default_screenshot_path,
        "check_info_then_leave",
        instructions,
        screen_change_before_first_instruction=False,
        screen_change_after_last_instruction=True,
    )


def test_check_all_passphrase_lengths(navigator: Navigator, default_screenshot_path: Path, device: Device):
    # Touch: all three bars are directly tappable; LENGTH_CHOOSE_* performs a single tap.
    # Nano: the length page always resets to bar 0 on entry, so we navigate bar by bar
    # with LENGTH_NEXT to capture each bar in a dedicated snapshot before confirming.

    instructions = [
        CustomNavInsID.HOME_TO_CHECK,  # Select words length
    ]
    if device.is_nano:
        instructions += [
            CustomNavInsID.LENGTH_NEXT,  # bar 1: 18 words
            CustomNavInsID.LENGTH_NEXT,  # bar 2: 24 words
            CustomNavInsID.LENGTH_SELECT,  # enter keyboard (24-word mode)
            CustomNavInsID.LENGTH_TO_PREVIOUS,  # back to bar 0
            CustomNavInsID.LENGTH_NEXT,  # bar 1: 18 words
            CustomNavInsID.LENGTH_SELECT,  # enter keyboard (18-word mode)
            CustomNavInsID.LENGTH_TO_PREVIOUS,  # back to bar 0
            CustomNavInsID.LENGTH_SELECT,  # enter keyboard (12-word mode)
        ]
    else:
        instructions += [
            CustomNavInsID.LENGTH_CHOOSE_24,
            CustomNavInsID.LENGTH_TO_PREVIOUS,
            CustomNavInsID.LENGTH_CHOOSE_18,
            CustomNavInsID.LENGTH_TO_PREVIOUS,
            CustomNavInsID.LENGTH_CHOOSE_12,
        ]
    instructions += [CustomNavInsID.LENGTH_TO_PREVIOUS]

    navigator.navigate_and_compare(
        default_screenshot_path,
        "check_all_passphrase_lengths",
        format_instructions(instructions),
        screen_change_before_first_instruction=False,
        screen_change_after_last_instruction=True,
    )


def test_check_previous_word(navigator: Navigator, default_screenshot_path: Path):
    instructions = format_instructions(
        [
            CustomNavInsID.HOME_TO_CHECK,
            CustomNavInsID.LENGTH_CHOOSE_24,
            NavIns(CustomNavInsID.KEYBOARD_WRITE, args=("rand",)),
            NavIns(CustomNavInsID.KEYBOARD_SELECT_SUGGESTION, args=(1,)),
            NavIns(CustomNavInsID.KEYBOARD_WRITE, args=("ok",)),
            NavIns(CustomNavInsID.KEYBOARD_SELECT_SUGGESTION, args=(1,)),
            CustomNavInsID.KEYBOARD_TO_PREVIOUS,
            CustomNavInsID.KEYBOARD_TO_PREVIOUS,
            CustomNavInsID.KEYBOARD_TO_PREVIOUS,
            CustomNavInsID.LENGTH_TO_PREVIOUS,
        ]
    )
    navigator.navigate_and_compare(
        default_screenshot_path,
        "check_previous_word",
        instructions,
        screen_change_before_first_instruction=False,
        screen_change_after_last_instruction=True,
    )
