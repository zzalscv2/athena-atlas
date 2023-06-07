#
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
'''Functions setting default flags for generating online HLT python configuration'''

_flags = None

def setDefaultOnlineFlagsOldStyle():
    from AthenaCommon.AthenaCommonFlags import athenaCommonFlags as acf
    from AthenaCommon.GlobalFlags import globalflags as gf
    acf.isOnline.set_Value_and_Lock(True)
    acf.FilesInput.set_Value_and_Lock([])
    gf.InputFormat.set_Value_and_Lock('bytestream')
    gf.DataSource.set_Value_and_Lock('data')


def setDefaultOnlineFlagsNewStyle(flags):
    from AthenaConfiguration.Enums import Format
    flags.Common.isOnline = True
    flags.Input.Files = []
    flags.Input.isMC = False
    flags.Input.Format = Format.BS
    flags.Trigger.doHLT = True  # This distinguishes the HLT setup from online reco (GM, EventDisplay)
    flags.Trigger.Online.isPartition = True  # athenaHLT and partition at P1
    flags.Scheduler.ShowDataDeps = False
    flags.Scheduler.ShowControlFlow = False
    flags.Scheduler.ShowDataFlow = False
    flags.Scheduler.EnableVerboseViews = False
    flags.Scheduler.AutoLoadUnmetDependencies = False
    flags.Input.FailOnUnknownCollections = True


def defaultOnlineFlags():
    """On first call will create ConfigFlags and return instance. This is only to be used within
    TrigPSC/TrigServices/athenaHLT as we cannot explicitly pass flags everywhere."""
    global _flags
    if _flags is None:
        setDefaultOnlineFlagsOldStyle()
        from AthenaConfiguration.AllConfigFlags import ConfigFlags
        _flags = ConfigFlags
        setDefaultOnlineFlagsNewStyle(_flags)
    return _flags
