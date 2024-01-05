#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
'''
@file TileDigiNoiseMonitorAlgorithm.py
@brief Python configuration of TileDigiNoiseMonitorAlgorithm algorithm for the Run III
'''
def TileDigiNoiseMonitoringConfig(flags, **kwargs):

    ''' Function to configure TileDigiNoiseMonitorAlgorithm algorithm in the monitoring system.'''

    # Define one top-level monitoring algorithm. The new configuration
    # framework uses a component accumulator.
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    result = ComponentAccumulator()

    from TileRecUtils.TileDQstatusConfig import TileDQstatusAlgCfg
    result.merge( TileDQstatusAlgCfg(flags) )

    from TileGeoModel.TileGMConfig import TileGMCfg
    result.merge(TileGMCfg(flags))

    from TileConditions.TileCablingSvcConfig import TileCablingSvcCfg
    result.merge( TileCablingSvcCfg(flags) )

    from TileConditions.TileBadChannelsConfig import TileBadChannelsCondAlgCfg
    result.merge( TileBadChannelsCondAlgCfg(flags, **kwargs) )

    kwargs.setdefault('fillPedestalDifference', True)
    if kwargs['fillPedestalDifference'] and 'TileCondToolNoiseSample' not in kwargs:
        from TileConditions.TileSampleNoiseConfig import TileCondToolNoiseSampleCfg
        sampleNoiseTool = result.popToolsAndMerge( TileCondToolNoiseSampleCfg(flags) )
        kwargs['TileCondToolNoiseSample'] = sampleNoiseTool

    kwargs.setdefault('CheckDCS', flags.Tile.useDCS)
    if kwargs['CheckDCS']:
        from TileConditions.TileDCSConfig import TileDCSCondAlgCfg
        result.merge( TileDCSCondAlgCfg(flags) )

    #kwargs.setdefault('TriggerChain', 'HLT_noalg_cosmiccalo_L1RD1_EMPTY') #FIXME
    kwargs.setdefault('TriggerTypes', [0x82])

    # The following class will make a sequence, configure algorithms, and link
    # them to GenericMonitoringTools
    from AthenaMonitoring import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(flags,'TileDigiNoiseMonitoring')

    from AthenaConfiguration.Enums import Format
    if flags.Input.Format is Format.POOL:
        kwargs.setdefault('TileDigitsContainer', 'TileDigitsFlt')

    # Adding an TileCellMonitorAlgorithm algorithm to the helper
    from AthenaConfiguration.ComponentFactory import CompFactory
    tileDigiNoiseMonAlg = helper.addAlgorithm(CompFactory.TileDigiNoiseMonitorAlgorithm, 'TileDigiNoiseMonAlg')

    for k, v in kwargs.items():
        setattr(tileDigiNoiseMonAlg, k, v)

    run = str(flags.Input.RunNumbers[0])

    # 1) Configure histogram with TileDigiNoiseMonAlg algorithm execution time
    executeTimeGroup = helper.addGroup(tileDigiNoiseMonAlg, 'TileDigiNoiseMonExecuteTime', 'Tile/')
    executeTimeGroup.defineHistogram('TIME_execute', path = 'DigiNoise', type='TH1F',
                                     title = 'Time for execute TileDigiNoiseMonAlg algorithm;time [#mus]',
                                     xbins = 100, xmin = 0, xmax = 100000)


    from TileMonitoring.TileMonitoringCfgHelper import addTileModuleChannelMapsArray

    # 1) Configure histograms with status of Tile channel pedestals per partition and gain
    addTileModuleChannelMapsArray(helper, tileDigiNoiseMonAlg, name = 'TileDigiNoisePed',
                                  title = 'Pedestal', path = 'Tile/DigiNoise', type = 'TProfile2D',
                                  value = 'pedestal', run = run, perGain = True, separator = '_')

    # 2) Configure histograms with status of Tile channel high frequency noise per partition and gain
    addTileModuleChannelMapsArray(helper, tileDigiNoiseMonAlg, name = 'TileDigiNoiseHFN',
                                  title = 'High frequency noise', path = 'Tile/DigiNoise', type = 'TProfile2D',
                                  value = 'HFN', run = run, perGain = True, separator = '_')


    accumalator = helper.result()
    result.merge(accumalator)
    return result

if __name__=='__main__':

    # Setup logs
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import INFO
    log.setLevel(INFO)

    # Set the Athena configuration flags
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultGeometryTags, defaultTestFiles
    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.RAW_RUN2
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN2
    flags.Output.HISTFileName = 'TileDigiNoiseMonitorOutput.root'
    flags.DQ.useTrigger = False
    flags.DQ.enableLumiAccess = False
    flags.Exec.MaxEvents = 3
    flags.fillFromArgs()
    flags.lock()

    # Initialize configuration object, add accumulator, merge, and run.
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)

    from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
    tileTypeNames = ['TileRawChannelContainer/TileRawChannelCnt', 'TileDigitsContainer/TileDigitsCnt']
    cfg.merge( ByteStreamReadCfg(flags, type_names = tileTypeNames) )

    cfg.merge( TileDigiNoiseMonitoringConfig(flags, TriggerChain = '') )

    cfg.printConfig(withDetails = True, summariseProps = True)
    flags.dump()

    cfg.store( open('TileDigiNoiseMonitorAlgorithm.pkl','wb') )

    sc = cfg.run()

    import sys
    # Success should be 0
    sys.exit(not sc.isSuccess())
