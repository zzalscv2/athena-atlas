#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

# menu components
from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequence, RecoFragmentsPool
from AthenaCommon.CFElements import parOR, seqAND
from ViewAlgs.ViewAlgsConf import EventViewCreatorAlgorithm
from DecisionHandling.DecisionHandlingConf import ViewCreatorPreviousROITool

def tag(ion):
    return 'precision' + ('HI' if ion is True else '') + 'Tracking'


def precisionTrackingSequence(flags, ion=False, variant=''):
    """ fourth step:  precision electron....."""

    from TriggerMenuMT.HLT.Egamma.TrigEgammaKeys import getTrigEgammaKeys
    TrigEgammaKeys = getTrigEgammaKeys(variant, ion=ion)
    caloClusters = TrigEgammaKeys.precisionElectronCaloClusterContainer
 

    InViewRoIs = "precisionTracking" + variant
    # EVCreator:
    precisionTrackingViewsMaker = EventViewCreatorAlgorithm("IM" + tag(ion) + variant)
    precisionTrackingViewsMaker.mergeUsingFeature=True # Merge inputs based on their Precision Calo feature
    precisionTrackingViewsMaker.RoITool = ViewCreatorPreviousROITool()
    precisionTrackingViewsMaker.InViewRoIs = InViewRoIs
    precisionTrackingViewsMaker.Views = tag(ion) + "Views" + variant
    precisionTrackingViewsMaker.ViewFallThrough = True
    precisionTrackingViewsMaker.RequireParentView = True
    
    # calling precision tracking
    from TriggerMenuMT.HLT.Electron.PrecisionTrackingRecoSequences import precisionTracking
    precisionTrackInViewSequence, trackParticles = precisionTracking(flags, InViewRoIs, ion, variant)

    precisionTrackingInViewAlgs = parOR(tag(ion) + "InViewAlgs" + variant, [precisionTrackInViewSequence])
    precisionTrackingViewsMaker.ViewNodeName = tag(ion) + "InViewAlgs" + variant

    # connect EVC and reco
    theSequence = seqAND(tag(ion) + "Sequence" + variant, [precisionTrackingViewsMaker, precisionTrackingInViewAlgs] )
    return (theSequence,precisionTrackingViewsMaker,caloClusters,trackParticles)



def precisionTrackingMenuSequence(flags, name, is_probe_leg=False, ion=False, variant=''):
    """ Creates precisionCalo MENU sequence """
    (sequence, precisionTrackingViewsMaker, caloClusters, trackParticles) = RecoFragmentsPool.retrieve(precisionTrackingSequence, flags, ion=ion, variant=variant)

    from TrigStreamerHypo.TrigStreamerHypoConf import TrigStreamerHypoAlg, TrigStreamerHypoTool
    thePrecisionTrackingHypo = TrigStreamerHypoAlg(name + tag(ion) + "Hypo" + variant)
    thePrecisionTrackingHypo.FeatureIsROI = False
    def acceptAllHypoToolGen(chainDict):
        return TrigStreamerHypoTool(chainDict["chainName"], Pass = True)
    return MenuSequence( flags,
                         Sequence    = sequence,
                         Maker       = precisionTrackingViewsMaker, 
                         Hypo        = thePrecisionTrackingHypo,
                         HypoToolGen = acceptAllHypoToolGen, # Note: TrigEgammaPrecisionTrackingHypoAlg does not use HypoTools
                         IsProbe     = is_probe_leg)


def precisionTrackingMenuSequence_LRT(flags, name, is_probe_leg=False):
    return precisionTrackingMenuSequence(flags, name, is_probe_leg=is_probe_leg, ion=False, variant='_LRT')
