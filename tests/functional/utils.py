from collections.abc import Iterable

from ragger.navigator import NavIns

from .navigator import CustomNavInsID


def format_instructions(
    instructions: Iterable[NavIns | CustomNavInsID],
) -> Iterable[NavIns]:
    return [NavIns(instruction) if isinstance(instruction, CustomNavInsID) else instruction for instruction in instructions]
