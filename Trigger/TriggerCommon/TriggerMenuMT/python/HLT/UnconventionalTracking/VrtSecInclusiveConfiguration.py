# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaCommon.CFElements import parOR
from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequence, RecoFragmentsPool
from AthenaCommon.Logging import logging

logging.getLogger().info("Importing %s",__name__)
log = logging.getLogger(__name__)


def VrtSecInclusiveSequence(flags):
    from TrigInDetConfig.ConfigSettings import getInDetTrigConfig
    fscfg = getInDetTrigConfig("fullScan")
    lrtcfg = getInDetTrigConfig("fullScanLRT")

    from TriggerMenuMT.HLT.UnconventionalTracking.CommonConfiguration import getCommonInDetFullScanLRTSequence

    ftf_seqs, im_alg, seqOut = RecoFragmentsPool.retrieve(getCommonInDetFullScanLRTSequence,flags)

    from TrigVrtSecInclusive.TrigVrtSecInclusiveConfig import TrigVrtSecInclusiveCfg
    theVSI = TrigVrtSecInclusiveCfg(flags, "TrigVrtSecInclusive", fscfg.tracks_FTF(), lrtcfg.tracks_FTF(), fscfg.vertex, "HLT_TrigVSIVertex", "HLT_TrigVSITrkPair")
    theVSI.recordTrkPair = False
    vtx_reco_algs = [theVSI]

    vsiseq = parOR("UncTrkrecoSeqVSI", [vtx_reco_algs])
    TrkSeq = parOR("UncTrkrecoSeqLRTVSI", [ftf_seqs, vsiseq])
    sequenceOut = "HLT_TrigVSIVertex"

    return (TrkSeq, im_alg, sequenceOut)




def VrtSecInclusiveMenuSequence(flags):
    from TrigLongLivedParticlesHypo.TrigVrtSecInclusiveHypoConfig import TrigVSIHypoToolFromDict
    from TrigLongLivedParticlesHypo.TrigVrtSecInclusiveHypoConfig import createTrigVSIHypoAlg

    ( TrkSeq, im_alg, sequenceOut) = RecoFragmentsPool.retrieve(VrtSecInclusiveSequence,flags)

    theHypoAlg = createTrigVSIHypoAlg(flags, "TrigVSIHypoAlg")

    from TrigEDMConfig.TriggerEDMRun3 import recordable
    theHypoAlg.verticesKey = recordable(sequenceOut)
    theHypoAlg.isViewBased = False

    log.info("Building the Step dictinary for TrigVSI!")
    return MenuSequence(flags,
                        Sequence    = TrkSeq,
                        Maker       = im_alg,
                        Hypo        = theHypoAlg,
                        HypoToolGen = TrigVSIHypoToolFromDict,
                        )
