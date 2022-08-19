# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def McEventSelectorCfg(flags, **kwargs):
    cfg = ComponentAccumulator()

    service = CompFactory.McCnvSvc()
    cfg.addService(service)
    cfg.addService(CompFactory.EvtPersistencySvc("EventPersistencySvc",
                                                 CnvServices=[service.getFullJobOptName()]))

    runNumber = flags.Input.RunNumber
    if isinstance(runNumber, type([])) and runNumber:
        runNumber = runNumber[0]
    kwargs.setdefault("RunNumber", runNumber)
    timeStamp = flags.Input.TimeStamp
    if isinstance(timeStamp, type([])) and timeStamp:
        timeStamp = timeStamp[0]
    kwargs.setdefault("InitialTimeStamp", timeStamp)

    evSel = CompFactory.McEventSelector("EventSelector", **kwargs)
    cfg.addService(evSel)
    cfg.setAppProperty("EvtSel", evSel.getFullJobOptName())

    return cfg
