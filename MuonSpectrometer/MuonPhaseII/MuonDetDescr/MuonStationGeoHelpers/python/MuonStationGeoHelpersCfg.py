# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def MuonLaySurfaceToolCfg(flags, name = "MuonStationLayerSurfaceTool", **kwargs):
    result = ComponentAccumulator()
    the_tool = CompFactory.MuonGMR4.MuonStationLayerSurfaceTool(name, **kwargs)
    result.addPublicTool(the_tool, primary = True)
    return result