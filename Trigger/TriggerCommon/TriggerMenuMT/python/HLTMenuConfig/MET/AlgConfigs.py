#
#  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
#

from .ConfigHelpers import AlgConfig, jetRecoDictForMET
from TrigEFMissingET.TrigEFMissingETMTConfig import getMETMonTool
from ..Menu.MenuComponents import RecoFragmentsPool
import GaudiKernel.SystemOfUnits as Units

class CellConfig(AlgConfig):
    @classmethod
    def algType(cls):
        return "cell"
    
    def __init__(self, **recoDict):
        super(CellConfig, self).__init__(**recoDict)
        from TrigT2CaloCommon.CaloDef import HLTFSCellMakerRecoSequence
        from TrigEFMissingET.TrigEFMissingETConf import HLT__MET__CellFex
        (cellMakerSeq, cellName) = HLTFSCellMakerRecoSequence()

        print ("Cell maker sequence is:")
        print (cellMakerSeq)

        self.inputs = [cellMakerSeq]
        self.fexAlg = self._make_fex_alg(
                HLT__MET__CellFex,
                CellName = cellName)

class TCConfig(AlgConfig):
    @classmethod
    def algType(cls):
        return "tc"

    def __init__(self, **recoDict):
        super(TCConfig, self).__init__(**recoDict)
        from TrigT2CaloCommon.CaloDef import HLTFSTopoRecoSequence
        from TrigEFMissingET.TrigEFMissingETConf import HLT__MET__TCFex
        # TODO - cluster calibration
        RoIs = self.inputMaker.RoIs
        tcSeq, clusterName = RecoFragmentsPool.retrieve(
                HLTFSTopoRecoSequence, RoIs)

        self.inputs = [tcSeq]
        self.fexAlg = self._make_fex_alg(
                HLT__MET__TCFex,
                ClusterName = clusterName)

class TCPufitConfig(AlgConfig):
    @classmethod
    def algType(cls):
        return "tcpufit"

    def __init__(self, **recoDict):
        super(TCPufitConfig, self).__init__(**recoDict)
        from TrigT2CaloCommon.CaloDef import HLTFSTopoRecoSequence
        from TrigEFMissingET.TrigEFMissingETConf import HLT__MET__TCPufitFex
        RoIs = self.inputMaker.RoIs
        tcSeq, clusterName = RecoFragmentsPool.retrieve(
                HLTFSTopoRecoSequence, RoIs)

        self.inputs = [tcSeq]
        self.fexAlg = self._make_fex_alg(
                HLT__MET__TCPufitFex,
                ClusterName = clusterName)

class MHTConfig(AlgConfig):
    @classmethod
    def algType(cls):
        return "mht"

    def __init__(self, **recoDict):
        super(MHTConfig, self).__init__(**recoDict)
        from ..Jet.JetRecoSequences import jetRecoSequence
        from TrigEFMissingET.TrigEFMissingETConf import HLT__MET__MHTFex
        jetRecoDict = jetRecoDictForMET(**recoDict)
        # TODO - right now jet calibration is hardcoded to EM
        jetRecoDict["calib"] = "em"
        jetRecoDict["jetCalib"] = "subjes"
        jetSeq, jetName = RecoFragmentsPool.retrieve(
                jetRecoSequence, None, dataSource="data", **jetRecoDict)

        self.inputs = [jetSeq]
        self.fexAlg = self._make_fex_alg(
                HLT__MET__MHTFex,
                JetName = jetName)

class TrkMHTConfig(AlgConfig):
    @classmethod
    def algType(cls):
        return "trkmht"

    def __init__(self, **recoDict):
        super(TrkMHTConfig, self).__init__(**recoDict)
        from ..Jet.JetRecoSequences import jetRecoSequence
        from TrigEFMissingET.TrigEFMissingETConf import HLT__MET__TrkMHTFex
        jetRecoDict = jetRecoDictForMET(trkopt="ftf", **recoDict)
        # TODO - right now jet calibration is hardcoded to EM
        jetRecoDict["calib"] = "em"
        jetSeq, jetName = RecoFragmentsPool.retrieve(
                jetRecoSequence, None, dataSource="data", **jetRecoDict)

        # These are the names set by the upstream algorithms. Unfortunately
        # these aren't passed to us - we just have to 'know' them
        tracks = "HLT_xAODTracks_FS"
        vertices = "HLT_EFHistoPrmVtx"
        tva = "JetTrackVtxAssoc_{trkopt}".format(**jetRecoDict)
        track_links = "GhostTrack_{trkopt}".format(**jetRecoDict)

        self.inputs = [jetSeq]
        self.fexAlg = self._make_fex_alg(
                HLT__MET__TrkMHTFex,
                JetName       = jetName,
                TrackName     = tracks,
                VertexName    = vertices,
                TVAName       = tva,
                TrackLinkName = track_links)
        self.fexAlg.TrackSelTool.CutLevel         = "Loose"
        self.fexAlg.TrackSelTool.maxZ0SinTheta    = 1.5
        self.fexAlg.TrackSelTool.maxD0overSigmaD0 = 3
        self.fexAlg.TrackSelTool.minPt            = 1 * Units.GeV
