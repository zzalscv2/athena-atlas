# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def FullScanRoICreatorToolCfg(flags,
                              name: str = "OfflineFullScanRoICreatorTool",
                              **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    kwargs.setdefault('RoIs', 'OfflineFullScanRegion')
    acc.setPrivateTools(CompFactory.FullScanRoICreatorTool(name, **kwargs))
    return acc

def EventViewCreatorAlgCfg(flags,
                           name: str = "EventViewCreatorAlg",
                           **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    # TO-DO: according to tracking pass we'd need to schedule a different RoI creator tool
    # for now we only create a full scan roi
    if 'RoICreatorTool' not in kwargs:
        kwargs.setdefault('RoICreatorTool', acc.popToolsAndMerge(FullScanRoICreatorToolCfg(flags)))

    acc.addEventAlgo(CompFactory.EventViewCreatorAlg(name, **kwargs))
    return acc
