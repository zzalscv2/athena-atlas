# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import ProductionStep
from AtlasGeoModel.GeoModelConfig import GeoModelCfg

def TileGMCfg(flags):
    result=GeoModelCfg(flags)

    result.getPrimary().DetectorTools += [ CompFactory.TileDetectorTool() ]
    if flags.Common.ProductionStep not in [ProductionStep.Simulation, ProductionStep.FastChain]:
        result.getPrimary().DetectorTools["TileDetectorTool"].GeometryConfig = "RECO"

    return result


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles

    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.RAW
    flags.lock()

    acc = TileGMCfg( flags )
    acc.store( open( "test.pkl", "wb" ) )
    print("All OK")
