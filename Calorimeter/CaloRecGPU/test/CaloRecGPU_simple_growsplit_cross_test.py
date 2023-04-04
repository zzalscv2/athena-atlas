# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#Outputs plots for comparing
#CPU growing with GPU growing
#and all possible combinations for the splitter:
#CPU growing with CPU splitting,
#GPU growing with GPU splitting,
#CPU growing with GPU splitting
#and GPU growing with CPU splitting.

from CaloRecGPU.CaloRecGPUConfigurator import CaloRecGPUConfigurator
import CaloRecGPUTesting

    
if __name__=="__main__":

    Configurator = CaloRecGPUConfigurator()
    
    PlotterConfig = CaloRecGPUTesting.PlotterConfigurator(["CPU_growing", "GPU_growing", "CPUCPU_splitting", "GPUGPU_splitting", "CPUGPU_splitting", "GPUCPU_splitting"],
                                                          ["growing", "CPU_to_GPUGPU_splitting", "CPU_to_CPUGPU_splitting", "CPU_to_GPUCPU_splitting"])   
    Configurator.DoMonitoring = True
    
    cfg, numevents = CaloRecGPUTesting.PrepareTest(Configurator)

    theKey="CaloCalTopoClustersNew"
    
    topoAcc = CaloRecGPUTesting.FullTestConfiguration(Configurator, TestGrow = True, TestSplit = True, DoCrossTests = True, PlotterConfigurator = PlotterConfig)

    topoAlg = topoAcc.getPrimary()
    topoAlg.ClustersOutputName=theKey
    
    cfg.merge(topoAcc)
    
    cfg.run(numevents)

