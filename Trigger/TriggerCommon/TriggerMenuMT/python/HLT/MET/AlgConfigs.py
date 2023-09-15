#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

from __future__ import annotations

import copy
import errno
import json
from typing import Any
import os

from .ConfigHelpers import AlgConfig, stringToMETRecoDict
from .METRecoSequencesConfig import (
    cellInputCfg,
    clusterInputCfg,
    jetInputCfg,
    jetRecoDictForMET,
    trackingInputCfg,
    pfoInputCfg,
    mergedPFOInputCfg,
    cvfClusterInputCfg,
)
from .StepOutput import StepOutput
from ..Menu.SignatureDicts import METChainParts
import GaudiKernel.SystemOfUnits as Units
import TrigEFMissingET.PUClassification as PUClassification
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Utils.unixtools import find_datafile


from AthenaCommon.Logging import logging

log = logging.getLogger(__name__)


def test_configs():
    """Make sure that all algorithms defined in the METChainParts have
    configurations

    Really, this is mainly to have something sensible to call in the
    ConfigHelpers file to succeed the ctest :(
    """
    unknown_algs = []
    for alg in METChainParts["EFrecoAlg"]:
        for subcls in AlgConfig._get_subclasses():
            if subcls.algType() == alg:
                break
        else:
            unknown_algs.append(alg)
    assert (
        len(unknown_algs) == 0
    ), "The following EFrecoAlgs do not have AlgConfig classes: " "{}".format(
        unknown_algs
    )


class CellConfig(AlgConfig):
    @classmethod
    def algType(cls):
        return "cell"

    def __init__(self, **recoDict):
        super(CellConfig, self).__init__(**recoDict)

    def make_reco_algs(self, flags, **recoDict) -> StepOutput:
        cells = cellInputCfg(flags, **recoDict)
        return self._append_fex(
            flags,
            CompFactory.HLT.MET.CellFex(self.fexName, CellName=cells["Cells"]),
            cells,
        )


class TCConfig(AlgConfig):
    @classmethod
    def algType(cls):
        return "tc"

    def __init__(self, calib, **recoDict):
        super(TCConfig, self).__init__(calib=calib, **recoDict)

    def make_reco_algs(self, flags, **recoDict):
        clusters = clusterInputCfg(flags, **recoDict)
        return self._append_fex(
            flags,
            CompFactory.HLT.MET.TCFex(self.fexName, ClusterName=clusters["Clusters"]),
            clusters,
        )


class TCPufitConfig(AlgConfig):
    @classmethod
    def algType(cls):
        return "tcpufit"

    def __init__(self, nSigma, **recoDict):
        super(TCPufitConfig, self).__init__(nSigma=nSigma, **recoDict)
        if nSigma == "default":
            nSigma = "sig50"
        # Strip off the 'sig' part of the string, convert the end to a float, then divide by 10
        self.n_sigma = float(nSigma[3:]) / 10.0

    def make_reco_algs(self, flags, **recoDict):
        clusters = clusterInputCfg(flags, **recoDict)
        return self._append_fex(
            flags,
            CompFactory.HLT.MET.TCPufitFex(
                self.fexName, ClusterName=clusters["Clusters"], NSigma=self.n_sigma
            ),
            clusters,
        )


class MHTConfig(AlgConfig):
    @classmethod
    def algType(cls):
        return "mht"

    def interpret_reco_dict(self) -> dict[str, Any]:
        # Force the cluster calibration to EM
        return self.recoDict | {"calib": "em"}

    def make_reco_algs(self, flags, **recoDict):
        jets = jetInputCfg(flags, **recoDict)
        return self._append_fex(
            flags, CompFactory.HLT.MET.MHTFex(self.fexName, JetName=jets["Jets"]), jets
        )


# NB: TrkMHT isn't ready to run with PF jets yet - for that we need to add an
# option for cPFOs
class TrkMHTConfig(AlgConfig):
    @classmethod
    def algType(cls):
        return "trkmht"

    def __init__(self, **recoDict):
        super(TrkMHTConfig, self).__init__(
            forceTracks=True,
            **recoDict,
        )

    def make_reco_algs(self, flags, **recoDict):
        inputs = StepOutput.merge(
            trackingInputCfg(flags, **recoDict),
            jetInputCfg(flags, force_tracks=True, **recoDict),
        )
        return self._append_fex(
            flags,
            CompFactory.HLT.MET.TrkMHTFex(
                self.fexName,
                JetName=inputs["Jets"],
                TrackName=inputs["Tracks"],
                VertexName=inputs["Vertices"],
                TVAName=inputs["TVA"],
                TrackLinkName=inputs["GhostTracksLabel"],
                TrackSelTool=CompFactory.InDet.InDetTrackSelectionTool(
                    CutLevel="Loose",
                    maxZ0SinTheta=1.5,
                    maxD0overSigmaD0=3,
                    minPt=1 * Units.GeV,
                ),
            ),
            inputs,
        )


class PFSumConfig(AlgConfig):
    @classmethod
    def algType(cls):
        return "pfsum"

    def make_reco_algs(self, flags, **recoDict):
        pfos = pfoInputCfg(flags, **recoDict)
        return self._append_fex(
            flags,
            CompFactory.HLT.MET.PFSumFex(
                self.fexName, NeutralPFOName=pfos["nPFOs"], ChargedPFOName=pfos["cPFOs"]
            ),
            pfos,
        )


