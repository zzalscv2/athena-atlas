#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def ActsGaussianSumFitterCfg(flags, name: str = "ActsGaussianSumFitter", **kwargs):
    result = ComponentAccumulator()

    if "TrackingGeometryTool" not in kwargs:
        from ActsConfig.ActsGeometryConfig import ActsTrackingGeometryToolCfg
        kwargs["TrackingGeometryTool"] = result.popToolsAndMerge(ActsTrackingGeometryToolCfg(flags))

    if "ExtrapolationTool" not in kwargs:
        from ActsConfig.ActsGeometryConfig import ActsExtrapolationToolCfg
        kwargs["ExtrapolationTool"] = result.popToolsAndMerge(
            ActsExtrapolationToolCfg(flags, MaxSteps=10000)
        ) # PrivateToolHandle

    if 'ATLASConverterTool' not in kwargs:
        from ActsConfig.ActsEventCnvConfig import ActsToTrkConverterToolCfg
        kwargs["ATLASConverterTool"] = result.popToolsAndMerge(ActsToTrkConverterToolCfg(flags))

    kwargs["RefitOnly"] = True # Track summary will be added in the algorithm

    if flags.Detector.GeometryITk:
        from InDetConfig.InDetBoundaryCheckToolConfig import ITkBoundaryCheckToolCfg

        kwargs.setdefault(
            "BoundaryCheckTool", result.popToolsAndMerge(ITkBoundaryCheckToolCfg(flags))
        )
    else:
        from InDetConfig.InDetBoundaryCheckToolConfig import InDetBoundaryCheckToolCfg

        kwargs.setdefault(
            "BoundaryCheckTool",
            result.popToolsAndMerge(InDetBoundaryCheckToolCfg(flags)),
        )

    result.setPrivateTools(CompFactory.ActsTrk.GaussianSumFitter(name, **kwargs))

    return result


