# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# ====================================================================
# IDTR2.py
# ====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory

streamName = "StreamDAOD_IDTR2"


def IDTR2Cfg(flags):

    acc = ComponentAccumulator()

    # LRT merge
    from DerivationFrameworkInDet.InDetToolsConfig import (
        TrackParticleMergerCfg)
    IDTR2TrackParticleMergerTool = acc.getPrimaryAndMerge(
        TrackParticleMergerCfg(
            flags,
            name="IDTR2TrackParticleMergerTool",
            TrackParticleLocation=["InDetTrackParticles",
                                   "InDetLargeD0TrackParticles"],
            OutputTrackParticleLocation="InDetWithLRTTrackParticles",
            CreateViewColllection=True))

    LRTMergeAug = CompFactory.DerivationFramework.CommonAugmentation(
        "InDetLRTMerge", AugmentationTools=[IDTR2TrackParticleMergerTool])
    acc.addEventAlgo(LRTMergeAug)

    # VrtSecInclusive
    from VrtSecInclusive.VrtSecInclusiveConfig import VrtSecInclusiveCfg
    acc.merge(VrtSecInclusiveCfg(
        flags,
        name="VrtSecInclusive",
        AugmentingVersionString="",
        FillIntermediateVertices=False,
        TrackLocation="InDetWithLRTTrackParticles"))

    # NewVrtSecInclusive
    from NewVrtSecInclusiveTool.NewVrtSecInclusiveConfig import (
        MaterialSVFinderToolCfg, DVFinderToolCfg)
    MaterialSVFinderTool = acc.popToolsAndMerge(
        MaterialSVFinderToolCfg(flags))
    acc.addEventAlgo(CompFactory.Rec.NewVrtSecInclusiveAlg(
        name="NewVrtSecInclusive_Material",
        TrackParticleContainer="InDetWithLRTTrackParticles",
        PrimaryVertexContainer="PrimaryVertices",
        BVertexContainerName="NewVrtSecInclusive_SecondaryVertices_Material",
        BVertexTool=MaterialSVFinderTool))

    DVFinderTool = acc.popToolsAndMerge(DVFinderToolCfg(flags))
    acc.addEventAlgo(CompFactory.Rec.NewVrtSecInclusiveAlg(
        name="NewVrtSecInclusive_DV",
        TrackParticleContainer="InDetWithLRTTrackParticles",
        PrimaryVertexContainer="PrimaryVertices",
        BVertexContainerName="NewVrtSecInclusive_SecondaryVertices_DV",
        BVertexTool=DVFinderTool))

    # V0Finder
    IDTR2V0ContainerName = "IDTR2RecoV0Candidates"
    IDTR2KshortContainerName = "IDTR2RecoKshortCandidates"
    IDTR2LambdaContainerName = "IDTR2RecoLambdaCandidates"
    IDTR2LambdabarContainerName = "IDTR2RecoLambdabarCandidates"

    from InDetConfig.InDetV0FinderConfig import V0MainDecoratorCfg
    V0Decorator = acc.popToolsAndMerge(V0MainDecoratorCfg(
        flags,
        name="IDTR2V0Decorator",
        V0ContainerName=IDTR2V0ContainerName,
        KshortContainerName=IDTR2KshortContainerName,
        LambdaContainerName=IDTR2LambdaContainerName,
        LambdabarContainerName=IDTR2LambdabarContainerName))

    from InDetConfig.InDetV0FinderConfig import IDTR2_V0FinderToolCfg
    V0FinderTool = acc.popToolsAndMerge(IDTR2_V0FinderToolCfg(
        flags,
        name="IDTR2_V0FinderTool",
        TrackParticleCollection="InDetWithLRTTrackParticles",
        V0ContainerName=IDTR2V0ContainerName,
        KshortContainerName=IDTR2KshortContainerName,
        LambdaContainerName=IDTR2LambdaContainerName,
        LambdabarContainerName=IDTR2LambdabarContainerName))

    IDTR2_Reco_V0Finder = CompFactory.DerivationFramework.Reco_V0Finder(
        name="IDTR2_Reco_V0Finder",
        V0FinderTool=V0FinderTool,
        Decorator=V0Decorator,
        V0ContainerName=IDTR2V0ContainerName,
        KshortContainerName=IDTR2KshortContainerName,
        LambdaContainerName=IDTR2LambdaContainerName,
        LambdabarContainerName=IDTR2LambdabarContainerName,
        CheckVertexContainers=['PrimaryVertices'])

    from InDetTrackSystematicsTools.InDetTrackSystematicsToolsConfig import TrackSystematicsAlgCfg
    acc.merge(TrackSystematicsAlgCfg(
        flags,
        InputTrackContainer="InDetWithLRTTrackParticles",
        OutputTrackContainer=(
            "InDetWithLRTTrackParticles_TRK_EFF_LARGED0_GLOBAL__1down")))

    V0FinderToolSyst = acc.popToolsAndMerge(IDTR2_V0FinderToolCfg(
        flags,
        name="IDTR2_V0FinderTool_Syst",
        TrackParticleCollection=(
            "InDetWithLRTTrackParticles_TRK_EFF_LARGED0_GLOBAL__1down"),
        V0ContainerName=IDTR2V0ContainerName + "Syst",
        KshortContainerName=IDTR2KshortContainerName + "Syst",
        LambdaContainerName=IDTR2LambdaContainerName + "Syst",
        LambdabarContainerName=IDTR2LambdabarContainerName + "Syst"))

    IDTR2_Reco_V0FinderSyst = CompFactory.DerivationFramework.Reco_V0Finder(
        name="IDTR2_Reco_V0Finder_Syst",
        V0FinderTool=V0FinderToolSyst,
        Decorator=V0Decorator,
        V0ContainerName=IDTR2V0ContainerName + "Syst",
        KshortContainerName=IDTR2KshortContainerName + "Syst",
        LambdaContainerName=IDTR2LambdaContainerName + "Syst",
        LambdabarContainerName=IDTR2LambdabarContainerName + "Syst",
        CheckVertexContainers=['PrimaryVertices'])

    skimmingTools = []
    augmentationTools = [IDTR2_Reco_V0Finder, IDTR2_Reco_V0FinderSyst]
    for t in augmentationTools:
        acc.addPublicTool(t)

    # Define the main kernel
    DerivationKernel = CompFactory.DerivationFramework.DerivationKernel
    acc.addEventAlgo(DerivationKernel("IDTR2Kernel",
                                      AugmentationTools=augmentationTools,
                                      SkimmingTools=skimmingTools))

    # ============================
    # Define contents of the format
    # =============================
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper

    IDTR2SlimmingHelper = SlimmingHelper(
        "IDTR2SlimmingHelper",
        NamesAndTypes=flags.Input.TypedCollections,
        flags=flags)

    IDTR2SlimmingHelper.AllVariables = [
        "EventInfo",
        "PrimaryVertices",
        "InDetTrackParticles",
        "InDetLargeD0TrackParticles",
        "TruthParticles",
        "TruthVertices"
    ]

    StaticContent = []
    StaticContent += ["xAOD::VertexContainer#VrtSecInclusive_SecondaryVertices"]
    StaticContent += ["xAOD::VertexAuxContainer#VrtSecInclusive_SecondaryVerticesAux."]

    for vertexContainer in ["NewVrtSecInclusive_SecondaryVertices_DV",
                            "NewVrtSecInclusive_SecondaryVertices_Material",
                            "IDTR2RecoV0Candidates",
                            "IDTR2RecoKshortCandidates",
                            "IDTR2RecoLambdaCandidates",
                            "IDTR2RecoLambdabarCandidates",
                            "IDTR2RecoV0CandidatesSyst",
                            "IDTR2RecoKshortCandidatesSyst",
                            "IDTR2RecoLambdaCandidatesSyst",
                            "IDTR2RecoLambdabarCandidatesSyst"]:
        StaticContent += ["xAOD::VertexContainer#%s" % vertexContainer]
        StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" %
                          vertexContainer]

    IDTR2SlimmingHelper.StaticContent = StaticContent

    IDTR2ItemList = IDTR2SlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(flags, "DAOD_IDTR2",
                              ItemList=IDTR2ItemList,
                              AcceptAlgs=["IDTR2Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_IDTR2",
                                AcceptAlgs=["IDTR2Kernel"],
                                createMetadata=[MetadataCategory.CutFlowMetaData]))

    return acc
