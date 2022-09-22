#
#  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.AllConfigFlags import ConfigFlags 

# menu components   
from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequence, RecoFragmentsPool, algorithmCAToGlobalWrapper
from AthenaCommon.CFElements import parOR, seqAND
from ViewAlgs.ViewAlgsConf import EventViewCreatorAlgorithm
from DecisionHandling.DecisionHandlingConf import ViewCreatorPreviousROITool
from TriggerMenuMT.HLT.Egamma.TrigEgammaKeys import getTrigEgammaKeys


def fastElectronSequence(ConfigFlags, variant=''):
    """ second step:  tracking....."""
    InViewRoIs = "EMFastElectronRoIs"+variant
  
    # EVCreator:
    fastElectronViewsMaker = EventViewCreatorAlgorithm("IMfastElectron"+variant)
    fastElectronViewsMaker.RoIsLink = "initialRoI" # Merge inputs based on their initial L1 ROI
    fastElectronViewsMaker.RoITool = ViewCreatorPreviousROITool()
    fastElectronViewsMaker.InViewRoIs = InViewRoIs
    fastElectronViewsMaker.Views = "EMElectronViews"+variant
    fastElectronViewsMaker.ViewFallThrough = True
    fastElectronViewsMaker.RequireParentView = True

    # Configure the reconstruction algorithm sequence
    from TriggerMenuMT.HLT.Electron.FastElectronRecoSequences import fastElectronRecoSequence
    (fastElectronRec, sequenceOut) = fastElectronRecoSequence(InViewRoIs, variant)
    
    # Suffix to distinguish probe leg sequences
    fastElectronInViewAlgs = parOR("fastElectronInViewAlgs" + variant, [fastElectronRec])
    fastElectronViewsMaker.ViewNodeName = "fastElectronInViewAlgs" + variant

    from TrigGenericAlgs.TrigGenericAlgsConfig import ROBPrefetchingAlgCfg_Si

    robPrefetchAlg = algorithmCAToGlobalWrapper(ROBPrefetchingAlgCfg_Si, ConfigFlags, nameSuffix=fastElectronViewsMaker.name())[0]
    fastElectronAthSequence = seqAND("fastElectronAthSequence" + variant, [fastElectronViewsMaker, robPrefetchAlg, fastElectronInViewAlgs] )
    return (fastElectronAthSequence, fastElectronViewsMaker, sequenceOut)

def fastElectronSequence_LRT(ConfigFlags):
    # This is SAME as fastElectronSequence but for variant "_LRT"
    return fastElectronSequence(ConfigFlags,"_LRT")


def fastElectronMenuSequence(is_probe_leg=False, variant=''):
    """ Creates 2nd step Electron  MENU sequence"""
    # retrieve the reco sequence+EVC
    theSequence = {
            ''      : fastElectronSequence,
            '_LRT'  : fastElectronSequence_LRT
            }
    (fastElectronAthSequence, fastElectronViewsMaker, sequenceOut) = RecoFragmentsPool.retrieve(theSequence[variant], ConfigFlags)
    # make the Hypo
    from TrigEgammaHypo.TrigEgammaHypoConf import TrigEgammaFastElectronHypoAlg
    TrigEgammaKeys = getTrigEgammaKeys(variant)

    theElectronHypo = TrigEgammaFastElectronHypoAlg("TrigEgammaFastElectronHypoAlg"+variant)
    theElectronHypo.Electrons = TrigEgammaKeys.fastElectronContainer

    theElectronHypo.RunInView=True

    from TrigEgammaHypo.TrigEgammaFastElectronHypoTool import TrigEgammaFastElectronHypoToolFromDict

    return  MenuSequence( Maker       = fastElectronViewsMaker,                                        
                          Sequence    = fastElectronAthSequence,
                          Hypo        = theElectronHypo,
                          HypoToolGen = TrigEgammaFastElectronHypoToolFromDict,
                          IsProbe=is_probe_leg)


def fastElectronMenuSequence_LRT(is_probe_leg=False):
    # This is to call fastElectronMenuSequence for the _LRT variant
    return fastElectronMenuSequence(is_probe_leg=is_probe_leg, variant='_LRT')
