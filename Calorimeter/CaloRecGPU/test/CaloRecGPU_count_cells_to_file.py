# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#Outputs cell information (and cell assignment, thus also some cluster information)
#to a textual format, for both CPU and GPU growing and splitting,
#and also for the cross-check versions (CPU growing with GPU splitting
#and GPU growing with CPU splitting).

import CaloRecGPUTestingConfig
    
if __name__=="__main__":

    flags, perfmon, numevents = CaloRecGPUTestingConfig.PrepareTest()

    flags.CaloRecGPU.OutputCountsToFile = True
    flags.CaloRecGPU.ClustersOutputName="CaloCalTopoClustersNew"
    flags.lock()
    
    topoAcc = CaloRecGPUTestingConfig.MinimalSetup(flags,perfmon)
    topoAcc.merge(CaloRecGPUTestingConfig.FullTestConfiguration(flags, TestGrow = True, TestSplit = True, DoCrossTests = True))

    topoAcc.run(numevents)

