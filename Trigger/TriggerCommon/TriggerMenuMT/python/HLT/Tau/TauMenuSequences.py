#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

# menu components
from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequence, RecoFragmentsPool

from TriggerMenuMT.HLT.Tau.TauRecoSequences import tauCaloMVASequence, tauFTFCoreSequence, tauFTFLRTSequence, tauFTFIsoSequence, tauFTFIsoBDTSequence, tauMVASequence, tauLLPSequence, tauLRTSequence, tauPrecIsoTrackSequence, tauPrecLRTTrackSequence

# ===============================================================================================
#      Calo MVA step
# ===============================================================================================

def tauCaloMVAMenuSeq(flags, name, is_probe_leg=False):
    (sequence, tauCaloMVAViewsMaker, sequenceOut) = RecoFragmentsPool.retrieve(tauCaloMVASequence,flags)

    # hypo
    from TrigTauHypo.TrigTauHypoConf import TrigTauCaloHypoAlg
    theTauCaloMVAHypo = TrigTauCaloHypoAlg(name+"L2CaloMVAHypo")
    theTauCaloMVAHypo.taujets     = sequenceOut

    from TrigTauHypo.TrigTauHypoTool import TrigL2TauHypoToolFromDict

    return  MenuSequence( flags,
                          Sequence    = sequence,
                          Maker       = tauCaloMVAViewsMaker,
                          Hypo        = theTauCaloMVAHypo,
                          HypoToolGen = TrigL2TauHypoToolFromDict,
                          IsProbe     = is_probe_leg )

# ===============================================================================================                                
#    Fast track finder (core) + TrackRoI Updater + PassBy Hypo step (tracktwoMVA)                                             
# ===============================================================================================                                                   

def tauFTFTauCoreSeq(flags, is_probe_leg=False):
    (sequence, ftfCoreViewsMaker, sequenceOut) = RecoFragmentsPool.retrieve(tauFTFCoreSequence,flags)

    from TrigTauHypo.TrigTauHypoConf import  TrigTrackPreSelHypoAlg
    fastTrkHypo                 = TrigTrackPreSelHypoAlg("TrackPreSelHypoAlg_PassByCore")
    fastTrkHypo.trackcollection = sequenceOut

    from TrigTauHypo.TrigTauHypoTool import TrigTauTrackHypoToolFromDict

    return  MenuSequence( flags,
                          Sequence    = sequence,
                          Maker       = ftfCoreViewsMaker,
                          Hypo        = fastTrkHypo,
                          HypoToolGen = TrigTauTrackHypoToolFromDict,
                          IsProbe     = is_probe_leg )

# ===============================================================================================
#    Fast track finder (LRT) + TrackRoI Updater + PassBy Hypo step
# ===============================================================================================

def tauFTFTauLRTSeq(flags, is_probe_leg=False):
    (sequence, ftfLRTViewsMaker, sequenceOut) = RecoFragmentsPool.retrieve(tauFTFLRTSequence,flags)

    from TrigTauHypo.TrigTauHypoConf import  TrigTrackPreSelHypoAlg
    fastTrkHypo                 = TrigTrackPreSelHypoAlg("TrackPreSelHypoAlg_PassByLRT")
    fastTrkHypo.trackcollection = sequenceOut
    fastTrkHypo.RoIForIDReadHandleKey = "UpdatedTrackLRTRoI"

    from TrigTauHypo.TrigTauHypoTool import TrigTauTrackHypoToolFromDict

    return  MenuSequence( flags,
                          Sequence    = sequence,
                          Maker       = ftfLRTViewsMaker,
                          Hypo        = fastTrkHypo,
                          HypoToolGen = TrigTauTrackHypoToolFromDict,
                          IsProbe     = is_probe_leg )

# ===============================================================================================                                                           
#   Fast track finder (iso) + Dummy Hypo step (tracktwoMVA)                                                     
# ===============================================================================================                                                            

def tauFTFTauIsoSeq(flags, is_probe_leg=False):
    (sequence, ftfIsoViewsMaker, sequenceOut) = RecoFragmentsPool.retrieve(tauFTFIsoSequence,flags )

    from TrigTauHypo.TrigTauHypoConf import  TrigTrackPreSelHypoAlg
    fastTrkHypo                 = TrigTrackPreSelHypoAlg("TrackPreSelHypoAlg_PassByIso")
    fastTrkHypo.trackcollection = sequenceOut

    from TrigTauHypo.TrigTauHypoTool import TrigTauTrackHypoToolFromDict

    return  MenuSequence( flags,
                          Sequence    = sequence,
                          Maker       = ftfIsoViewsMaker,
                          Hypo        = fastTrkHypo,
                          HypoToolGen = TrigTauTrackHypoToolFromDict,
                          IsProbe     = is_probe_leg )

# ===============================================================================================                                                            
#   Fast track finder (iso bdt) + Dummy Hypo step (tracktwoMVABDT)                                                                                           
# ===============================================================================================                                 

def tauFTFTauIsoBDTSeq(flags, is_probe_leg=False):
    (sequence, ftfIsoBDTViewsMaker, sequenceOut) = RecoFragmentsPool.retrieve(tauFTFIsoBDTSequence,flags )

    from TrigTauHypo.TrigTauHypoConf import  TrigTrackPreSelHypoAlg
    fastTrkHypo                 = TrigTrackPreSelHypoAlg("TrackPreSelHypoAlg_PassByIsoBDT")
    fastTrkHypo.trackcollection = sequenceOut
    fastTrkHypo.RoIForIDReadHandleKey = "UpdatedTrackBDTRoI"

    from TrigTauHypo.TrigTauHypoTool import TrigTauTrackHypoToolFromDict

    return  MenuSequence( flags,
                          Sequence    = sequence,
                          Maker       = ftfIsoBDTViewsMaker,
                          Hypo        = fastTrkHypo,
                          HypoToolGen = TrigTauTrackHypoToolFromDict,
                          IsProbe     = is_probe_leg )

