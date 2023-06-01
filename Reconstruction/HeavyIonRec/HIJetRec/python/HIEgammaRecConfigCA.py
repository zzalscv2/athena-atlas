#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#


from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def HIEgammaRecCfg(flags):
    acc=ComponentAccumulator()

    from HIJetRec.HIJetRecConfigCA import HIModulatorCfg
    modulator=acc.popToolsAndMerge(HIModulatorCfg(flags,
                                                  # caveat: mod_key has to be the same as in HIJetRecConfigCA
                                                  mod_key="HIEventShapeWeighted_iter1_Modulate", 
                                                  suffix="iter1"))
    
    # get subtracted cells 
    from HIJetRec.SubtractedCellGetterCA import SubtractedCellGetterCfgCA
    acc.merge(SubtractedCellGetterCfgCA(flags, modulator))

    # make subtracted topo clusters
    from CaloRec.CaloTopoClusterConfig import CaloTopoClusterCfg
    acc.merge(CaloTopoClusterCfg(flags,
                                 cellsname=flags.HeavyIon.Egamma.SubtractedCells,
                                 clustersname=flags.HeavyIon.Egamma.CaloTopoCluster,
                                 clustersnapname="SubtractedCaloTopoClusters"))

    return acc
