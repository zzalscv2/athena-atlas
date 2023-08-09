# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#Outputs plots for comparing CPU and GPU splitting.

from CaloRecGPU.CaloRecGPUConfigurator import CaloRecGPUConfigurator
import CaloRecGPUTesting
    
if __name__=="__main__":

    Configurator = CaloRecGPUConfigurator()
    
    PlotterConfig = CaloRecGPUTesting.PlotterConfigurator(["CPU_splitting", "GPU_splitting"], ["splitting"])
    
    Configurator.DoMonitoring = True
    
    cfg, numevents = CaloRecGPUTesting.PrepareTest(Configurator)

    theKey="CaloCalTopoClustersNew"
    
    topoAcc = CaloRecGPUTesting.FullTestConfiguration(Configurator, TestSplit = True, PlotterConfigurator = PlotterConfig)

    topoAlg = topoAcc.getPrimary()
    topoAlg.ClustersOutputName=theKey
    
    cfg.merge(topoAcc)
    
    cfg.run(numevents)

