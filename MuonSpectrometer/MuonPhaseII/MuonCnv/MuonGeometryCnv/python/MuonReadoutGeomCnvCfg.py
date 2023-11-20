
#Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def MuonReadoutGeometryCnvAlgCfg(flags,name="MuonDetectorManagerCondAlg", **kwargs):
    result = ComponentAccumulator()
    from MuonCondAlgR4.ConditionsConfig import ActsGeomContextAlgCfg
    result.merge(ActsGeomContextAlgCfg(flags))
    from MuonStationGeoHelpers.MuonStationGeoHelpersCfg import MuonLaySurfaceToolCfg
    kwargs.setdefault("LayerGeoTool", result.getPrimaryAndMerge(MuonLaySurfaceToolCfg(flags)))
    the_alg = CompFactory.MuonReadoutGeomCnvAlg(name=name, **kwargs)
    result.addCondAlgo(the_alg, primary = True)
    return result
