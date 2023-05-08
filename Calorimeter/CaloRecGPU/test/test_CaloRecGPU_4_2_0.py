#!/usr/bin/env python
# art-description: GPU Topological (Topo-Automaton) Clustering test: 4 2 0 thresholds.
# art-type: local
# art-include: master/Athena
# art-architecture: '#&nvidia'
# art-output: expert-monitoring.root

# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from CaloRecGPU.CaloRecGPUConfigurator import CaloRecGPUConfigurator
import CaloRecGPUTesting


def do_test(files):
    Configurator = CaloRecGPUConfigurator()
    
    PlotterConfig = CaloRecGPUTesting.PlotterConfigurator(["CPU_growing", "GPU_growing", "CPU_splitting", "GPU_splitting"], ["growing", "splitting"])
    
    Configurator.DoMonitoring = True
    
    cfg, numevents = CaloRecGPUTesting.PrepareTest(Configurator, files, parse_command_arguments = False)

    Configurator.TopoClusterSeedCutsInAbsE = False
    Configurator.TopoClusterCellCutsInAbsE = False
    Configurator.TopoClusterNeighborCutsInAbsE = False
    Configurator.SplitterUseNegativeClusters = False
    
            
    Configurator.TopoClusterSNRSeedThreshold = 4.0
    Configurator.TopoClusterSNRGrowThreshold = 2.0
    Configurator.TopoClusterSNRCellThreshold = 0.0
    
    Configurator.ClusterSize = "Topo_420"
    
    theKey="CaloCalTopoClustersNew"
    
    topoAcc = CaloRecGPUTesting.FullTestConfiguration(Configurator, TestGrow = True, TestSplit = True, PlotterConfigurator = PlotterConfig)

    topoAlg = topoAcc.getPrimary()
    topoAlg.ClustersOutputName=theKey
    
    cfg.merge(topoAcc)
    
    cfg.run(numevents)
    

if __name__=="__main__":
    do_test(["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TrigInDetValidation/samples/mc15_13TeV.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.recon.RDO.e3698_s2608_s2183_r7195/RDO.06752780._000001.pool.root.1",
             "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TrigInDetValidation/samples/mc15_13TeV.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.recon.RDO.e3698_s2608_s2183_r7195/RDO.06752780._000002.pool.root.1",
             "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TrigInDetValidation/samples/mc15_13TeV.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.recon.RDO.e3698_s2608_s2183_r7195/RDO.06752780._000003.pool.root.1" ])
#    do_test(["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TrigEgammaValidation/valid3.147917.Pythia8_AU2CT10_jetjet_JZ7W.recon.RDO.e3099_s2578_r6596_tid05293007_00/RDO.05293007._000001.pool.root.1"],
#             "jets")