# ===============================================================================================
#     Tau Precision MVA Alg + EFMVHypo step   (tracktwoMVA)
# ===============================================================================================

def tauTrackTwoMVASeq(flags, is_probe_leg=False):
    (sequence, mvaViewsMaker, sequenceOut) = RecoFragmentsPool.retrieve(tauMVASequence,flags )

    from TrigTauHypo.TrigTauHypoConf import  TrigEFTauMVHypoAlg
    precisionHypo = TrigEFTauMVHypoAlg("EFTauMVHypoAlgMVA")
    precisionHypo.taujetcontainer = sequenceOut

    from TrigTauHypo.TrigTauHypoTool import TrigEFTauMVHypoToolFromDict

    return  MenuSequence( flags,
                          Sequence    = sequence,
                          Maker       = mvaViewsMaker,
                          Hypo        = precisionHypo,
                          HypoToolGen = TrigEFTauMVHypoToolFromDict,
                          IsProbe     = is_probe_leg )

# ===============================================================================================
#     Tau Precision LLP Alg + EFMVHypo step   (tracktwoLLP)
# ===============================================================================================

def tauTrackTwoLLPSeq(flags, is_probe_leg=False):
    (sequence, mvaViewsMaker, sequenceOut) = RecoFragmentsPool.retrieve(tauLLPSequence,flags )

    from TrigTauHypo.TrigTauHypoConf import  TrigEFTauMVHypoAlg
    precisionHypo = TrigEFTauMVHypoAlg("EFTauMVHypoAlgLLP")
    precisionHypo.taujetcontainer = sequenceOut

    from TrigTauHypo.TrigTauHypoTool import TrigEFTauMVHypoToolFromDict

    return  MenuSequence( flags,
                          Sequence    = sequence,
                          Maker       = mvaViewsMaker,
                          Hypo        = precisionHypo,
                          HypoToolGen = TrigEFTauMVHypoToolFromDict,
                          IsProbe     = is_probe_leg )

# ===============================================================================================
#     Tau Precision LRT Alg + EFMVHypo step   (LRT)
# ===============================================================================================

def tauTrackLRTSeq(flags, is_probe_leg=False):
    (sequence, mvaViewsMaker, sequenceOut) = RecoFragmentsPool.retrieve(tauLRTSequence,flags)

    from TrigTauHypo.TrigTauHypoConf import  TrigEFTauMVHypoAlg
    precisionHypo = TrigEFTauMVHypoAlg("EFTauMVHypoAlgLRT")
    precisionHypo.taujetcontainer = sequenceOut

    from TrigTauHypo.TrigTauHypoTool import TrigEFTauMVHypoToolFromDict

    return  MenuSequence( flags,
                          Sequence    = sequence,
                          Maker       = mvaViewsMaker,
                          Hypo        = precisionHypo,
                          HypoToolGen = TrigEFTauMVHypoToolFromDict,
                          IsProbe     = is_probe_leg )

# ===============================================================================================                                                            
#     Precision Tracking + TrkPrecHypo step   (tracktwoEF, tracktwoMVA, tracktwoMVABDT)                                                                                               
# ===============================================================================================                                                           

def tauPrecTrackIsoSeq(flags, is_probe_leg=False):
    (sequence, precTrackViewsMaker, sequenceOut) = RecoFragmentsPool.retrieve(tauPrecIsoTrackSequence,flags )

    from TrigTauHypo.TrigTauHypoConf import  TrigTrkPrecHypoAlg
    precTrkHypo = TrigTrkPrecHypoAlg("TrkPrecIsoHypoAlg")
    precTrkHypo.trackparticles        = sequenceOut
    precTrkHypo.RoIForIDReadHandleKey = ""

    from TrigTauHypo.TrigTauHypoTool import TrigTrkPrecHypoToolFromDict

    return  MenuSequence( flags,
                          Sequence    = sequence,
                          Maker       = precTrackViewsMaker,
                          Hypo        = precTrkHypo,
                          HypoToolGen = TrigTrkPrecHypoToolFromDict,
                          IsProbe     = is_probe_leg )

# ===============================================================================================
#     Precision Tracking (LRT) + TrkPrecHypo step
# ===============================================================================================

def tauPrecTrackLRTSeq(flags, is_probe_leg=False):
    (sequence, precTrackViewsMaker, sequenceOut) = RecoFragmentsPool.retrieve(tauPrecLRTTrackSequence,flags)

    from TrigTauHypo.TrigTauHypoConf import  TrigTrkPrecHypoAlg
    precTrkHypo = TrigTrkPrecHypoAlg("TrkPrecLRTHypoAlg")
    precTrkHypo.trackparticles        = sequenceOut
    precTrkHypo.RoIForIDReadHandleKey = ""

    from TrigTauHypo.TrigTauHypoTool import TrigTrkPrecHypoToolFromDict

    return  MenuSequence( flags,
                          Sequence    = sequence,
                          Maker       = precTrackViewsMaker,
                          Hypo        = precTrkHypo,
                          HypoToolGen = TrigTrkPrecHypoToolFromDict,
                          IsProbe     = is_probe_leg )
