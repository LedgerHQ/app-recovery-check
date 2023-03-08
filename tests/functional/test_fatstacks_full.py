from time import sleep

from pytest import fixture
from ragger.backend import BackendInterface

from .app import Screen
from .utils import assert_current_equals, SCREENSHOTS


SPECULOS_MNEMONIC = "glory promote mansion idle axis finger extra " \
    "february uncover one trip resource lawn turtle enact monster " \
    "seven myth punch hobby comfort wild raise skin"


def test_nominal_full_passphrase_check(screen: Screen, backend: BackendInterface):
    # going to choose mnemonic length
    screen.home.action()
    assert_current_equals(backend, SCREENSHOTS / "passphrase_length.png")
    # choosing 3d (24 words)
    screen.choice_list.choose(1)
    assert_current_equals(backend, SCREENSHOTS / "first_24.png")
    for word in SPECULOS_MNEMONIC.split():
        # 4 letters are enough to discriminate the correct word
        screen.keyboard.write(word[:4])
        # choosing 1st suggestion
        screen.suggestions.choose(1)
    sleep(0.1)
    assert_current_equals(backend, SCREENSHOTS / "correct.png")
    screen.dismiss.tap() # exit the result screen to the home page
    screen.exit()


def test_nominal_full_passphrase_check_error_wrong_passphrase(screen: Screen, backend: BackendInterface):
    screen.home.action()
    assert_current_equals(backend, SCREENSHOTS / "passphrase_length.png")
    # choosing 1st (12 words)
    screen.choice_list.choose(3)
    assert_current_equals(backend, SCREENSHOTS / "first_12.png")
     # only the 12 first words
    for word in SPECULOS_MNEMONIC.split()[:12]:
        screen.keyboard.write(word[:4])
        screen.suggestions.choose(1)
    sleep(0.1)
    assert_current_equals(backend, SCREENSHOTS / "incorrect.png")
    screen.dismiss.tap() # exit the result screen to the home page
    screen.exit()
