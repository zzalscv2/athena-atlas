#  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from InDetConfig.SiClusterizationToolConfig import ITkClusterMakerToolCfg, ITkPixelRDOToolCfg

def ActsTrkITkPixelClusteringToolCfg(flags, name="ActsITkPixelClusteringTool", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("PixelRDOTool", acc.popToolsAndMerge(ITkPixelRDOToolCfg(flags)))
    kwargs.setdefault("ClusterMakerTool", acc.popToolsAndMerge(ITkClusterMakerToolCfg(flags)))
    kwargs.setdefault("AddCorners", True)
    kwargs.setdefault("ErrorStrategy", 1)
    acc.setPrivateTools(CompFactory.ActsTrk.PixelClusteringTool(name, **kwargs))
    return acc
                        
