#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

""" Helpers for creating input reco sequences

Classes:
    AlgInputConfig - base class for individual input configs
    InputConfigRegistry - class to gather input configurations together

Together, these classes implement a rudimentary dependency resolution method.
The main unit in this are the inputs being created. These are referred to by
nicknames rather than the container names directly as these can vary depending
on the reconstruction parameters provided. For example, the clusters input
config creates the "Clusters" input. This corresponds to a StoreGate key of
'HLT_TopoCaloClustersFS' if the cluster calibration is 'em' and
'HLT_TopoCaloClustersLCFS' if the cluster calibration is 'lcw'.

This file just provides the underlying mechanisms. For the concrete example see
METRecoSequences, which will be referenced in the following explanation.

Each config declares the inputs it will produce and on what inputs it depends.
The dependencies are allowed to be different for different reco configurations
but for simplicity's sake the inputs produced are always the same. For example
the jet input config in METRecoSequences produces ["Jets", "JetDef"] and its
dependencies are ["Clusters"] if no tracks are needed and ["Clusters", "Tracks"
"Vertices"] otherwise.

Each config also has a create_sequence method which builds the actual reco
sequence. This method returns a two-tuple. The first element is the reco
sequence which makes the inputs, the second is a dictionary mapping the produced
input's nicknames to their values. These values are usually storegate keys but
can be any object. For example, the jet input config (with the correct reco
configuration) would produce
{
    "Jets" : "HLT_AntiKt4EMTopoJets_subjesIS",
    "JetDef" : jetdef
}
for this mapping, where jetdef is a python object produced by the jet reco
sequence.

The dependency resolution is done by the InputConfigRegistry class. This
maintains a list of known input configs where no two can claim to produce the
same input. The main one used here is the default_inputs object created in
METRecoSequences. The 'build_steps' method is then used to create the necessary
sequences in the correct order, split into their steps and also to collate and 
return the mapping of input nicknames to values.

The same system is also used for component-accumulator based configuration.

TODO: Once the legacy is completely removed, we may no longer need this as CA merging
should handle most of the complexity here...
"""

from __future__ import annotations

from abc import ABC, abstractmethod
from collections import defaultdict
from collections.abc import Sequence, Iterable
from typing import Any

from AthenaCommon.Logging import logging
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.AllConfigFlags import AthConfigFlags

logging.getLogger().info("Importing %s", __name__)
log = logging.getLogger(__name__)


class AlgInputConfig(ABC):
    """Base class for building up inputs for the FEXes"""

    def __init__(self, produces: Iterable[str], step: int | None = None):
        """Initialise the class

        =========
        Arguments
        =========
        produces: The nicknames of the inputs this config produces
        step: The step in which the input sequences should run

        step can be None, in which case it will be calculated as the maximum
        of all the steps of the input's dependencies. This means that an input
        with no dependencies must have its step set explicitly
        """
        self._produces = tuple(produces)
        self._step = step

    @property
    def step(self) -> int | None:
        """The reconstruction step this input runs in"""
        return self._step

    @property
    def produces(self) -> tuple[str, ...]:
        """Which (nicknamed) inputs does this config produce"""
        return self._produces

    @abstractmethod
    def dependencies(self, recoDict: dict[str, str]) -> tuple[str, ...]:
        """Given the reco parameters, which inputs does this config depend on"""
        pass

    @abstractmethod
    def create_sequence(
        self,
        flags: AthConfigFlags,
        inputs: dict[str, Any],
        RoIs: str | None,
        recoDict: dict[str, str],
    ) -> tuple[list, dict[str, Any]]:
        """Create the specified reconstruction sequences

        Returns a 2-tuple containing the sequences and a mapping from input nickname to
        the produced input (usually a SG key)

        Parameters
        ----------
        flags : AthConfigFlags
            The configuration flags
        inputs : dict[str, Any]
            Inputs from other sequences
        RoIs : str | None
            The RoI for this step
        recoDict : dict[str, str]
            The configuration reco dict

        Returns
        -------
        list
            A list of configured sequences
        dict[str, Any]
            Mapping from input nickname to the produced input (usually a SG key)
        """

    @abstractmethod
    def create_accumulator(
        self,
        flags: AthConfigFlags,
        inputs: dict[str, Any],
        RoIs: str | None,
        recoDict: dict[str, str],
    ) -> tuple[ComponentAccumulator, dict[str, Any]]:
        """Create the specified reconstruction CAs

        Returns a 2-tuple containing the accumulator and a mapping from input nickname
        to the produced input (usually a SG key)

        Parameters
        ----------
        flags : AthConfigFlags
            The configuration flags
        inputs : dict[str, Any]
            Inputs from other sequences
        RoIs : str | None
            The RoI for this step
        recoDict : dict[str, str]
            The configuration reco dict

        Returns
        -------
        ComponentAccumulator
            The produced CA
        dict[str, Any]
            Mapping from input nickname to the produced input (usually a SG key)
        """
        raise NotImplementedError(
            f"{self.produces} does not provide a CA implementation!"
        )


