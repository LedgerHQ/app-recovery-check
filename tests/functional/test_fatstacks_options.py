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


def test_check_info_then_leave(screen: Screen, client: BackendInterface):
    screen.home.info()
    assert_current_equals(client, SCREENSHOTS / "info.png")
    screen.quit_info.tap()
    assert_current_equals(client, SCREENSHOTS / "welcome.png")
    screen.exit()


def test_check_all_passphrase_lengths(screen: Screen, client: BackendInterface):
    screen.home.action()
    assert_current_equals(client, SCREENSHOTS / "passphrase_length.png")
    for choice, length in [(1, 24), (2, 18), (3, 12)]:
        screen.choice_list.choose(choice)
        assert_current_equals(client, SCREENSHOTS / f"first_{length}.png")
        screen.navigation.tap()
        assert_current_equals(client, SCREENSHOTS / "passphrase_length.png")


def test_check_previous_word(screen: Screen, client: BackendInterface):
    screen.home.action()
    screen.choice_list.choose(1)
    assert_current_equals(client, SCREENSHOTS / "first_24.png")
    tries = ["rand", "ok"]
    for word in tries:
        screen.keyboard.write(word[:4])
        screen.suggestions.choose(1)
    # coming back N time, should bring back to the first word page
    for _ in tries:
        screen.navigation.tap()
    assert_current_equals(client, SCREENSHOTS / "first_24.png")
    # one more 'back' tap will bring us to the passphrase length choice page
    screen.navigation.tap()
    assert_current_equals(client, SCREENSHOTS / "passphrase_length.png")
