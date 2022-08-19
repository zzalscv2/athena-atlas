# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

"""Define method to construct configured Tile Wiener raw channel builder tool"""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def TileRawChannelBuilderWienerCfg(flags, **kwargs):
    """Return component accumulator with configured private Tile Wiener raw channel builder tool

    Arguments:
        flags  -- Athena configuration flags (ConfigFlags)
    """

    name = kwargs.pop('name', 'TileRawChannelBuilderWiener')
    kwargs.setdefault('TileRawChannelContainer', 'TileRawChannelWiener')

    from LumiBlockComps.BunchCrossingCondAlgConfig import BunchCrossingCondAlgCfg
    acc = BunchCrossingCondAlgCfg(flags)

    if 'TileCondToolNoiseSample' not in kwargs:
        from TileConditions.TileSampleNoiseConfig import TileCondToolNoiseSampleCfg
        sampleNoiseTool = acc.popToolsAndMerge( TileCondToolNoiseSampleCfg(flags) )
        kwargs['TileCondToolNoiseSample'] = sampleNoiseTool

    kwargs.setdefault('correctTime', flags.Tile.correctTime)
    kwargs.setdefault('MC', flags.Input.isMC)
    kwargs.setdefault('BestPhase', False)
    kwargs.setdefault('MaxIterations', 5) # iterative mode on
    kwargs.setdefault('Minus1Iteration', True)
    kwargs.setdefault('PedestalMode', 1)
    kwargs.setdefault('AmplitudeCorrection', False) # don't need correction after iterations
    kwargs.setdefault('TimeCorrection', False)      # don't need correction after iterations

    if flags.Tile.correctTime and 'TileCondToolTiming' not in kwargs:
        from TileConditions.TileTimingConfig import TileCondToolTimingCfg
        timingTool = acc.popToolsAndMerge( TileCondToolTimingCfg(flags) )
        kwargs['TileCondToolTiming'] = timingTool

    TileRawChannelBuilderWiener=CompFactory.TileRawChannelBuilderWienerFilter
    from TileRecUtils.TileRawChannelBuilderConfig import TileRawChannelBuilderCfg
    rawChanBuilder = acc.popToolsAndMerge( TileRawChannelBuilderCfg(flags, name, TileRawChannelBuilderWiener, **kwargs) )
    acc.setPrivateTools(rawChanBuilder)

    return acc


if __name__ == "__main__":

    from AthenaConfiguration.AllConfigFlags import ConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import DEBUG

    # Test setup
    log.setLevel(DEBUG)

    ConfigFlags.Input.Files = defaultTestFiles.RAW
    ConfigFlags.Tile.RunType = 'PHY'
    ConfigFlags.Tile.NoiseFilter = 1
    ConfigFlags.lock()

    ConfigFlags.dump()

    acc = ComponentAccumulator()

    #printing Configurables isn't reliable with GaudiConfig2
    acc.popToolsAndMerge( TileRawChannelBuilderWienerCfg(ConfigFlags) )

    acc.printConfig(withDetails = True, summariseProps = True)
    acc.store( open('TileRawChannelBuilderWiener.pkl','wb') )
