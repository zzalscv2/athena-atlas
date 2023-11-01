# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaCommon.CFElements import parOR
from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequenceCA
from AthenaCommon.Logging import logging
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

logging.getLogger().info("Importing %s",__name__)
log = logging.getLogger(__name__)


def VrtSecInclusiveSequence(flags):
    from TrigInDetConfig.ConfigSettings import getInDetTrigConfig
    fscfg = getInDetTrigConfig("fullScan")
    lrtcfg = getInDetTrigConfig("fullScanLRT")

    
    from TriggerMenuMT.HLT.UnconventionalTracking.CommonConfiguration import getCommonInDetFullScanLRTSequence
    selAcc,reco = getCommonInDetFullScanLRTSequence(flags)

    acc = ComponentAccumulator()

    from TrigVrtSecInclusive.TrigVrtSecInclusiveConfig import TrigVrtSecInclusiveCfg
    theVSI = TrigVrtSecInclusiveCfg(flags, "TrigVrtSecInclusive", fscfg.tracks_FTF(), lrtcfg.tracks_FTF(), fscfg.vertex, "HLT_TrigVSIVertex", "HLT_TrigVSITrkPair",recordTrkPair=False)

    vsiseq = parOR("UncTrkrecoSeqVSI")
    acc.addSequence(vsiseq)
    acc.merge(theVSI)
    
    reco.mergeReco(acc)
    
    sequenceOut = "HLT_TrigVSIVertex"

    return (selAcc, reco, sequenceOut)




def VrtSecInclusiveMenuSequence(flags):
    from TrigLongLivedParticlesHypo.TrigVrtSecInclusiveHypoConfig import TrigVSIHypoToolFromDict
    from TrigLongLivedParticlesHypo.TrigVrtSecInclusiveHypoConfig import createTrigVSIHypoAlgCfg

    ( selAcc, reco, sequenceOut) = VrtSecInclusiveSequence(flags)

    from TrigEDMConfig.TriggerEDMRun3 import recordable
    theHypoAlg = createTrigVSIHypoAlgCfg(flags, "TrigVSIHypoAlg",
                                         verticesKey=recordable(sequenceOut),
                                         vtxCountKey = recordable("HLT_TrigVSI_VtxCount"),
                                         isViewBased=False)
    selAcc.mergeReco(reco)
    selAcc.addHypoAlgo(theHypoAlg)
    
    log.info("Building the Step dictinary for TrigVSI!")
    return MenuSequenceCA(flags,
                          selAcc,
                          HypoToolGen = TrigVSIHypoToolFromDict)
                          
