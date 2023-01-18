import pytest
from pathlib import Path
from typing import Optional

from ragger.firmware import Firmware
from ragger.backend import SpeculosBackend, LedgerCommBackend, LedgerWalletBackend, BackendInterface


def __str__(self):        # also tried __repr__()
        # Attempt to print the 'select' attribute in "pytest -v" output
        return self.select


APPLICATION = (Path(__file__).parent.parent / "elfs" / "recovery_check.elf").resolve()
BACKENDS = ["speculos", "ledgercomm", "ledgerwallet"]
DEVICES = ["nanos", "nanox", "nanosp", "stax", "all"]
FIRMWARES = [
        Firmware('stax', '1.0'),
]


def pytest_addoption(parser):
    parser.addoption("--device", choices=DEVICES, required=True)
    parser.addoption("--backend", choices=BACKENDS, default="speculos")
    parser.addoption("--display", action="store_true", default=False)
    parser.addoption("--golden_run", action="store_true", default=False)
    parser.addoption("--log_apdu_file", action="store", default=None)


@pytest.fixture(scope="session")
def backend_name(pytestconfig):
    return pytestconfig.getoption("backend")


@pytest.fixture(scope="session")
def display(pytestconfig):
    return pytestconfig.getoption("display")


@pytest.fixture(scope="session")
def golden_run(pytestconfig):
    return pytestconfig.getoption("golden_run")


@pytest.fixture(scope="session")
def log_apdu_file(pytestconfig):
    filename = pytestconfig.getoption("log_apdu_file")
    return Path(filename).resolve() if filename is not None else None


# Glue to call every test that depends on the firmware once for each required firmware
def pytest_generate_tests(metafunc):
    if "firmware" in metafunc.fixturenames:
        fw_list = []
        ids = []
        device = metafunc.config.getoption("device")
        backend_name = metafunc.config.getoption("backend")
        if device == "all":
            if backend_name != "speculos":
                raise ValueError("Invalid device parameter on this backend")
            # Add all supported firmwares
            for fw in FIRMWARES:
                fw_list.append(fw)
                ids.append(fw.device + " " + fw.version)
        else:
            # Enable firmware for demanded device
            for fw in FIRMWARES:
                if device == fw.device:
                    fw_list.append(fw)
                    ids.append(fw.device + " " + fw.version)
        metafunc.parametrize("firmware", fw_list, ids=ids, scope="session")


def prepare_speculos_args(firmware: Firmware, display: bool):
    speculos_args = []
    if display:
        speculos_args += ["--display", "qt"]
    return ([APPLICATION], {"args": speculos_args})


def create_backend(backend_name: str, firmware: Firmware, display: bool, log_apdu_file: Optional[Path]):
    if backend_name.lower() == "ledgercomm":
        return LedgerCommBackend(firmware=firmware, interface="hid", log_apdu_file=log_apdu_file)
    elif backend_name.lower() == "ledgerwallet":
        return LedgerWalletBackend(firmware=firmware, log_apdu_file=log_apdu_file)
    elif backend_name.lower() == "speculos":
        args, kwargs = prepare_speculos_args(firmware, display)
        return SpeculosBackend(*args, firmware=firmware, log_apdu_file=log_apdu_file, **kwargs)
    else:
        raise ValueError(f"Backend '{backend_name}' is unknown. Valid backends are: {BACKENDS}")


@pytest.fixture
def client(backend_name, firmware, display, log_apdu_file):
    with create_backend(backend_name, firmware, display, log_apdu_file) as b:
        yield b


@pytest.fixture(autouse=True)
def use_only_on_backend(request, backend_name: str):
    if request.node.get_closest_marker('use_on_backend'):
        current_backend = request.node.get_closest_marker('use_on_backend').args[0]
        if current_backend != backend:
            pytest.skip(f'skipped on this backend: "{current_backend}"')


def pytest_configure(config):
  config.addinivalue_line(
        "markers", "use_only_on_backend(backend): skip test if not on the specified backend",
  )
