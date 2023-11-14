# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequenceCA, SelectionCA, InEventRecoCA
from AthenaConfiguration.ComponentFactory import CompFactory

from AthenaCommon.Logging import logging

logging.getLogger().info("Importing %s",__name__)
log = logging.getLogger(__name__)


def FullScanLRTMenuSequence(flags):
    from TrigInDetConfig.ConfigSettings import getInDetTrigConfig
    lrtcfg = getInDetTrigConfig("fullScanLRT")

    # Construct the full reco sequence
    from TriggerMenuMT.HLT.UnconventionalTracking.CommonConfiguration import getCommonInDetFullScanLRTCfg
    from TriggerMenuMT.HLT.Jet.JetMenuSequencesConfig import getTrackingInputMaker
    reco = InEventRecoCA("UncFSVSIreco",inputMaker=getTrackingInputMaker("ftf"))
    reco.mergeReco( getCommonInDetFullScanLRTCfg(flags) )

    from ..CommonSequences.FullScanDefs import trkFSRoI
    from TrigInDetConfig.TrigInDetConfig import trigInDetPrecisionTrackingCfg
    reco.mergeReco(trigInDetPrecisionTrackingCfg(flags, trkFSRoI, lrtcfg.input_name,in_view=False))

    # Construct the SelectionCA to hold reco + hypo
    selAcc = SelectionCA("UncFSLRTSeq")
    selAcc.mergeReco(reco)

    from TrigLongLivedParticlesHypo.TrigFullScanLRTHypoTool import TrigLRTHypoToolFromDict
    from TrigEDMConfig.TriggerEDMRun3 import recordable
    
    theHypoAlg = CompFactory.FastTrackFinderLRTHypoAlg("FullScanLRTHypoAlg",
                                                       trackCountKey = recordable("HLT_FSLRT_TrackCount"),
                                                       tracksKey = lrtcfg.tracks_IDTrig(),
                                                       )
    selAcc.addHypoAlgo(theHypoAlg)

    log.info("Building the Step dictinary for FullScanLRT!")
    return MenuSequenceCA(flags,
                          selAcc,
                          HypoToolGen = TrigLRTHypoToolFromDict)

