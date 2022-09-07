# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

if __name__=="__main__":

    from AthenaConfiguration.AllConfigFlags import ConfigFlags as cfgFlags

    cfgFlags.Concurrency.NumThreads=8
    cfgFlags.Input.isMC=True
    cfgFlags.Input.Files= ["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/RecExRecoTest/mc20e_13TeV/valid1.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.ESD.e4993_s3227_r12689/myESD.pool.root"]
    cfgFlags.Output.AODFileName="output_AOD.root"
    cfgFlags.Output.doWriteAOD=True
    #We need to rebuild the origin topoclusters for the MET, but the jet software will not rerun the relevant algorithms
    #if Input.Collections contains the container name. So here we remove them to trick the jet software.
    #Note you cannot directly remove them with cfgFlags.Input.Collections.remove('blah')
    myList = cfgFlags.Input.Collections
    myList.remove('EMOriginTopoClusters')
    myList.remove('LCOriginTopoClusters')
    cfgFlags.Input.Collections = myList
    cfgFlags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(cfgFlags)

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg.merge(PoolReadCfg(cfgFlags))

    from eflowRec.PFRun3Config import PFFullCfg
    cfg.merge(PFFullCfg(cfgFlags))

    from eflowRec.PFRun3Config import PFTauFELinkCfg
    cfg.merge(PFTauFELinkCfg(cfgFlags))

    from eflowRec.PFRun3Remaps import ListRemaps

    list_remaps=ListRemaps()
    for mapping in list_remaps:
        cfg.merge(mapping)    

    #Given we rebuild topoclusters from the ESD, we should also redo the matching between topoclusters and muon clusters.
    #The resulting links are used to create the global GPF muon-FE links.
    from AthenaConfiguration.ComponentFactory import CompFactory
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    result = ComponentAccumulator()    
    result.addEventAlgo(CompFactory.ClusterMatching.CaloClusterMatchLinkAlg("MuonTCLinks", ClustersToDecorate="MuonClusterCollection"))
    cfg.merge(result)

    from AthenaCommon.Configurable import Configurable
    Configurable.configurableRun3Behavior = True

    from JetRecConfig.JetRecConfig import JetRecCfg
    from JetRecConfig.StandardSmallRJets import AntiKt4EMPFlow
    from JetRecConfig.JetConfigFlags import jetInternalFlags  
    jetInternalFlags.isRecoJob = True
    cfg.merge( JetRecCfg(cfgFlags,AntiKt4EMPFlow) )     
    
    #Now do MET config

    #The MET soft term needs the EM and LC origin topoclusters.
    #Since we rebuilt the topoclusters, we must also rebuild these.
    from JetRecConfig.JetRecConfig import JetInputCfg
    from JetRecConfig.StandardJetConstits import stdConstitDic as cst
    cfg.merge(JetInputCfg(cfgFlags,cst.EMTopoOrigin))
    cfg.merge(JetInputCfg(cfgFlags,cst.LCTopoOrigin))

    #Now build the pflow MET association map and then add the METMaker algorithm
    from METReconstruction.METAssociatorCfg import METAssociatorCfg
    cfg.merge(METAssociatorCfg(cfgFlags, 'AntiKt4EMPFlow'))

    from METUtilities.METMakerConfig import getMETMakerAlg
    metCA=ComponentAccumulator()    
    metCA.addEventAlgo(getMETMakerAlg('AntiKt4EMPFlow'))
    cfg.merge(metCA)

    cfg.run()
