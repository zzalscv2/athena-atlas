#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

"""Helper class to define and merge step outputs"""

from __future__ import annotations

from collections.abc import Mapping
from typing import Any, Iterable, Iterator, Optional
import logging

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.AccumulatorCache import AccumulatorCachable

log = logging.getLogger(__name__)


class StepOutput(AccumulatorCachable, Mapping[str, Any]):
    """Helper class to define and merge step outputs

    The reconstruction sequences for the trigger are broken down into steps. This class
    holds:
    - A list of ComponentAccumulators (one per active step)
    - A dictionary containing nicknames for the outputs of the current steps

    Dictionary access on this object accesses output names.

    Note that the step indices here are the steps specific to the jet/MET sequences,
    not the full number of steps. At present there are three of these:
    0: Calo-only reconstruction
    1: Jet-RoI tracking (MET does not participate)
    2: Full-scan tracking

    However, this particular class does not make any requirements on what these steps
    are or how many there are.
    """

    def __init__(
        self, steps: Iterable[ComponentAccumulator]=(), outputs: dict[str, Any]={}
    ) -> None:
        self._steps = list(steps)
        self._outputs = dict(outputs)

    @classmethod
    def create(
        cls,
        ca: ComponentAccumulator,
        inputs: Optional[StepOutput] = None,
        step_idx: Optional[int] = None,
        **outputs,
    ) -> StepOutput:
        """Helper method to create a step output from a single CA and its inputs

        Parameters
        ----------
        ca : ComponentAccumulator
            The main component accumulator
        inputs : Optional[StepOutput]
            The inputs (if any) to the the main CA
        step_idx : Optional[int]
            Allow specifying a step. If not provided will be the max step out of the
            inputs or 0 if there are none
        outputs : Any
            Any named outputs from this step
        """
        if step_idx is None:
            step_idx = inputs.max_step_idx if inputs else 0
        steps = [ComponentAccumulator() for _ in range(step_idx + 1)]
        steps[step_idx].merge(ca)
        output = StepOutput(steps, outputs)
        if inputs:
            output.merge_other(inputs)
        return output

    @classmethod
    def merge(cls, *inputs: StepOutput) -> StepOutput:
        """Form a new output by merging multiple others"""
        output = StepOutput()
        for i in inputs:
            output.merge_other(i)
        return output

    @property
    def max_step_idx(self) -> int:
        """Get the maximum step index in this output"""
        return len(self._steps) - 1

    @property
    def steps(self) -> tuple[ComponentAccumulator, ...]:
        """The step-separated ComponentAccumulators"""
        return tuple(self._steps)
    
    def add_output_prefix(self, prefix: str) -> None:
        """Add a prefix to all of our outputs to prevent conflicts"""
        self._outputs = {prefix + k: v for k, v in self.items()}

    def merge_other(
        self, other: StepOutput, rename_outputs: dict[str, str] = {}
    ) -> None:
        """Merge another StepOutput into this

        Parameters
        ----------
        other : StepOutput
            The other output to merge
        rename_outputs : dict[str, str]
            Optionally rename some output names so there are no clashes
        """
        for step_idx, ca in enumerate(other.steps):
            self.merge_ca(ca, step_idx)
        self.__merge_outputs(
            {rename_outputs.get(k, k): v for k, v in other._outputs.items()}
        )

    def merge_ca(self, ca: ComponentAccumulator, step_idx: int, /, **outputs: Any) -> None:
        """Merge a single CA into this

        Parameters
        ----------
        ca : ComponentAccumulator
            The component accumulator to merge
        step_idx : int
            The step index in which to merge it
        **outputs : Any
            Any named outputs from the CA
        """
        if step_idx >= self.max_step_idx:
            self._steps += [ComponentAccumulator() for _ in range(step_idx - self.max_step_idx)]
        self._steps[step_idx].merge(ca)
        self.__merge_outputs(outputs)

    def __merge_outputs(self, outputs: dict[str, Any]) -> None:
        overlap = {
            key
            for key in set(self._outputs) & set(outputs)
            if self[key] != outputs[key]
        }
        if overlap:
            log.error("Incompatible definitions for named outputs: ")
            for key in overlap:
                log.error(f"\t{key}: {self[key]} -> {outputs[key]}")
            raise ValueError(str(overlap))
        self._outputs.update(outputs)

    def __getitem__(self, key: str) -> Any:
        return self._outputs[key]
    
    def __len__(self) -> int:
        return len(self._outputs)
    
    def __iter__(self) -> Iterator[str]:
        return iter(self._outputs)

    def _cacheEvict(self):
        for ca in self.steps:
            ca._cacheEvict()
