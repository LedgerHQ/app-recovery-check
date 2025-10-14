from typing import Generator, Any
from pytest import fixture, skip

from ledgered.devices import Device

from ragger.backend import BackendInterface

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
def navigator(backend: BackendInterface, device: Device, golden_run: bool) -> Generator[Any, Any, Any]:
    touchNav = TouchNavigator(backend, device, golden_run)
    yield touchNav

# Tests are not supported on Nano devices
@fixture(scope="session")
def skip_tests_for_unsupported_devices(device: Device):
    if device.is_nano:
        skip(f"Device {device.name} is not supported")
