from pytest import fixture
from ragger.backend import BackendInterface
from ragger.conftest import configuration

from .app import Screen
from .utils import assert_current_equals, SCREENSHOTS

###########################
### CONFIGURATION START ###
###########################

# You can configure optional parameters by overriding the value of ragger.configuration.OPTIONAL_CONFIGURATION
# Please refer to ragger/conftest/configuration.py for their descriptions and accepted values

#########################
### CONFIGURATION END ###
#########################

# Pull all features from the base ragger conftest using the overridden configuration
pytest_plugins = ("ragger.conftest.base_conftest", )

from time import sleep

@fixture
def screen(backend: BackendInterface):
    s = Screen(backend, backend.firmware)
    backend.finger_touch(600, 0)
    sleep(1)
    assert_current_equals(backend, SCREENSHOTS / "welcome.png")
    return s
