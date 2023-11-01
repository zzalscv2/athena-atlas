# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.CFElements import seqAND
from AthenaCommon.Logging import logging
from ..CommonSequences.FullScanInDetConfig import commonInDetFullScanCfg,commonInDetLRTCfg
from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequenceCA, SelectionCA, InEventRecoCA
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator


logging.getLogger().info("Importing %s",__name__)
log = logging.getLogger(__name__)

# ---------------------

def getFullScanRecoOnlySequence(flags):

    from TrigStreamerHypo.TrigStreamerHypoConfig import StreamerHypoToolGenerator

    from TriggerMenuMT.HLT.Jet.JetMenuSequencesConfig import getTrackingInputMaker
    reco = InEventRecoCA("UncTrkreco",inputMaker=getTrackingInputMaker("ftf"))
    

    selAcc = SelectionCA("UncTrkrecoSel")

    acc = ComponentAccumulator()
    seqReco = seqAND("UncTrkrecoSeq")
    acc.addSequence(seqReco)
    
    trkseq = commonInDetFullScanCfg(flags)
    acc.merge(trkseq, sequenceName=seqReco.name)
    
    reco.mergeReco(acc)
    selAcc.mergeReco(reco)
    
    HypoAlg = CompFactory.TrigStreamerHypoAlg("UncTrkDummyStream")
    selAcc.addHypoAlgo(HypoAlg)

    log.debug("Building the menu sequence for FullScanRecoOnlySequence")
    return MenuSequenceCA(flags,
                          selAcc,
                          HypoToolGen = StreamerHypoToolGenerator)



def getCommonInDetFullScanLRTSequence(flags):
    from TrigInDetConfig.ConfigSettings import getInDetTrigConfig
    std_cfg = getInDetTrigConfig("fullScan" )
    lrt_cfg = getInDetTrigConfig("fullScanLRT")

    selAcc = SelectionCA("UncLRTSeq")
    
    from ..CommonSequences.FullScanDefs import  trkFSRoI
    reco = InEventRecoCA(name="_Jet_TrackingStep",
                         RoIs = trkFSRoI,
                         RoITool = CompFactory.ViewCreatorInitialROITool())

    acc = ComponentAccumulator()
    std_seq = seqAND("UncTrkrecoSeq")
    acc.addSequence(std_seq)
    acc.merge(commonInDetFullScanCfg(flags),sequenceName=std_seq.name)
    
    lrt_seq = seqAND("UncTrklrtstagerecoSeq")
    acc.addSequence(lrt_seq)
    acc.merge(commonInDetLRTCfg(flags, std_cfg, lrt_cfg))
    
    reco.mergeReco(acc)

    return (selAcc, reco)
