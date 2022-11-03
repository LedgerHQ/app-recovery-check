from pathlib import Path
from time import time

from ragger.backend import BackendInterface

SCREENSHOTS = (Path(__file__).parent.parent / "screenshots").resolve()


def assert_current_equals(client: BackendInterface, existing: Path):
    current = client._client.get_screenshot()
    assert_equal(current, existing)


def assert_equal(image: bytes, existing: Path):
    error_file = existing.parent / f'{existing.stem}_{time()}{existing.suffix}'
    try:
        with existing.open('rb') as filee:
            assert image == filee.read(), \
                f"Given bytes does not match image '{existing}'. Check '{error_file}' for comparison"
    except:
        with error_file.open('wb') as filee:
            filee.write(image)
        raise
