# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#Outputs plots and textual information
#to compare CPU with GPU moments calculation.

import CaloRecGPUTestingConfig
from PlotterConfigurator import PlotterConfigurator
    
if __name__=="__main__":

    PlotterConfig = PlotterConfigurator(["CPU_moments", "GPU_moments"], ["moments"], DoMoments = True)
    
    flags, perfmon, numevents = CaloRecGPUTestingConfig.PrepareTest()
    flags.CaloRecGPU.UseAbsEnergyMoments = True
    flags.CaloRecGPU.ClustersOutputName="CaloCalTopoClustersNew"
    flags.lock()

    topoAcc = CaloRecGPUTestingConfig.MinimalSetup(flags,perfmon)

    topoAcc.merge(CaloRecGPUTestingConfig.FullTestConfiguration(flags, TestMoments = True, PlotterConfigurator = PlotterConfig))

    topoAcc.run(numevents)
