# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaCommon.CFElements import parOR
from ..CommonSequences.FullScanDefs import trkFSRoI
from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequenceCA
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

from AthenaCommon.Logging import logging

logging.getLogger().info("Importing %s",__name__)
log = logging.getLogger(__name__)


def FullScanLRTTriggerSequence(flags):
    from TrigInDetConfig.ConfigSettings import getInDetTrigConfig
    lrtcfg = getInDetTrigConfig("fullScanLRT")

    from TriggerMenuMT.HLT.UnconventionalTracking.CommonConfiguration import getCommonInDetFullScanLRTSequence

    selAcc,reco,trkAcc = getCommonInDetFullScanLRTSequence(flags)

    from TrigInDetConfig.TrigInDetConfig import trigInDetPrecisionTrackingCfg

    acc = ComponentAccumulator()
    pt_seq = parOR("UncTrkrecoSeqfslrtpt")
    acc.addSequence(pt_seq)
    
    acc.merge(trigInDetPrecisionTrackingCfg(flags, trkFSRoI, lrtcfg.input_name,in_view=False), sequenceName=pt_seq.name)
    
    reco.mergeReco(trkAcc)
    reco.mergeReco(acc)
    
    sequenceOut = lrtcfg.tracks_IDTrig()

    return (selAcc, reco, sequenceOut)





def FullScanLRTTriggerMenuSequence(flags):
    from TrigLongLivedParticlesHypo.TrigFullScanLRTHypoTool import TrigLRTHypoToolFromDict
    from TrigEDMConfig.TriggerEDMRun3 import recordable

    selAcc, reco, sequenceOut = FullScanLRTTriggerSequence(flags)
    
    theHypoAlg = CompFactory.FastTrackFinderLRTHypoAlg("FullScanLRTHypoAlg",
                                                       trackCountKey = recordable("HLT_FSLRT_TrackCount"),
                                                       tracksKey = sequenceOut,
                                                       )
    selAcc.mergeReco(reco)
    selAcc.addHypoAlgo(theHypoAlg)

    log.info("Building the Step dictinary for FullScanLRT!")
    return MenuSequenceCA(flags,
                          selAcc,
                          HypoToolGen = TrigLRTHypoToolFromDict)

