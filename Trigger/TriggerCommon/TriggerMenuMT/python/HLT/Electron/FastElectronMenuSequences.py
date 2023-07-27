#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

from TriggerMenuMT.HLT.Egamma.TrigEgammaKeys import getTrigEgammaKeys

# menu components   
from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequenceCA, SelectionCA, InViewRecoCA, menuSequenceCAToGlobalWrapper
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentFactory import isComponentAccumulatorCfg
from AthenaConfiguration.AccumulatorCache import AccumulatorCache

@AccumulatorCache
def fastElectronSequenceCfg(flags, name, variant='', is_probe_leg = False):
    """ second step:  tracking....."""

    InViewRoIs = "EMFastElectronRoIs"+variant

    roiTool = CompFactory.ViewCreatorPreviousROITool()
    reco = InViewRecoCA("EMElectron"+variant, RoITool = roiTool, InViewRoIs = InViewRoIs, RequireParentView = True, isProbe=is_probe_leg)

    # Configure the reconstruction algorithm sequence
    from TriggerMenuMT.HLT.Electron.FastElectronRecoSequences import fastElectronRecoSequence

    from TrigGenericAlgs.TrigGenericAlgsConfig import ROBPrefetchingAlgCfg_Si
 
    robPrefetchAlg = ROBPrefetchingAlgCfg_Si(flags, nameSuffix='IM_'+reco.name)

    reco.mergeReco(fastElectronRecoSequence(flags, name, InViewRoIs, variant))
    
    theFastElectronHypo = CompFactory.TrigEgammaFastElectronHypoAlg("TrigEgammaFastElectronHypoAlg"+variant)
    TrigEgammaKeys = getTrigEgammaKeys(variant)
    theFastElectronHypo.Electrons = TrigEgammaKeys.fastElectronContainer
    theFastElectronHypo.RunInView = True
    from TrigEgammaHypo.TrigEgammaFastElectronHypoTool import TrigEgammaFastElectronHypoToolFromDict

    selAcc = SelectionCA('FastElectronMenuSequence'+variant,isProbe=is_probe_leg)
    selAcc.mergeReco(reco, robPrefetchCA=robPrefetchAlg)
    selAcc.addHypoAlgo(theFastElectronHypo)

    return MenuSequenceCA(flags,selAcc,HypoToolGen=TrigEgammaFastElectronHypoToolFromDict,isProbe=is_probe_leg)


def fastElectronSequenceCfg_lrt(flags, name, is_probe_leg=False):
    # This is to call fastElectronMenuSequence for the _LRT variant
    return fastElectronSequenceCfg(flags, name, is_probe_leg=is_probe_leg, variant='_LRT')


def fastElectronMenuSequence(flags, name="FastElectron", is_probe_leg=False):
    """Creates secpond step photon sequence"""

    if isComponentAccumulatorCfg():
        return fastElectronSequenceCfg(flags, name = name, is_probe_leg=is_probe_leg)
    else: 
        return menuSequenceCAToGlobalWrapper(fastElectronSequenceCfg, flags, name = name, is_probe_leg=is_probe_leg)


def fastElectronMenuSequence_LRT(flags,name="FastElectron", is_probe_leg=False):
    """Creates secpond step photon sequence"""

    if isComponentAccumulatorCfg():
        return fastElectronSequenceCfg_lrt(flags, name = name, is_probe_leg=is_probe_leg)
    else:
        return menuSequenceCAToGlobalWrapper(fastElectronSequenceCfg_lrt, flags, name = name, is_probe_leg=is_probe_leg)
