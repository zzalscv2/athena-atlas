# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#Measures per-thread times for CPU and GPU implementations,
#without synchronization on the GPU side
#(thus only the total GPU time is accurate).

import CaloRecGPUTestingConfig
    
if __name__=="__main__":

    flags, perfmon, numevents = CaloRecGPUTestingConfig.PrepareTest()
    flags.CaloRecGPU.MeasureTimes = True
    flags.CaloRecGPU.ClustersOutputName="CaloCalTopoClustersNew"
    flags.lock()

    topoAcc = CaloRecGPUTestingConfig.MinimalSetup(flags,perfmon)

    topoAcc.merge(CaloRecGPUTestingConfig.FullTestConfiguration(flags,TestGrow=True, TestSplit=True, SkipSyncs=True))

    topoAcc.run(numevents)

    
