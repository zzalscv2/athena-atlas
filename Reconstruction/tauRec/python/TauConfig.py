# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import BeamType, LHCPeriod


def TauBuildAlgCfg(flags):

    result = ComponentAccumulator()

    # Schedule total noise cond alg
    from CaloTools.CaloNoiseCondAlgConfig import CaloNoiseCondAlgCfg
    result.merge(CaloNoiseCondAlgCfg(flags, "totalNoise"))
    # Schedule electronic noise cond alg (needed for LC weights)
    result.merge(CaloNoiseCondAlgCfg(flags, "electronicNoise"))

    # get tools from holder
    import tauRec.TauToolHolder as tauTools

    tools = []
    tools.append( result.popToolsAndMerge(tauTools.JetSeedBuilderCfg(flags)) )

    # FIXME: placeholder, tool not implemented yet
    # for electron-removed taus, check that seed jets are close to an electron
    if flags.Tau.ActiveConfig.doTauEleRM:
        tools.append( result.popToolsAndMerge(tauTools.TauElectronExcluderCfg(flags)) )

    # run vertex finder only in case vertexing is available
    if flags.Tau.isStandalone or flags.Tracking.doVertexFinding:
        tools.append( result.popToolsAndMerge(tauTools.TauVertexFinderCfg(flags)) )

    tools.append( result.popToolsAndMerge(tauTools.TauAxisCfg(flags)) )
    tools.append( result.popToolsAndMerge(tauTools.TauTrackFinderCfg(flags)) )

    tools.append( result.popToolsAndMerge(tauTools.TauClusterFinderCfg(flags)) )
    tools.append( result.popToolsAndMerge(tauTools.TauVertexedClusterDecoratorCfg(flags)) )

    if flags.Beam.Type is not BeamType.Cosmics:
        if flags.Tau.doRNNTrackClass:
            tools.append( result.popToolsAndMerge(tauTools.TauTrackRNNClassifierCfg(flags)) )
        tools.append( result.popToolsAndMerge(tauTools.EnergyCalibrationLCCfg(flags)) )

    tools.append( result.popToolsAndMerge(tauTools.CellVariablesCfg(flags)) )
    tools.append( result.popToolsAndMerge(tauTools.ElectronVetoVarsCfg(flags)) )
    tools.append( result.popToolsAndMerge(tauTools.TauShotFinderCfg(flags)) )

    if flags.Tau.doPi0Clus:
        tools.append( result.popToolsAndMerge(tauTools.Pi0ClusterFinderCfg(flags)) )

    # TauBuildAlg AKA TauProcessorAlg
    TauProcessorAlg = CompFactory.getComp("TauProcessorAlg")
    BuildAlg = TauProcessorAlg(name                           = flags.Tau.ActiveConfig.prefix+"TauCoreBuilderAlg",
                               Key_jetInputContainer          = flags.Tau.ActiveConfig.SeedJetCollection,
                               Key_tauOutputContainer         = flags.Tau.ActiveConfig.TauJets_tmp,
                               Key_tauTrackOutputContainer    = flags.Tau.ActiveConfig.TauTracks,
                               Key_tauShotClusOutputContainer = flags.Tau.ActiveConfig.TauShotClusters,
                               Key_tauShotClusLinkContainer   = flags.Tau.ActiveConfig.TauShotClustersLinks,
                               Key_tauShotPFOOutputContainer  = flags.Tau.ActiveConfig.TauShotPFOs,
                               Key_tauPi0CellOutputContainer  = flags.Tau.ActiveConfig.TauCommonPi0Cells,
                               MaxEta                         = flags.Tau.SeedMaxEta,
                               MinPt                          = flags.Tau.SeedMinPt,
                               MaxNTracks                     = flags.Tau.MaxNTracks,
                               Tools                          = tools,
                               CellMakerTool                  = result.popToolsAndMerge(tauTools.TauCellFinalizerCfg(flags)))

    if flags.GeoModel.Run is LHCPeriod.Run4:
        BuildAlg.PixelDetEleCollKey="ITkPixelDetectorElementCollection"
        BuildAlg.SCTDetEleCollKey="ITkStripDetectorElementCollection"
        BuildAlg.TRTDetEleContKey=""

    result.addEventAlgo(BuildAlg)
    return result


