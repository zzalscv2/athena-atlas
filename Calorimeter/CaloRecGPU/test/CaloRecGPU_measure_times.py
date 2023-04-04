# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#Measures per-thread times for CPU and GPU implementations,
#without synchronization on the GPU side
#(thus only the total GPU time is accurate).

from CaloRecGPU.CaloRecGPUConfigurator import CaloRecGPUConfigurator
import CaloRecGPUTesting
    
if __name__=="__main__":

    Configurator = CaloRecGPUConfigurator()
    
    cfg, numevents = CaloRecGPUTesting.PrepareTest(Configurator)

    Configurator.MeasureTimes = True
    
    theKey="CaloCalTopoClustersNew"
    
    topoAcc = CaloRecGPUTesting.FullTestConfiguration(Configurator, TestGrow=True, TestSplit=True, SkipSyncs=True)

    topoAlg = topoAcc.getPrimary()
    topoAlg.ClustersOutputName=theKey
    
    cfg.merge(topoAcc)
    
    cfg.run(numevents)

