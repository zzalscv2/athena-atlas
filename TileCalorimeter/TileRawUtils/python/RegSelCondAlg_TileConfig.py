# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

"""Define methods to construct configured RegSelCondAlg_Tile conditions algorithm"""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def RegSelCondAlg_TileCfg(flags, **kwargs):
    """Return component accumulator with configured RegSelCondAlg_Tile conditions algorithm"""

    kwargs.setdefault('name', 'RegSelCondAlg_Tile')
    
    acc = ComponentAccumulator()

    from TileGeoModel.TileGMConfig import TileGMCfg
    acc.merge( TileGMCfg(flags) )

    from TileByteStream.TileHid2RESrcIDConfig import TileHid2RESrcIDCondAlgCfg
    acc.merge( TileHid2RESrcIDCondAlgCfg(flags, ForHLT=True) )
    
    RegSelCondAlg_Tile = CompFactory.RegSelCondAlg_Tile
    acc.addCondAlgo( RegSelCondAlg_Tile(**kwargs) )

    return acc



if __name__ == "__main__":

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultGeometryTags, defaultTestFiles
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import INFO
    
    # Test setup
    log.setLevel(INFO)

    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.RAW_RUN2
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN2
    flags.lock()

    # Initialize configuration object, add accumulator, merge, and run.
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)

    from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
    cfg.merge( ByteStreamReadCfg(flags) )

    cfg.merge( RegSelCondAlg_TileCfg(flags) )

    cfg.printConfig(withDetails = True, summariseProps = True)
    cfg.store( open('RegSelCondAlg_Tile.pkl','wb') )

    sc = cfg.run(3)

    import sys
    # Success should be 0
    sys.exit(not sc.isSuccess())