def TauCaloAlgCfg(flags):

    result = ComponentAccumulator()

    # Schedule total noise cond alg
    from CaloTools.CaloNoiseCondAlgConfig import CaloNoiseCondAlgCfg
    result.merge(CaloNoiseCondAlgCfg(flags,"totalNoise"))
    # Schedule electronic noise cond alg (needed for LC weights)
    result.merge(CaloNoiseCondAlgCfg(flags,"electronicNoise"))

    from CaloRec.CaloTopoClusterConfig import caloTopoCoolFolderCfg
    result.merge(caloTopoCoolFolderCfg(flags))

    from LArBadChannelTool.LArBadChannelConfig import LArBadChannelCfg
    result.merge(LArBadChannelCfg(flags))

    from TileConditions.TileBadChannelsConfig import TileBadChannelsCondAlgCfg
    result.merge( TileBadChannelsCondAlgCfg(flags) )

    # get tools from holder
    import tauRec.TauToolHolder as tauTools

    CaloClusterMaker = CompFactory.getComp("CaloClusterMaker")
    CaloTopoForTausMaker = CaloClusterMaker (flags.Tau.ActiveConfig.prefix+"TauPi0SubtractedClusterMaker")
    CaloTopoForTausMaker.ClustersOutputName = flags.Tau.ActiveConfig.TauPi0Clusters_tmp
    CaloTopoForTausMaker.ClusterMakerTools = [result.popToolsAndMerge(tauTools.TauCaloTopoClusterMakerCfg(flags)),
                                              result.popToolsAndMerge(tauTools.TauCaloTopoClusterSplitterCfg(flags))]

    CaloTopoForTausMaker.ClusterCorrectionTools += [result.popToolsAndMerge(tauTools.TauCaloClusterBadChannelCfg(flags))]
    CaloTopoForTausMaker.ClusterCorrectionTools += [result.popToolsAndMerge(tauTools.TauCaloClusterMomentsMakerCfg(flags))]

    if flags.Calo.TopoCluster.doCellWeightCalib:
        CaloTopoForTausMaker.ClusterCorrectionTools += [result.popToolsAndMerge(tauTools.TauCaloClusterCellWeightCalibCfg(flags))]

    if flags.Calo.TopoCluster.doTopoClusterLocalCalib:
        CaloTopoForTausMaker.ClusterCorrectionTools += [result.popToolsAndMerge(tauTools.TauCaloClusterLocalCalibCfg(flags)),
                                                        result.popToolsAndMerge(tauTools.TauCaloOOCCalibCfg(flags)),
                                                        result.popToolsAndMerge(tauTools.TauCaloOOCPi0CalibCfg(flags)),
                                                        result.popToolsAndMerge(tauTools.TauCaloDMCalibCfg(flags))]

    result.addEventAlgo(CaloTopoForTausMaker)

    relinkAlg = CompFactory.ClusterCellRelinkAlg(name            = flags.Tau.ActiveConfig.prefix+'ClusterCellRelinkAlg',
                                                 Cells           = 'AllCalo',
                                                 ClustersInput   = flags.Tau.ActiveConfig.TauPi0Clusters_tmp,
                                                 ClustersOutput  = flags.Tau.ActiveConfig.TauPi0Clusters,
                                                 CellLinksOutput = flags.Tau.ActiveConfig.TauPi0ClustersLinks)
    result.addEventAlgo(relinkAlg)
    return result


