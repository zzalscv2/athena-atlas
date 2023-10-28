# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def MuonDetectorBuilderToolCfg(flags, name="MuonDetectorBuilderTool", **kwargs):
    result = ComponentAccumulator()
    from MuonStationGeoHelpers.MuonStationGeoHelpersCfg import ActsMuonChamberToolCfg
    kwargs.setdefault("ChamberBuilder", result.getPrimaryAndMerge(ActsMuonChamberToolCfg(flags)))
    theTool = CompFactory.ActsTrk.MuonDetectorBuilderTool(name, **kwargs)
    result.addPublicTool(theTool, primary = True)
    return result