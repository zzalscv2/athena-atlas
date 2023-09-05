# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def MuonLaySurfaceToolCfg(flags, name = "MuonStationLayerSurfaceTool", **kwargs):
    result = ComponentAccumulator()
    the_tool = CompFactory.MuonGMR4.MuonStationLayerSurfaceTool(name, **kwargs)
    result.addPublicTool(the_tool, primary = True)
    return result

def ActsMuonChamberToolCfg(flags, name = "ActsMuonChamberTool", **kwargs):
    result = ComponentAccumulator()
    from MuonGeoModelR4.MuonGeoModelConfig import MuonGeoUtilityToolCfg
    kwargs.setdefault("GeoUtilTool", result.getPrimaryAndMerge(MuonGeoUtilityToolCfg(flags)))
    the_tool = CompFactory.MuonGMR4.ActsMuonChamberTool(name,**kwargs)
    result.addPublicTool(the_tool, primary = True)
    return result