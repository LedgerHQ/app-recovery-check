from time import sleep
from pathlib import Path

from pytest import fixture
from ragger.backend import BackendInterface

from .app import Screen
from .utils import assert_current_equals, SCREENSHOTS


SPECULOS_MNEMONIC = "glory promote mansion idle axis finger extra " \
    "february uncover one trip resource lawn turtle enact monster " \
    "seven myth punch hobby comfort wild raise skin"

@fixture
def screen(client: BackendInterface):
    s = Screen(client, client.firmware)
    client.finger_touch(600, 0)
    assert_current_equals(client, SCREENSHOTS / "welcome.png")
    return s


def test_nominal_full_passphrase_check(screen: Screen, client: BackendInterface):
    # going to choose mnemonic length
    screen.center.tap()
    assert_current_equals(client, SCREENSHOTS / "passphrase_length.png")
    # choosing 3d (24 words)
    screen.choice_list.choose(3)
    assert_current_equals(client, SCREENSHOTS / "first_24.png")
    for word in SPECULOS_MNEMONIC.split():
        # 4 letters are enough to discriminate the correct word
        screen.keyboard.write(word[:4])
        # choosing 1st suggestion
        screen.suggestions.choose(1)
    sleep(0.1)
    assert_current_equals(client, SCREENSHOTS / "correct.png")
    screen.exit()


def test_check_info_then_leave(screen: Screen, client: BackendInterface):
    screen.info.tap()
    assert_current_equals(client, SCREENSHOTS / "info.png")
    screen.footer.tap()
    assert_current_equals(client, SCREENSHOTS / "welcome.png")
    screen.exit()


def test_nominal_full_passphrase_check_error_wrong_passphrase(screen: Screen, client: BackendInterface):
    screen.center.tap()
    assert_current_equals(client, SCREENSHOTS / "passphrase_length.png")
    # choosing 1st (12 words)
    screen.choice_list.choose(1)
    assert_current_equals(client, SCREENSHOTS / "first_12.png")
     # only the 12 first words
    for word in SPECULOS_MNEMONIC.split()[:12]:
        screen.keyboard.write(word[:4])
        screen.suggestions.choose(1)
    sleep(0.1)
    assert_current_equals(client, SCREENSHOTS / "incorrect.png")
    screen.exit()


def test_check_previous_word(screen: Screen, client: BackendInterface):
    screen.center.tap()
    screen.choice_list.choose(1)
    assert_current_equals(client, SCREENSHOTS / "first_12.png")
    tries = ["rand", "ok"]
    for word in tries:
        screen.keyboard.write(word[:4])
        screen.suggestions.choose(1)
    # coming back N time, should bring back to the first word page
    for _ in tries:
        screen.navigation.tap()
    assert_current_equals(client, SCREENSHOTS / "first_12.png")
    # one more 'back' tap will bring us to the passphrase length choice page
    screen.navigation.tap()
    assert_current_equals(client, SCREENSHOTS / "passphrase_length.png")
