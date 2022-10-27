# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def ActsTrkLiveMonitoringHistSvcCfg(flags) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    histSvc = CompFactory.THistSvc(Output = ["EXPERT DATAFILE='acts-expert-monitoring.root', OPT='RECREATE'"])
    acc.addService(histSvc)
    return acc

def ActsTrkITkPixelClusterizationLiveMonitoringToolCfg(flags,
                                                       name: str = "ActsTrkItkPixelClusterizationLiveMonitoringTool",
                                                       **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    
    from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
    monTool = GenericMonitoringTool(name)
    
    monTool.defineHistogram('TIME_execute', path='EXPERT', type='TH1F', title='Time for execute',
                            xbins=100, xmin=0, xmax=10000)
    
    acc.setPrivateTools(monTool)
    acc.merge(ActsTrkLiveMonitoringHistSvcCfg(flags))
    return acc

def ActsTrkITkStripClusterizationLiveMonitoringToolCfg(flags,
                                                       name: str = "ActsTrkItkStripClusterizationLiveMonitoringTool",
                                                       **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    
    from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
    monTool = GenericMonitoringTool(name)
    
    monTool.defineHistogram('TIME_execute', path='EXPERT', type='TH1F', title='Time for execute',
                            xbins=100, xmin=0, xmax=10000)
    
    acc.setPrivateTools(monTool)
    acc.merge(ActsTrkLiveMonitoringHistSvcCfg(flags))
    return acc


def ActsTrkPixelSpacePointFormatioLiveMonitoringToolCfg(flags,
                                                        name: str = "ActsTrkPixelSpacePointFormatioLiveMonitoringTool",
                                                        **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    
    from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
    monTool = GenericMonitoringTool(name)
    
    monTool.defineHistogram('TIME_execute', path='EXPERT', type='TH1F', title='Time for execute',
                            xbins=100, xmin=0, xmax=1000)
    monTool.defineHistogram('numPixSpacePoints', path='EXPERT', type='TH1I', title='Number of Pixel Space Points',
                            xbins=100, xmin=0, xmax=1000000)    

    acc.setPrivateTools(monTool)
    acc.merge(ActsTrkLiveMonitoringHistSvcCfg(flags))
    return acc

def ActsTrkStripSpacePointFormatioLiveMonitoringToolCfg(flags,
                                                        name: str = "ActsTrkStripSpacePointFormatioLiveMonitoringTool",
                                                        **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    
    from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
    monTool = GenericMonitoringTool(name)
    
    monTool.defineHistogram('TIME_execute', path='EXPERT', type='TH1F', title='Time for execute',
                            xbins=100, xmin=0, xmax=1000)
    monTool.defineHistogram('numStripSpacePoints', path='EXPERT', type='TH1I', title='Number of Strip Space Points',
                            xbins=100, xmin=0, xmax=1000000)
    monTool.defineHistogram('numStripOverlapSpacePoints', path='EXPERT', type='TH1I', title='Number of Strip Overlap Space Points',
                            xbins=100, xmin=0, xmax=100000)
    
    acc.setPrivateTools(monTool)
    acc.merge(ActsTrkLiveMonitoringHistSvcCfg(flags))
    return acc

def ActsTrkITkPixelSeedingLiveMonitoringCfg(flags,
                                            name: str = "ActsTrkITkPixelSeedingLiveMonitoring",
                                            **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
    monTool = GenericMonitoringTool(name)
    
    monTool.defineHistogram('TIME_execute', path='EXPERT', type='TH1F', title='Time for execute',
                            xbins=100, xmin=0, xmax=50000)
    
    acc.setPrivateTools(monTool)
    acc.merge(ActsTrkLiveMonitoringHistSvcCfg(flags))     
    return acc
    
def ActsTrkITkStripSeedingLiveMonitoringCfg(flags,
                                            name: str = "ActsTrkITkStripSeedingLiveMonitoring",
                                            **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    
    from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
    monTool = GenericMonitoringTool(name)
    
    monTool.defineHistogram('TIME_execute', path='EXPERT', type='TH1F', title='Time for execute',
                            xbins=100, xmin=0, xmax=50000)
    
    acc.setPrivateTools(monTool)
    acc.merge(ActsTrkLiveMonitoringHistSvcCfg(flags))     
    return acc
    
