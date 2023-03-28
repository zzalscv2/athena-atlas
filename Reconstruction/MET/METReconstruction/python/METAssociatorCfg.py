# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration

from METReconstruction.METAssocCfg import AssocConfig, METAssocConfig,getMETAssocAlg
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def METAssociatorCfg(configFlags, jetType):

    components = ComponentAccumulator()

    from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg   
    extrapCfg = AtlasExtrapolatorCfg(configFlags)
    extrapCfg.popPrivateTools()
    components.merge(extrapCfg)

    modConstKey = ""
    modClusColls = {}
    if configFlags.MET.UseTracks:
        modConstKey="OriginCorr"
        modClusColls={
            'LCOriginCorrClusters':'LCOriginTopoClusters',
            'EMOriginCorrClusters':'EMOriginTopoClusters'
            }

    if jetType == "AntiKt4LCTopo":
        associators = [AssocConfig('LCJet'),
                    AssocConfig('Muon'),
                    AssocConfig('Ele'),
                    AssocConfig('Gamma'),
                    AssocConfig('Tau'),
                    AssocConfig('Soft')]
        cfg_akt4lc = METAssocConfig('AntiKt4LCTopo',
                                    configFlags,
                                    associators,
                                    doPFlow=False,
                                    modConstKey=modConstKey,
                                    modClusColls=modClusColls
                                    )
        components_akt4lc= getAssocCA(cfg_akt4lc,METName='AntiKt4LCTopo')
        components.merge(components_akt4lc)

    elif jetType == "AntiKt4EMTopo":
        associators = [AssocConfig('EMJet'),
                    AssocConfig('Muon'),
                    AssocConfig('Ele'),
                    AssocConfig('Gamma'),
                    AssocConfig('Tau'),
                    AssocConfig('Soft')]
        cfg_akt4em = METAssocConfig('AntiKt4EMTopo',
                                    configFlags,
                                    associators,
                                    doPFlow=False,
                                    modConstKey=modConstKey,
                                    modClusColls=modClusColls
                                    )
        components_akt4em= getAssocCA(cfg_akt4em,METName='AntiKt4EMTopo')
        components.merge(components_akt4em)

    elif jetType == "AntiKt4EMPFlow":
        associators = [AssocConfig('PFlowJet'),
                       AssocConfig('Muon'),
                       AssocConfig('Ele'),
                       AssocConfig('Gamma'),
                       AssocConfig('Tau'),
                       AssocConfig('Soft')]
        cfg_akt4pf = METAssocConfig('AntiKt4EMPFlow',
                                    configFlags,
                                    associators,
                                    doPFlow=True,
                                    usePFOLinks=configFlags.MET.UseFELinks
                                    )
        components_akt4pf= getAssocCA(cfg_akt4pf,METName='AntiKt4EMPFlow')
        components.merge(components_akt4pf)
    else:
        raise RuntimeError("Jet type not recognized by METAssociatorCfg: {}".format(jetType) )
    return components
    
    
def getAssocCA(config,METName=''):
    components = ComponentAccumulator()
    components.merge(config.accumulator)
    assocAlg = getMETAssocAlg(algName='METAssociation_'+METName,configs={config.suffix:config})
    components.addEventAlgo(assocAlg)
    return components
