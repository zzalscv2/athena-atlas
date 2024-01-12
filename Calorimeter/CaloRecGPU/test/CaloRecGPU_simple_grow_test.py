# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#Outputs plots for comparing CPU and GPU growing.

import CaloRecGPUTestingConfig
from PlotterConfigurator import PlotterConfigurator

if __name__=="__main__":

    
    PlotterConfig = PlotterConfigurator(["CPU_growing", "GPU_growing"], ["growing"])
    
    flags, perfmon, numevents = CaloRecGPUTestingConfig.PrepareTest()
    flags.CaloRecGPU.DoMonitoring = True
    flags.CaloRecGPU.ClustersOutputName="CaloCalTopoClustersNew"
    flags.lock()

    topoAcc = CaloRecGPUTestingConfig.MinimalSetup(flags,perfmon)

    topoAcc.merge(CaloRecGPUTestingConfig.FullTestConfiguration(flags, TestGrow = True, PlotterConfigurator = PlotterConfig))


    topoAcc.run(numevents)

