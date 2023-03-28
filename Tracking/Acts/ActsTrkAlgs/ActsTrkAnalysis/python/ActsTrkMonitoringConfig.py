# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def ActsTrkMonitoringHistSvcCfg(flags) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    histSvc = CompFactory.THistSvc(Output = ["EXPERT DATAFILE='acts-expert-monitoring.root', OPT='RECREATE'"])
    acc.addService(histSvc)
    return acc

def ActsTrkITkPixelClusterizationMonitoringToolCfg(flags,
                                                   name: str = "ActsTrkItkPixelClusterizationMonitoringTool",
                                                   **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    
    from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
    monTool = GenericMonitoringTool(flags, name)
    
    monTool.defineHistogram('TIME_execute', path='EXPERT', type='TH1F', title='Time for execute',
                            xbins=100, xmin=0, xmax=10000)
    
    acc.setPrivateTools(monTool)
    acc.merge(ActsTrkMonitoringHistSvcCfg(flags))
    return acc

def ActsTrkITkStripClusterizationMonitoringToolCfg(flags,
                                                   name: str = "ActsTrkItkStripClusterizationMonitoringTool",
                                                   **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    
    from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
    monTool = GenericMonitoringTool(flags, name)
    
    monTool.defineHistogram('TIME_execute', path='EXPERT', type='TH1F', title='Time for execute',
                            xbins=100, xmin=0, xmax=10000)
    
    acc.setPrivateTools(monTool)
    acc.merge(ActsTrkMonitoringHistSvcCfg(flags))
    return acc


def ActsTrkPixelSpacePointFormationMonitoringToolCfg(flags,
                                                     name: str = "ActsTrkPixelSpacePointFormatioMonitoringTool",
                                                     **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    
    from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
    monTool = GenericMonitoringTool(flags, name)
    
    monTool.defineHistogram('TIME_execute', path='EXPERT', type='TH1F', title='Time for execute',
                            xbins=100, xmin=0, xmax=1000)
    monTool.defineHistogram('numPixSpacePoints', path='EXPERT', type='TH1I', title='Number of Pixel Space Points',
                            xbins=100, xmin=0, xmax=1000000)    

    acc.setPrivateTools(monTool)
    acc.merge(ActsTrkMonitoringHistSvcCfg(flags))
    return acc

def ActsTrkStripSpacePointFormationMonitoringToolCfg(flags,
                                                     name: str = "ActsTrkStripSpacePointFormationMonitoringTool",
                                                     **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    
    from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
    monTool = GenericMonitoringTool(flags, name)
    
    monTool.defineHistogram('TIME_execute', path='EXPERT', type='TH1F', title='Time for execute',
                            xbins=100, xmin=0, xmax=1000)
    monTool.defineHistogram('numStripSpacePoints', path='EXPERT', type='TH1I', title='Number of Strip Space Points',
                            xbins=100, xmin=0, xmax=1000000)
    monTool.defineHistogram('numStripOverlapSpacePoints', path='EXPERT', type='TH1I', title='Number of Strip Overlap Space Points',
                            xbins=100, xmin=0, xmax=100000)
    
    acc.setPrivateTools(monTool)
    acc.merge(ActsTrkMonitoringHistSvcCfg(flags))
    return acc

def ActsTrkITkPixelSeedingMonitoringCfg(flags,
                                        name: str = "ActsTrkITkPixelSeedingMonitoring",
                                        **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
    monTool = GenericMonitoringTool(flags, name)
    
    monTool.defineHistogram('TIME_execute', path='EXPERT', type='TH1F', title='Time for execute',
                            xbins=50, xmin=0, xmax=10000)
    monTool.defineHistogram('TIME_seedCreation', path='EXPERT', type='TH1F', title='Time for seed creation',
                            xbins=50, xmin=0, xmax=10000)
    monTool.defineHistogram('TIME_parameterEstimation', path='EXPERT', type='TH1F', title='Time for parameter estimation',
                            xbins=50, xmin=0, xmax=10000)
    
    acc.setPrivateTools(monTool)
    acc.merge(ActsTrkMonitoringHistSvcCfg(flags))     
    return acc
    
def ActsTrkITkStripSeedingMonitoringCfg(flags,
                                        name: str = "ActsTrkITkStripSeedingMonitoring",
                                        **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    
    from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
    monTool = GenericMonitoringTool(flags, name)
    
    monTool.defineHistogram('TIME_execute', path='EXPERT', type='TH1F', title='Time for execute',
                            xbins=50, xmin=0, xmax=10000)
    monTool.defineHistogram('TIME_seedCreation', path='EXPERT', type='TH1F', title='Time for seed creation',
                            xbins=50, xmin=0, xmax=10000)
    monTool.defineHistogram('TIME_parameterEstimation', path='EXPERT', type='TH1F', title='Time for parameter estimation',
                            xbins=50, xmin=0, xmax=10000)
    
    acc.setPrivateTools(monTool)
    acc.merge(ActsTrkMonitoringHistSvcCfg(flags))     
    return acc

def ActsTrkFindingMonitoringCfg(flags,
                                name: str = "ActsTrkFindingLiveMonitoring",
                                **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    
    from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
    monTool = GenericMonitoringTool(flags, name)
    
    monTool.defineHistogram('TIME_execute', path='EXPERT', type='TH1F', title="Time for execute",
                            xbins=100, xmin=0, xmax=70000)
    
    acc.setPrivateTools(monTool)
    acc.merge(ActsTrkMonitoringHistSvcCfg(flags))
    return acc
