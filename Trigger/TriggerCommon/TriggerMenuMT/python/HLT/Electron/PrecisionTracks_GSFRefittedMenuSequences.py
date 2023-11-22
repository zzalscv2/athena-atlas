#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

# menu components
from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequenceCA, SelectionCA, InViewRecoCA, menuSequenceCAToGlobalWrapper
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentFactory import isComponentAccumulatorCfg

def tag(ion):
    return 'precision' + ('HI' if ion is True else '') + 'Tracking_GSFRefitted'

def precisionTracks_GSFRefittedSequenceCfg(flags, name, ion=False, variant='', is_probe_leg = False):
    """ sixth step:  GSF refitting of precision track....."""

    inViewRoIs = "precisionTracks_GSFRefitted"+variant

    roiTool = CompFactory.ViewCreatorPreviousROITool()
    reco = InViewRecoCA(tag(ion)+variant, 
                            RoITool = roiTool,
                            InViewRoIs = inViewRoIs, 
                            RequireParentView = True, 
                            mergeUsingFeature = True,
                            ViewFallThrough = True,
                            isProbe=is_probe_leg)

    # calling GSF refitter
    from TriggerMenuMT.HLT.Electron.PrecisionTracks_GSFRefittedSequence import precisionTracks_GSFRefitted
    precisionTracks_GSFRefittedInViewSequence = precisionTracks_GSFRefitted(flags, inViewRoIs, ion, variant)

    reco.mergeReco(precisionTracks_GSFRefittedInViewSequence)

    selAcc = SelectionCA(name + tag(ion) +variant, isProbe=is_probe_leg)
    selAcc.mergeReco(reco)

    thePrecisionTrack_GSFRefittedHypo = CompFactory.TrigStreamerHypoAlg(name + tag(ion) + "Hypo" + variant)
    thePrecisionTrack_GSFRefittedHypo.FeatureIsROI = False
    selAcc.addHypoAlgo(thePrecisionTrack_GSFRefittedHypo)
    def acceptAllHypoToolGen(chainDict):
        return CompFactory.TrigStreamerHypoTool(chainDict["chainName"], Pass = True)
    return MenuSequenceCA(flags,selAcc,HypoToolGen=acceptAllHypoToolGen,isProbe=is_probe_leg)

def precisionTracks_GSFRefittedMenuSequenceCfg_lrt(flags, name, is_probe_leg=False):
    return precisionTracks_GSFRefittedSequenceCfg(flags, name, is_probe_leg=is_probe_leg, ion=False, variant='_LRTGSF')

def precisionTracks_GSFRefittedMenuSequence(flags, name, ion=False, is_probe_leg=False):
    """Creates sixth step electron sequence"""

    if isComponentAccumulatorCfg():
        return precisionTracks_GSFRefittedSequenceCfg(flags, name=name, ion=ion, variant='_GSF', is_probe_leg = is_probe_leg)
    else:
        return menuSequenceCAToGlobalWrapper(precisionTracks_GSFRefittedSequenceCfg, flags, name=name, ion=ion, variant='_GSF', is_probe_leg = is_probe_leg)

def precisionTracks_GSFRefittedMenuSequence_LRT(flags, name, is_probe_leg=False):
    """Creates sixth step photon sequence"""

    if isComponentAccumulatorCfg():
        return precisionTracks_GSFRefittedMenuSequenceCfg_lrt(flags, name=name, is_probe_leg=is_probe_leg)
    else:
        return menuSequenceCAToGlobalWrapper(precisionTracks_GSFRefittedMenuSequenceCfg_lrt, flags, name=name, is_probe_leg=is_probe_leg)

