#
#  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.AllConfigFlags import ConfigFlags

# menu components   
from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequence, RecoFragmentsPool,algorithmCAToGlobalWrapper
from AthenaCommon.CFElements import parOR, seqAND
from ViewAlgs.ViewAlgsConf import EventViewCreatorAlgorithm
from DecisionHandling.DecisionHandlingConf import ViewCreatorCentredOnClusterROITool


def fastTrackingSequence(ConfigFlags, variant=''):
    """ second step:  tracking....."""
    from TriggerMenuMT.HLT.Egamma.TrigEgammaKeys import getTrigEgammaKeys
    TrigEgammaKeys = getTrigEgammaKeys(variant)
    IDTrigConfig = TrigEgammaKeys.IDTrigConfig
    RoIs = "EMIDRoIs"+variant
    # EVCreator:
    fastTrackingViewsMaker = EventViewCreatorAlgorithm("IMfastTracking"+variant)
    fastTrackingViewsMaker.mergeUsingFeature=True
    fastTrackingViewsMaker.RoIsLink = "initialRoI" # Merge inputs based on their initial L1 ROI
    # Spawn View on SuperRoI encompassing all clusters found within the L1 RoI
    roiTool = ViewCreatorCentredOnClusterROITool()
    roiTool.AllowMultipleClusters = False # If True: SuperROI mode. If False: highest eT cluster in the L1 ROI
    roiTool.RoisWriteHandleKey = TrigEgammaKeys.fastTrackingRoIContainer
    roiTool.RoIEtaWidth = IDTrigConfig.etaHalfWidth
    roiTool.RoIPhiWidth = IDTrigConfig.phiHalfWidth
    fastTrackingViewsMaker.RoITool = roiTool
    fastTrackingViewsMaker.InViewRoIs = RoIs
    fastTrackingViewsMaker.Views = "EMFastTrackingViews"+variant
    fastTrackingViewsMaker.ViewFallThrough = True
    fastTrackingViewsMaker.RequireParentView = True

    # calling fast tracking
    from TriggerMenuMT.HLT.Electron.FastTrackingRecoSequences import fastTracking
    fastTrackingInViewSequence, trackParticles = fastTracking(RoIs,variant)

    fastTrackingInViewAlgs = parOR("fastTrackingInViewAlgs"+variant, [fastTrackingInViewSequence])
    fastTrackingViewsMaker.ViewNodeName = "fastTrackingInViewAlgs"+variant

    # connect EVC and reco
    from TrigGenericAlgs.TrigGenericAlgsConfig import ROBPrefetchingAlgCfg_Si
    robPrefetchAlg = algorithmCAToGlobalWrapper(ROBPrefetchingAlgCfg_Si,ConfigFlags,nameSuffix=fastTrackingViewsMaker.name())[0]
    theSequence = seqAND("fastTrackingSequence"+variant, [fastTrackingViewsMaker, robPrefetchAlg, fastTrackingInViewAlgs] )
    return (theSequence,fastTrackingViewsMaker,trackParticles)


def fastTrackingMenuSequence(name, is_probe_leg=False, variant=''):
    """ Creates precisionCalo MENU sequence """
    (sequence, fastTrackingViewsMaker, trackParticles) = RecoFragmentsPool.retrieve(fastTrackingSequence, ConfigFlags , variant=variant)

    from TrigStreamerHypo.TrigStreamerHypoConf import TrigStreamerHypoAlg, TrigStreamerHypoTool
    theFastTrackingHypo = TrigStreamerHypoAlg(name + "fastTrackingHypo"+variant)
    theFastTrackingHypo.FeatureIsROI = False
    def acceptAllHypoToolGen(chainDict):
        return TrigStreamerHypoTool(chainDict["chainName"], Pass = True)
    return MenuSequence( Sequence    = sequence,
                         Maker       = fastTrackingViewsMaker, 
                         Hypo        = theFastTrackingHypo,
                         HypoToolGen = acceptAllHypoToolGen,
                         IsProbe     = is_probe_leg)


def fastTrackingMenuSequence_LRT(name, is_probe_leg=False):
   return fastTrackingMenuSequence(name, is_probe_leg=is_probe_leg, variant='_LRT')
