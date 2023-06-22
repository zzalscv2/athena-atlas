# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def getAlg422(cfg):
    for comp in cfg.getSequence().Members:
        if comp.name == 'CaloTopoClusters422Maker':
            return comp
    return None
    
def fixMakerTool422(alg):
    if not hasattr(alg, 'ClusterMakerTools'):
        return False
    
    for tool in alg.ClusterMakerTools:
        if tool.name == 'TopoMaker':
            assert tool.CellThresholdOnEorAbsEinSigma == 0
            tool.CellThresholdOnEorAbsEinSigma = 2.0
            return True

    return False

def fixSnapshotTool422(alg):
    if not hasattr(alg, 'ClusterCorrectionTools'):
        return False

    for tool in alg.ClusterCorrectionTools:
        
        if tool.name == 'CaloClusterSnapshot':
            assert tool.OutputName == 'CaloTopoClusters'
            tool.OutputName = 'CaloTopoClusters422Snap'
            return True

    return False


def Add422Cfg(flags):

    cfg = ComponentAccumulator()
    
    from CaloRec.CaloTopoClusterConfig import CaloTopoClusterCfg 
    
    calo_acc422 = CaloTopoClusterCfg(flags,clustersname='CaloTopoClusters422')
    
    alg = getAlg422(calo_acc422)
    assert fixMakerTool422(alg)
    assert fixSnapshotTool422(alg)
    
    cfg.merge(calo_acc422)

    return cfg
