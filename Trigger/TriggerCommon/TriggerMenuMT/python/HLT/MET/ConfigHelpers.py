# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

""" Helper functions for configuring MET chains"""

from __future__ import annotations
from typing import Any
from collections.abc import Iterable

from AthenaCommon.CFElements import seqAND
from AthenaConfiguration.ComponentAccumulator import conf2toConfigurable
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.AthConfigFlags import AthConfigFlags
from ..Menu.SignatureDicts import METChainParts_Default, METChainParts
from ..MET.AlgInputConfig import InputConfigRegistry
from ..Config.MenuComponents import (
    RecoFragmentsPool,
    ChainStep,
    MenuSequence,
    InEventRecoCA,
    SelectionCA,
    MenuSequenceCA,
)
from copy import copy
from ..CommonSequences.FullScanDefs import trkFSRoI
from AthenaCommon.Logging import logging
from TrigEFMissingET.TrigEFMissingETConfig import getMETMonTool
from abc import ABC, abstractmethod
from string import ascii_uppercase
from TrigMissingETHypo.TrigMissingETHypoConfig import TrigMETHypoToolFromDict
from DecisionHandling.DecisionHandlingConfig import ComboHypoCfg


def streamer_hypo_tool(chainDict):
    return CompFactory.TrigStreamerHypoTool(chainDict["chainName"])


log = logging.getLogger(__name__)

# The keys from the MET chain dict that directly affect reconstruction
# The order here is important as it also controls the dict -> string conversion

recoKeys = [
    "EFrecoAlg",
    "calib",
    "constitType",
    "constitmod",
    "jetCalib",
    "nSigma",
    "addInfo",
]
metFSRoIs: list[str | None] = ["", None, trkFSRoI]


def metRecoDictToString(recoDict: dict[str, str], skipDefaults: bool = True) -> str:
    """Convert a dictionary containing reconstruction keys to a string

    Any key (from recoKeys) missing will just be skipped.

    Parameters
    ----------
    recoDict : dict[str, str]
        The reconstruction dictionary
    skipDefaults : bool, optional
        If true, skip any values that match the default, by default True

    Returns
    -------
    str
        The fixed string representation
    """
    return "_".join(
        recoDict[k]
        for k in recoKeys
        if k in recoDict
        and (not skipDefaults or recoDict[k] != METChainParts_Default[k])
    )


def stringToMETRecoDict(string: str) -> dict[str, Any]:
    """Convert a string to a MET reco dict"""
    defaults = copy(METChainParts_Default)
    # Now go through the parts of the string and fill in the dict
    for part in string.split("_"):
        for key, values in METChainParts.items():
            if not isinstance(values, list):
                continue
            if part in values:
                if isinstance(defaults[key], list):
                    defaults[key].append(part)  # type: ignore[attr-defined]
                else:
                    defaults[key] = part
    return defaults


