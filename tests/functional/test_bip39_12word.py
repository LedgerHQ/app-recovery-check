from pytest import fixture
from pytest import mark
from ragger.navigator import NavInsID
from ragger.conftest import configuration

@fixture(scope='session')
def set_seed():
    # Seed taken from https://github.com/BlockchainCommons/crypto-commons/blob/master/Docs/sskr-test-vector.md#128-bit-seed
    configuration.OPTIONAL.CUSTOM_SEED = "fly mule excess resource treat plunge nose soda reflect adult ramp planet"

def nanos_bip39_12word(backend, navigator):
    backend.wait_for_text_on_screen("Check BIP39", 5)
    backend.wait_for_text_on_screen("recovery phras", 1)
    instructions = [
        NavInsID.BOTH_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.BOTH_CLICK,
        NavInsID.BOTH_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.BOTH_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.BOTH_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.BOTH_CLICK,
        NavInsID.BOTH_CLICK,
        NavInsID.BOTH_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.BOTH_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.BOTH_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.BOTH_CLICK,
        NavInsID.BOTH_CLICK,
        NavInsID.BOTH_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.BOTH_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.BOTH_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.BOTH_CLICK,
        NavInsID.BOTH_CLICK,
        NavInsID.BOTH_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.BOTH_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.BOTH_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.BOTH_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.BOTH_CLICK,
        NavInsID.BOTH_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.BOTH_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.BOTH_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.BOTH_CLICK,
        NavInsID.BOTH_CLICK,
        NavInsID.BOTH_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.BOTH_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.BOTH_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.BOTH_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.BOTH_CLICK,
        NavInsID.BOTH_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.BOTH_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.BOTH_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.BOTH_CLICK,
        NavInsID.BOTH_CLICK,
        NavInsID.BOTH_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.BOTH_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.BOTH_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.BOTH_CLICK,
        NavInsID.BOTH_CLICK,
        NavInsID.BOTH_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.BOTH_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.BOTH_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.BOTH_CLICK,
        NavInsID.BOTH_CLICK,
        NavInsID.BOTH_CLICK,
        NavInsID.BOTH_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.BOTH_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.BOTH_CLICK,
        NavInsID.BOTH_CLICK,
        NavInsID.BOTH_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.BOTH_CLICK,
        NavInsID.BOTH_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.BOTH_CLICK,
        NavInsID.BOTH_CLICK,
        NavInsID.BOTH_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.LEFT_CLICK,
        NavInsID.BOTH_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.BOTH_CLICK,
        NavInsID.BOTH_CLICK,
        NavInsID.RIGHT_CLICK,
        NavInsID.BOTH_CLICK
    ]
    navigator.navigate(instructions, screen_change_before_first_instruction=False)
    backend.wait_for_text_on_screen("BIP39 Phrase", 5)
    backend.wait_for_text_on_screen("is correct", 1)
    navigator.navigate([NavInsID.RIGHT_CLICK], screen_change_before_first_instruction=False)
    backend.wait_for_text_on_screen("Quit", 1)
    navigator.navigate([NavInsID.RIGHT_CLICK], screen_change_before_first_instruction=False)
    backend.wait_for_text_on_screen("Generate", 1)
    backend.wait_for_text_on_screen("SSKR phrases", 1)
    navigator.navigate([NavInsID.BOTH_CLICK], screen_change_before_first_instruction=False)
    backend.wait_for_text_on_screen("Select number", 1)
    backend.wait_for_text_on_screen("of shares", 1)
    navigator.navigate([NavInsID.RIGHT_CLICK,
                        NavInsID.RIGHT_CLICK,
                        NavInsID.RIGHT_CLICK,
                        NavInsID.BOTH_CLICK],
                        screen_change_before_first_instruction=False)
    backend.wait_for_text_on_screen("Select", 1)
    backend.wait_for_text_on_screen("threshold", 1)
    navigator.navigate([NavInsID.RIGHT_CLICK,
                        NavInsID.RIGHT_CLICK,
                        NavInsID.BOTH_CLICK],
                        screen_change_before_first_instruction=False)
    backend.wait_for_text_on_screen("SSKR Share #1", 5)
    backend.wait_for_text_on_screen("tuna acid epic gyro", 1)
    navigator.navigate_until_text(NavInsID.RIGHT_CLICK, [], "SSKR Share #2", 20, screen_change_before_first_instruction=False)
    backend.wait_for_text_on_screen("tuna acid epic gyro", 1)
    navigator.navigate_until_text(NavInsID.RIGHT_CLICK, [], "SSKR Share #3", 20, screen_change_before_first_instruction=False)
    backend.wait_for_text_on_screen("tuna acid epic gyro", 1)
    navigator.navigate_until_text(NavInsID.RIGHT_CLICK, [], "Quit", 20, screen_change_before_first_instruction=False)

@mark.use_on_backend("speculos")
def test_bip39_12word(firmware, backend, navigator, set_seed):
    if firmware.device == "nanos":
        nanos_bip39_12word(backend, navigator)