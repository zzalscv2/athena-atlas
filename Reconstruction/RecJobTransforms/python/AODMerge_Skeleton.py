# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# temporarily force no global config flags
from AthenaConfiguration import AllConfigFlags
del AllConfigFlags.ConfigFlags


def fromRunArgs(runArgs):

    # Setup logging
    from AthenaCommon.Logging import logging
    log = logging.getLogger('AODMerge_tf')
    log.info( '****************** STARTING AOD MERGING *****************' )

    # Print arguments
    log.info('**** Transformation run arguments')
    log.info(str(runArgs))

    # Setup configuration flags
    log.info('**** Setting up configuration flags')

    from PyJobTransforms.CommonRunArgsToFlags import commonRunArgsToFlags
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    commonRunArgsToFlags(runArgs, flags)

    if hasattr(runArgs, 'inputAODFile'):
        flags.Input.Files = runArgs.inputAODFile
    else:
        raise RuntimeError('Please provide an input AOD file (via --inputAODFile)')

    if hasattr(runArgs, 'outputAOD_MRGFile'):
        flags.Output.AODFileName = runArgs.outputAOD_MRGFile
    else:
        raise RuntimeError('Please provide an output AOD file (via --outputAOD_MRGFile)')

    # Setup perfmon flags from runargs
    from PerfMonComps.PerfMonConfigHelpers import setPerfmonFlagsFromRunArgs
    setPerfmonFlagsFromRunArgs(flags, runArgs)

    # Pre-include
    from PyJobTransforms.TransformUtils import processPreExec, processPreInclude, processPostExec, processPostInclude
    log.info('**** Processing preInclude')
    processPreInclude(runArgs, flags)

    # Pre-exec
    log.info('**** Processing preExec')
    processPreExec(runArgs, flags)

    # Lock configuration flags
    log.info('**** Locking configuration flags')
    flags.lock()

    # Set up necessary job components
    log.info('**** Setting up job components')

    # Main services
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)

    # Input reading
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg.merge(PoolReadCfg(flags))

    # Output writing
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    cfg.merge(OutputStreamCfg(flags, 'AOD'))
    StreamAOD = cfg.getEventAlgo('OutputStreamAOD')
    StreamAOD.ForceRead = True
    StreamAOD.TakeItemsFromInput = True

    # Add in-file MetaData
    from xAODMetaDataCnv.InfileMetaDataConfig import InfileMetaDataCfg
    cfg.merge(InfileMetaDataCfg(flags, 'AOD'))

    # This part is needed for (un)packing cell containers
    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    cfg.merge(LArGMCfg(flags))
    from TileGeoModel.TileGMConfig import TileGMCfg
    cfg.merge(TileGMCfg(flags))

    # Add PerfMon
    if flags.PerfMon.doFastMonMT or flags.PerfMon.doFullMonMT:
        from PerfMonComps.PerfMonCompsConfig import PerfMonMTSvcCfg
        cfg.merge(PerfMonMTSvcCfg(flags))

    # Set EventPrintoutInterval to 100 events
    cfg.getService(cfg.getAppProps()["EventLoop"]).EventPrintoutInterval = 100

    # Post-include
    log.info('**** Processing postInclude')
    processPostInclude(runArgs, flags, cfg)

    # Post-exec
    log.info('**** Processing postExec')
    processPostExec(runArgs, flags, cfg)

    # Now run the job and exit accordingly
    sc = cfg.run()
    import sys
    sys.exit(not sc.isSuccess())
