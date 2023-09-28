#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequence
from AthenaCommon.CFElements import parOR
from AthenaCommon.CFElements import seqAND
from TrigEDMConfig.TriggerEDMRun3 import recordable
from ViewAlgs.ViewAlgsConf import EventViewCreatorAlgorithm
from DecisionHandling.DecisionHandlingConf import ViewCreatorInitialROITool
import AthenaCommon.SystemOfUnits as Units
from TrigMinBias.TrigMinBiasMonitoring import MbtsHypoToolMonitoring

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.AccumulatorCache import AccumulatorCache

from ..Config.MenuComponents import InViewRecoCA, InEventRecoCA, SelectionCA, MenuSequenceCA

from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)



########
# to move into TrigMinBiasHypoConfigMT?

def SPCountHypoToolGen(chainDict):
    hypo = CompFactory.SPCountHypoTool(chainDict["chainName"])
    if "hmt" in chainDict["chainName"]:
        hypo.sctSP = int(chainDict["chainParts"][0]["hypoSPInfo"].strip("sp"))
    if "mb_sptrk_" in chainDict["chainName"] or "mb_sp_" in chainDict["chainName"]:
        hypo.pixCL = 2
        hypo.sctSP = 3
    if "mb_excl" in chainDict["chainName"]:
        hypo.pixCLMax = 150 # TODO revisit tightening those
        hypo.sctSPMax = 150 # as above
    if "sp_pix" in chainDict["chainName"]:
        hypo.pixCL = int(chainDict["chainParts"][0]["hypoSPInfo"].strip("pix"))
    if "sp_vpix" in chainDict["chainName"]:
        hypo.pixCL = -1  # Remove any cut on mininum number of Pixel and SCT SpacePoints
        hypo.sctSP = -1
        hypo.pixCLMax = int(chainDict["chainParts"][0]["hypoSPInfo"].strip("vpix"))
    if "nototpix" in chainDict["chainName"]:
        hypo.pixCL = -1  # Remove any cut on mininum number of Pixel and SCT SpacePoints
        hypo.sctSP = -1
        hypo.pixCLnoToT = int(chainDict["chainParts"][0]["hypoSPInfo"].strip("nototpix"))

    return hypo


def TrackCountHypoToolGen(chainDict):
    def ptToGeV(v):        
        if "p" in v:
            sv = v.split("p")
            return int(sv[0])+0.1*int(sv[1])
        else:
            return int(v)

    from TrigMinBias.TrigMinBiasConf import TrackCountHypoTool
    hypo = TrackCountHypoTool(chainDict["chainName"])
    if "hmt" in chainDict["chainName"]:
        hypo.minNtrks = int(chainDict["chainParts"][0]["hypoTrkInfo"].strip("trk"))
        hypo.minPt = 200*Units.MeV
        if "pusup" in chainDict["chainName"]:
            hypo.maxVertexZ = 10*Units.mm
    if "mb_sptrk" in chainDict["chainName"]:
        hypo.minPt = 100*Units.MeV
        hypo.maxZ0 = 401*Units.millimeter
    if "mb_sptrk_pt" in chainDict["chainName"]:
        hypo.minPt = ptToGeV(chainDict["chainParts"][0]["hypoPtInfo"].strip("pt"))*Units.GeV
        hypo.maxZ0 = 401*Units.millimeter
    if "excl" in chainDict["chainName"]:
        hypo.exclusive = True
        hypo.minPt = ptToGeV(chainDict["chainParts"][0]["hypoPtInfo"].strip("pt"))*Units.GeV
        trk = chainDict["chainParts"][0]["hypoTrkInfo"]
        # this is string of the form 4trk6 - 'number'trk'number'
        limits =  trk[0:trk.index("trk")], trk[trk.index("trk")+3:]
        hypo.minNtrks = int(limits[0])
        hypo.maxNtrks = int(limits[1])

        # will set here cuts
    return hypo

def MbtsHypoToolGen(flags, chainDict):
    hypo = CompFactory.MbtsHypoTool(chainDict["chainName"]) # to now no additional settings
    if chainDict["chainParts"][0]["extra"] in ["vetombts2in", "vetospmbts2in"]:
        hypo.MbtsCounters=2
        hypo.MBTSMode=1
        hypo.Veto=True
    if '_all_' in chainDict["chainName"]:
        hypo.AcceptAll = True
        if "mbMon:online" in chainDict["monGroups"]:
            hypo.MonTool = MbtsHypoToolMonitoring(flags)
    else:  #default, one counter on each side
        hypo.MbtsCounters=1
    return hypo



def TrigZVertexHypoToolGen(chainDict):
    hypo = CompFactory.TrigZVertexHypoTool(chainDict["chainName"])
    if "pusup" in chainDict["chainName"]:
        hypo.minWeight = int(chainDict["chainParts"][0]["pileupInfo"].strip("pusup"))
    else:
        hypo.minWeight = -1 # pass always

    return hypo

@AccumulatorCache
def MinBiasSPSel(flags):

    reco = InViewRecoCA("SPCountingReco")
    minBiasFlags = flags.cloneAndReplace("Tracking.ActiveConfig","Trigger.InDetTracking.minBias")

    from TrigInDetConfig.InDetTrigSequence import InDetTrigSequence
    seq = InDetTrigSequence(minBiasFlags,
                            minBiasFlags.Tracking.ActiveConfig.input_name, # this is already in the flags, maybe we would nto need to pass it in the future?
                            rois   = str(reco.inputMaker().InViewRoIs),
                            inView = str(reco.inputMaker().Views))
    spMakingCA = seq.sequence("spacePointFormation")

    reco.mergeReco(spMakingCA)
    
    # TODO, this is a dependancy of SPcounting
    vdv = CompFactory.AthViews.ViewDataVerifier( "VDVSPCountingInputs",
                                                  DataObjects = [( 'PixelID' , 'DetectorStore+PixelID' ),
                                                                 ( 'SCT_ID'  , 'DetectorStore+SCT_ID' )] )
    reco.addRecoAlgo(vdv)

    from TrigMinBias.MinBiasCountersConfig import SPCounterRecoAlgCfg
    reco.mergeReco(SPCounterRecoAlgCfg(flags))

    selAcc = SelectionCA("MinBiasSPCounting")
    selAcc.mergeReco(reco)
    spCountHypo = CompFactory.SPCountHypoAlg(SpacePointsKey=recordable("HLT_SpacePointCounts"))
    selAcc.addHypoAlgo(spCountHypo)
    return selAcc

