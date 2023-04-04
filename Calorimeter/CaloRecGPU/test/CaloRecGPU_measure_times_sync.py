# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#Measures per-thread times for CPU and GPU implementations,
#with synchronization on the GPU side
#(which means that kernel launch overheads aren't hidden,
# but the fraction of time spent in the various steps
# of the GPU implementation can be measured).

from CaloRecGPU.CaloRecGPUConfigurator import CaloRecGPUConfigurator
import CaloRecGPUTesting
    
if __name__=="__main__":

    Configurator = CaloRecGPUConfigurator()
    
    cfg, numevents = CaloRecGPUTesting.PrepareTest(Configurator)

    Configurator.MeasureTimes = True
    
    theKey="CaloCalTopoClustersNew"
    
    topoAcc = CaloRecGPUTesting.FullTestConfiguration(Configurator, TestGrow=True, TestSplit=True, SkipSyncs = False)

    topoAlg = topoAcc.getPrimary()
    topoAlg.ClustersOutputName=theKey
    
    cfg.merge(topoAcc)
    
    cfg.run(numevents)

