#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#


from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def HIEgammaRecCfg(flags):
    acc=ComponentAccumulator()
    
    # get HI jets setup
    from HIJetRec.HIJetRecConfigCA import HIJetRecCfg
    acc_hijets=HIJetRecCfg(flags)

    # get modulator from HI jets
    jahi_stct = acc_hijets.getEventAlgo("jetalgHI_subtoclustertool")
    modulator=jahi_stct.Tools[0].Modulator

    # merge HI jets CA to ensure their reconstruction
    acc.merge(acc_hijets)


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
