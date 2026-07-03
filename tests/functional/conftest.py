from collections.abc import Generator
from typing import Any

from ledgered.devices import Device
from pytest import fixture
from ragger.backend import BackendInterface

from .navigator import NanoNavigator, TouchNavigator

###########################
### CONFIGURATION START ###
###########################

# You can configure optional parameters by overriding the value of ragger.configuration.OPTIONAL_CONFIGURATION
# Please refer to ragger/conftest/configuration.py for their descriptions and accepted values

#########################
### CONFIGURATION END ###
#########################

# Pull all features from the base ragger conftest using the overridden configuration
pytest_plugins = ("ragger.conftest.base_conftest",)


@fixture
def navigator(backend: BackendInterface, device: Device, golden_run: bool) -> Generator[Any, Any, Any]:
    if device.is_nano:
        yield NanoNavigator(backend, device, golden_run)
    else:
        yield TouchNavigator(backend, device, golden_run)