def MinBiasSPSequenceCfg(flags):
    selAcc = MinBiasSPSel(flags)
    return MenuSequenceCA(flags, selAcc, HypoToolGen = SPCountHypoToolGen)

def MinBiasZVertexFinderSequenceCfg(flags):
    recoAcc = InViewRecoCA(name="ZVertFinderReco", InViewRoIs="InputRoI", RequireParentView=True)
    vdv = CompFactory.AthViews.ViewDataVerifier( "VDVZFinderInputs",
                                                  DataObjects = [( 'SpacePointContainer' , 'StoreGateSvc+PixelTrigSpacePoints'),
                                                                 ( 'PixelID' , 'DetectorStore+PixelID' ) ])

    recoAcc.addRecoAlgo(vdv)
    from IDScanZFinder.ZFinderAlgConfig import  MinBiasZFinderCfg
    recoAcc.mergeReco( MinBiasZFinderCfg(flags) )
    selAcc = SelectionCA("ZVertexFinderSel")
    selAcc.mergeReco(recoAcc)
    selAcc.addHypoAlgo( CompFactory.TrigZVertexHypoAlg("TrigZVertexHypoAlg", ZVertexKey=recordable("HLT_vtx_z")))
    return MenuSequenceCA(flags, selAcc, HypoToolGen = TrigZVertexHypoToolGen)


def MinBiasTrkSequence(flags):

        trkInputMakerAlg = EventViewCreatorAlgorithm("IM_TrkEventViewCreator")
        trkInputMakerAlg.ViewFallThrough = True
        trkInputMakerAlg.RoITool = ViewCreatorInitialROITool()
        trkInputMakerAlg.InViewRoIs = "InputRoI" # contract with the consumer
        trkInputMakerAlg.Views = "TrkView"
        trkInputMakerAlg.RequireParentView = True

        from TrigInDetConfig.utils import getFlagsForActiveConfig
        flagsWithTrk = getFlagsForActiveConfig(flags, "minBias", log)

        from TrigInDetConfig.InDetTrigSequence import InDetTrigSequence
        seq = InDetTrigSequence(flagsWithTrk, flagsWithTrk.Tracking.ActiveConfig.input_name, 
                                rois = trkInputMakerAlg.InViewRoIs, inView = "VDVMinBiasIDTracking")

        from TriggerMenuMT.HLT.Config.MenuComponents import extractAlgorithmsAndAppendCA
        algs = extractAlgorithmsAndAppendCA(seq.sequence("OfflineNoDataPrep"))

        #vdv = algs[0]
        #assert vdv.DataObjects, "Likely not ViewDataVerifier, does not have DataObjects property"
        #vdv.DataObjects += [("xAOD::TrigCompositeContainer", "HLT_vtx_z")]


        from ..Config.MenuComponents import algorithmCAToGlobalWrapper # this will disappear once whole sequence would be configured at once
        from TrigMinBias.MinBiasCountersConfig import TrackCounterHypoAlgCfg
        trackCountHypo = algorithmCAToGlobalWrapper(TrackCounterHypoAlgCfg, flags)[0]

        trkRecoSeq = parOR("TrkRecoSeq", algs)
        trkSequence = seqAND("TrkSequence", [trkInputMakerAlg, trkRecoSeq])
        trkInputMakerAlg.ViewNodeName = trkRecoSeq.name()

        return MenuSequence(flags,
                            Sequence    = trkSequence,
                            Maker       = trkInputMakerAlg,
                            Hypo        = trackCountHypo,
                            HypoToolGen = TrackCountHypoToolGen)

def MinBiasMbtsSequenceCfg(flags):
    recoAcc = InEventRecoCA(name="Mbts")
    from TrigMinBias.MbtsConfig import MbtsFexCfg, MbtsSGInputCfg
    fex = MbtsFexCfg(flags, MbtsBitsKey = recordable("HLT_MbtsBitsContainer"))
    recoAcc.mergeReco(fex)
    selAcc = SelectionCA("MbtsSel")
    hypo = CompFactory.MbtsHypoAlg("MbtsHypoAlg", MbtsBitsKey = fex.getPrimary().MbtsBitsKey)
    selAcc.mergeReco(recoAcc)
    selAcc.addHypoAlgo(hypo)

    return MenuSequenceCA(flags,
                          selAcc,
                          HypoToolGen = MbtsHypoToolGen,
                          globalRecoCA = MbtsSGInputCfg(flags))


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.lock()
    zf = MinBiasZVertexFinderSequenceCfg(flags)
    zf.ca.printConfig(withDetails=True)
    from ..Config.MenuComponents import menuSequenceCAToGlobalWrapper
    zfms = menuSequenceCAToGlobalWrapper(MinBiasZVertexFinderSequenceCfg, flags)


    mb = MinBiasMbtsSequenceCfg(flags)
    mb.ca.printConfig()
    mbms = menuSequenceCAToGlobalWrapper(MinBiasMbtsSequenceCfg, flags)