class InputConfigRegistry:
    """Build up a registry of input configurations"""

    def __init__(self) -> None:
        self._configs: dict[str, AlgInputConfig] = {}

    def add_input(self, config: AlgInputConfig) -> None:
        """Add a new input configuration

        Parameters
        ----------
        config : AlgInputConfig
            The new input configuration

        Raises
        ------
        ValueError
            The input claims to produce an input that is already produced by another
        """
        overlap = [x for x in config.produces if x in self._configs]
        if overlap:
            raise ValueError(
                f"Attempting to add an input config that produces {overlap} but these "
                "are already present"
            )
        for x in config.produces:
            self._configs[x] = config

    def build_steps(
        self,
        flags: AthConfigFlags,
        requested: Iterable[str],
        RoIs: Sequence[str | None],
        recoDict: dict[str, str],
        return_ca: bool = False,
    ) -> tuple[list | ComponentAccumulator, dict[str, Any]]:
        """_summary_

        Parameters
        ----------
        flags : AthConfigFlags
            The configuration flags
        requested : Iterable[str]
            The required inputs
        RoIs : Sequence[str | None]
            An input RoI per step
        recoDict : dict[str, str]
            The configuration reco dict
        return_ca : bool, optional
            Whether to return a ComponentAccumulator, by default False
        """
        # The input sequences, keyed by step
        steps: defaultdict[int, ComponentAccumulator | list[Any]] = (
            defaultdict(ComponentAccumulator) if return_ca else defaultdict(list)
        )
        # The mapping of input nickname to storegate key
        inputs: dict[str, Any] = {}
        # Internal mapping of input nickname to step
        input_steps: dict[str, int] = {}
        log.debug("Producing steps for requested inputs: %s", requested)
        for name in requested:
            this_steps = self._create_input(
                flags,
                name,
                RoIs,
                recoDict,
                input_steps=input_steps,
                inputs=inputs,
                return_ca=return_ca,
            )
            for step, reco in this_steps.items():
                if return_ca:
                    steps[step].merge(reco)  # type: ignore[union-attr]
                else:
                    steps[step] += reco
        # Now convert the steps into a list, filling in empty steps with empty
        # lists/CA
        all_steps = tuple(steps[idx] for idx in range(max(steps.keys()) + 1))
        log.debug("Built steps for inputs: %s", inputs)
        log.debug("Steps are:\n%s", all_steps)
        return all_steps, inputs

    def _create_input(
        self,
        flags: AthConfigFlags,
        name: str,
        RoIs: Sequence[str | None],
        recoDict: dict[str, str],
        input_steps: dict[str, int],
        inputs: dict[str, Any],
        return_ca: bool = False,
        _seen: list[str] | None = None,
    ) -> defaultdict[int, ComponentAccumulator | list]:
        """Create an input and all its dependencies

        Parameters
        ----------
        flags : AthConfigFlags
            The configuration flags
        name : str
            The requested input
        RoIs : Sequence[str | None]
            An input RoI per step
        recoDict : dict[str, str]
            The configuration reco dict
        input_steps : dict[str, int]
            Steps for any existing inputs. Will be updated
        inputs : dict[str, Any]
            Existing inputs
        return_ca : bool, optional
            Whether to return a CA, by default False
        _seen : list[str] | None, optional
            Internal variable to track cyclic dependencies, by default None

        Returns
        -------
        steps
        where steps is a list of input sequences, one for each step
        The provided input_steps and inputs parameters are also updated with
        the new inputs that have been produced
        """
        log.debug("Creating inputs for %s", name)
        if _seen is None:
            _seen = []
        elif name in _seen:
            raise RuntimeError(
                "Circular dependency: {}".format(" -> ".join(_seen + [name]))
            )

        steps: defaultdict[int, ComponentAccumulator | list[Any]] = (
            defaultdict(ComponentAccumulator) if return_ca else defaultdict(list)
        )
        if name in input_steps:
            log.debug("Input already created")
            # We've already seen this step so return dummies
            return steps
        try:
            config = self._configs[name]
        except KeyError:
            raise KeyError(f"Requested input {name} not defined")

        dependencies = config.dependencies(recoDict)
        log.debug("Dependencies are %s", dependencies)
        for dep_name in dependencies:
            # The reco steps for the inputs
            dep_steps = self._create_input(
                flags,
                dep_name,
                RoIs,
                recoDict,
                input_steps,
                inputs,
                return_ca,
                _seen + [name],
            )
            # The step number for this dependencey
            dep_step = input_steps[dep_name]
            if config.step is not None and dep_step > config.step:
                raise ValueError(
                    f"Dependency {dep_name} is in a later step '{dep_step}' than "
                    f"{name} which requires it (step = {config.step})"
                )
            # Add these reco sequences to our output lists
            for step, reco in dep_steps.items():
                if return_ca:
                    steps[step].merge(reco)  # type: ignore[union-attr]
                else:
                    steps[step] += reco

        if config.step is None:
            if len(dependencies) == 0:
                raise ValueError(f"Unable to work out step for input config {name}!")
            # If the config doesn't specify a step then we run this as early as possible
            # i.e. in the latest step of all its dependencies
            this_step = max(input_steps[dep] for dep in dependencies)
        else:
            this_step = config.step

        log.debug("%s step is %i", name, this_step)
        # Finally, add *our* info
        if this_step >= len(RoIs):
            raise ValueError(
                f"Step {this_step} is greater than the number of RoIs ({RoIs})"
            )
        builder = config.create_accumulator if return_ca else config.create_sequence

        reco, produced_inputs = builder(flags, inputs, RoIs[this_step], recoDict)
        if return_ca:
            steps[this_step].merge(reco)  # type: ignore[union-attr]
        else:
            steps[this_step] += reco
        inputs.update(produced_inputs)
        # Add this to the list of things we've already seen, along with everything else
        # it's made
        for made in produced_inputs.keys():
            input_steps[made] = this_step
        return steps
