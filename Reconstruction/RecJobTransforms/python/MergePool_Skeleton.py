# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# temporarily force no global config flags
from AthenaConfiguration import AllConfigFlags
del AllConfigFlags.ConfigFlags
import re

def fromRunArgs(runArgs):

    # Setup logging
    from AthenaCommon.Logging import logging
    log = logging.getLogger('MergePool_Skeleton')
    log.info( '****************** STARTING MergePool MERGING *****************' )

    # Print arguments
    log.info('**** Transformation run arguments')
    log.info(str(runArgs))

    # Setup configuration flags
    log.info('**** Setting up configuration flags')

    from PyJobTransforms.CommonRunArgsToFlags import commonRunArgsToFlags
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    commonRunArgsToFlags(runArgs, flags)

    DAODStream = ''
    if hasattr(runArgs, "inputPOOL_MRG_INPUTFile"):
        flags.Input.Files = runArgs.inputPOOL_MRG_INPUTFile
        if runArgs.inputPOOL_MRG_INPUTFileType == "AOD":
            # Extract DAODStream from filename since it's not stored in the runArgs
            m = re.search(r'DAOD_[A-Z,0-9]+', runArgs.outputPOOL_MRG_OUTPUTFile)
            if m:
                 DAODStream = m.group(0)
                 outputFileName = runArgs.outputPOOL_MRG_OUTPUTFile
                 flagString = f'Output.{DAODStream}FileName'
                 flags.addFlag(flagString, outputFileName)
                 flags.addFlag(f'Output.doWrite{DAODStream}', True)
                 flags.Output.doWriteDAOD = True
                 flags.Output.doWriteAOD = False
                 log.info(f'**** Found DAODStream {DAODStream}')
                 log.info(flags.dump())
            else:
                 flags.Output.AODFileName = runArgs.outputPOOL_MRG_OUTPUTFile

        elif runArgs.inputPOOL_MRG_INPUTFileType == "ESD":
            flags.Output.ESDFileName = runArgs.outputPOOL_MRG_OUTPUTFile
    else:
        raise RuntimeError('Please provide an inputPOOL_MRG_INPUTFile')

    # DAOD comes in many flavours, so automate transforming this into a "standard" AOD argument
    # This is not used in the POOLMergeAthena step
    DAOD_Input_Key = [ k for k in dir(runArgs) if k.startswith("inputDAOD") and k.endswith("File") ]
    if len(DAOD_Input_Key) == 1:
        flags.Input.Files = getattr(runArgs, DAOD_Input_Key[0])

    DAOD_Output_Key = [ k for k in dir(runArgs) if k.startswith("outputDAOD") and k.endswith("_MRGFile") ]
    if len(DAOD_Output_Key) == 1:
        flags.Output.AODFileName = getattr(runArgs, DAOD_Output_Key[0])

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

    if flags.Output.doWriteAOD:
        # Configure AOD output
        from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
        cfg.merge(OutputStreamCfg(flags, 'AOD'))
        StreamAOD = cfg.getEventAlgo('OutputStreamAOD')
        StreamAOD.ForceRead = True
        StreamAOD.TakeItemsFromInput = True
        # Add in-file MetaData
        from xAODMetaDataCnv.InfileMetaDataConfig import InfileMetaDataCfg
        cfg.merge(InfileMetaDataCfg(flags, 'AOD'))
        log.info("**** Configured AOD writing")

    if flags.Output.doWriteDAOD:
        # Configure DAOD output
        from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
        cfg.merge(OutputStreamCfg(flags, f'{DAODStream}'))
        StreamDAOD = cfg.getEventAlgo(f'OutputStream{DAODStream}')
        StreamDAOD.ForceRead = True
        StreamDAOD.TakeItemsFromInput = True
        # Add in-file MetaData
        from xAODMetaDataCnv.InfileMetaDataConfig import InfileMetaDataCfg
        cfg.merge(InfileMetaDataCfg(flags, f'{DAODStream}'))
        log.info("**** Configured DAOD writing")

    if flags.Output.doWriteESD:
        # Configure ESD output
        from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
        cfg.merge(OutputStreamCfg(flags, 'ESD'))
        StreamESD = cfg.getEventAlgo('OutputStreamESD')
        StreamESD.ForceRead = True
        StreamESD.TakeItemsFromInput = True

        # Needed for Trk::Tracks TPCnv
        from TrkEventCnvTools.TrkEventCnvToolsConfigCA import (TrkEventCnvSuperToolCfg)
        cfg.merge(TrkEventCnvSuperToolCfg(flags))
        # Needed for MetaData
        from xAODMetaDataCnv.InfileMetaDataConfig import InfileMetaDataCfg
        cfg.merge(InfileMetaDataCfg(flags, "ESD"))
        log.info("ESD ItemList: %s", cfg.getEventAlgo("OutputStreamESD").ItemList)
        log.info("ESD MetadataItemList: %s", cfg.getEventAlgo("OutputStreamESD").MetadataItemList)
        log.info("**** Configured ESD writing")

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
