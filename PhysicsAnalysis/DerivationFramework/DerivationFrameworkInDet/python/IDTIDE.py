# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#!/usr/bin/env python
#====================================================================
# IDTIDE.py
# Contact: atlas-cp-tracking-denseenvironments@cern.ch
# Component accumulator version - replaces IDTIDE1  
# IMPORTANT: this is NOT an AOD based derived data type but one built
# during reconstruction from HITS or RAW. It consequently has to be 
# run from Reco_tf  
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.CFElements import seqAND
from AthenaCommon.Constants import INFO

# Main algorithm config
def IDTIDEKernelCfg(configFlags, name='IDTIDEKernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel) for IDTIDE"""
    acc = ComponentAccumulator()
    DerivationKernel = CompFactory.DerivationFramework.DerivationKernel

    # Sequence for skimming kernel (if running on data) -> PrepDataToxAOD -> ID TIDE kernel
    # sequence to be used for algorithm which should run before the IDTIDEPresel
    # Disabled as currently blocks decoration of Z0 and thus crashes thinning
    acc.addSequence( seqAND('IDTIDESequence') )
    # sequence for algorithms which should run after the preselection but which can run in parallel
    #acc.addSequence( parOR('IDTIDESeqAfterPresel'), parentName = 'IDTIDESequence')
    # Sequence for skimming after pre-selection
    #acc.addSequence( seqAND('IDTIDESkimmingSequence'), parentName = 'IDTIDESequence')
    # Post processing sequence
    #acc.addSequence( parOR('IDTIDEPostProcSequence'), parentName = 'IDTIDESkimmingSequence' )

    #====================================================================
    # AUGMENTATION TOOLS
    #====================================================================
    augmentationTools = []
    tsos_augmentationTools=[]

    # TrackToVertexIPEstimator
    from TrkConfig.TrkVertexFitterUtilsConfig import TrackToVertexIPEstimatorCfg
    IDTIDEIPETool = acc.getPrimaryAndMerge(TrackToVertexIPEstimatorCfg(configFlags))

    # Add unbiased track parameters to track particles
    from DerivationFrameworkInDet.InDetToolsConfig import TrackToVertexWrapperCfg
    IDTIDETrackToVertexWrapper = acc.getPrimaryAndMerge(TrackToVertexWrapperCfg(
        configFlags,
        name = "IDTIDETrackToVertexWrapper",
        TrackToVertexIPEstimator = IDTIDEIPETool,
        DecorationPrefix = "IDTIDE",
        ContainerName = "InDetTrackParticles")
    )
    augmentationTools.append(IDTIDETrackToVertexWrapper)

    from InDetConfig.InDetUsedInFitTrackDecoratorToolConfig import (
        InDetUsedInFitTrackDecoratorToolCfg)
    IDTIDEUsedInFitDecoratorTool = acc.popToolsAndMerge(
        InDetUsedInFitTrackDecoratorToolCfg(
            configFlags,
            name     = "IDTIDEUsedInFitDecoratorTool",
            AMVFVerticesDecoName = "TTVA_AMVFVertices",
            AMVFWeightsDecoName  = "TTVA_AMVFWeights",
            TrackContainer       = "InDetTrackParticles",
            VertexContainer      = "PrimaryVertices" ))

    from DerivationFrameworkInDet.InDetToolsConfig import UsedInVertexFitTrackDecoratorCfg
    IDTIDEUsedInFitDecorator = acc.getPrimaryAndMerge(UsedInVertexFitTrackDecoratorCfg(
        configFlags,
        name                   = "IDTIDEUsedInFitDecorator",
        UsedInFitDecoratorTool = IDTIDEUsedInFitDecoratorTool )
    )
    augmentationTools.append(IDTIDEUsedInFitDecorator)

    # @TODO eventually computed for other extra outputs. Possible to come  up with a solution to use a common Z0AtPV if there is more than one client ?
    from DerivationFrameworkInDet.InDetToolsConfig import TrackParametersAtPVCfg
    DFCommonZ0AtPV = acc.getPrimaryAndMerge(TrackParametersAtPVCfg(
        configFlags,
        name                       = "DFCommonZ0AtPV",
        TrackParticleContainerName = "InDetTrackParticles",
        VertexContainerName        = "PrimaryVertices",
        Z0SGEntryName              = "IDTIDEInDetTrackZ0AtPV" )
    )
    augmentationTools.append(DFCommonZ0AtPV)

    from DerivationFrameworkInDet.InDetToolsConfig import TrackStateOnSurfaceDecoratorCfg
    from InDetConfig.TRT_ElectronPidToolsConfig import TRT_dEdxToolCfg
    InDetTRT_dEdxTool = acc.popToolsAndMerge(TRT_dEdxToolCfg(configFlags, name = "InDetTRT_dEdxTool"))
    acc.addPublicTool( InDetTRT_dEdxTool )
    DFTSOS = acc.getPrimaryAndMerge(TrackStateOnSurfaceDecoratorCfg(
        configFlags,
        name = "DFTrackStateOnSurfaceDecorator",
        ContainerName = "InDetTrackParticles",
        IsSimulation = configFlags.Input.isMC,
        DecorationPrefix = "",
        StoreTRT   = configFlags.Detector.EnableTRT,
        AddExtraEventInfo = False,  # never decorate EventInfo with TRTPhase, doubt this is useful for IDTIDE
        TRT_ToT_dEdx = InDetTRT_dEdxTool,
        PRDtoTrackMap= "",# + InDetKeys.UnslimmedTracks() if  jobproperties.PrimaryDPDFlags.WriteDAOD_IDTRKVALIDStream.get_Value() else "",
        StoreSCT   = configFlags.Detector.EnableSCT,
        StorePixel = configFlags.Detector.EnablePixel,
        OutputLevel =INFO)
    )
    tsos_augmentationTools.append(DFTSOS)

    # Add decoration with truth parameters if running on simulation
    # No idea what to do with this
    #if configFlags.Input.isMC:
    #    # add track parameter decorations to truth particles but only if the decorations have not been applied already
    #    import InDetPhysValMonitoring.InDetPhysValDecoration
    #    meta_data = InDetPhysValMonitoring.InDetPhysValDecoration.getMetaData()
    #    from AthenaCommon.Logging import logging
    #    logger = logging.getLogger( "DerivationFramework" )
    #    if len(meta_data) == 0 :
    #        truth_track_param_decor_alg = InDetPhysValMonitoring.InDetPhysValDecoration.getInDetPhysValTruthDecoratorAlg()
    #        if  InDetPhysValMonitoring.InDetPhysValDecoration.findAlg([truth_track_param_decor_alg.getName()]) == None :
    #            IDTIDESequencePre += truth_track_param_decor_alg
    #        else :
    #            logger.info('Decorator %s already present not adding again.' % (truth_track_param_decor_alg.getName() ))
    #    else :
    #        logger.info('IDPVM decorations to track particles already applied to input file not adding again.')


    #====================================================================
    # SKIMMING TOOLS 
    #====================================================================
    skimmingTools = []
    if not configFlags.Input.isMC:

        sel_jet600 = 'AntiKt4EMPFlowJets.JetConstitScaleMomentum_pt >= 600.*GeV'
        sel_jet800 = 'AntiKt4EMPFlowJets.JetConstitScaleMomentum_pt >= 800.*GeV'
        sel_jet1000 = 'AntiKt4EMPFlowJets.JetConstitScaleMomentum_pt >= 1000.*GeV'

        desd_jetA = '( HLT_j110_pf_ftf_preselj80_L1J30 || HLT_j175_pf_ftf_preselj140_L1J50 || HLT_j260_pf_ftf_preselj200_L1J75 )'
        desd_jetC = '( HLT_j360_pf_ftf_preselj225_L1J100 )'
        desd_jetD = '( HLT_j420_pf_ftf_preselj225_L1J100 && !HLT_j460_pf_ftf_preselj225_L1J100 )'
        desd_jetE = '( HLT_j460_pf_ftf_preselj225_L1J100 )'
        desd_jetF = '( HLT_j460_pf_ftf_preselj225_L1J100 && count('+sel_jet600+')>0 && count('+sel_jet800+')==0 )'
        desd_jetG = '( HLT_j460_pf_ftf_preselj225_L1J100 && count('+sel_jet800+')>0 && count('+sel_jet1000+')==0 )'
        desd_jetH = '( HLT_j460_pf_ftf_preselj225_L1J100 && count('+sel_jet1000+')>0 )'


        prescaleA = 20
        prescaleC = 40
        prescaleD = 30
        prescaleE = 20
        prescaleF = 10
        prescaleG = 5
        #prescaleH = 1 Unused
  
        from DerivationFrameworkTools.DerivationFrameworkToolsConfig import (xAODStringSkimmingToolCfg,
                                                                             PrescaleToolCfg,
                                                                             FilterCombinationANDCfg,
                                                                             FilterCombinationORCfg)
         
        IDTIDE_SkimmingToolA = acc.getPrimaryAndMerge(xAODStringSkimmingToolCfg(configFlags, name = "IDTIDE_SkimmingToolA", expression = desd_jetA))
        IDTIDE_PrescaleToolA = acc.getPrimaryAndMerge(PrescaleToolCfg(configFlags, name="IDTIDE_PrescaleToolA", Prescale=prescaleA))
        IDTIDE_ANDToolA = acc.getPrimaryAndMerge(FilterCombinationANDCfg(configFlags, name="IDTIDE_ANDToolA", FilterList=[IDTIDE_SkimmingToolA,IDTIDE_PrescaleToolA] ))
 
        IDTIDE_SkimmingToolC = acc.getPrimaryAndMerge(xAODStringSkimmingToolCfg(configFlags, name = "IDTIDE_SkimmingToolC", expression = desd_jetC))
        IDTIDE_PrescaleToolC = acc.getPrimaryAndMerge(PrescaleToolCfg(configFlags, name="IDTIDE_PrescaleToolC", Prescale=prescaleC))
        IDTIDE_ANDToolC = acc.getPrimaryAndMerge(FilterCombinationANDCfg(configFlags, name="IDTIDE_ANDToolC", FilterList=[IDTIDE_SkimmingToolC,IDTIDE_PrescaleToolC] ))

        IDTIDE_SkimmingToolD = acc.getPrimaryAndMerge(xAODStringSkimmingToolCfg(configFlags, name = "IDTIDE_SkimmingToolD", expression = desd_jetD))
        IDTIDE_PrescaleToolD = acc.getPrimaryAndMerge(PrescaleToolCfg(configFlags, name="IDTIDE_PrescaleToolD", Prescale=prescaleD))
        IDTIDE_ANDToolD = acc.getPrimaryAndMerge(FilterCombinationANDCfg(configFlags, name="IDTIDE_ANDToolD", FilterList=[IDTIDE_SkimmingToolD,IDTIDE_PrescaleToolD] ))

        IDTIDE_SkimmingToolE = acc.getPrimaryAndMerge(xAODStringSkimmingToolCfg(configFlags, name = "IDTIDE_SkimmingToolE", expression = desd_jetE))
        IDTIDE_PrescaleToolE = acc.getPrimaryAndMerge(PrescaleToolCfg(configFlags, name="IDTIDE_PrescaleToolE", Prescale=prescaleE))
        IDTIDE_ANDToolE = acc.getPrimaryAndMerge(FilterCombinationANDCfg(configFlags, name="IDTIDE_ANDToolE", FilterList=[IDTIDE_SkimmingToolE,IDTIDE_PrescaleToolE] ))

        IDTIDE_SkimmingToolF = acc.getPrimaryAndMerge(xAODStringSkimmingToolCfg(configFlags, name = "IDTIDE_SkimmingToolF", expression = desd_jetF))
        IDTIDE_PrescaleToolF = acc.getPrimaryAndMerge(PrescaleToolCfg(configFlags, name="IDTIDE_PrescaleToolF", Prescale=prescaleF))
        IDTIDE_ANDToolF = acc.getPrimaryAndMerge(FilterCombinationANDCfg(configFlags, name="IDTIDE_ANDToolF", FilterList=[IDTIDE_SkimmingToolF,IDTIDE_PrescaleToolF] ))

        IDTIDE_SkimmingToolG = acc.getPrimaryAndMerge(xAODStringSkimmingToolCfg(configFlags, name = "IDTIDE_SkimmingToolG", expression = desd_jetG))
        IDTIDE_PrescaleToolG = acc.getPrimaryAndMerge(PrescaleToolCfg(configFlags, name="IDTIDE_PrescaleToolG", Prescale=prescaleG))
        IDTIDE_ANDToolG = acc.getPrimaryAndMerge(FilterCombinationANDCfg(configFlags, name="IDTIDE_ANDToolG", FilterList=[IDTIDE_SkimmingToolG,IDTIDE_PrescaleToolG] ))

        IDTIDE_SkimmingToolH = acc.getPrimaryAndMerge(xAODStringSkimmingToolCfg(configFlags, name = "IDTIDE_SkimmingToolH", expression = desd_jetH))        
 
        IDTIDE_ORTool = acc.getPrimaryAndMerge(FilterCombinationORCfg(configFlags, 
                                                                      name="IDTIDELogicalCombination", 
                                                                      FilterList=[IDTIDE_ANDToolA,IDTIDE_ANDToolC,IDTIDE_ANDToolD,IDTIDE_ANDToolE,IDTIDE_ANDToolF,IDTIDE_ANDToolG,IDTIDE_SkimmingToolH] ))

        skimmingTools.append(IDTIDE_ORTool)
    # End of: if not configFlags.Input.isMC    
    
    IDTIDEKernelPresel = DerivationKernel("IDTIDEKernelPresel", SkimmingTools = skimmingTools)
    acc.addEventAlgo( IDTIDEKernelPresel, sequenceName="IDTIDESequence" )   

    #Setup decorators tools
    #if configFlags.Detector.EnableTRT:
    from InDetConfig.InDetPrepRawDataToxAODConfig import InDetTRT_PrepDataToxAODCfg
    acc.merge(InDetTRT_PrepDataToxAODCfg(configFlags, name = "xAOD_TRT_PrepDataToxAOD", 
                                         OutputLevel  = INFO,
                                         WriteSDOs    = False,
                                         UseTruthInfo = configFlags.Input.isMC)) 
    #if configFlags.Detector.EnableSCT:
    from InDetConfig.InDetPrepRawDataToxAODConfig import InDetSCT_PrepDataToxAODCfg
    acc.merge(InDetSCT_PrepDataToxAODCfg(configFlags, name = "xAOD_SCT_PrepDataToxAOD",
                                         OutputLevel  = INFO,
                                         WriteSiHits  = False,
                                         WriteSDOs    = False,
                                         UseTruthInfo = configFlags.Input.isMC))   
 
    #if configFlags.Detector.EnablePixel:
    #  #if need_pix_ToTList : # What to do with this flag?
    #    #from PixelCalibAlgs.PixelCalibAlgsConf import PixelChargeToTConversion 
    #    #PixelChargeToTConversionSetter = PixelChargeToTConversion(name = "PixelChargeToTConversionSetter",
    #    #                                                          ExtraOutputs = ['PixelClusters_ToTList'])
    #    ## IDTIDESeqAfterPresel += PixelChargeToTConversionSetter 
    #    #topSequence += PixelChargeToTConversionSetter
    #    #_info("Add Pixel xAOD ToTConversionSetter: %s Properties: %s", PixelChargeToTConversionSetter, PixelChargeToTConversionSetter.properties())
    from InDetConfig.TrackRecoConfig import ClusterSplitProbabilityContainerName
    from InDetConfig.InDetPrepRawDataToxAODConfig import InDetPixelPrepDataToxAODCfg
    acc.merge(InDetPixelPrepDataToxAODCfg(configFlags, name = "xAOD_Pixel_PrepDataToxAOD",
                                          OutputLevel  = INFO, 
                                          ClusterSplitProbabilityName = ClusterSplitProbabilityContainerName(configFlags),
                                          UseTruthInfo = configFlags.Input.isMC))

    #====================================================================
    # THINNING TOOLS 
    #====================================================================
    thinningTools = []

    # TrackParticles directly
    from DerivationFrameworkInDet.InDetToolsConfig import TrackParticleThinningCfg
    kw = {}
    if not configFlags.Detector.EnablePixel:
      kw['InDetTrackStatesPixKey'] = ''
      kw['InDetTrackMeasurementsPixKey'] = ''
    if not configFlags.Detector.EnableSCT:
      kw['InDetTrackStatesSctKey'] = ''
      kw['InDetTrackMeasurementsSctKey'] = ''
    if not configFlags.Detector.EnableTRT:
      kw['InDetTrackStatesTrtKey'] = ''
      kw['InDetTrackMeasurementsTrtKey'] = ''
    IDTIDEThinningTool = acc.getPrimaryAndMerge(TrackParticleThinningCfg(
        configFlags,
        name = "IDTIDEThinningTool",
        StreamName              = kwargs['StreamName'],
        SelectionString         = "abs(IDTIDEInDetTrackZ0AtPV) < 5.0",
        InDetTrackParticlesKey  = "InDetTrackParticles",
        ThinHitsOnTrack         = False,#If true, Complains about missing PixelMSOSs #InDetDxAODFlags.ThinHitsOnTrack(),
        **kw)
    )
    thinningTools.append(IDTIDEThinningTool)

    # MC truth thinning
    if configFlags.Input.isMC:
        from DerivationFrameworkMCTruth.TruthDerivationToolsConfig import MenuTruthThinningCfg
        IDTIDETruthThinningTool = acc.getPrimaryAndMerge(MenuTruthThinningCfg(
            configFlags,
            name = "IDTIDETruthThinningTool",
            StreamName                 = kwargs['StreamName'],
            WritePartons               = True,
            WriteHadrons               = True,
            WriteBHadrons              = True, 
            WriteGeant                 = True,
            GeantPhotonPtThresh        = 20000,
            WriteTauHad                = True,
            PartonPtThresh             = -1.0,
            WriteBSM                   = True,
            WriteBosons                = True,
            WriteBosonProducts         = True, 
            WriteBSMProducts           = True,
            WriteTopAndDecays          = True, 
            WriteEverything            = True,
            WriteAllLeptons            = True,
            WriteLeptonsNotFromHadrons = True,
            WriteStatus3               = True,
            WriteFirstN                = -1,
            PreserveAncestors          = True, 
            PreserveGeneratorDescendants = True))
        thinningTools.append(IDTIDETruthThinningTool)
    
    #====================================================================
    # CREATE THE DERIVATION KERNEL ALGORITHM AND PASS THE ABOVE TOOLS  
    #====================================================================
    acc.addEventAlgo(DerivationKernel(
        name, 
        AugmentationTools = augmentationTools,
        SkimmingTools = skimmingTools,
        ThinningTools = [],
        RunSkimmingFirst = True,
        OutputLevel =INFO), sequenceName="IDTIDESequence")   
    #sequenceName = "IDTIDESkimmingSequence" )

    # shared between IDTIDE and IDTRKVALID
    acc.addEventAlgo(DerivationKernel(
        name = "DFTSOSKernel",
        AugmentationTools = tsos_augmentationTools,
        ThinningTools = [],
        OutputLevel =INFO), sequenceName="IDTIDESequence")
    #sequenceName = "IDTIDEPostProcSequence" )
   
    acc.addEventAlgo(DerivationKernel(
        name = "IDTIDEThinningKernel",
        AugmentationTools = [],
        ThinningTools = thinningTools,
        OutputLevel =INFO), sequenceName="IDTIDESequence")
    #sequenceName = "IDTIDEPostProcSequence")
 
    return acc

# Main config
def IDTIDECfg(configFlags):
    """Main config fragment for IDTIDE"""
    acc = ComponentAccumulator()

    # Main algorithm (kernel)
    acc.merge(IDTIDEKernelCfg(configFlags, name="IDTIDEKernel", StreamName = 'StreamDAOD_IDTIDE'))

    # =============================
    # Define contents of the format
    # =============================
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import InfileMetaDataCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    IDTIDESlimmingHelper = SlimmingHelper("IDTIDESlimmingHelper", NamesAndTypes = configFlags.Input.TypedCollections, ConfigFlags = configFlags)

    AllVariables = []
    StaticContent = []
    SmartCollections = []
    ExtraVariables = []

    IDTIDESlimmingHelper.AppendToDictionary.update ({
        "EventInfo":"xAOD::EventInfo", "EventInfoAux":"xAOD::EventAuxInfo",
        "Muons":"xAOD::MuonContainer", "MuonsAux":"xAOD::MuonAuxContainer", 
        "Electrons":"xAOD::ElectronContainer", "ElectronsAux":"xAOD::ElectronAuxContainer",
        "Photons":"xAOD::PhotonContainer", "PhotonsAux":"xAOD::PhotonAuxContainer",
        "JetETMissNeutralParticleFlowObjects":"xAOD::FlowElementContainer", "JetETMissNeutralParticleFlowObjectsAux":"xAOD::FlowElementAuxContainer",
        "JetETMissChargedParticleFlowObjects":"xAOD::FlowElementContainer", "JetETMissChargedParticleFlowObjectsAux":"xAOD::FlowElementAuxContainer",
        "TauJets":"xAOD::TauJetContainer", "TauJetsAux":"xAOD::TauJetAuxContainer",
        "InDetTrackParticles":"xAOD::TrackParticleContainer", "InDetTrackParticlesAux":"xAOD::TrackParticleAuxContainer",
        "InDetLargeD0TrackParticles":"xAOD::TrackParticleContainer", "InDetLargeD0TrackParticlesAux":"xAOD::TrackParticleAuxContainer", 
        "PixelClusters":"xAOD::TrackMeasurementValidationContainer", "PixelClustersAux":"xAOD::TrackMeasurementValidationAuxContainer", 
        "SCT_Clusters":"xAOD::TrackMeasurementValidationContainer", "SCT_ClustersAux":"xAOD::TrackMeasurementValidationAuxContainer", 
        "Kt4EMTopoOriginEventShape":"xAOD::EventShape", "Kt4EMTopoOriginEventShapeAux":"xAOD::EventShapeAuxInfo",
        "Kt4LCTopoOriginEventShape":"xAOD::EventShape", "Kt4LCTopoOriginEventShapeAux":"xAOD::EventShapeAuxInfo",
        "NeutralParticleFlowIsoCentralEventShape":"xAOD::EventShape", "NeutralParticleFlowIsoCentralEventShapeAux":"xAOD::EventShapeAuxInfo",
        "NeutralParticleFlowIsoForwardEventShape":"xAOD::EventShape", "NeutralParticleFlowIsoForwardEventShapeAux":"xAOD::EventShapeAuxInfo",
        "TopoClusterIsoCentralEventShape":"xAOD::EventShape", "TopoClusterIsoCentralEventShapeAux":"xAOD::EventShapeAuxInfo",
        "TopoClusterIsoForwardEventShape":"xAOD::EventShape", "TopoClusterIsoForwardEventShapeAux":"xAOD::EventShapeAuxInfo"}
    )

    SmartCollections += ["Muons", "Electrons", "Photons"]

    AllVariables += ["EventInfo", 
        "JetETMissNeutralParticleFlowObjects", 
        "JetETMissChargedParticleFlowObjects",
        "InDetTrackParticles", 
        "InDetLargeD0TrackParticles", 
        "PixelClusters", 
        "SCT_Clusters", 
        "Kt4EMTopoOriginEventShape", 
        "Kt4LCTopoOriginEventShape", 
        "NeutralParticleFlowIsoCentralEventShape", 
        "NeutralParticleFlowIsoForwardEventShape",
        "TopoClusterIsoCentralEventShape", 
        "TopoClusterIsoForwardEventShape", 
    ]

    IDTIDESlimmingHelper.AppendToDictionary.update({"TauJets":"xAOD::TauJetContainer", "TauJetsAux": "xAOD::TauJetAuxContainer",
        "Kt4EMPFlowEventShape":"xAOD::EventShape", "Kt4EMPFlowEventShapeAux":"xAOD::EventShapeAuxInfo",
        "PrimaryVertices":"xAOD::VertexContainer", "PrimaryVerticesAux":"xAOD::VertexAuxContainer",
        "InDetTrackParticlesClusterAssociations":"xAOD::TrackParticleClusterAssociationContainer", "InDetTrackParticlesClusterAssociationsAux":"xAOD::TrackParticleClusterAssociationAuxContainer",
        "AntiKt4EMTopoJets":"xAOD::JetContainer", "AntiKt4EMTopoJetsAux":"xAOD::JetAuxContainer",
        "AntiKt4EMPFlowJets":"xAOD::JetContainer", "AntiKt4EMPFlowJetsAux":"xAOD::JetAuxContainer",
        "BTagging_AntiKt4EMTopo":"xAOD::BTaggingContainer", "BTagging_AntiKt4EMTopoAux":"xAOD::BTaggingAuxContainer",
        "BTagging_AntiKt4EMPFlow":"xAOD::BTaggingContainer", "BTagging_AntiKt4EMPFlowAux":"xAOD::BTaggingAuxContainer"}
    )

    ExtraVariables += ["TauJets.ABS_ETA_LEAD_TRACK.ClusterTotalEnergy.ClustersMeanCenterLambda.ClustersMeanEMProbability.ClustersMeanFirstEngDens.ClustersMeanPresamplerFrac.ClustersMeanSecondLambda.EMFRACTIONATEMSCALE_MOVEE3.EMFracFixed.GhostMuonSegmentCount.LeadClusterFrac.NNDecayMode.NNDecayModeProb_1p0n.NNDecayModeProb_1p1n.NNDecayModeProb_1pXn.NNDecayModeProb_3p0n.NNDecayModeProb_3pXn.PFOEngRelDiff.PanTau_DecayModeExtended.TAU_ABSDELTAETA.TAU_ABSDELTAPHI.TAU_SEEDTRK_SECMAXSTRIPETOVERPT.UpsilonCluster.absipSigLeadTrk.chargedFELinks.etHotShotDR1.etHotShotDR1OverPtLeadTrk.etHotShotWin.etHotShotWinOverPtLeadTrk.etaCombined.hadLeakFracFixed.leadTrackProbHT.mCombined.mu.nConversionTracks.nFakeTracks.nModifiedIsolationTracks.nVtxPU.neutralFELinks.passThinning.phiCombined.ptCombined.ptIntermediateAxisEM.rho"]
    ExtraVariables += ["PrimaryVertices.sumPt2.x.y.z"]

    AllVariables += ["Kt4EMPFlowEventShape", "InDetTrackParticlesClusterAssociations", 
                     "AntiKt4EMTopoJets", "AntiKt4EMPFlowJets",
                     "BTagging_AntiKt4EMTopo", "BTagging_AntiKt4EMPFlow"]

    if configFlags.Detector.EnablePixel:     
        IDTIDESlimmingHelper.AppendToDictionary.update( {'PixelMSOSs':'xAOD::TrackStateValidationContainer','PixelMSOSsAux':'xAOD::TrackStateValidationAuxContainer'} )
        AllVariables += ["PixelMSOSs"]
  
    if configFlags.Detector.EnableSCT:
        IDTIDESlimmingHelper.AppendToDictionary.update( {'SCT_MSOSs':'xAOD::TrackStateValidationContainer','SCT_MSOSsAux':'xAOD::TrackStateValidationAuxContainer'} )
        AllVariables += ["SCT_MSOSs"]
  
    if configFlags.Detector.EnableTRT:
        IDTIDESlimmingHelper.AppendToDictionary.update( {'TRT_MSOSs':'xAOD::TrackStateValidationContainer','TRT_MSOSsAux':'xAOD::TrackStateValidationAuxContainer'} )
        AllVariables += ["TRT_MSOSs"]
  
    if configFlags.Input.isMC:

        IDTIDESlimmingHelper.AppendToDictionary.update({"AntiKt4TruthJets":"xAOD::JetContainer", "AntiKt4TruthJetsAux":"xAOD::JetAuxContainer",
            "JetInputTruthParticles":"xAOD::TruthParticleContainer",
            "JetInputTruthParticlesNoWZ":"xAOD::TruthParticleContainer",
            "TruthEvents":"xAOD::TruthEventContainer", "TruthEventsAux":"xAOD::TruthEventAuxContainer",
            "TruthParticles":"xAOD::TruthParticleContainer", "TruthParticlesAux":"xAOD::TruthParticleAuxContainer",
            "egammaTruthParticles":"xAOD::TruthParticleContainer", "egammaTruthParticlesAux":"xAOD::TruthParticleAuxContainer",
            "MuonTruthParticles":"xAOD::TruthParticleContainer", "MuonTruthParticlesAux":"xAOD::TruthParticleAuxContainer",
            "LRTegammaTruthParticles":"xAOD::TruthParticleContainer", "LRTegammaTruthParticlesAux":"xAOD::TruthParticleAuxContainer",
            "TruthVertices":"xAOD::TruthVertexContainer", "TruthVerticesAux":"xAOD::TruthVertexAuxContainer"}
        )
    
        AllVariables += ["AntiKt4TruthJets", 
                         "JetInputTruthParticles",
                         "JetInputTruthParticlesNoWZ",
                         "TruthEvents", 
                         "TruthParticles",
                         "egammaTruthParticles",
                         "MuonTruthParticles",
                         "LRTegammaTruthParticles",
                         "TruthVertices" 
                        ]

        list_aux = ["BHadronsFinal", "BHadronsInitial", "BQuarksFinal",  
                    "CHadronsFinal", "CHadronsInitial", "CQuarksFinal",
                    "HBosons", "Partons", "TQuarksFinal", "TausFinal", "WBosons", "ZBosons"] 
        for item in list_aux:
            label = "TruthLabel"+item
            labelAux = label+"Aux"
            IDTIDESlimmingHelper.AppendToDictionary.update( { label : "xAOD::TruthParticleContainer", labelAux : "xAOD::TruthParticleAuxContainer"} )
            AllVariables += [label]
    # End of isMC block      

    IDTIDESlimmingHelper.IncludeTriggerNavigation = True   # Trigger info is actually stored only when running on data...
    IDTIDESlimmingHelper.IncludeAdditionalTriggerContent = True 
    
    IDTIDESlimmingHelper.AllVariables = AllVariables
    IDTIDESlimmingHelper.StaticContent = StaticContent
    IDTIDESlimmingHelper.SmartCollections = SmartCollections
    IDTIDESlimmingHelper.ExtraVariables = ExtraVariables
    
    # Output stream    
    IDTIDEItemList = IDTIDESlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(configFlags, "DAOD_IDTIDE", ItemList=IDTIDEItemList, AcceptAlgs=["IDTIDEKernel"]))
    acc.merge(InfileMetaDataCfg(configFlags, "DAOD_IDTIDE", AcceptAlgs=["IDTIDEKernel"]))

    return acc


