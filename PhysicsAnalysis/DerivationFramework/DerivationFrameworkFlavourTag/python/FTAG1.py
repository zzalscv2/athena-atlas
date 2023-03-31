# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#!/usr/bin/env python
#====================================================================
# DAOD_FTAG1.py
# This defines DAOD_FTAG1, an unskimmed DAOD format for Run 3.
# It contains the variables and objects needed for the large majority 
# of physics analyses in ATLAS.
# It requires the flag FTAG1 in Derivation_tf.py   
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


# Main algorithm config
def FTAG1KernelCfg(flags, name='FTAG1Kernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel) for FTAG1"""
    acc = ComponentAccumulator()

    # Common augmentations
    from DerivationFrameworkPhys.PhysCommonConfig import PhysCommonAugmentationsCfg
    acc.merge(PhysCommonAugmentationsCfg(flags, TriggerListsHelper = kwargs['TriggerListsHelper']))

    augmentationTools = []
    # Add V0Tool
    if flags.BTagging.AddV0Finder:
        acc.merge(V0ToolCfg(flags, augmentationTools=augmentationTools, tool_name_prefix="FTAG1", container_name_prefix="FTAG"))

    # thinning tools
    thinningTools = []

    # Finally the kernel itself
    DerivationKernel = CompFactory.DerivationFramework.DerivationKernel
    acc.addEventAlgo(DerivationKernel(name, AugmentationTools = augmentationTools, ThinningTools = thinningTools))       
    return acc


def FTAG1CoreCfg(flags, name_tag='FTAG1', extra_SmartCollections=None, extra_AllVariables=None, trigger_option=''):

    if extra_SmartCollections is None: extra_SmartCollections = []
    if extra_AllVariables is None: extra_AllVariables = []


    acc = ComponentAccumulator()

    # ============================
    # Define contents of the format
    # =============================
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import InfileMetaDataCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    
    FTAG1SlimmingHelper = SlimmingHelper(name_tag+"SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)

    # Many of these are added to AllVariables below as well. We add
    # these items in both places in case some of the smart collections
    # add variables from some other collection. For flavor tagging,
    # for example will add jet variables.
    FTAG1SlimmingHelper.SmartCollections = [
                                           "Electrons",
                                           "Muons",
                                           "PrimaryVertices",
                                           "InDetTrackParticles",
                                           "AntiKt4EMPFlowJets",
                                           "BTagging_AntiKt4EMPFlow",
                                           "AntiKtVR30Rmax4Rmin02PV0TrackJets",
                                           "BTagging_AntiKtVR30Rmax4Rmin02Track",
                                           "MET_Baseline_AntiKt4EMPFlow",
                                           "AntiKt10UFOCSSKJets",
                                           "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets",
                                          ]
    if len(extra_SmartCollections)>0:
        for a_container in extra_SmartCollections:
            if a_container not in FTAG1SlimmingHelper.SmartCollections:
                FTAG1SlimmingHelper.SmartCollections.append(a_container)

    FTAG1SlimmingHelper.AllVariables = [
            "EventInfo",
            "PrimaryVertices",
            "InDetTrackParticles",
            "InDetLargeD0TrackParticles",
            "AntiKt4EMPFlowJets",
            "AntiKtVR30Rmax4Rmin02PV0TrackJets",
            "BTagging_AntiKt4EMPFlow",
            "BTagging_AntiKtVR30Rmax4Rmin02Track",
            "BTagging_AntiKt4EMPFlowJFVtx",
            "BTagging_AntiKt4EMPFlowSecVtx",
            "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets",
            "UFOCSSK",
            "GlobalChargedParticleFlowObjects",
            "GlobalNeutralParticleFlowObjects",
            "CHSGChargedParticleFlowObjects",
            "CHSGNeutralParticleFlowObjects",
            "TruthParticles",
            "TruthVertices",
            "TruthBottom", "TruthElectrons","TruthMuons","TruthTaus",
            ]
    if len(extra_AllVariables)>0:
        for a_container in extra_AllVariables:
            if a_container not in FTAG1SlimmingHelper.AllVariables:
                FTAG1SlimmingHelper.AllVariables.append(a_container)

    if flags.BTagging.Pseudotrack:
        FTAG1SlimmingHelper.AllVariables += [ "InDetPseudoTrackParticles" ]

    if flags.BTagging.Trackless:
        FTAG1SlimmingHelper.AllVariables += [
                "JetAssociatedPixelClusters",
                "JetAssociatedSCTClusters",
                ]

    if flags.BTagging.RunNewVrtSecInclusive:
        FTAG1SlimmingHelper.AppendToDictionary.update({'NVSI_SecVrt_Tight' : 'xAOD::VertexContainer','NVSI_SecVrt_TightAux' : 'xAOD::VertexAuxContainer',
                                                       'NVSI_SecVrt_Medium' : 'xAOD::VertexContainer','NVSI_SecVrt_MediumAux' : 'xAOD::VertexAuxContainer',
                                                       'NVSI_SecVrt_Loose' : 'xAOD::VertexContainer','NVSI_SecVrt_LooseAux' : 'xAOD::VertexAuxContainer'})

    # Append to dictionary


    from DerivationFrameworkFlavourTag import FtagBaseContent

    # Static content
    StaticContent = []
    if flags.BTagging.AddV0Finder:
        FTAGV0ContainerName = "FTAGRecoV0Candidates"
        FTAGKshortContainerName = "FTAGRecoKshortCandidates"
        FTAGLambdaContainerName = "FTAGRecoLambdaCandidates"
        FTAGLambdabarContainerName = "FTAGRecoLambdabarCandidates"
        StaticContent += ["xAOD::VertexContainer#%s"        %                 FTAGV0ContainerName]
        StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % FTAGV0ContainerName]
        StaticContent += ["xAOD::VertexContainer#%s"        %                 FTAGKshortContainerName]
        StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % FTAGKshortContainerName]
        StaticContent += ["xAOD::VertexContainer#%s"        %                 FTAGLambdaContainerName]
        StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % FTAGLambdaContainerName]
        StaticContent += ["xAOD::VertexContainer#%s"        %                 FTAGLambdabarContainerName]
        StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % FTAGLambdabarContainerName]
        CascadeCollections = []
        CascadeCollections += ["FTAGJpsiKshortCascadeSV2", "FTAGJpsiKshortCascadeSV1"]
        CascadeCollections += ["FTAGJpsiLambdaCascadeSV2", "FTAGJpsiLambdaCascadeSV1"]
        CascadeCollections += ["FTAGJpsiLambdabarCascadeSV2", "FTAGJpsiLambdabarCascadeSV1"]
        for cascades in CascadeCollections:
            StaticContent += ["xAOD::VertexContainer#%s"   %     cascades]
            StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % cascades]


    
    FtagBaseContent.add_static_content_to_SlimmingHelper(FTAG1SlimmingHelper, StaticContent)


    if flags.BTagging.RunNewVrtSecInclusive:
        excludedVertexAuxData = "-vxTrackAtVertex.-MvfFitInfo.-isInitialized.-VTAV"
        FTAG1SlimmingHelper.StaticContent += ["xAOD::VertexContainer#NVSI_SecVrt_Loose", "xAOD::VertexContainer#NVSI_SecVrt_Medium", "xAOD::VertexContainer#NVSI_SecVrt_Tight"]
        FTAG1SlimmingHelper.StaticContent += ["xAOD::VertexAuxContainer#NVSI_SecVrt_LooseAux."+excludedVertexAuxData]
        FTAG1SlimmingHelper.StaticContent += ["xAOD::VertexAuxContainer#NVSI_SecVrt_MediumAux."+excludedVertexAuxData ]
        FTAG1SlimmingHelper.StaticContent += ["xAOD::VertexAuxContainer#NVSI_SecVrt_TightAux."+excludedVertexAuxData]

    # Add truth containers
    if flags.Input.isMC:
        FtagBaseContent.add_truth_to_SlimmingHelper(FTAG1SlimmingHelper)

    # Add ExtraVariables
    FtagBaseContent.add_ExtraVariables_to_SlimmingHelper(FTAG1SlimmingHelper)
   
    # Trigger content
    FtagBaseContent.trigger_setup(FTAG1SlimmingHelper, trigger_option)


    # Output stream    
    FTAG1ItemList = FTAG1SlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(flags, "DAOD_"+name_tag, ItemList=FTAG1ItemList, AcceptAlgs=[name_tag+"Kernel"]))
    acc.merge(InfileMetaDataCfg(flags, "DAOD_"+name_tag, AcceptAlgs=[name_tag+"Kernel"]))

    return acc