def TauRunnerAlgCfg(flags):

    result=ComponentAccumulator()

    # get tools from holder
    import tauRec.TauToolHolder as tauTools

    tools = []

    tools.append( result.popToolsAndMerge(tauTools.Pi0ClusterCreatorCfg(flags)) )
    tools.append( result.popToolsAndMerge(tauTools.Pi0ClusterScalerCfg(flags)) )
    tools.append( result.popToolsAndMerge(tauTools.Pi0ScoreCalculatorCfg(flags)) )
    tools.append( result.popToolsAndMerge(tauTools.Pi0SelectorCfg(flags)) )

    if flags.Tau.isStandalone or flags.Tracking.doVertexFinding:
        tools.append(result.popToolsAndMerge(tauTools.TauVertexVariablesCfg(flags)) )

    tools.append( result.popToolsAndMerge(tauTools.TauCommonCalcVarsCfg(flags)) )
    tools.append( result.popToolsAndMerge(tauTools.TauSubstructureCfg(flags)) )

    if flags.Tau.doPanTau:
        import PanTauAlgs.JobOptions_Main_PanTau_New as pantau
        tools.append( result.popToolsAndMerge(pantau.PanTauCfg(flags)) )

    tools.append(result.popToolsAndMerge(tauTools.TauCombinedTESCfg(flags)) )
    # these tools need pantau info
    if flags.Beam.Type is not BeamType.Cosmics:
        tools.append( result.popToolsAndMerge(tauTools.MvaTESVariableDecoratorCfg(flags)) )
        tools.append( result.popToolsAndMerge(tauTools.MvaTESEvaluatorCfg(flags)) )

    tools.append( result.popToolsAndMerge(tauTools.TauAODSelectorCfg(flags)) )

    if flags.Tau.doTauDiscriminant:
        tools.append( result.popToolsAndMerge(tauTools.TauIDVarCalculatorCfg(flags)) )
        tools.append( result.popToolsAndMerge(tauTools.TauJetRNNEvaluatorCfg(flags)) )
        tools.append( result.popToolsAndMerge(tauTools.TauWPDecoratorJetRNNCfg(flags)) )
        tools.append( result.popToolsAndMerge(tauTools.TauEleRNNEvaluatorCfg(flags)) )
        tools.append( result.popToolsAndMerge(tauTools.TauWPDecoratorEleRNNCfg(flags)) )
        tools.append( result.popToolsAndMerge(tauTools.TauDecayModeNNClassifierCfg(flags)) )

    TauRunnerAlg = CompFactory.getComp("TauRunnerAlg")
    RunnerAlg = TauRunnerAlg(name                           = flags.Tau.ActiveConfig.prefix+"TauRecRunnerAlg",
                             Key_tauInputContainer          = flags.Tau.ActiveConfig.TauJets_tmp,
                             Key_Pi0ClusterInputContainer   = flags.Tau.ActiveConfig.TauPi0Clusters,
                             Key_tauOutputContainer         = flags.Tau.ActiveConfig.TauJets,
                             Key_neutralPFOOutputContainer  = flags.Tau.ActiveConfig.TauNeutralPFOs,
                             Key_hadronicPFOOutputContainer = flags.Tau.ActiveConfig.TauHadronicPFOs,
                             Key_chargedPFOOutputContainer  = flags.Tau.ActiveConfig.TauChargedPFOs,
                             Key_vertexOutputContainer      = flags.Tau.ActiveConfig.TauSecondaryVertices,
                             Key_pi0Container               = flags.Tau.ActiveConfig.TauFinalPi0s,
                             Tools                          = tools)

    result.addEventAlgo(RunnerAlg)
    return result


