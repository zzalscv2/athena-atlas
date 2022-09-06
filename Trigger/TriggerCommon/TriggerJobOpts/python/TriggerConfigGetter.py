# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import CAtoGlobalWrapper
from RecExConfig.RecFlags  import rec

from AthenaCommon.GlobalFlags import globalflags
from AthenaCommon.Logging import logging

_log = logging.getLogger( "TriggerConfigGetter.py" )

def TriggerConfigGetter(flags=None):
    if flags is None:
        from AthenaConfiguration.AllConfigFlags import ConfigFlags
        flags = ConfigFlags

    _log.info("The following flags are set:")
    _log.info("globalflags.InputFormat             : %s", globalflags.InputFormat())
    _log.info("globalflags.DataSource              : %s", globalflags.DataSource())
    _log.info("rec.read.*                          : RDO: %s, ESD: %s, AOD: %s, TAG: %s", rec.readRDO(), rec.readESD(), rec.readAOD(), rec.readTAG())
    _log.info("rec.doWrite.*                       : ESD: %s, AOD: %s, TAG: %s", rec.doWriteESD(), rec.doWriteAOD(), rec.doWriteTAG())

    from AthenaCommon.AppMgr import ServiceMgr as svcMgr

    _log.info('Creating the Trigger Configuration Services')
    _log.info("flags.Trigger.EDMVersion: %i", flags.Trigger.EDMVersion)
    if flags.Trigger.EDMVersion >= 3:
        if flags.Trigger.InputContainsConfigMetadata:
            if not hasattr(svcMgr, 'xAODConfigSvc'):
                from TrigConfxAOD.TrigConfxAODConf import TrigConf__xAODConfigSvc
                svcMgr += TrigConf__xAODConfigSvc('xAODConfigSvc')
        else: # Does not have xAODMeta
            # Run-3 Trigger Configuration Services (just producing menu data)
            from TrigConfigSvc.TrigConfigSvcCfg import TrigConfigSvcCfg
            CAtoGlobalWrapper(TrigConfigSvcCfg,flags)

    if not flags.Trigger.InputContainsConfigMetadata:
        setupxAODWriting(flags)
    else:
        _log.info("Input file already has xAOD trigger metadata. Will not re-create it.")

    # all went fine we are configured
    return True


def setupxAODWriting(flags):
    """
    Method setting up the writing of the ROOT-readable trigger configuration
    metadata.
    """

    _log.info( "ESD/AOD writing enabled, will set up xAOD trigger "
                "configuration metadata writing" )

    # Get the algorithm sequence:
    from AthenaCommon.AlgSequence import AlgSequence
    topAlgs = AlgSequence()

    # Add the algorithm creating the trigger configuration metadata for
    # the output:
    if flags.Trigger.EDMVersion == 1 or flags.Trigger.EDMVersion == 2:
        from TrigConfigSvc.TrigConfigSvcCfg import TrigConfigSvcCfg
        CAtoGlobalWrapper(TrigConfigSvcCfg, flags)

        # xAODConfigSvc for accessing the Run-3 converted menu
        from TrigConfxAOD.TrigConfxAODConf import TrigConf__xAODConfigSvc
        from AthenaCommon.AppMgr import ServiceMgr as svcMgr
        svcMgr += TrigConf__xAODConfigSvc('xAODConfigSvc',
                                            UseInFileMetadata = False)

    elif flags.Trigger.EDMVersion < 1:
        raise RuntimeError("Invalid EDMVersion=%s " % flags.Trigger.EDMVersion)


    from TrigConfxAOD.TrigConfxAODConf import TrigConf__xAODMenuWriterMT, TrigConf__KeyWriterTool
    topAlgs += TrigConf__xAODMenuWriterMT(KeyWriterTool=TrigConf__KeyWriterTool('KeyWriterToolOffline'))

    # Schedule also the prescale conditions algs
    from AthenaCommon.Configurable import ConfigurableRun3Behavior
    with ConfigurableRun3Behavior():
        from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator, appendCAtoAthena
        from TrigConfigSvc.TrigConfigSvcCfg import L1PrescaleCondAlgCfg, HLTPrescaleCondAlgCfg, BunchGroupCondAlgCfg
        acc = ComponentAccumulator()
        acc.merge( L1PrescaleCondAlgCfg( flags ) )
        acc.merge( BunchGroupCondAlgCfg( flags ) )
        acc.merge( HLTPrescaleCondAlgCfg( flags ) )
    appendCAtoAthena( acc )
