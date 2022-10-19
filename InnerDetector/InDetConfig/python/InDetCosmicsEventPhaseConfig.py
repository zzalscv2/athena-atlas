# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
# Configuration of InDetCosmicsEventPhase package
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def InDetCosmicsEventPhaseToolCfg(flags, name='InDetCosmicsEventPhaseTool', **kwargs) :
    from TRT_ConditionsAlgs.TRT_ConditionsAlgsConfig import TRTPhaseCondCfg
    acc = TRTPhaseCondCfg(flags) # To produce input TRTCond::AverageT0 conditions

    kwargs.setdefault("UseNewEP", True)
    kwargs.setdefault("GlobalOffset", -3.125 if flags.Input.isMC else 8)

    if "TRTCalDbTool" not in kwargs:
        from TRT_ConditionsServices.TRT_ConditionsServicesConfig import TRT_CalDbToolCfg
        kwargs.setdefault("TRTCalDbTool", acc.popToolsAndMerge(
            TRT_CalDbToolCfg(flags)))

    acc.setPrivateTools(CompFactory.InDet.InDetCosmicsEventPhaseTool(name, **kwargs))
    return acc

def InDetFixedWindowTrackTimeToolCfg(flags, name='InDetFixedWindowTrackTimeTool', **kwargs) :
    from TRT_ConditionsAlgs.TRT_ConditionsAlgsConfig import TRTPhaseCondCfg
    acc = TRTPhaseCondCfg(flags) # To produce input TRTCond::AverageT0 conditions

    kwargs.setdefault("UseNewEP"     , True)
    kwargs.setdefault("WindowCenter" , -8.5)
    kwargs.setdefault("WindowSize"   , 7)
    kwargs.setdefault("GlobalOffset", -3.125 if flags.Input.isMC else 8)

    if "TRTCalDbTool" not in kwargs:
        from TRT_ConditionsServices.TRT_ConditionsServicesConfig import TRT_CalDbToolCfg
        kwargs.setdefault("TRTCalDbTool", acc.popToolsAndMerge(
            TRT_CalDbToolCfg(flags)))

    acc.setPrivateTools(CompFactory.InDet.InDetFixedWindowTrackTimeTool(name, **kwargs))
    return acc

def InDetSlidingWindowTrackTimeToolCfg(flags, name='InDetSlidingWindowTrackTimeTool', **kwargs) :
    from TRT_ConditionsAlgs.TRT_ConditionsAlgsConfig import TRTPhaseCondCfg
    acc = TRTPhaseCondCfg(flags) # To produce input TRTCond::AverageT0 conditions

    kwargs.setdefault("UseNewEP"         , True)
    kwargs.setdefault("NumberIterations" , 5)
    kwargs.setdefault("WindowSize"       , 7)
    kwargs.setdefault("GlobalOffset", -3.125 if flags.Input.isMC else 8)

    if "TRTCalDbTool" not in kwargs:
        from TRT_ConditionsServices.TRT_ConditionsServicesConfig import TRT_CalDbToolCfg
        kwargs.setdefault("TRTCalDbTool", acc.popToolsAndMerge(
            TRT_CalDbToolCfg(flags)))

    acc.setPrivateTools(CompFactory.InDet.InDetSlidingWindowTrackTimeTool(name, **kwargs))
    return acc

def InDetCosmicsEventPhaseCfg(flags, name='InDetCosmicsEventPhase', **kwargs):
    acc = ComponentAccumulator()

    if "TRTCalDbTool" not in kwargs:
        from TRT_ConditionsServices.TRT_ConditionsServicesConfig import TRT_CalDbToolCfg
        kwargs.setdefault("TRTCalDbTool", acc.popToolsAndMerge(
            TRT_CalDbToolCfg(flags)))

    if "TrackSummaryTool" not in kwargs:
        from TrkConfig.TrkTrackSummaryToolConfig import InDetTrackSummaryToolCfg
        InDetTrackSummaryTool = acc.popToolsAndMerge(InDetTrackSummaryToolCfg(flags))
        acc.addPublicTool(InDetTrackSummaryTool)
        kwargs.setdefault("TrackSummaryTool" , InDetTrackSummaryTool)

    if "EventPhaseTool" not in kwargs:
        EventPhaseTool = acc.popToolsAndMerge(InDetCosmicsEventPhaseToolCfg(flags))
        acc.addPublicTool(EventPhaseTool)
        kwargs.setdefault("EventPhaseTool", EventPhaseTool)

    acc.addEventAlgo(CompFactory.InDet.InDetCosmicsEventPhase(name, **kwargs))
    return acc


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import ConfigFlags

    numThreads=1
    ConfigFlags.Concurrency.NumThreads=numThreads
    ConfigFlags.Concurrency.NumConcurrentEvents=numThreads # Might change this later, but good enough for the moment.

    from AthenaConfiguration.TestDefaults import defaultTestFiles
    ConfigFlags.Input.Files = defaultTestFiles.RDO_RUN2
    ConfigFlags.lock()
    ConfigFlags.dump()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    top_acc = MainServicesCfg(ConfigFlags)
    
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    top_acc.merge(PoolReadCfg(ConfigFlags))

    from TRT_GeoModel.TRT_GeoModelConfig import TRT_ReadoutGeometryCfg
    top_acc.merge(TRT_ReadoutGeometryCfg( ConfigFlags ))

    top_acc.merge(InDetCosmicsEventPhaseCfg(ConfigFlags,
                                            InputTracksNames=['TRTTracks_Phase', 'ExtendedTracksPhase']))

    iovsvc = top_acc.getService('IOVDbSvc')
    iovsvc.OutputLevel=5
    top_acc.printConfig()
    top_acc.run(25)
    top_acc.store(open("test_InDetCosmicsEventPhaseConfig.pkl", "wb"))  ##comment out to store top_acc into pkl file
