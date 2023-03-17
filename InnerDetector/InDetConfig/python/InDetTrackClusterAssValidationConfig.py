# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
# Configuration of InDetTrackClusterAssValidation package
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import BeamType

def InDetTrackClusterAssValidationCfg(flags, name='InDetTrackClusterAssValidation', **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault("SpacePointsPixelName", "PixelSpacePoints")
    kwargs.setdefault("SpacePointsSCTName", "SCT_SpacePoints")
    kwargs.setdefault("SpacePointsOverlapName", "OverlapSpacePoints")
    kwargs.setdefault("RadiusMin", 0.)
    kwargs.setdefault("MinNumberClustersTRT", 0)
    kwargs.setdefault("usePixel", flags.Detector.EnablePixel)
    kwargs.setdefault("useSCT", flags.Detector.EnableSCT)
    kwargs.setdefault("useTRT", flags.Detector.EnableTRT)

    if flags.Beam.Type in [BeamType.Cosmics, BeamType.SingleBeam]:
        kwargs.setdefault("MomentumCut",
                          flags.InDet.Tracking.ActiveConfig.minPT \
                          if flags.Beam.Type==BeamType.Cosmics else 0)
        kwargs.setdefault("RadiusMax",   9999999.)
        kwargs.setdefault("RapidityCut", 9999999.)
        kwargs.setdefault("MinNumberClusters", 8)
        kwargs.setdefault("MinNumberSpacePoints", 4)

    else:
        kwargs.setdefault("MomentumCut",
                          2 * flags.InDet.Tracking.ActiveConfig.minPT)
        kwargs.setdefault("RadiusMax", 20.)
        kwargs.setdefault("RapidityCut",
                          flags.InDet.Tracking.ActiveConfig.maxEta)
        kwargs.setdefault("MinNumberClusters",
                          flags.InDet.Tracking.ActiveConfig.minClusters)
        kwargs.setdefault("MinNumberSpacePoints", 3)

    acc.addEventAlgo(CompFactory.InDet.TrackClusterAssValidation(name, **kwargs))
    return acc

def ITkTrackClusterAssValidationCfg(flags, name='ITkTrackClusterAssValidation', **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault("usePixel", flags.Detector.EnableITkPixel)
    kwargs.setdefault("useStrip", flags.Detector.EnableITkStrip)
    kwargs.setdefault("MomentumCut",
                      max(flags.ITk.Tracking.ActiveConfig.minPT))
    kwargs.setdefault("RapidityCut",
                      flags.ITk.Tracking.ActiveConfig.maxEta)
    kwargs.setdefault("EtaBins", flags.ITk.Tracking.ActiveConfig.etaBins)
    kwargs.setdefault("PtCuts",
                      (len(flags.ITk.Tracking.ActiveConfig.etaBins)-1)*[max(flags.ITk.Tracking.ActiveConfig.minPT)])
    kwargs.setdefault("MinNumberClustersCuts",
                      flags.ITk.Tracking.ActiveConfig.minClusters)

    acc.addEventAlgo(CompFactory.ITk.TrackClusterAssValidation(name, **kwargs))
    return acc
