#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

# menu components
from ..Config.MenuComponents import MenuSequenceCA, SelectionCA, InViewRecoCA, menuSequenceCAToGlobalWrapper
from AthenaConfiguration.AccumulatorCache import AccumulatorCache
from AthenaConfiguration.ComponentFactory import CompFactory, isComponentAccumulatorCfg

def tag(ion):
    return 'precision' + ('HI' if ion is True else '') + 'Tracking'


@AccumulatorCache
def precisionTrackingMenuSequenceCfg(flags, ion=False, variant='', is_probe_leg = False):
    """ fourth step:  precision electron....."""

    inViewRoIs = "precisionTracking" + variant

    from TriggerMenuMT.HLT.Electron.PrecisionTrackingRecoSequences import precisionTracking
    precisionTrackingReco = precisionTracking(flags, inViewRoIs, ion, variant)

    # preparing roiTool
    roiTool = CompFactory.ViewCreatorPreviousROITool()

    viewName = tag(ion)+variant
    precisionInDetReco = InViewRecoCA(viewName,
                                      RoITool=roiTool, # view maker args
                                      ViewFallThrough = True,
                                      RequireParentView=True,
                                      mergeUsingFeature=True,
                                      InViewRoIs=inViewRoIs,
                                      isProbe=is_probe_leg)

    precisionInDetReco.mergeReco(precisionTrackingReco)
    selAcc=SelectionCA(viewName, isProbe=is_probe_leg)
    selAcc.mergeReco(precisionInDetReco)


    precisionElectronHypoAlg = CompFactory.TrigStreamerHypoAlg("Electron"+tag(ion)+"Hypo"+variant)
    precisionElectronHypoAlg.FeatureIsROI = False
    selAcc.addHypoAlgo(precisionElectronHypoAlg)
    def acceptAllHypoToolGen(chainDict):
        return CompFactory.TrigStreamerHypoTool(chainDict["chainName"], Pass = True)
    return MenuSequenceCA(flags,selAcc,HypoToolGen=acceptAllHypoToolGen,isProbe=is_probe_leg)


def precisionTrackingMenuSequence(flags, name, is_probe_leg=False, ion=False, variant=''):
    """Creates fifth step of electron sequence"""
    if isComponentAccumulatorCfg():
        return precisionTrackingMenuSequenceCfg(flags, ion=ion, variant=variant, is_probe_leg=is_probe_leg)
    else: 
        return menuSequenceCAToGlobalWrapper(precisionTrackingMenuSequenceCfg, flags, ion=ion, variant=variant, is_probe_leg=is_probe_leg)


def precisionTrackingMenuSequence_LRT(flags, name, is_probe_leg=False):
    return precisionTrackingMenuSequence(flags, name, is_probe_leg=is_probe_leg, ion=False, variant='_LRT')
