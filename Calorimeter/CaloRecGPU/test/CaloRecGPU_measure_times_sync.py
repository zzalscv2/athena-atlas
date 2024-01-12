# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#Measures per-thread times for CPU and GPU implementations,
#with synchronization on the GPU side
#(which means that kernel launch overheads aren't hidden,
# but the fraction of time spent in the various steps
# of the GPU implementation can be measured).

import CaloRecGPUTestingConfig
    
if __name__=="__main__":

    flags, perfmon, numevents = CaloRecGPUTestingConfig.PrepareTest()
    flags.CaloRecGPU.MeasureTimes = True
    flags.CaloRecGPU.ClustersOutputName="CaloCalTopoClustersNew"
    flags.lock()

    topoAcc = CaloRecGPUTestingConfig.MinimalSetup(flags,perfmon)

    topoAcc.merge(CaloRecGPUTestingConfig.FullTestConfiguration(flags, TestGrow=True, TestSplit=True, SkipSyncs = False))

    topoAcc.run(numevents)

