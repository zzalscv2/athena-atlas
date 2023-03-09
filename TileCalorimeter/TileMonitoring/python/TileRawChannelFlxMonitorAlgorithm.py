#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

'''
@file TileDigitsFlxMonitorAlgorithm.py
@brief Python configuration of TileDigitsFlxMonitorAlgorithm algorithm for the Run III
'''

def TileRawChannelFlxMonitoringConfig(flags, fragIDs=[0x201, 0x402], **kwargs):
    '''Function to configures TileRawChannelFlxMonAlg algorithms in the monitoring system.'''

    kwargs.setdefault('TileRawChannelContainerLegacy', 'TileRawChannelFit')
    kwargs.setdefault('TileRawChannelContainerFlx', 'TileRawChannelFlxFit')
    kwargs.setdefault('FelixScale', 1)
    
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    result = ComponentAccumulator()

    from TileConditions.TileCablingSvcConfig import TileCablingSvcCfg
    result.merge( TileCablingSvcCfg(flags) )

    from AthenaMonitoring import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(flags, 'TileRawChannelFlxMonitoring')

    from AthenaConfiguration.ComponentFactory import CompFactory
    TileRawChannelFlxMonitorAlgorithm = CompFactory.TileRawChannelFlxMonitorAlgorithm
    tileRawChannelFlxMonAlg = helper.addAlgorithm(TileRawChannelFlxMonitorAlgorithm, 'TileRawChannelFlxMonAlg')
    tileRawChannelFlxMonAlg.TriggerChain = ''

    for k, v in kwargs.items():
        setattr(tileRawChannelFlxMonAlg, k, v)

    # Configure histogram with TileRawChannelFlxMonAlg algorithm execution time
    executeTimeGroup = helper.addGroup(tileRawChannelFlxMonAlg, 'TileRawChannelFlxMonExecuteTime', 'Tile')
    executeTimeGroup.defineHistogram('TIME_execute', path = 'RawChannel', type='TH1F',
                                     title = 'Time for execute TileRawChannelFlxMonAlg algorithm;time [#mus]',
                                     xbins = 300, xmin = 0, xmax = 300000)

    runNumber = flags.Input.RunNumber[0]
    from TileCalibBlobObjs.Classes import TileCalibUtils as Tile

    modules = []
    if fragIDs:
        for fragID in fragIDs:
            ros = fragID >> 8
            drawer = fragID & 0x3F
            modules += [Tile.getDrawerString(ros, drawer)]
    else:
        for ros in range(1, Tile.MAX_ROS):
            for drawer in range(0, Tile.MAX_DRAWER):
                modules += [Tile.getDrawerString(ros, drawer)]

    channelLegacyGroup = helper.addGroup(tileRawChannelFlxMonAlg, 'TileRawChannelLegacySummary', 'Tile/Legacy/RawChannel')
    for moduleName in modules:
        for gainName in ['HG', 'LG']:
            title = f'Run {str(runNumber)} {moduleName} {gainName}: Amplitude from channel ;Channel;Amplitude'
            name = f'{moduleName}_{gainName}_channel,{moduleName}_{gainName}_Summary_Legacy;{moduleName}_Summary_Legacy_{gainName}'
            path = moduleName
            channelLegacyGroup.defineHistogram(name, title = title, path = path, type = 'TProfile',
                                               xbins = 48, xmin = -0.5, xmax = 47.5)

    channelFelixGroup = helper.addGroup(tileRawChannelFlxMonAlg, 'TileRawChannelFlxSummary', 'Tile/Felix/RawChannel')
    for moduleName in modules:
        for gainName in ['HG', 'LG']:
            title = f'Run {str(runNumber)} {moduleName} {gainName}: Amplitude from channel-FELIX;Channel;Amplitude'
            name = f'{moduleName}_{gainName}_channel,{moduleName}_{gainName}_Summary_Felix;{moduleName}_Felix_{gainName}'
            path = moduleName
            channelFelixGroup.defineHistogram(name, title = title, path = path, type = 'TProfile',
                                              xbins = 48, xmin = -0.5, xmax = 47.5)

    channelCompareGroup = helper.addGroup(tileRawChannelFlxMonAlg, 'TileRawChannelDiffLegacyFlx', 'Tile/Compare/RawChannel')
    for moduleName in modules:
        for gainName in ['HG', 'LG']:
            title = f'Run {str(runNumber)} {moduleName} {gainName}: Amplitude difference ;Channel;Amplitude'
            name = f'{moduleName}_{gainName}_channel,{moduleName}_{gainName}_Diff;{moduleName}_Diff_{gainName}'
            path = moduleName
            channelCompareGroup.defineHistogram(name, title = title, path = path, type = 'TProfile',
                                                xbins = 48, xmin = -0.5, xmax = 47.5)
    
    channelCompareGroup = helper.addGroup(tileRawChannelFlxMonAlg, 'TileRawChannelDiffLegacyFlx_Legacy', 'Tile/Compare/RawChannel')
    for moduleName in modules:
        for gainName in ['HG', 'LG']:
            title = f'Run {str(runNumber)} {moduleName} {gainName}: Amplitude difference_Legacy ;Channel;Amplitude'
            name = f'{moduleName}_{gainName}_channel,{moduleName}_{gainName}_Diff;{moduleName}_Diff_{gainName}'
            path = moduleName
            channelCompareGroup.defineHistogram(name, title = title, path = path, type = 'TProfile',
                                                xbins = 48, xmin = -0.5, xmax = 47.5)

    accumalator = helper.result()
    result.merge(accumalator)
    return result



if __name__=='__main__':

    # Setup logs
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import INFO
    log.setLevel(INFO)

    # Set the Athena configuration flags
    from AthenaConfiguration.AllConfigFlags import ConfigFlags

    from AthenaConfiguration.TestDefaults import defaultTestFiles
    ConfigFlags.Input.Files = defaultTestFiles.RAW_RUN2
    ConfigFlags.Output.HISTFileName = 'TileRawChannelFlxMonitorOutput.root'
    ConfigFlags.DQ.useTrigger = False
    ConfigFlags.DQ.enableLumiAccess = False
    ConfigFlags.Exec.MaxEvents = 3
    ConfigFlags.fillFromArgs()
    ConfigFlags.lock()

    # Initialize configuration object, add accumulator, merge, and run.
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(ConfigFlags)

    from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
    tileTypeNames = ['TileRawChannelContainerLegacy/TileRawChannelFit',
                     'TileRawChannelContainerFlx/TileRawChannelFlxFit'
                     'TileBeamElemContainer/TileBeamElemCnt',
                     'TileDigitsContainer/TileDigitsCnt']
    cfg.merge( ByteStreamReadCfg(ConfigFlags, type_names = tileTypeNames) )
    cfg.getService('ByteStreamCnvSvc').ROD2ROBmap = [ "-1" ]

    runNumber = ConfigFlags.Input.RunNumber[0]
    from AthenaConfiguration.ComponentFactory import CompFactory
    cfg.addPublicTool( CompFactory.TileROD_Decoder(fullTileMode = runNumber) )

    cfg.merge( TileRawChannelFlxMonitoringConfig(ConfigFlags,
                                              TileRawChannelContainerLegacy='TileRawChannelFit',
                                              TileRawChannelContainerFlx='TileRawChannelFlxFit',
                                              fillHistogramsForDSP=True) )

    ConfigFlags.dump()

    cfg.store( open('TileRawChannelMonitorAlgorithm.pkl','wb') )

    sc = cfg.run()

    import sys
    # Success should be 0
    sys.exit(not sc.isSuccess())