def FTAG1Cfg(flags):

    acc = ComponentAccumulator()

    # Get the lists of triggers needed for trigger matching.
    # This is needed at this scope (for the slimming) and further down in the config chain
    # for actually configuring the matching, so we create it here and pass it down
    # TODO: this should ideally be called higher up to avoid it being run multiple times in a train
    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
    FTAG1TriggerListsHelper = TriggerListsHelper(flags)
   
    # name_tag has to be consistent between KernelCfg and CoreCfg
    FTAG1_name_tag = 'FTAG1'

    # Common augmentations
    acc.merge(FTAG1KernelCfg(flags, name=FTAG1_name_tag + "Kernel", StreamName = 'StreamDAOD_'+FTAG1_name_tag, TriggerListsHelper = FTAG1TriggerListsHelper))
    # Content of FTAG1 
    acc.merge(FTAG1CoreCfg(flags, FTAG1_name_tag))

    return acc


def V0ToolCfg(flags, augmentationTools=None, tool_name_prefix="FTAG1", container_name_prefix="FTAG"):
    
    acc = ComponentAccumulator()
    
    if augmentationTools is None:
        augmentationTools = []
    
    from DerivationFrameworkBPhys.commonBPHYMethodsCfg import BPHY_V0ToolCfg, BPHY_InDetDetailedTrackSelectorToolCfg, BPHY_VertexPointEstimatorCfg, BPHY_TrkVKalVrtFitterCfg
    from JpsiUpsilonTools.JpsiUpsilonToolsConfig import PrimaryVertexRefittingToolCfg, JpsiFinderCfg
    from TrkConfig.AtlasExtrapolatorConfig import InDetExtrapolatorCfg

    V0Tools = acc.popToolsAndMerge(BPHY_V0ToolCfg(flags, tool_name_prefix))
    acc.addPublicTool(V0Tools)
    vkalvrt = acc.popToolsAndMerge(BPHY_TrkVKalVrtFitterCfg(flags, tool_name_prefix))        # VKalVrt vertex fitter
    acc.addPublicTool(vkalvrt)
    trackselect = acc.popToolsAndMerge(BPHY_InDetDetailedTrackSelectorToolCfg(flags, tool_name_prefix))
    acc.addPublicTool(trackselect)
    vpest = acc.popToolsAndMerge(BPHY_VertexPointEstimatorCfg(flags, tool_name_prefix))
    acc.addPublicTool(vpest)
    JpsiFinder = acc.popToolsAndMerge(JpsiFinderCfg(flags,
            name                        = tool_name_prefix+"JpsiFinder",
            muAndMu                     = True,
            muAndTrack                  = False,
            TrackAndTrack               = False,
            assumeDiMuons               = True,
            invMassUpper                = 4000.0,
            invMassLower                = 2600.0,
            Chi2Cut                     = 200.,
            oppChargesOnly              = True,
            combOnly                    = True,
            atLeastOneComb              = False,
            useCombinedMeasurement      = False, # Only takes effect if combOnly=True   
            muonCollectionKey           = "Muons",
            TrackParticleCollection     = "InDetTrackParticles",
            V0VertexFitterTool          = None,             # V0 vertex fitter
            useV0Fitter                 = False,                   # if False a TrkVertexFitterTool will be used
            TrkVertexFitterTool         = vkalvrt,        # VKalVrt vertex fitter
            TrackSelectorTool           = trackselect,
            VertexPointEstimator        = vpest,
            useMCPCuts                  = False))
    acc.addPublicTool(JpsiFinder)
    JpsiSelectAndWrite   = CompFactory.DerivationFramework.Reco_Vertex(
            name                   = tool_name_prefix+"JpsiSelectAndWrite",
            VertexSearchTool       = JpsiFinder,
            OutputVtxContainerName = container_name_prefix+"JpsiCandidates",
            PVContainerName        = "PrimaryVertices",
            V0Tools                = V0Tools,
            PVRefitter             = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags)),
            RefPVContainerName     = "SHOULDNOTBEUSED",
            DoVertexType = 1)
    Select_Jpsi2mumu = CompFactory.DerivationFramework.Select_onia2mumu(
            name                  = tool_name_prefix+"_Select_Jpsi2mumu",
            HypothesisName        = "Jpsi",
            InputVtxContainerName = container_name_prefix+"JpsiCandidates",
            V0Tools               = V0Tools,
            VtxMassHypo           = 3096.916,
            MassMin               = 2600.0,
            MassMax               = 4000.0,
            Chi2Max               = 200,
            DoVertexType =1)
    V0ContainerName = container_name_prefix+"RecoV0Candidates"
    KshortContainerName = container_name_prefix+"RecoKshortCandidates"
    LambdaContainerName = container_name_prefix+"RecoLambdaCandidates"
    LambdabarContainerName = container_name_prefix+"RecoLambdabarCandidates"

    from InDetConfig.InDetV0FinderConfig import V0MainDecoratorCfg
    V0Decorator = acc.popToolsAndMerge(V0MainDecoratorCfg(
            flags,
            name = tool_name_prefix+"V0Decorator",
            V0Tools = V0Tools,
            V0ContainerName = V0ContainerName,
            KshortContainerName = KshortContainerName,
            LambdaContainerName = LambdaContainerName,
            LambdabarContainerName = LambdabarContainerName))
    acc.addPublicTool(V0Decorator)

    from DerivationFrameworkBPhys.V0ToolConfig import BPHY_InDetV0FinderToolCfg
    Reco_V0Finder   = CompFactory.DerivationFramework.Reco_V0Finder(
            name                   = tool_name_prefix+"_Reco_V0Finder",
            V0FinderTool           = acc.popToolsAndMerge(BPHY_InDetV0FinderToolCfg(flags,tool_name_prefix,
                V0ContainerName = V0ContainerName,
                KshortContainerName = KshortContainerName,
                LambdaContainerName = LambdaContainerName,
                LambdabarContainerName = LambdabarContainerName)),
            Decorator              = V0Decorator,
            V0ContainerName        = V0ContainerName,
            KshortContainerName    = KshortContainerName,
            LambdaContainerName    = LambdaContainerName,
            LambdabarContainerName = LambdabarContainerName,
            CheckVertexContainers  = [container_name_prefix+'JpsiCandidates'])
    JpsiV0VertexFit = CompFactory.Trk.TrkVKalVrtFitter(
            name                 = "JpsiV0VertexFit",
            Extrapolator         = acc.popToolsAndMerge(InDetExtrapolatorCfg(flags)),
            FirstMeasuredPoint   = False,
            CascadeCnstPrecision = 1e-6,
            MakeExtendedVertex   = True)
    acc.addPublicTool(JpsiV0VertexFit)
    JpsiKshort  = CompFactory.DerivationFramework.JpsiPlusV0Cascade(
            name                    = tool_name_prefix+"JpsiKshort",
            V0Tools                 = V0Tools,
            HypothesisName          = "Bd",
            TrkVertexFitterTool     = JpsiV0VertexFit,
            V0Hypothesis            = 310,
            PVRefitter             = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags)),
            JpsiMassLowerCut        = 2800.,
            JpsiMassUpperCut        = 4000.,
            V0MassLowerCut          = 400.,
            V0MassUpperCut          = 600.,
            MassLowerCut            = 4300.,
            MassUpperCut            = 6300.,
            RefitPV                 = True,
            RefPVContainerName      = container_name_prefix+"RefittedPrimaryVertices2",
            JpsiVertices            = container_name_prefix+"JpsiCandidates",
            CascadeVertexCollections= [container_name_prefix+"JpsiKshortCascadeSV2", container_name_prefix+"JpsiKshortCascadeSV1"],
            V0Vertices              = V0ContainerName)

    JpsiLambda   = CompFactory.DerivationFramework.JpsiPlusV0Cascade(
            name                    = tool_name_prefix+"JpsiLambda",
            V0Tools                 = V0Tools,
            HypothesisName          = "Lambda_b",
            TrkVertexFitterTool     = JpsiV0VertexFit,
            PVRefitter             = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags)),
            V0Hypothesis            = 3122,
            JpsiMassLowerCut        = 2800.,
            JpsiMassUpperCut        = 4000.,
            V0MassLowerCut          = 1050.,
            V0MassUpperCut          = 1250.,
            MassLowerCut            = 4600.,
            MassUpperCut            = 6600.,
            RefitPV                 = True,
            RefPVContainerName      = container_name_prefix+"RefittedPrimaryVertices3",
            JpsiVertices            = container_name_prefix+"JpsiCandidates",
            CascadeVertexCollections= [container_name_prefix+"JpsiLambdaCascadeSV2", container_name_prefix+"JpsiLambdaCascadeSV1"],
            V0Vertices              = V0ContainerName)
    JpsiLambdabar         = CompFactory.DerivationFramework.JpsiPlusV0Cascade(
            name                    = tool_name_prefix+"JpsiLambdabar",
            HypothesisName          = "Lambda_bbar",
            V0Tools                 = V0Tools,
            TrkVertexFitterTool     = JpsiV0VertexFit,
            PVRefitter             = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags)),
            V0Hypothesis            = -3122,
            JpsiMassLowerCut        = 2800.,
            JpsiMassUpperCut        = 4000.,
            V0MassLowerCut          = 1050.,
            V0MassUpperCut          = 1250.,
            MassLowerCut            = 4600.,
            MassUpperCut            = 6600.,
            RefitPV                 = True,
            RefPVContainerName      = container_name_prefix+"RefittedPrimaryVertices4",
            JpsiVertices            = container_name_prefix+"JpsiCandidates",
            CascadeVertexCollections= [container_name_prefix+"JpsiLambdabarCascadeSV2", container_name_prefix+"JpsiLambdabarCascadeSV1"],
            V0Vertices              = V0ContainerName)

    _augmentationTools = [JpsiSelectAndWrite,  Select_Jpsi2mumu,
            Reco_V0Finder, JpsiKshort, JpsiLambda, JpsiLambdabar,
            ]
    for t in  _augmentationTools : acc.addPublicTool(t)
    augmentationTools += _augmentationTools
    return acc
