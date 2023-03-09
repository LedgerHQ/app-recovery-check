from pathlib import Path
from pytest import fixture
from ragger.backend import BackendInterface
from ragger.conftest import configuration
from ragger.firmware import Firmware

from .navigator import StaxNavigator


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

FUNCTIONAL_TESTS_DIR = Path("tests/functional/").resolve()


@fixture(scope="session")
def functional_test_directory() -> Path:
    yield FUNCTIONAL_TESTS_DIR


@fixture
def navigator(backend: BackendInterface, firmware: Firmware) -> StaxNavigator:
    navigator = StaxNavigator(backend, firmware)
    yield navigator