class PFOPufitConfig(AlgConfig):
    @classmethod
    def algType(cls):
        return "pfopufit"

    def __init__(self, nSigma, **recoDict):
        super(PFOPufitConfig, self).__init__(nSigma=nSigma, **recoDict)
        if nSigma == "default":
            nSigma = "sig50"
        # Strip off the 'sig' part of the string, convert the end to a float, then divide by 10
        self.n_sigma = float(nSigma[3:]) / 10.0

    def make_reco_algs(self, flags, **recoDict):
        pfos = mergedPFOInputCfg(flags, **recoDict)
        return self._append_fex(
            flags,
            CompFactory.HLT.MET.PUSplitPufitFex(
                self.fexName,
                InputName=pfos["MergedPFOs"],
                InputCategoryName=pfos["PUCategory"],
                NeutralThresholdMode=PUClassification.NeutralForward,
                NSigma=self.n_sigma,
            ),
            pfos,
        )


class CVFPufitConfig(AlgConfig):
    @classmethod
    def algType(cls):
        return "cvfpufit"

    def __init__(self, nSigma, **recoDict):
        super(CVFPufitConfig, self).__init__(nSigma=nSigma, **recoDict)
        if nSigma == "default":
            nSigma = "sig50"
        # Strip off the 'sig' part of the string, convert the end to and int, then divide by 10
        self.n_sigma = int(nSigma[3:]) / 10.0

    def make_reco_algs(self, flags, **recoDict):
        clusters = cvfClusterInputCfg(flags, **recoDict)
        return self._append_fex(
            flags,
            CompFactory.HLT.MET.PUSplitPufitFex(
                self.fexName,
                InputName=clusters["Clusters"],
                InputCategoryName=clusters["PUCategory"],
                NeutralThresholdMode=PUClassification.NeutralForward,
                NSigma=self.n_sigma,
            ),
            clusters,
        )


class MHTPufitConfig(AlgConfig):
    @classmethod
    def algType(cls):
        return "mhtpufit"

    def __init__(self, nSigma, **recoDict):
        super(MHTPufitConfig, self).__init__(nSigma=nSigma, **recoDict)
        if nSigma == "default":
            nSigma = "sig50"
        # Strip off the 'sig' part of the string, convert the end to and int, then divide by 10
        self.n_sigma = int(nSigma[3:]) / 10.0

    def interpret_reco_dict(self) -> dict[str, Any]:
        # Set the jet calibration
        dct = copy.copy(self.recoDict)
        if dct["jetCalib"] == "default":
            dct["jetCalib"] = "subjesgscIS"
        return dct

    def make_reco_algs(self, flags, **recoDict):
        inputs = StepOutput.merge(jetInputCfg(flags, **recoDict))
        jrd = jetRecoDictForMET(**recoDict)
        calibHasAreaSub = "sub" in jrd["jetCalib"]
        calibHasAreaSub = False # TODO: Fix this - there was a bug in the old implementation
        if calibHasAreaSub:
            from JetRecConfig.JetRecConfig import instantiateAliases
            from JetRecConfig.JetInputConfig import getEventShapeName

            instantiateAliases(inputs["JetDef"])
            rhoKey = getEventShapeName(inputs["JetDef"], "HLT_")
        else:
            rhoKey = ""
        if recoDict["constitType"] == "pf":
            inputs.merge_other(mergedPFOInputCfg(flags, **recoDict))
            input_key = "MergedPFOs"
        else:
            inputs.merge_other(clusterInputCfg(flags, **recoDict))
            input_key = "Clusters"
        return self._append_fex(
            flags,
            CompFactory.HLT.MET.MHTPufitFex(
                self.fexName,
                InputJetsName=inputs["Jets"],
                InputName=inputs[input_key],
                JetCalibIncludesAreaSub=calibHasAreaSub,
                JetEventShapeName=rhoKey,
                NSigma=self.n_sigma,
            ),
            inputs,
        )


class NNHLTConfig(AlgConfig):
    @classmethod
    def algType(cls):
        return "nn"

    def __init__(self, **recoDict):
        self.file_name = "TrigEFMissingET/20220429/NNsingleLayerRed.json"
        # Locate the file on the calib path
        full_name = find_datafile(
            self.file_name, os.environ["CALIBPATH"].split(os.pathsep)
        )
        if full_name is None:
            raise FileNotFoundError(
                errno.ENOENT, "File not found on CALIBPATH", self.file_name
            )
        with open(full_name, "r") as fp:
            network = json.load(fp)
        # Read the names of the algorithms used from the network file. The network file contains
        # the container+aux read from it, e.g. HLT_MET_cell.met so we strip off the HLT_MET_ prefix
        # and the .XX suffix
        # The variables containers can appear multiple times (e.g. met and sumet) therefore we have
        # to make sure to only add each once
        self.input_mets = []
        for dct in network["inputs"]:
            for dct2 in dct["variables"]:
                met = dct2["name"].removeprefix("HLT_MET_").partition(".")[0]
                if met not in self.input_mets:
                    self.input_mets.append(met)

        super().__init__(**recoDict)

    def make_reco_algs(self, flags, **recoDict) -> StepOutput:
        inputs = StepOutput()
        met_names: list[str] = []
        for alg in self.input_mets:
            cfg = AlgConfig.fromRecoDict(flags, **stringToMETRecoDict(alg))
            met_names.append(cfg.outputKey)
            output = cfg.make_reco_algs(flags, **cfg.interpret_reco_dict())
            output.add_output_prefix(f"{alg}.")
            inputs.merge_other(output)
        return self._append_fex(
            flags,
            CompFactory.HLT.MET.NNHLTFex(
                self.fexName, TriggerMETs=met_names, InputFileName=self.file_name
            ),
            inputs,
        )
