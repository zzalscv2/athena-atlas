# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

def fromRunArgs(runArgs):

    # Setup logging
    from AthenaCommon.Logging import logging
    log = logging.getLogger('MergePool_Skeleton')
    log.info('****************** STARTING MergePool MERGING *****************')

    # Print arguments
    log.info('**** Transformation run arguments')
    log.info(str(runArgs))

    # Setup configuration flags
    log.info('**** Setting up configuration flags')

    from PyJobTransforms.CommonRunArgsToFlags import commonRunArgsToFlags
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    commonRunArgsToFlags(runArgs, flags)

    # This will be used to store the stream type that's being merged
    streamToMerge = None

    # First deal w/ the generic case
    if hasattr(runArgs, 'inputPOOL_MRG_INPUTFile'):
        # Cache the input file type from runArgs
        fileType = runArgs.inputPOOL_MRG_INPUTFileType
        if fileType == 'AOD':
            # See if we're dealing w/ a DAOD file instead of a primary AOD one
            # Extract stream from filename since it's not stored in the runArgs
            # We MUST find a better way of doing this...
            import re
            m = re.search(r'DAOD_[A-Z,0-9]+', runArgs.inputPOOL_MRG_INPUTFile[0])
            if m:
                fileType = m.group(0)
        elif fileType == 'ESD':
            # Here do something if we'd like to treat DESDs differently like DAODs
            # Currently DESDs are treated as primary ESDs
            pass

        # Now set the input file name appropriately
        setattr(runArgs, f'input{fileType}File', runArgs.inputPOOL_MRG_INPUTFile)

        # Now set the output file name appropriately
        if hasattr(runArgs, 'outputPOOL_MRG_OUTPUTFile'):
            setattr(runArgs, f'output{fileType}_MRGFile', runArgs.outputPOOL_MRG_OUTPUTFile)
        else:
            raise RuntimeError('Please provide an output POOL file (via --outputPOOL_MRG_OUTPUTFile)')

    # Deal w/ the AOD specific case
    if hasattr(runArgs, 'inputAODFile'):
        streamToMerge = 'AOD'
        flags.Input.Files = runArgs.inputAODFile
        if hasattr(runArgs, 'outputAOD_MRGFile'):
            flags.Output.AODFileName = runArgs.outputAOD_MRGFile
        else:
            raise RuntimeError('Please provide an output AOD file (via --outputAOD_MRGFile)')

    # Deal w/ the ESD specific case
    if hasattr(runArgs, 'inputESDFile'):
        streamToMerge = 'ESD'
        flags.Input.Files = runArgs.inputESDFile
        if hasattr(runArgs, 'outputESD_MRGFile'):
            flags.Output.ESDFileName = runArgs.outputESD_MRGFile
        else:
            raise RuntimeError('Please provide an output ESD file (via --outputESD_MRGFile)')

    # DAOD comes in many flavours, so automate transforming this into a 'standard' AOD argument
    DAOD_Input_Key = [ k for k in dir(runArgs) if k.startswith('inputDAOD') and k.endswith('File') ]
    if len(DAOD_Input_Key) == 1:
        streamToMerge = DAOD_Input_Key[0].removeprefix('input').removesuffix('File')
        flags.Input.Files = getattr(runArgs, DAOD_Input_Key[0])
        if hasattr(runArgs, f'output{streamToMerge}_MRGFile'):
            flags.addFlag(f'Output.{streamToMerge}FileName', getattr(runArgs, f'output{streamToMerge}_MRGFile'))
            flags.addFlag(f'Output.doWrite{streamToMerge}', True)
            flags.Output.doWriteDAOD = True
        else:
            raise RuntimeError(f'Please provide an output {streamToMerge} file (via --output{streamToMerge}_MRGFile)')

    # Double check we have something to merge
    if not streamToMerge:
        raise RuntimeError('Could not figure out what stream type is being merged [allowed: ESD, AOD, or any DAOD type]')

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

    # To respect --athenaopts 
    log.info('**** Processing athenaopts')
    flags.fillFromArgs()

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

    # Configure the output stream
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    cfg.merge(OutputStreamCfg(flags, streamToMerge))
    Stream = cfg.getEventAlgo(f'OutputStream{streamToMerge}')
    Stream.ForceRead = True
    Stream.TakeItemsFromInput = True
    # Add in-file MetaData
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    cfg.merge(
        SetupMetaDataForStreamCfg(
            flags,
            streamToMerge,
        )
    )

    log.info(f'**** Configured {streamToMerge} writing')

    # Configure extra bits that are needed for TP conversion
    for item in flags.Input.TypedCollections:
        ctype, cname = item.split('#')
        if ctype.startswith('Trk'):
            from TrkEventCnvTools.TrkEventCnvToolsConfigCA import TrkEventCnvSuperToolCfg
            cfg.merge(TrkEventCnvSuperToolCfg(flags))
        if ctype.startswith('Calo') or ctype.startswith('LAr'):
            from LArGeoAlgsNV.LArGMConfig import LArGMCfg
            cfg.merge(LArGMCfg(flags))
        if ctype.startswith('Calo') or ctype.startswith('Tile'):
            from TileGeoModel.TileGMConfig import TileGMCfg
            cfg.merge(TileGMCfg(flags))
        if ctype.startswith('Muon'):
            from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg
            cfg.merge(MuonGeoModelCfg(flags))

    # Add PerfMon
    if flags.PerfMon.doFastMonMT or flags.PerfMon.doFullMonMT:
        from PerfMonComps.PerfMonCompsConfig import PerfMonMTSvcCfg
        cfg.merge(PerfMonMTSvcCfg(flags))

    # Set EventPrintoutInterval to 100 events
    cfg.getService(cfg.getAppProps()['EventLoop']).EventPrintoutInterval = 100

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
