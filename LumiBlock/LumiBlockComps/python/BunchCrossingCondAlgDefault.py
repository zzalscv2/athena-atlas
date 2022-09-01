# Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.AlgSequence import AthSequencer
from AthenaCommon.Logging import logging


def BunchCrossingCondAlgDefault():
    name = 'BunchCrossingCondAlgDefault'
    mlog = logging.getLogger(name)

    condSeq = AthSequencer ('AthCondSeq')

    if hasattr (condSeq, name):
        return getattr (condSeq, name)

    
    from LumiBlockComps.LumiBlockCompsConf import BunchCrossingCondAlg
    from IOVDbSvc.CondDB import conddb
    from AthenaCommon.BeamFlags import beamFlags

    run1=(conddb.dbdata == 'COMP200')
    folder = None
    bgkey = ''

    if conddb.isMC and beamFlags.bunchStructureSource() != 1:
        mlog.info('This is MC, trying to reset beamFlags.bunchStructureSource to 1')
        beamFlags.bunchStructureSource = 1

    if beamFlags.bunchStructureSource() == 1:
        folder = "/Digitization/Parameters"
        from AthenaCommon.DetFlags import DetFlags
        if not DetFlags.digitize.any_on():
            if not conddb.folderRequested(folder):
                mlog.info("Adding folder %s", folder)
                conddb.addFolderWithTag('', folder, 'HEAD',
                                        className = 'AthenaAttributeList')
            else:
                mlog.info("Folder %s already requested", folder)
        else:
            # Here we are in a digitization job, so the
            # /Digitization/Parameters metadata is not present in the
            # input file and will be created during the job
            mlog.info("Folder %s will be created during the job.", folder)
    elif beamFlags.bunchStructureSource() == 2:
        bgkey = 'L1BunchGroup'  # unless we use in file metadata...
        from AthenaCommon.AppMgr import ServiceMgr as svcMgr
        if getattr(svcMgr, 'xAODConfigSvc') and svcMgr.xAODConfigSvc.UseInFileMetadata:
            from AthenaConfiguration.AllConfigFlags import ConfigFlags
            if 'TriggerMenuJson_BG' not in ConfigFlags.Input.MetadataItems:
                # this is for when we need to configure the BunchGroupCondAlg with info extracted from converted JSON
                # in this case avoid using the xAODConfigSvc, because it will be set up incorrectly
                configFlags_with_DB = ConfigFlags.clone()
                configFlags_with_DB.Trigger.triggerConfig = 'FILE'
                from AthenaCommon.Configurable import ConfigurableRun3Behavior
                with ConfigurableRun3Behavior():
                    from AthenaConfiguration.AllConfigFlags import ConfigFlags
                    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator, appendCAtoAthena
                    from TrigConfigSvc.TrigConfigSvcCfg import BunchGroupCondAlgCfg
                    acc = ComponentAccumulator()
                    acc.merge(BunchGroupCondAlgCfg(configFlags_with_DB))
                appendCAtoAthena(acc)
            else:
                bgkey = ''
        else:
            from AthenaCommon.Configurable import ConfigurableRun3Behavior
            with ConfigurableRun3Behavior():
                from AthenaConfiguration.AllConfigFlags import ConfigFlags
                from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator, appendCAtoAthena
                from TrigConfigSvc.TrigConfigSvcCfg import BunchGroupCondAlgCfg
                acc = ComponentAccumulator()
                acc.merge(BunchGroupCondAlgCfg(ConfigFlags))
            appendCAtoAthena(acc)
        # this probably fails for reading R21 ESD but not going to support that case
    elif beamFlags.bunchStructureSource() == 0:
        folder = '/TDAQ/OLC/LHC/FILLPARAMS'
        # Mistakenly created as multi-version folder, must specify HEAD 
        conddb.addFolderWithTag('TDAQ', folder, 'HEAD',
                                className = 'AthenaAttributeList')
    # other possibilities for BunchStructureSource: assume we are running in a context
    # where dependencies are set up
        
    alg = BunchCrossingCondAlg(name,
                               Run1=run1,
                               FillParamsFolderKey =folder,
                               Mode=beamFlags.bunchStructureSource(),
                               L1BunchGroupCondData=bgkey )

    condSeq += alg

    return alg
