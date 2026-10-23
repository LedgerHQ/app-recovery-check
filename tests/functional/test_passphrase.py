from collections.abc import Generator
from pathlib import Path

from mnemonic import Mnemonic
from pytest import fixture
from ragger.bip.seed import SPECULOS_MNEMONIC
from ragger.navigator import NavIns
from ragger.navigator.navigator import Navigator

from .navigator import CustomNavInsID
from .utils import format_instructions

# BIP39 passphrase the device of this module is unlocked with
PASSPHRASE = "ledger"


# Speculos holds the seed of SPECULOS_MNEMONIC with PASSPHRASE, as a device unlocked with it
@fixture(scope="module")
def additional_speculos_arguments() -> list[str]:
    return ["--seed", "hex:" + Mnemonic.to_seed(SPECULOS_MNEMONIC, PASSPHRASE).hex()]


@fixture
def passphrase_enabled(navigator: Navigator) -> Generator[None, None, None]:
    toggle = format_instructions(
        [
            CustomNavInsID.HOME_TO_SETTINGS,
            CustomNavInsID.SETTINGS_TOGGLE_PASSPHRASE,
            CustomNavInsID.SETTINGS_TO_HOME,
        ]
    )
    navigator.navigate(toggle, screen_change_before_first_instruction=False)
    yield
    navigator.navigate(toggle, screen_change_before_first_instruction=False)


def _enter_words() -> list[NavIns | CustomNavInsID]:
    instructions: list[NavIns | CustomNavInsID] = [
        CustomNavInsID.HOME_TO_CHECK,
        CustomNavInsID.LENGTH_CHOOSE_24,
    ]
    for word in SPECULOS_MNEMONIC.split():
        instructions += [
            NavIns(CustomNavInsID.KEYBOARD_WRITE, args=(word[:4],)),
            NavIns(CustomNavInsID.KEYBOARD_SELECT_SUGGESTION, args=(1,)),
        ]
    return instructions


def _check_with_passphrase(navigator: Navigator, default_screenshot_path: Path, passphrase: str, test_name: str):
    navigator.navigate(
        format_instructions(_enter_words()),
        screen_change_before_first_instruction=False,
        screen_change_after_last_instruction=False,
    )
    navigator.navigate_and_compare(
        default_screenshot_path,
        test_name,
        format_instructions(
            [
                NavIns(CustomNavInsID.PASSPHRASE_WRITE, args=(passphrase,)),
                CustomNavInsID.PASSPHRASE_CONFIRM,
                CustomNavInsID.RESULT_TO_HOME,
            ]
        ),
        screen_change_before_first_instruction=True,
        screen_change_after_last_instruction=False,
    )


def test_bip39_passphrase_check_ok(passphrase_enabled, navigator: Navigator, default_screenshot_path: Path):
    _check_with_passphrase(navigator, default_screenshot_path, PASSPHRASE, "bip39_passphrase_check_ok")


def test_bip39_passphrase_check_wrong(passphrase_enabled, navigator: Navigator, default_screenshot_path: Path):
    _check_with_passphrase(navigator, default_screenshot_path, PASSPHRASE[:-1], "bip39_passphrase_check_wrong")


def test_bip39_passphrase_check_missing(navigator: Navigator, default_screenshot_path: Path):
    # setting disabled: the words alone don't match a device unlocked with a passphrase
    navigator.navigate(
        format_instructions(_enter_words()),
        screen_change_before_first_instruction=False,
        screen_change_after_last_instruction=False,
    )
    navigator.navigate_and_compare(
        default_screenshot_path,
        "bip39_passphrase_check_missing",
        format_instructions([CustomNavInsID.RESULT_TO_HOME]),
        screen_change_before_first_instruction=True,
        screen_change_after_last_instruction=False,
    )


def test_bip39_passphrase_back_to_last_word(passphrase_enabled, navigator: Navigator, default_screenshot_path: Path):
    instructions = _enter_words()
    navigator.navigate(
        format_instructions(instructions[:-1]),
        screen_change_before_first_instruction=False,
        screen_change_after_last_instruction=False,
    )
    last_word = SPECULOS_MNEMONIC.split()[-1]
    navigator.navigate_and_compare(
        default_screenshot_path,
        "bip39_passphrase_back_to_last_word",
        format_instructions(
            [
                instructions[-1],
                CustomNavInsID.PASSPHRASE_TO_PREVIOUS,
                NavIns(CustomNavInsID.KEYBOARD_WRITE, args=(last_word[:4],)),
                NavIns(CustomNavInsID.KEYBOARD_SELECT_SUGGESTION, args=(1,)),
                NavIns(CustomNavInsID.PASSPHRASE_WRITE, args=(PASSPHRASE,)),
                CustomNavInsID.PASSPHRASE_CONFIRM,
                CustomNavInsID.RESULT_TO_HOME,
            ]
        ),
        screen_change_before_first_instruction=False,
        screen_change_after_last_instruction=False,
    )
