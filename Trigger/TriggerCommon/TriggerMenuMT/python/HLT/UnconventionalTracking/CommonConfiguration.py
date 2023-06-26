# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.CFElements import seqAND, parOR
from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequence, RecoFragmentsPool
from AthenaCommon.Logging import logging

logging.getLogger().info("Importing %s",__name__)
log = logging.getLogger(__name__)

# ---------------------

def getCommonInDetFullScanSequence(flags):

    from TriggerMenuMT.HLT.Jet.JetMenuSequences import getTrackingInputMaker
    InputMakerAlg=getTrackingInputMaker("ftf")

    from TrigInDetConfig.ConfigSettings import getInDetTrigConfig
    from TrigInDetConfig.utils import getInDetFlagsForSignature
    IDTrigConfig = getInDetTrigConfig("jet")
    flagsWithTrk = getInDetFlagsForSignature(flags,IDTrigConfig.name)

    from ..CommonSequences.FullScanDefs import trkFSRoI
    from TrigInDetConfig.InDetTrigFastTracking import makeInDetTrigFastTrackingNoView
    TrkInputNoViewAlg = makeInDetTrigFastTrackingNoView(flagsWithTrk, config=IDTrigConfig, rois=trkFSRoI)

    from TrigInDetConfig.InDetTrigVertices import makeInDetTrigVertices
    
    vtxAlgs = makeInDetTrigVertices(flagsWithTrk, "jet", IDTrigConfig.tracks_FTF(), IDTrigConfig.vertex_jet, IDTrigConfig, adaptiveVertex=IDTrigConfig.adaptiveVertex_jet)
    prmVtx = vtxAlgs[-1]

    TrkSeq = [InputMakerAlg,TrkInputNoViewAlg, prmVtx]
    sequenceOut = IDTrigConfig.tracks_FTF()

    return (TrkSeq, InputMakerAlg, sequenceOut)

# ---------------------

def getFullScanRecoOnlySequence(flags):

    from TrigStreamerHypo.TrigStreamerHypoConf import TrigStreamerHypoAlg
    from TrigStreamerHypo.TrigStreamerHypoConfig import StreamerHypoToolGenerator

    ( TrkSeq, InputMakerAlg, sequenceOut) = RecoFragmentsPool.retrieve(getCommonInDetFullScanSequence,flags)

    HypoAlg = TrigStreamerHypoAlg("UncTrkDummyStream")

    log.debug("Building the menu sequence for FullScanRecoOnlySequence")
    return MenuSequence( flags,
                         Sequence    = seqAND("UncTrkrecoSeq", TrkSeq),
                         Maker       = InputMakerAlg,
                         Hypo        = HypoAlg,
                         HypoToolGen = StreamerHypoToolGenerator )

def getCommonInDetFullScanLRTSequence(flags):
    from TrigInDetConfig.ConfigSettings import getInDetTrigConfig
    std_cfg = getInDetTrigConfig("jet" )
    lrt_cfg = getInDetTrigConfig("fullScanLRT")

    from TriggerMenuMT.HLT.Jet.JetMenuSequences import getTrackingInputMaker
    InputMakerAlg=getTrackingInputMaker("ftf")

    from ..CommonSequences.FullScanDefs import trkFSRoI
    from TrigInDetConfig.InDetTrigFastTracking import makeInDetTrigFastTrackingNoView

    ftf_algs = makeInDetTrigFastTrackingNoView(flags, config=std_cfg, secondStageConfig = lrt_cfg, rois=trkFSRoI)

    #create two sequencers first contains everything apart from the final two algs
    std_seq = seqAND("UncTrkrecoSeq", [InputMakerAlg, ftf_algs[0:-2]])
    #sequence for the lrt objects final two algs
    lrt_seq = seqAND("UncTrklrtstagerecoSeq", ftf_algs[-2:len(ftf_algs)])

    combined_seq = parOR("UncTrklrtrecoSeq", [std_seq, lrt_seq])

    return (combined_seq, InputMakerAlg, lrt_cfg.tracks_FTF())