class AlgConfig(ABC):
    """Base class to describe algorithm configurations

    Each individual 'EFrecoAlg' should be described by *one* AlgConfig subclass.
    It must provide its list of required inputs to the constructor and override
    the make_fex method

    The name of fexAlg *must* be self.fexName and the METContainerKey property
    *must* be set to self.outputKey (but this class usually should take care of
    this).

    The subclass must also implement the @classmethod 'algType' which returns
    the EFrecoAlg string that it describes.
    """

    @classmethod
    def algType(cls) -> str:
        """The algorithm that this object configures - this corresponds to the
        EFrecoAlg in the METChainParts dictionary

        Note that no two subclasses of AlgConfig can describe the same algorithm
        (as identified by this string).
        """
        raise NotImplementedError("algType not implemented by subclass!")

    def __init__(self, inputs: Iterable[str] = [], inputRegistry:InputConfigRegistry | None=None, **recoDict: str):
        """Initialise the base class

        =========
        Arguments
        =========
        inputs: The nicknames of the inputs that this FEX uses
        inputRegistry:
            The InputConfigRegistry instance to use. Usually this can be left
            as None and then METRecoSequences.default_inputs will be used.
            However, this parameter is provided in case a FEX requires a vastly
            different set of input objects
        recoDict: Pass *all* the keys required for the recoDict
        """

        # Make sure that we got *all* the keys (i.e. the subclass didn't
        # inadvertently steal one of them from us)
        alg_type = self.algType()
        assert all(k in recoDict for k in recoKeys), (
            f"AlgConfig.__init__ for {alg_type} did not receive all the recoKeys"
            " - this suggests a problem in the subclass __init__ method!"
        )
        self.recoDict = copy(recoDict)
        self._suffix = metRecoDictToString(recoDict)
        if inputRegistry is None:
            from .METRecoSequences import default_inputs

            inputRegistry = default_inputs
        self._registry = inputRegistry
        self._inputs = tuple(inputs)

    def make_fex(self, flags: AthConfigFlags, name: str, inputs: dict[str, Any]) -> Any:
        """Create the fex from its name and the inputs dict"""

        return conf2toConfigurable(self.make_fex_accumulator(flags, name, inputs))

    # TODO: Should this return a CA not a component?

    @abstractmethod
    def make_fex_accumulator(self, flags: AthConfigFlags, name: str, inputs: dict[str, Any]) -> Any:
        """Create the CA for the fex from its name and the inputs dict"""
        pass

    @property
    def inputRegistry(self) -> InputConfigRegistry:
        """The InputConfigRegistry object used to build the input sequences"""
        return self._registry

    @property
    def outputKey(self) -> str:
        """The MET container object produced by this algorithm"""
        from TrigEDMConfig.TriggerEDMRun3 import recordable

        return recordable("HLT_MET_{}".format(self._suffix))

    @property
    def fexName(self) -> str:
        """The name of the algorithm made by this configuration"""
        return "EFMET_{}".format(self._suffix)

    def getMonTool(self, flags):
        """ Create the monitoring tool """
        return getMETMonTool(flags)

    def recoAlgorithms(self, flags):
        """Get the reconstruction algorithms (split by step) without the input makers"""
        if hasattr(self, "_recoAlgorithms"):
            return self._recoAlgorithms
        # Retrieve the inputss
        log.verbose("Create inputs for %s", self._suffix)
        steps, inputs = self.inputRegistry.build_steps(
            flags, self._inputs, metFSRoIs, self.recoDict
        )
        fex = self.make_fex(flags, self.fexName, inputs)
        fex.MonTool = self.getMonTool(flags)
        fex.METContainerKey = self.outputKey
        # Add the FEX to the last list
        steps[-1].append(fex)
        self._recoAlgorithms = steps
        return self._recoAlgorithms

    def athSequences(self, flags):
        """Get the reco sequences (split by step)"""
        if hasattr(self, "_athSequences"):
            return self._athSequences

        inputMakers = self.inputMakers()
        reco = self.recoAlgorithms(flags)
        # Put the input makers at the start
        sequences = [
            []
            if step == []
            else seqAND(
                f"METAthSeq_step{idx}_{self._suffix}", [inputMakers[idx]] + step
            )
            for idx, step in enumerate(reco)
        ]
        self._athSequences = sequences
        return self._athSequences

    def menuSequences(self, flags):

        """Get the menu sequences (split by step)"""
        if hasattr(self, "_menuSequences"):
            return self._menuSequences

        sequences = []
        inputMakers = self.inputMakers()
        ath_sequences = self.athSequences(flags)
        for idx, seq in enumerate(ath_sequences):
            if idx == len(ath_sequences) - 1:
                hypo = conf2toConfigurable(self.make_hypo_alg())
                hypo_tool = TrigMETHypoToolFromDict
            else:
                hypo = conf2toConfigurable(self.make_passthrough_hypo_alg(idx))
                hypo_tool = streamer_hypo_tool
            sequences.append(
                MenuSequence(
                    flags,
                    Sequence=seq,
                    Maker=inputMakers[idx],
                    Hypo=hypo,
                    HypoToolGen=hypo_tool,
                )
                if seq != []
                else []
            )
        self._menuSequences = sequences
        return self._menuSequences

    def name_step(self, idx) -> str:
        return f"step{ascii_uppercase[idx]}_{self._suffix}"

    def make_steps(self, flags, chainDict):
        """Create the actual chain steps"""
        # NB - we index the steps using uppercase letters 'A', 'B', etc
        # This technically means that there is an upper limit of 26 on the
        # number of different steps that can be provided this way, but it seems
        # unlikely that we'll actually run into this limit. If we do, it
        # shouldn't be a problem to change it
        steps = []

        for idx, seq in enumerate(self.menuSequences(flags)):
            steps += [
                ChainStep(
                    self.name_step(idx),
                    [seq] if seq != [] else [],
                    multiplicity=[1] if seq != [] else [],
                    chainDicts=[chainDict],
                )
            ]

        return steps

    def make_reco_ca(self, flags):
        """Make the reconstruction sequences for the new JO style"""
        # Retrieve the inputs
        log.verbose("Create inputs for %s", self._suffix)
        steps, inputs = self.inputRegistry.build_steps(
            flags, self._inputs, metFSRoIs, self.recoDict, return_ca=True
        )
        # Create the FEX and add it to the last input sequence
        fex = self.make_fex_accumulator(flags, self.fexName, inputs)
        fex.MonTool = self.getMonTool(flags)
        fex.METContainerKey = self.outputKey
        steps[-1].addEventAlgo(fex)
        return steps

    def make_accumulator_steps(self, flags, chainDict, noComboHypo=False):
        """Make the full accumulator steps"""
        # The 'noComboHypo' argument is a workaround to avoid errors in jobs
        # that convert CA configuration into legacy sequences. If we create
        # a ChainStep with ConfigurableCABehavior on, the constructor caches
        # a GaudiConfig2 ComboHypo. So we disable the combo creation when
        # making the CA, and then enable it when making a legacy ChainStep.

        # Get the reco sequences
        reco_sequences = self.make_reco_ca(flags)
        output_steps = []
        # build up the output steps
        # We have to merge together the CAs containing the reconstruction sequences
        # and the InEventRecoCAs that contain the input makers
        for step_idx, reco_sequence in enumerate(reco_sequences):
            reco_acc = self.inputMakerCA(step_idx)
            sel_acc = None
            step_name = f"Empty_METStep{step_idx}"
            if (
                reco_acc is not None
            ):  # Define reco and hypo algorithms only if reco_acc is not None
                if step_idx == len(reco_sequences) - 1:
                    # If this is the last step we have to add the hypo alg
                    hypo_alg = self.make_hypo_alg()
                    hypo_tool = TrigMETHypoToolFromDict
                else:
                    # Otherwise, we add a passthrough hypo
                    hypo_alg = self.make_passthrough_hypo_alg(step_idx)
                    hypo_tool = streamer_hypo_tool

                # Now merge the reco sequence into the InEventRecoCA
                reco_acc.merge(reco_sequence)

                # Create a selection CA
                sel_acc = SelectionCA('METAthSeq_'+self.name_step(step_idx))
                # Merge in the reconstruction sequence
                sel_acc.mergeReco(reco_acc)
                # Add its hypo alg
                sel_acc.addHypoAlgo(hypo_alg)

                step_name = sel_acc.name

            # Build the menu sequence and create the actual chainStep
            output_steps.append(
                ChainStep(
                    name=step_name,
                    multiplicity=[] if sel_acc is None else [1],
                    chainDicts=[chainDict],
                    Sequences=[]
                    if sel_acc is None
                    else [
                        MenuSequenceCA(
                            flags, selectionCA=sel_acc, HypoToolGen=hypo_tool
                        )
                    ],
                    # If converting from CA to legacy we need to avoid
                    # caching the GaudiConfig2 ComboHypo
                    comboHypoCfg=None if noComboHypo else ComboHypoCfg,
                )
            )

        return output_steps

    def make_hypo_alg(self):
        """The hypo alg used for this configuration"""
        return CompFactory.TrigMissingETHypoAlg(
            f"METHypoAlg_{self._suffix}", METContainerKey=self.outputKey
        )

    def make_passthrough_hypo_alg(self, step):
        return CompFactory.TrigStreamerHypoAlg(
            "METPassThroughHypo_" + self.name_step(step)
        )

    def inputMakerCA(self, idx):
        """Get the InEventRecoCA for the given step index"""
        from TrigT2CaloCommon.CaloDef import clusterFSInputMaker
        from AthenaConfiguration.ComponentFactory import CompFactory
        from ..CommonSequences.FullScanDefs import trkFSRoI

        name = self.name_step(idx) + "Reco"
        if idx == 0:
            return InEventRecoCA(name, inputMaker=clusterFSInputMaker())
        elif idx == 1:
            return None
        elif idx == 2:
            return InEventRecoCA(
                name,
                inputMaker=CompFactory.InputMakerForRoI(
                    "IM_Jet_TrackingStep",
                    RoITool=CompFactory.ViewCreatorInitialROITool(),
                    RoIs=trkFSRoI,
                ),
            )
        else:
            raise KeyError(f"No input maker for step {idx}")

    def inputMakers(self):
        """The input makers for each step"""
        if hasattr(self, "_inputMakers"):
            return self._inputMakers
        from ..Jet.JetMenuSequencesConfig import getCaloInputMaker, getTrackingInputMaker

        self._inputMakers = [getCaloInputMaker(), None, getTrackingInputMaker("ftf")]
        return self._inputMakers

    @classmethod
    def _get_subclasses(cls):
        """Provides a way to iterate over all subclasses of this class"""
        for subcls in cls.__subclasses__():
            for subsubcls in subcls.__subclasses__():
                yield subsubcls
            yield subcls

    @classmethod
    def _makeCls(cls, flags, **kwargs):
        """This is a rather horrible work-around.

        The RecoFragmentsPool approach wants a function that takes a set of
        flags. However our class constructors don't do this (and passing
        in a class doesn't *quite* fit what the RecoFragmentsPool is expecting.
        So instead we pass this function...
        """
        return cls(**kwargs)

    @classmethod
    def fromRecoDict(cls, flags, EFrecoAlg, **recoDict):
        for subcls in cls._get_subclasses():
            if subcls.algType() == EFrecoAlg:
                return RecoFragmentsPool.retrieve(
                    subcls._makeCls, flags, EFrecoAlg=EFrecoAlg, **recoDict
                )

        raise ValueError("Unknown EFrecoAlg '{}' requested".format(EFrecoAlg))


# Load all the defined configurations
from . import AlgConfigs

# Make sure that there is an AlgConfig for every EFrecoAlg
AlgConfigs.test_configs()
