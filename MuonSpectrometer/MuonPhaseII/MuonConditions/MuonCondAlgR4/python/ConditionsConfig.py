
# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def ActsMuonAlignCondAlgCfg(flags, name="ActsMuonAlignCondAlg", **kwargs):
    result = ComponentAccumulator()
    ### The Acts Muon align cond alg only works with the new Detector manager
    if not flags.Muon.setupGeoModelXML: return result
    from MuonStationGeoHelpers.MuonStationGeoHelpersCfg import MuonLaySurfaceToolCfg
    kwargs.setdefault("LayerGeoTool", result.getPrimaryAndMerge(MuonLaySurfaceToolCfg(flags)))   
    the_alg = CompFactory.ActsMuonAlignCondAlg(name, **kwargs)
    result.addCondAlgo(the_alg)
    return result
def ActsGeomContextAlgCfg(flags, name="ActsGeomContextAlg", **kwargs):
    result = ComponentAccumulator()
    ### The Acts Muon align cond alg only works with the new Detector manager
    if not flags.Muon.setupGeoModelXML: return result   
    the_alg = CompFactory.ActsMuonGeomContextAlg(name, **kwargs)
    result.addCondAlgo(the_alg)
    return result

    
