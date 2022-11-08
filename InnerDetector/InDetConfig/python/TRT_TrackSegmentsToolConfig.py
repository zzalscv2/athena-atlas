# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
# Configuration of TRT_TrackSegmentsTool_xk package
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def TRT_TrackSegmentsMakerCondAlg_ATLxkCfg(flags, name = 'InDetTRT_SeedsMakerCondAlg', extension = '', **kwargs):
    from TRT_GeoModel.TRT_GeoModelConfig import TRT_ReadoutGeometryCfg
    acc = TRT_ReadoutGeometryCfg(flags) # To produce TRT_DetElementContainer

    if "PropagatorTool" not in kwargs:
        from TrkConfig.TrkExRungeKuttaPropagatorConfig import RungeKuttaPropagatorCfg
        kwargs.setdefault("PropagatorTool", acc.popToolsAndMerge(
            RungeKuttaPropagatorCfg(flags)))

    kwargs.setdefault("NumberMomentumChannel", flags.InDet.Tracking.ActivePass.TRTSegFinderPtBins)

    acc.addCondAlgo(CompFactory.InDet.TRT_TrackSegmentsMakerCondAlg_ATLxk(name, **kwargs))
    return acc

def TRT_TrackSegmentsMaker_BarrelCosmicsCfg(flags, name='InDetTRTSegmentsMaker', **kwargs) :
    acc = ComponentAccumulator()

    kwargs.setdefault("TrtManagerLocation", 'TRT')
    kwargs.setdefault("TRT_ClustersContainer", 'TRT_DriftCirclesUncalibrated')
    kwargs.setdefault("IsMagneticFieldOn", flags.BField.solenoidOn)

    acc.setPrivateTools(CompFactory.InDet.TRT_TrackSegmentsMaker_BarrelCosmics(name, **kwargs))
    return acc

def TRT_TrackSegmentsMaker_ATLxkCfg(flags, name = 'InDetTRT_SeedsMaker',
                                    extension = '',
                                    InputCollections = None,
                                    **kwargs):
    #
    # --- cut values
    #
    if extension == "_TRT":
        # TRT Subdetector segment finding
        MinNumberDCs   = flags.InDet.Tracking.ActivePass.minTRTonly
        pTmin          = flags.InDet.Tracking.ActivePass.minPT
        sharedFrac     = flags.InDet.Tracking.ActivePass.maxTRTonlyShared
    else:
        # TRT-only/back-tracking segment finding
        MinNumberDCs   = flags.InDet.Tracking.ActivePass.minSecondaryTRTonTrk
        pTmin          = flags.InDet.Tracking.ActivePass.minSecondaryPt
        sharedFrac     = flags.InDet.Tracking.ActivePass.maxSecondaryTRTShared


    acc = TRT_TrackSegmentsMakerCondAlg_ATLxkCfg(flags, name = 'InDetTRT_SeedsMakerCondAlg'+ extension,
                                                 pTmin = pTmin)

    if "PropagatorTool" not in kwargs:
        from TrkConfig.TrkExRungeKuttaPropagatorConfig import RungeKuttaPropagatorCfg
        kwargs.setdefault("PropagatorTool", acc.popToolsAndMerge(
            RungeKuttaPropagatorCfg(flags)))

    if "TrackExtensionTool" not in kwargs:
        from InDetConfig.TRT_TrackExtensionToolConfig import TRT_TrackExtensionToolCfg
        kwargs.setdefault("TrackExtensionTool", acc.popToolsAndMerge(
            TRT_TrackExtensionToolCfg(flags)))

    kwargs.setdefault("TRT_ClustersContainer", 'TRT_DriftCircles')
    kwargs.setdefault("PRDtoTrackMap", 'InDetSegmentPRDtoTrackMap'+extension if InputCollections is not None else '')
    kwargs.setdefault("RemoveNoiseDriftCircles", False)
    kwargs.setdefault("MinNumberDriftCircles", MinNumberDCs)
    kwargs.setdefault("NumberMomentumChannel", flags.InDet.Tracking.ActivePass.TRTSegFinderPtBins)
    kwargs.setdefault("pTmin", pTmin)
    kwargs.setdefault("sharedFrac", sharedFrac)

    acc.setPrivateTools(CompFactory.InDet.TRT_TrackSegmentsMaker_ATLxk(name, **kwargs))
    return acc