def TauOutputCfg(flags):

    from OutputStreamAthenaPool.OutputStreamConfig import addToESD,addToAOD
    result=ComponentAccumulator()

    # common to AOD and ESD
    TauAODList = []
    TauAODList += [ "xAOD::TauJetContainer#{}"             .format(flags.Tau.ActiveConfig.TauJets) ]
    TauAODList += [ "xAOD::TauTrackContainer#{}"           .format(flags.Tau.ActiveConfig.TauTracks) ]
    TauAODList += [ "xAOD::TauTrackAuxContainer#{}Aux."    .format(flags.Tau.ActiveConfig.TauTracks) ]
    TauAODList += [ "xAOD::VertexContainer#{}"             .format(flags.Tau.ActiveConfig.TauSecondaryVertices) ]
    TauAODList += [ "xAOD::VertexAuxContainer#{}Aux.-vxTrackAtVertex".format(flags.Tau.ActiveConfig.TauSecondaryVertices) ]
    TauAODList += [ "xAOD::CaloClusterContainer#{}"        .format(flags.Tau.ActiveConfig.TauPi0Clusters) ]
    TauAODList += [ "xAOD::CaloClusterAuxContainer#{}Aux." .format(flags.Tau.ActiveConfig.TauPi0Clusters) ]
    TauAODList += [ "CaloClusterCellLinkContainer#{}_links".format(flags.Tau.ActiveConfig.TauPi0Clusters) ]
    TauAODList += [ "xAOD::CaloClusterContainer#{}"        .format(flags.Tau.ActiveConfig.TauShotClusters)]
    TauAODList += [ "xAOD::CaloClusterAuxContainer#{}Aux." .format(flags.Tau.ActiveConfig.TauShotClusters)]
    TauAODList += [ "CaloClusterCellLinkContainer#{}_links".format(flags.Tau.ActiveConfig.TauShotClusters) ]
    TauAODList += [ "xAOD::ParticleContainer#{}"           .format(flags.Tau.ActiveConfig.TauFinalPi0s) ]
    TauAODList += [ "xAOD::ParticleAuxContainer#{}Aux."    .format(flags.Tau.ActiveConfig.TauFinalPi0s) ]
    TauAODList += [ "xAOD::PFOContainer#{}"                .format(flags.Tau.ActiveConfig.TauShotPFOs) ]
    TauAODList += [ "xAOD::PFOAuxContainer#{}Aux."         .format(flags.Tau.ActiveConfig.TauShotPFOs) ]
    TauAODList += [ "xAOD::PFOContainer#{}"                .format(flags.Tau.ActiveConfig.TauNeutralPFOs) ]
    TauAODList += [ "xAOD::PFOAuxContainer#{}Aux."         .format(flags.Tau.ActiveConfig.TauNeutralPFOs) ]
    TauAODList += [ "xAOD::PFOContainer#{}"                .format(flags.Tau.ActiveConfig.TauHadronicPFOs) ]
    TauAODList += [ "xAOD::PFOAuxContainer#{}Aux."         .format(flags.Tau.ActiveConfig.TauHadronicPFOs) ]

    # Set common to ESD too
    TauESDList = list(TauAODList)

    # add AOD specific
    #Also remove GlobalFELinks - these are links between FlowElement (FE) containers created in jet finding and taus. Since these transient FE containers are not in the AOD, we should not write out these links.
    TauAODList += [ "xAOD::TauJetAuxContainer#{}Aux.-VertexedClusters.-mu.-nVtxPU.-ABS_ETA_LEAD_TRACK.-TAU_ABSDELTAPHI.-TAU_ABSDELTAETA.-absipSigLeadTrk.-passThinning.-chargedGlobalFELinks.-neutralGlobalFELinks"
                    .format(flags.Tau.ActiveConfig.TauJets) ]

    # addEOD specific
    #Also remove GlobalFELinks - these are links between FlowElement (FE) containers created in jet finding and taus. Since these transient FE containers are not in the AOD, we should not write out these links.
    TauESDList += [ "xAOD::TauJetAuxContainer#{}Aux.-VertexedClusters.-chargedGlobalFELinks.-neutralGlobalFELinks".format(flags.Tau.ActiveConfig.TauJets) ]
    TauESDList += [ "xAOD::PFOContainer#{}"        .format(flags.Tau.ActiveConfig.TauChargedPFOs) ]
    TauESDList += [ "xAOD::PFOAuxContainer#{}Aux." .format(flags.Tau.ActiveConfig.TauChargedPFOs) ]

    result.merge(addToESD(flags,TauESDList))
    result.merge(addToAOD(flags,TauAODList))
    return result


def DiTauOutputCfg(flags):

   from OutputStreamAthenaPool.OutputStreamConfig import addToESD,addToAOD
   result=ComponentAccumulator()

   DiTauOutputList  = [ "xAOD::DiTauJetContainer#DiTauJets" ]
   DiTauOutputList += [ "xAOD::DiTauJetAuxContainer#DiTauJetsAux." ]

   result.merge(addToESD(flags,DiTauOutputList))
   result.merge(addToAOD(flags,DiTauOutputList))
   return result


