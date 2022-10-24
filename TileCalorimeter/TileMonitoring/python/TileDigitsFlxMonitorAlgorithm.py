#
#  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#

'''
@file TileDigitsFlxMonitorAlgorithm.py
@brief Python configuration of TileDigitsFlxMonitorAlgorithm algorithm for the Run III
'''

def TileDigitsFlxMonitoringConfig(flags, fragIDs=[0x201, 0x402], **kwargs):
    '''Function to configures TileDigitsFlxMonAlg algorithms in the monitoring system.'''

    kwargs.setdefault('TileDigitsContainer', 'TileDigitsCnt')
    kwargs.setdefault('FirstSample', 0)
    kwargs.setdefault('LastSample', 7)
    kwargs.setdefault('FragIDs', fragIDs)

    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    result = ComponentAccumulator()

    from TileConditions.TileCablingSvcConfig import TileCablingSvcCfg
    result.merge( TileCablingSvcCfg(flags) )

    from AthenaMonitoring import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(flags, 'TileDigitsFlxMonitoring')

    from AthenaConfiguration.ComponentFactory import CompFactory
    TileDigitsFlxMonitorAlgorithm = CompFactory.TileDigitsFlxMonitorAlgorithm
    tileDigitsFlxMonAlg = helper.addAlgorithm(TileDigitsFlxMonitorAlgorithm, 'TileDigitsFlxMonAlg')
    tileDigitsFlxMonAlg.TriggerChain = ''

    for k, v in kwargs.items():
        setattr(tileDigitsFlxMonAlg, k, v)

    # Configure histogram with TileDigitsFlxMonAlg algorithm execution time
    executeTimeGroup = helper.addGroup(tileDigitsFlxMonAlg, 'TileDigitsFlxMonExecuteTime', 'Tile/Felix')
    executeTimeGroup.defineHistogram('TIME_execute', path = 'Digits', type='TH1F',
                                     title = 'Time for execute TileDigitsFlxMonAlg algorithm;time [#mus]',
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

    channelHFNGroup = helper.addGroup(tileDigitsFlxMonAlg, 'TileFlxMonHFN', 'Tile/Felix/Digits')
    for moduleName in modules:
        for gainName in ['HG', 'LG']:
            title = f'Run {str(runNumber)} {moduleName} {gainName}: Mean RMS in event (HFN);Channel;HFN'
            name = f'{moduleName}_{gainName}_channel,{moduleName}_{gainName}_HFN;{moduleName}_HFN_{gainName}'
            path = moduleName
            channelHFNGroup.defineHistogram(name, title = title, path = path, type = 'TProfile',
                                            xbins = 48, xmin = -0.5, xmax = 47.5)

    channelPedGroup = helper.addGroup(tileDigitsFlxMonAlg, 'TileFlxMonPed', 'Tile/Felix/Digits')
    for moduleName in modules:
        for gainName in ['HG', 'LG']:
            title = f'Run {str(runNumber)} {moduleName} {gainName}: Pedestal, sample[0];Channel;ADC'
            name = f'{moduleName}_{gainName}_channel,{moduleName}_{gainName}_Pedestal;{moduleName}_Pedestal_{gainName}'
            path = moduleName
            channelPedGroup.defineHistogram(name, title = title, path = path, type = 'TProfile',
                                                xbins = 48, xmin = -0.5, xmax = 47.5)

    channelSamplesGroup = helper.addGroup(tileDigitsFlxMonAlg, 'TileFlxMonSamples', 'Tile/Felix/Digits')
    for moduleName in modules:
        for channel in range(0, Tile.MAX_CHAN):
            for gainName in ['HG', 'LG']:
                title = (f'Run {str(runNumber)} {moduleName} channel {str(channel)} {gainName}: Samples;Sample [ADC];N')
                name = f'{moduleName}_ch_{str(channel)}_{gainName}_samples'
                path = moduleName
                channelSamplesGroup.defineHistogram(name, title = title, path = path, type = 'TH1F',
                                                    xbins = 4096, xmin = -0.5, xmax = 4095.5)


    accumalator = helper.result()
    result.merge(accumalator)
    return result


if __name__=='__main__':
    # Setup the Run III behavior
    from AthenaCommon.Configurable import Configurable
    Configurable.configurableRun3Behavior = 1

    # Setup logs
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import INFO
    log.setLevel(INFO)

    # Set the Athena configuration flags
    from AthenaConfiguration.AllConfigFlags import ConfigFlags
    parser = ConfigFlags.getArgumentParser()
    parser.add_argument('--postExec', help='Code to execute after setup')
    parser.add_argument('--digits', default="TileDigitsCnt", help='Tile digits container to be monitored')
    parser.add_argument('--frag-ids', dest='fragIDs', nargs="*", default=['0x201','0x402'], help='Tile Frag IDs of modules to be monitored. Empty=ALL')
    args, _ = parser.parse_known_args()

    fragIDs = [int(fragID, base=16) for fragID in args.fragIDs]

    from AthenaConfiguration.TestDefaults import defaultTestFiles
    ConfigFlags.Input.Files = defaultTestFiles.RAW
    ConfigFlags.Output.HISTFileName = 'TileDigitsFlxMonitorOutput.root'
    ConfigFlags.DQ.useTrigger = False
    ConfigFlags.DQ.enableLumiAccess = False
    ConfigFlags.Exec.MaxEvents = 3
    ConfigFlags.fillFromArgs(parser=parser)
    ConfigFlags.lock()

    # Initialize configuration object, add accumulator, merge, and run.
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(ConfigFlags)

    from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
    tileTypeNames = [f'TileDigitsContainer/{args.digits}']
    cfg.merge( ByteStreamReadCfg(ConfigFlags, type_names = tileTypeNames) )

    tileDigitsFlxMonitorAccumulator  = TileDigitsFlxMonitoringConfig(ConfigFlags,
                                                                     fragIDs = fragIDs,
                                                                     TileDigitsContainer=args.digits)

    cfg.merge(tileDigitsFlxMonitorAccumulator)

    # Any last things to do?
    if args.postExec:
        log.info('Executing postExec: %s', args.postExec)
        exec(args.postExec)

    cfg.printConfig(withDetails = True, summariseProps = True)
    ConfigFlags.dump()

    cfg.store( open('TileDigitsFlxMonitorAlgorithm.pkl','wb') )

    sc = cfg.run()

    import sys
    # Success should be 0
    sys.exit(not sc.isSuccess())
