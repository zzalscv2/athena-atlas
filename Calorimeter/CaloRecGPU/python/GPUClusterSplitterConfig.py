# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from CaloRecGPU.CaloRecGPUConfigurator import CaloRecGPUConfigurator
    
if __name__=="__main__":

    Configurator = CaloRecGPUConfigurator()
    
    Configurator.TestGPUGrowing = False
    Configurator.TestGPUSplitting = True
    
    cfg, numevents = Configurator.PrepareTest(["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/RecExRecoTest/mc20e_13TeV/valid1.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.ESD.e4993_s3227_r12689/myESD.pool.root"])

    theKey="CaloCalTopoClustersNew"
    
    topoAcc = Configurator.HybridClusterProcessorConf()

    topoAlg = topoAcc.getPrimary()
    topoAlg.ClustersOutputName=theKey
    
    cfg.merge(topoAcc)
    
    cfg.run(numevents)