def TauxAODthinngCfg(flags):

    result = ComponentAccumulator()

    tauThinAlg = CompFactory.TauThinningAlg(name                 = flags.Tau.ActiveConfig.prefix+"TauThinningAlg",
                                            Taus                 = flags.Tau.ActiveConfig.TauJets,
                                            TauTracks            = flags.Tau.ActiveConfig.TauTracks,
                                            TauNeutralPFOs       = flags.Tau.ActiveConfig.TauNeutralPFOs,
                                            TauPi0Clusters       = flags.Tau.ActiveConfig.TauPi0Clusters,
                                            TauPi0CellLinks      = flags.Tau.ActiveConfig.TauPi0ClustersLinks,
                                            TauFinalPi0s         = flags.Tau.ActiveConfig.TauFinalPi0s,
                                            TauShotPFOs          = flags.Tau.ActiveConfig.TauShotPFOs,
                                            TauShotClusters      = flags.Tau.ActiveConfig.TauShotClusters,
                                            TauShotCellLinks     = flags.Tau.ActiveConfig.TauShotClustersLinks,
                                            TauHadronicPFOs      = flags.Tau.ActiveConfig.TauHadronicPFOs,
                                            TauSecondaryVertices = flags.Tau.ActiveConfig.TauSecondaryVertices)
    result.addEventAlgo(tauThinAlg)
    return result


def TauReconstructionCfg(flags):

    result = ComponentAccumulator()

    # standard tau reconstruction
    flags_TauRec = flags.cloneAndReplace("Tau.ActiveConfig", "Tau.TauRec")

    result.merge(TauBuildAlgCfg(flags_TauRec))

    result.merge(TauCaloAlgCfg(flags_TauRec))

    result.merge(TauRunnerAlgCfg(flags_TauRec))

    if (flags.Output.doWriteESD or flags.Output.doWriteAOD):
        result.merge(TauOutputCfg(flags_TauRec))

    if (flags.Output.doWriteAOD and flags.Tau.ThinTaus):
        result.merge(TauxAODthinngCfg(flags_TauRec))

    # FIXME: placeholder, tool not implemented yet
    # electron-subtracted tau reconstruction
    if flags.Tau.doTauEleRMRec:

        flags_TauEleRM = flags.cloneAndReplace("Tau.ActiveConfig", "Tau.TauEleRM")

        result.merge(TauElecSubtractAlgCfg(flags_TauEleRM))

        # jet reclustering
        # FIXME: config to be checked
        from JetRecConfig.JetRecConfig import JetRecCfg
        from JetRecConfig.StandardSmallRJets import AntiKt4LCTopo

        AntiKt4LCTopo_ElecRM = AntiKt4LCTopo.clone(suffix="_ElecRM")
        AntiKt4LCTopo_ElecRM.inputdef.inputname = flags_TauEleRM.Tau.ActiveConfig.CaloCalTopoClusters_EleRM
        # FIXME: could also feed electron-subtracted tracks for ghost matching, although not strictly needed
        result.merge(JetRecCfg(flags_TauEleRM, AntiKt4LCTopo_ElecRM))

        result.merge(TauBuildAlgCfg(flags_TauEleRM))

        result.merge(TauCaloAlgCfg(flags_TauEleRM))

        result.merge(TauRunnerAlgCfg(flags_TauEleRM))

        if (flags.Output.doWriteESD or flags.Output.doWriteAOD):
            result.merge(TauOutputCfg(flags_TauEleRM))

        if (flags.Output.doWriteAOD and flags.Tau.ThinTaus):
            result.merge(TauxAODthinngCfg(flags_TauEleRM))

    # had-had boosted ditaus
    if flags.Tau.doDiTauRec:
        from DiTauRec.DiTauBuilderConfig import DiTauBuilderCfg
        result.merge(DiTauBuilderCfg(flags))

        if (flags.Output.doWriteESD or flags.Output.doWriteAOD):
            result.merge(DiTauOutputCfg(flags))

    return result


