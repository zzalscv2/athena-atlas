# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.CFElements import seqAND, parOR
from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequence, RecoFragmentsPool
from AthenaCommon.Logging import logging
from ..Config.MenuComponents import algorithmCAToGlobalWrapper
from ..CommonSequences.FullScanInDetSequences import getCommonInDetFullScanSequence
from ..CommonSequences.FullScanInDetConfig import commonInDetLRTCfg

logging.getLogger().info("Importing %s",__name__)
log = logging.getLogger(__name__)

# ---------------------

def getFullScanRecoOnlySequence(flags):

    from TrigStreamerHypo.TrigStreamerHypoConf import TrigStreamerHypoAlg
    from TrigStreamerHypo.TrigStreamerHypoConfig import StreamerHypoToolGenerator

    TrkSeq, sequenceOut = RecoFragmentsPool.retrieve(getCommonInDetFullScanSequence,flags)

    from TriggerMenuMT.HLT.Jet.JetMenuSequencesConfig import getTrackingInputMaker
    InputMakerAlg=getTrackingInputMaker("ftf")

    HypoAlg = TrigStreamerHypoAlg("UncTrkDummyStream")

    log.debug("Building the menu sequence for FullScanRecoOnlySequence")
    return MenuSequence( flags,
                         Sequence    = seqAND("UncTrkrecoSeq", [InputMakerAlg]+TrkSeq),
                         Maker       = InputMakerAlg,
                         Hypo        = HypoAlg,
                         HypoToolGen = StreamerHypoToolGenerator )


def getCommonInDetFullScanLRTSequence(flags):
    from TrigInDetConfig.ConfigSettings import getInDetTrigConfig
    std_cfg = getInDetTrigConfig("fullScan" )
    lrt_cfg = getInDetTrigConfig("fullScanLRT")

    from TriggerMenuMT.HLT.Jet.JetMenuSequencesConfig import getTrackingInputMaker
    InputMakerAlg=getTrackingInputMaker("ftf")

    #create two sequencers first contains everything apart from the final two algs
    std_algs, std_trks = getCommonInDetFullScanSequence(flags)
    std_seq = seqAND("UncTrkrecoSeq", [InputMakerAlg]+std_algs)

    lrt_algs = algorithmCAToGlobalWrapper(commonInDetLRTCfg, flags, std_cfg, lrt_cfg)

    #sequence for the lrt objects final two algs
    lrt_seq = seqAND("UncTrklrtstagerecoSeq", lrt_algs)
    combined_seq = parOR("UncTrklrtrecoSeq", [std_seq, lrt_seq])

    return (combined_seq, InputMakerAlg, lrt_cfg.tracks_FTF())
