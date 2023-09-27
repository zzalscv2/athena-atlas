#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

from TriggerMenuMT.HLT.Egamma.TrigEgammaKeys import getTrigEgammaKeys

# menu components   
from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequenceCA, SelectionCA, InViewRecoCA, menuSequenceCAToGlobalWrapper
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentFactory import isComponentAccumulatorCfg

def tag(ion):
    return 'precision' + ('HI' if ion is True else '') + 'Electron'


def precisionElectronSequenceCfg(flags, ion=False, variant='',is_probe_leg = False):
    """ seventh step:  precision electron....."""
    
    inViewRoIs = "precisionElectron"
    probeInfo = '_probe' if  is_probe_leg else ''
    roiTool = CompFactory.ViewCreatorPreviousROITool()
    reco = InViewRecoCA(tag(ion)+variant, RoITool = roiTool, InViewRoIs = inViewRoIs, RequireParentView = True, isProbe=is_probe_leg)
    
    # Configure the reconstruction algorithm sequence
    from TriggerMenuMT.HLT.Electron.PrecisionElectronRecoSequences import precisionElectronRecoSequence
    reco.mergeReco(precisionElectronRecoSequence(flags, inViewRoIs, ion, doGSF=False, doLRT = 'LRT' in variant))
    TrigEgammaKeys = getTrigEgammaKeys(variant, ion=ion)
    selAcc = SelectionCA('PrecisionElectronMenuSequence'+variant,isProbe=is_probe_leg)
    
    from TrigEgammaHypo.TrigEgammaPrecisionElectronHypoTool import TrigEgammaPrecisionElectronHypoToolFromDict, TrigEgammaPrecisionElectronHypoAlgCfg

    selAcc.mergeReco(reco)
    selAcc.mergeHypo(TrigEgammaPrecisionElectronHypoAlgCfg(flags, "TrigEgamma"+tag(ion)+"HypoAlg_noGSF"+variant+probeInfo, TrigEgammaKeys.precisionElectronContainer ))
    return MenuSequenceCA(flags,selAcc,HypoToolGen=TrigEgammaPrecisionElectronHypoToolFromDict, isProbe=is_probe_leg)

def precisionElectronSequenceCfg_lrt(flags,is_probe_leg=False):
    # This is to call precisionElectronMenuSequence for the _LRT variant
    return precisionElectronSequenceCfg(flags, ion=False, variant='_LRT',is_probe_leg=is_probe_leg)


def precisionElectronMenuSequence(flags, is_probe_leg=False, ion=False,  variant=''):
    """Creates seventh step of electron sequence"""

    if isComponentAccumulatorCfg():
        return precisionElectronSequenceCfg(flags, ion=ion, variant=variant, is_probe_leg=is_probe_leg)
    else: 
        return menuSequenceCAToGlobalWrapper(precisionElectronSequenceCfg, flags, ion=ion, variant=variant, is_probe_leg=is_probe_leg)


def precisionElectronMenuSequence_LRT(flags, is_probe_leg=False):
    """Creates seventh step of electron sequence"""

    if isComponentAccumulatorCfg():
        return precisionElectronSequenceCfg_lrt(flags, is_probe_leg=is_probe_leg)
    else:
        return menuSequenceCAToGlobalWrapper(precisionElectronSequenceCfg_lrt, flags, is_probe_leg=is_probe_leg)