# FIXME: placeholder, algorithm not implemented yet
def TauElecSubtractAlgCfg(flags):

   result = ComponentAccumulator()

   from ElectronPhotonSelectorTools.AsgElectronLikelihoodToolsConfig import AsgElectronLikelihoodToolCfg
   from ElectronPhotonSelectorTools.LikelihoodEnums import LikeEnum
   from ElectronPhotonSelectorTools.ElectronLikelihoodToolMapping import electronLHmenu

   ElectronLHSelectorTight = result.popToolsAndMerge(AsgElectronLikelihoodToolCfg(flags,
                                                                                  name    = flags.Tau.ActiveConfig.prefix+"ElectronLHSelectorTight",
                                                                                  quality = LikeEnum.Tight,
                                                                                  menu    = electronLHmenu.offlineMC21))

   tauElecSubtractAlg = CompFactory.TauElecSubtractAlg(name                        = flags.Tau.ActiveConfig.prefix+"TauElecSubtractAlg",
                                                       Key_ElectronsInput          = 'Electrons',
                                                       Key_ClustersInput           = 'CaloCalTopoClusters',
                                                       Key_ClustersOutput          = flags.Tau.ActiveConfig.CaloCalTopoClusters_EleRM,
                                                       Key_IDTracksInput           = 'InDetTrackParticles',
                                                       Key_IDTracksOutput          = flags.Tau.ActiveConfig.TrackCollection,
                                                       Key_RemovalDirectionsOutput = flags.Tau.ActiveConfig.ElectronDirections,
                                                       ElectronLHTool              = ElectronLHSelectorTight)
   result.addEventAlgo(tauElecSubtractAlg)
   return result


# This is an example config for scheduling TauJet_MuonRM in AOD
def TauAODrunnerAlgCfg(flags):
    from DerivationFrameworkTau.TauCommonConfig import AddMuonRemovalTauAODReRecoAlgCfg
    result = AddMuonRemovalTauAODReRecoAlgCfg(flags)
    return result


if __name__=="__main__":

    from AthenaConfiguration.AllConfigFlags import ConfigFlags

    from AthenaCommon.AlgSequence import AlgSequence
    topSequence = AlgSequence()

    ConfigFlags.Input.Files = ["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/RecExRecoTest/mc21_13p6TeV/ESDFiles/mc21_13p6TeV.421450.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep_fct.recon.ESD.e8445_e8447_s3822_r13565/ESD.28877240._000046.pool.root.1"]
    # Use latest MC21 tag to pick up latest muon folders apparently needed
    ConfigFlags.IOVDb.GlobalTag = "OFLCOND-MC21-SDR-RUN3-10"
    ConfigFlags.Output.ESDFileName = "ESD.pool.root"
    ConfigFlags.Output.AODFileName = "AOD.pool.root"

    nThreads=1
    ConfigFlags.Concurrency.NumThreads = nThreads
    if nThreads>0:
        ConfigFlags.Scheduler.ShowDataDeps = True
        ConfigFlags.Scheduler.ShowDataFlow = True
        ConfigFlags.Scheduler.ShowControlFlow = True
        ConfigFlags.Concurrency.NumConcurrentEvents = nThreads

    from AthenaCommon.AlgScheduler import AlgScheduler
    AlgScheduler.ShowControlFlow( True )
    AlgScheduler.ShowDataDependencies( True )
    AlgScheduler.setDataLoaderAlg('SGInputLoader')

    # Update once new jet flags are available
    from JetRec.JetRecFlags import jetFlags
    if not jetFlags.useTracks():
        ConfigFlags.Tau.doTJVA = False  # switch off TJVA

    ConfigFlags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg

    cfg=MainServicesCfg(ConfigFlags)
    cfg.merge(PoolReadCfg(ConfigFlags))

    StoreGateSvc=CompFactory.StoreGateSvc
    cfg.addService(StoreGateSvc("DetectorStore"))

    # this delcares to the scheduler that EventInfo object comes from the input
    loadFromSG = [('xAOD::EventInfo', 'StoreGateSvc+EventInfo'),
                  ( 'AthenaAttributeList' , 'StoreGateSvc+Input' ),
                  ( 'CaloCellContainer' , 'StoreGateSvc+AllCalo' )]
    cfg.addEventAlgo(CompFactory.SGInputLoader(Load=loadFromSG), sequenceName="AthAlgSeq")

    # print "Dump flags:"
    # ConfigFlags.Tau.dump()

    cfg.merge(TauReconstructionCfg(ConfigFlags))

    from SGComps.AddressRemappingConfig import AddressRemappingCfg
    rename_maps = [ '%s#%s->%s' % ("xAOD::TauJetContainer", "TauJets", "old_TauJets"),
                    '%s#%s->%s' % ("xAOD::TauJetAuxContainer", "TauJetsAux.", "old_TauJetsAux.")]
    cfg.merge( AddressRemappingCfg(rename_maps) )

    cfg.run(10)
