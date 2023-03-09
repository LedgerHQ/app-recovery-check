from ragger.navigator import NavIns
from typing import Iterable, Union

from .navigator import NavInsID


def format_instructions(instructions: Iterable[Union[NavIns, NavInsID]]) -> Iterable[NavIns]:
    return [NavIns(instruction) if isinstance(instruction, NavInsID) else instruction
            for instruction in instructions]
