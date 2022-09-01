# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

#********************************************************************
# METCommonConfig.py
# Configures DF MET content building and association tools
# Component accumulator version  
#********************************************************************



def METCommonCfg(ConfigFlags):
    """Configure MET for the derivation framework"""

    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    from METReconstruction.METAssociatorCfg import METAssociatorCfg

    acc = ComponentAccumulator()

    metDefs = ['AntiKt4EMTopo']
    if ConfigFlags.MET.DoPFlow:
        metDefs.append('AntiKt4EMPFlow')
        
    for metDef in metDefs:
        acc.merge(METAssociatorCfg(ConfigFlags, metDef))

    return acc

def METCustomVtxCfg(ConfigFlags, vxColl, jetColl, constituentColl):
    from METReconstruction.METAssocCfg import METAssocConfig, AssocConfig
    from METReconstruction.METAssociatorCfg import getAssocCA

    associators = [AssocConfig('CustomJet', jetColl+'Jets'),
                    AssocConfig('Muon'),
                    AssocConfig('Ele'),
                    AssocConfig('Gamma'),
                    AssocConfig('Tau'),
                    AssocConfig('Soft')]

    cfg = METAssocConfig(jetColl,
                         ConfigFlags,
                         buildconfigs = associators,
                         doPFlow = ('PFlow' in jetColl),
                         usePFOLinks = ConfigFlags.MET.UseFELinks,
                         modConstKey = constituentColl)

    for assoc in cfg.assoclist:
        assoc.PrimVxColl = vxColl

    return getAssocCA(cfg,METName='CustomJet')