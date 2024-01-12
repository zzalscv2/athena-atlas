# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#Outputs plots for comparing
#CPU growing with GPU growing
#and CPU growing + splitting with GPU growing + splitting

import CaloRecGPUTestingConfig
from PlotterConfigurator import PlotterConfigurator
    
if __name__=="__main__":

    PlotterConfig = PlotterConfigurator(["CPU_growing", "GPU_growing", "CPU_splitting", "GPU_splitting"], ["growing", "splitting"])

    flags, perfmon, numevents = CaloRecGPUTestingConfig.PrepareTest()
    flags.CaloRecGPU.DoMonitoring = True
    flags.CaloRecGPU.ClustersOutputName="CaloCalTopoClustersNew"
    flags.lock()

    topoAcc = CaloRecGPUTestingConfig.MinimalSetup(flags,perfmon)

    topoAcc.merge(CaloRecGPUTestingConfig.FullTestConfiguration(flags, TestGrow = True, TestSplit = True, PlotterConfigurator = PlotterConfig))

    topoAcc.run(numevents)

