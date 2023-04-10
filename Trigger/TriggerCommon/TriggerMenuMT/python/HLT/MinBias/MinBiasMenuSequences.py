#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequence
from AthenaCommon.CFElements import parOR
from AthenaCommon.CFElements import seqAND
from TrigEDMConfig.TriggerEDMRun3 import recordable
from ViewAlgs.ViewAlgsConf import EventViewCreatorAlgorithm
from DecisionHandling.DecisionHandlingConf import ViewCreatorInitialROITool
from TrigInDetConfig.ConfigSettings import getInDetTrigConfig
import AthenaCommon.SystemOfUnits as Units
from TrigMinBias.TrigMinBiasMonitoring import MbtsHypoToolMonitoring

from AthenaConfiguration.ComponentFactory import CompFactory
from ..Config.MenuComponents import InViewRecoCA, InEventRecoCA, SelectionCA, MenuSequenceCA


########
# to move into TrigMinBiasHypoConfigMT?

def SPCountHypoToolGen(chainDict):
    from TrigMinBias.TrigMinBiasConf import SPCountHypoTool
    hypo = SPCountHypoTool(chainDict["chainName"])
    if "hmt" in chainDict["chainName"]:
        hypo.sctSP = int(chainDict["chainParts"][0]["hypoSPInfo"].strip("sp"))
    if "mb_sptrk" in chainDict["chainName"]:
        hypo.pixCL = 2
        hypo.sctSP = 3
    if "mb_excl" in chainDict["chainName"]:
        hypo.pixCLMax = 150 # TODO revisit tightening those
        hypo.sctSPMax = 150 # as above
    if "sp_pix" in chainDict["chainName"]:
        hypo.pixCL = int(chainDict["chainParts"][0]["hypoSPInfo"].strip("pix"))
    return hypo



def TrackCountHypoToolGen(chainDict):
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
        hypo.minPt = int(chainDict["chainParts"][0]["hypoPtInfo"].strip("pt"))*Units.GeV
        hypo.maxZ0 = 401*Units.millimeter
    if "mb_excl" in chainDict["chainName"]:
        hypo.exclusive = True
        hypo.minPt = int(chainDict["chainParts"][0]["hypoPtInfo"].strip("pt"))*Units.GeV
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
    


### Now the sequences

def MinBiasSPSequence(flags):
    spAlgsList = []
    from TrigMinBias.TrigMinBiasConf import SPCountHypoAlg

    spInputMakerAlg = EventViewCreatorAlgorithm("IM_SPEventViewCreator")
    spInputMakerAlg.ViewFallThrough = True
    spInputMakerAlg.RoITool = ViewCreatorInitialROITool()
    spInputMakerAlg.InViewRoIs = "InputRoI"
    spInputMakerAlg.Views = "SPView"
    idTrigConfig = getInDetTrigConfig('minBias')

    from TrigInDetConfig.InDetTrigFastTracking import makeInDetTrigFastTracking
    idAlgs, verifier = makeInDetTrigFastTracking(flags, config=idTrigConfig,
                                     rois=spInputMakerAlg.InViewRoIs, 
                                     viewVerifier='IDDataPrepCosmicsDataVerifier', 
                                     doFTF=False)
    verifier.DataObjects += [('TrigRoiDescriptorCollection', 'StoreGateSvc+InputRoI'),
                             ('SCT_ID', 'DetectorStore+SCT_ID'),
                             ('PixelID', 'DetectorStore+PixelID'),
                             ('TagInfo', 'DetectorStore+ProcessingTags')]

    # Make sure required objects are still available at whole-event level
    from AthenaCommon.AlgSequence import AlgSequence
    topSequence = AlgSequence()
    topSequence.SGInputLoader.Load += [('SCT_ID', 'DetectorStore+SCT_ID'),
                                       ('PixelID', 'DetectorStore+PixelID'),
                                       ('TagInfo', 'DetectorStore+ProcessingTags')]

#    spAlgsList = idAlgs[:-2]
    spAlgsList = idAlgs

    from ..Config.MenuComponents import algorithmCAToGlobalWrapper # this will disappear once whole sequence would be configured at once
    from TrigMinBias.MinBiasCountersConfig import SPCounterRecoAlgCfg
    spCount = algorithmCAToGlobalWrapper(SPCounterRecoAlgCfg, flags)[0]

    spRecoSeq = parOR("spRecoSeq", spAlgsList + [spCount])
    spSequence = seqAND("spSequence", [spInputMakerAlg, spRecoSeq])
    spInputMakerAlg.ViewNodeName = spRecoSeq.name()


    spCountHypo =SPCountHypoAlg()
    spCountHypo.SpacePointsKey=recordable("HLT_SpacePointCounts")

    return MenuSequence(flags,
                        Sequence    = spSequence,
                        Maker       = spInputMakerAlg,
                        Hypo        = spCountHypo,
                        HypoToolGen = SPCountHypoToolGen )

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

        # prepare algorithms to run in views, first,
        # inform scheduler that input data is available in parent view (has to be done by hand)
        idTrigConfig = getInDetTrigConfig('minBias')

        from TrigInDetConfig.EFIDTracking import makeInDetPatternRecognition
        algs,_ = makeInDetPatternRecognition(flags, idTrigConfig, verifier='VDVMinBiasIDTracking')
        vdv = algs[0]
        assert vdv.DataObjects, "Likely not ViewDataVerifier, does not have DataObjects property"
        vdv.DataObjects += [("xAOD::TrigCompositeContainer", "HLT_vtx_z")]


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


