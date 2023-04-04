# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#Outputs cell information (and cell assignment, thus also some cluster information)
#to a textual format, for both CPU and GPU growing and splitting,
#and also for the cross-check versions (CPU growing with GPU splitting
#and GPU growing with CPU splitting).

from CaloRecGPU.CaloRecGPUConfigurator import CaloRecGPUConfigurator
import CaloRecGPUTesting
    
if __name__=="__main__":

    Configurator = CaloRecGPUConfigurator()
        
    cfg, numevents = CaloRecGPUTesting.PrepareTest(Configurator)

    Configurator.OutputCountsToFile = True
    
    theKey="CaloCalTopoClustersNew"
    
    topoAcc = CaloRecGPUTesting.FullTestConfiguration(Configurator, TestGrow = True, TestSplit = True, DoCrossTests = True)

    topoAlg = topoAcc.getPrimary()
    topoAlg.ClustersOutputName=theKey
    
    cfg.merge(topoAcc)
    
    cfg.run(numevents)

