#Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def CsvMdtDriftCircleDumpCfg(flags, name="CsvDriftCircleDumper", **kwargs):
    result = ComponentAccumulator()
    the_alg = CompFactory.CsvMdtDriftCircleDumperMuonCnv(name=name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result

def CsvMuonSimHitDumpCfg(flags, name="CsvMuonSimHitDumper", **kwargs):
    result = ComponentAccumulator()
    from MuonStationGeoHelpers.MuonStationGeoHelpersCfg import MuonLaySurfaceToolCfg
    kwargs.setdefault("LayerGeoTool", result.getPrimaryAndMerge(MuonLaySurfaceToolCfg(flags)))
   

    the_alg = CompFactory.CsvMuonSimHitDumperMuonCnv(name = name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result
