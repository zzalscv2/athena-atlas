#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import BeamType
from InDetConfig.SiClusterizationToolConfig import ITkClusterMakerToolCfg, ITkPixelRDOToolCfg
from SCT_ConditionsTools.ITkStripConditionsToolsConfig import ITkStripConditionsSummaryToolCfg
from SiLorentzAngleTool.ITkStripLorentzAngleConfig import ITkStripLorentzAngleToolCfg


def ActsTrkITkPixelClusteringToolCfg(flags, name="ActsITkPixelClusteringTool", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("PixelRDOTool", acc.popToolsAndMerge(ITkPixelRDOToolCfg(flags)))
    kwargs.setdefault("ClusterMakerTool", acc.popToolsAndMerge(ITkClusterMakerToolCfg(flags)))
    kwargs.setdefault("AddCorners", True)
    kwargs.setdefault("ErrorStrategy", 1)
    acc.setPrivateTools(CompFactory.ActsTrk.PixelClusteringTool(name, **kwargs))
    return acc


def ActsTrkITkStripClusteringToolCfg(flags, name="ActsITkStripClusteringTool", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("StripConditionsTool", acc.popToolsAndMerge(ITkStripConditionsSummaryToolCfg(flags)))
    kwargs.setdefault("LorentzAngleTool", acc.popToolsAndMerge(ITkStripLorentzAngleToolCfg(flags)))

    if flags.ITk.selectStripIntimeHits:
        coll_25ns = flags.Beam.BunchSpacing<=25 and flags.Beam.Type is BeamType.Collisions
        kwargs.setdefault("timeBins", "01X" if coll_25ns else "X1X")

    acc.setPrivateTools(CompFactory.ActsTrk.StripClusteringTool(name, **kwargs))
    return acc

