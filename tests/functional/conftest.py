from pathlib import Path
from pytest import fixture, skip
from ragger.backend import BackendInterface
from ragger.firmware import Firmware

from .navigator import TouchNavigator


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


@fixture
def navigator(backend: BackendInterface, firmware: Firmware, golden_run: bool) -> TouchNavigator:
    navigator = TouchNavigator(backend, firmware, golden_run)
    yield navigator

# Tests are not supported on Nano devices
@fixture(scope="session")
def skip_tests_for_unsupported_devices(firmware: Firmware):
    if firmware.is_nano:
        skip(f"Device {firmware.name} is not supported")
