# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

def configureRecoForPFlowCfg(cfgFlags):

    #Given we rebuild topoclusters from the ESD, we should also redo the matching between topoclusters and muon clusters.
    #The resulting links are used to create the global GPF muon-FE links.
    from AthenaConfiguration.ComponentFactory import CompFactory
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    cfg = ComponentAccumulator()    
    cfg.addEventAlgo(CompFactory.ClusterMatching.CaloClusterMatchLinkAlg("MuonTCLinks", ClustersToDecorate="MuonClusterCollection"))

    from JetRecConfig.JetRecConfig import JetRecCfg
    from JetRecConfig.StandardSmallRJets import AntiKt4EMPFlow, AntiKt4LCTopo
    from JetRecConfig.JetConfigFlags import jetInternalFlags  
    jetInternalFlags.isRecoJob = True
    cfg.merge( JetRecCfg(cfgFlags,AntiKt4EMPFlow) )     
    cfg.merge( JetRecCfg(cfgFlags,AntiKt4LCTopo) )
    
    #Now do MET config

    #The MET soft term needs the EM and LC origin topoclusters.
    #Since we rebuilt the topoclusters, we must also rebuild these.
    from JetRecConfig.JetRecConfig import JetInputCfg
    from JetRecConfig.StandardJetConstits import stdConstitDic as cst
    cfg.merge(JetInputCfg(cfgFlags,cst.EMTopoOrigin))
    cfg.merge(JetInputCfg(cfgFlags,cst.LCTopoOrigin))

    from eflowRec.PFCfg import PFGlobalFlowElementLinkingCfg
    cfg.merge(PFGlobalFlowElementLinkingCfg(cfgFlags))

    from tauRec.TauConfig import TauReconstructionCfg
    cfg.merge(TauReconstructionCfg(cfgFlags))

    from eflowRec.PFTauRemaps import PFTauRemaps
    tauRemaps = PFTauRemaps()
    for mapping in tauRemaps:
        cfg.merge(mapping)

    #Now build the pflow MET association map and then add the METMaker algorithm
    from METReconstruction.METAssociatorCfg import METAssociatorCfg
    cfg.merge(METAssociatorCfg(cfgFlags, 'AntiKt4EMPFlow'))

    from METUtilities.METMakerConfig import getMETMakerAlg
    metCA=ComponentAccumulator()    
    metCA.addEventAlgo(getMETMakerAlg('AntiKt4EMPFlow'))
    cfg.merge(metCA)

    return cfg

