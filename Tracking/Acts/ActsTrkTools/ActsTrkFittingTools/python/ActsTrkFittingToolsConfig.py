#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

from ActsInterop import UnitConstants

from ActsGeometry.ActsGeometryConfig import (
    ActsExtrapolationToolCfg,
    ActsTrackingGeometryToolCfg,
)
from TrkConfig.TrkTrackSummaryToolConfig import InDetTrackSummaryToolCfg
from ActsTrkEventCnv.ActsTrkEventCnvConfig import ActsToTrkConverterToolCfg


def ActsKalmanFitterCfg(flags, name: str = "ActsKalmanFitter", **kwargs):
    result = ComponentAccumulator()

    # Make sure this is set correctly!
    #  /eos/project-a/acts/public/MaterialMaps/ATLAS/material-maps-Pixel-SCT.json

    if "TrackingGeometryTool" not in kwargs:
        kwargs["TrackingGeometryTool"] = result.popToolsAndMerge(ActsTrackingGeometryToolCfg(flags)) # PrivateToolHandle

    if "ExtrapolationTool" not in kwargs:
        kwargs["ExtrapolationTool"] = result.popToolsAndMerge(
            ActsExtrapolationToolCfg(flags, MaxSteps=10000)
        ) # PrivateToolHandle

    result.addPublicTool(result.popToolsAndMerge(ActsExtrapolationToolCfg(flags, MaxSteps=10000))) # FIXME redundant?

    kwargs.setdefault("ReverseFilteringPt", 1.0 * UnitConstants.GeV)
    kwargs.setdefault("OverstepLimit", 300 * UnitConstants.um)

    actsConverter = result.popToolsAndMerge(ActsToTrkConverterToolCfg(flags))
    kwargs["ATLASConverterTool"] = actsConverter

    if "SummaryTool" not in kwargs:
        summary = result.getPrimaryAndMerge(
            InDetTrackSummaryToolCfg(flags, 
                                     #  doHolesInDet=False
                                     )
        )
        kwargs["SummaryTool"] = summary

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

    result.setPrivateTools(CompFactory.ActsKalmanFitter(name, **kwargs))

    return result

