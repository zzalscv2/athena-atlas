# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#Outputs plots and textual information
#to compare CPU with GPU moments calculation.

from CaloRecGPU.CaloRecGPUConfigurator import CaloRecGPUConfigurator
import CaloRecGPUTesting

    
if __name__=="__main__":

    Configurator = CaloRecGPUConfigurator()
        
    PlotterConfig = CaloRecGPUTesting.PlotterConfigurator(["CPU_growing", "GPU_growing", "CPU_splitting", "GPU_splitting", "CPU_moments", "GPU_moments"], ["growing", "splitting", "moments"])
    
    Configurator.DoMonitoring = True
    
    cfg, numevents = CaloRecGPUTesting.PrepareTest(Configurator)

    theKey="CaloCalTopoClustersNew"
    
    topoAcc = CaloRecGPUTesting.FullTestConfiguration(Configurator, TestMoments = True, PlotterConfigurator = PlotterConfig)

    topoAlg = topoAcc.getPrimary()
    topoAlg.ClustersOutputName=theKey
    
    cfg.merge(topoAcc)
    
    cfg.run(numevents)

