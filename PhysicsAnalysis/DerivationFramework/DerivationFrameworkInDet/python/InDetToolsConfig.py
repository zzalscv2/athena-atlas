# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# ==============================================================================
# Provides configs for the tools used for building/thinning tracking related
# object containers and decorations in the DAODs
# ==============================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import BeamType

# Track collection merger


def TrackParticleMergerCfg(
        flags, name="TrackParticleMerger", **kwargs):
    """Configure the track particle merger tool"""
    acc = ComponentAccumulator()

    kwargs.setdefault("TrackParticleLocation",
                      ["InDetTrackParticles", "InDetLargeD0TrackParticles"])
    kwargs.setdefault("OutputTrackParticleLocation",
                      "InDetWithLRTTrackParticles")
    kwargs.setdefault("CreateViewColllection", True)

    acc.addPublicTool(CompFactory.DerivationFramework.TrackParticleMerger(
        name, **kwargs), primary=True)
    return acc

def InDetLRTMergeCfg(flags, name="InDetLRTMerge", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("AugmentationTools",
                      [acc.getPrimaryAndMerge(TrackParticleMergerCfg(flags))])
    acc.addEventAlgo(
        CompFactory.DerivationFramework.CommonAugmentation(name, **kwargs))
    return acc

# Used in vertex fit track decorator


def UsedInVertexFitTrackDecoratorCfg(flags, name, **kwargs):
    """Configure the UsedInVertexFitTrackDecorator"""
    acc = ComponentAccumulator()
    acc.addPublicTool(
        CompFactory.DerivationFramework.UsedInVertexFitTrackDecorator(
            name, **kwargs), primary=True)
    return acc



def HardScatterVertexDecoratorCfg(flags, name = "DFCommonHSDecorator", **kwargs):
    """Configure the hard process vertex decorator"""
    acc = ComponentAccumulator()
    from InDetConfig.InDetHardScatterSelectionToolConfig import InDetHardScatterSelectionToolCfg     
    kwargs.setdefault("HardScatterSelectionTool", acc.getPrimaryAndMerge(InDetHardScatterSelectionToolCfg(flags, name = "HSSelectionTool",
                                                                                                                ReturnDeco = False)))
    kwargs.setdefault("VertexContainerName", "PrimaryVertices")
    kwargs.setdefault("HardScatterDecoName", "hardScatterVertexLink")
    the_tool = CompFactory.DerivationFramework.HardScatterVertexDecorator(name = "HardScatterDecorTool", **kwargs) 
    acc.addPublicTool(the_tool, primary=True)
    the_alg = CompFactory.DerivationFramework.CommonAugmentation(name, AugmentationTools=[the_tool])
    acc.addEventAlgo(the_alg)
    return acc

# TrackStateOnSurface decorator

def TrackStateOnSurfaceDecoratorCfg(flags, name, **kwargs):
    """Configure the TSOS decorator"""
    acc = ComponentAccumulator()

    # To produce SCT_DetectorElementCollection
    from SCT_GeoModel.SCT_GeoModelConfig import SCT_ReadoutGeometryCfg
    acc.merge(SCT_ReadoutGeometryCfg(flags))

    kwargs.setdefault("DecorationPrefix", "notSet")

    if "TrackExtrapolator" not in kwargs:
        from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
        AtlasExtrapolator = acc.popToolsAndMerge(AtlasExtrapolatorCfg(flags))
        acc.addPublicTool(AtlasExtrapolator)
        kwargs.setdefault("TrackExtrapolator", AtlasExtrapolator)

    if "HoleSearch" not in kwargs:
        from InDetConfig.InDetTrackHoleSearchConfig import (
            InDetTrackHoleSearchToolCfg)
        InDetHoleSearchTool = acc.popToolsAndMerge(
            InDetTrackHoleSearchToolCfg(flags))
        acc.addPublicTool(InDetHoleSearchTool)
        kwargs.setdefault("HoleSearch", InDetHoleSearchTool)

    kwargs.setdefault("IsSimulation", flags.Input.isMC)
    kwargs.setdefault("StorePixel", flags.Tracking.writeExtendedSi_PRDInfo)
    kwargs.setdefault("StoreSCT", flags.Tracking.writeExtendedSi_PRDInfo)
    kwargs.setdefault("StoreTRT", flags.Tracking.writeExtendedTRT_PRDInfo)
    kwargs.setdefault("AddExtraEventInfo", flags.Beam.Type is BeamType.Cosmics)

    if kwargs["StoreTRT"] and "TRT_ToT_dEdx" not in kwargs:
        from InDetConfig.TRT_ElectronPidToolsConfig import TRT_dEdxToolCfg
        InDetTRT_dEdxTool = acc.popToolsAndMerge(TRT_dEdxToolCfg(flags))
        acc.addPublicTool(InDetTRT_dEdxTool)
        kwargs.setdefault("TRT_ToT_dEdx", InDetTRT_dEdxTool)

    kwargs.setdefault("DecorationPrefix", "")
    kwargs.setdefault("PRDtoTrackMap", "PRDtoTrackMapCombinedInDetTracks")

    acc.addPublicTool(
        CompFactory.DerivationFramework.TrackStateOnSurfaceDecorator(
            name, **kwargs), primary=True)
    return acc

def TSOS_CommonKernelCfg(flags, name="TSOS_CommonKernel",
                         listOfExtensions=[]):
    acc = ComponentAccumulator()

    listOfAugmTools = []
    for extension in listOfExtensions:
        TrackStateOnSurfaceDecorator = acc.getPrimaryAndMerge(
            TrackStateOnSurfaceDecoratorCfg(
                flags, name = f"{extension}TrackStateOnSurfaceDecorator",
                ContainerName = f"InDet{extension}TrackParticles",
                PixelMsosName = f"{extension}Pixel_MSOSs",
                SctMsosName = f"{extension}SCT_MSOSs",
                TrtMsosName = f"{extension}TRT_MSOSs"))
        TrackStateOnSurfaceDecorator.DecorationPrefix = "Reco_"
        listOfAugmTools.append(TrackStateOnSurfaceDecorator)

    acc.addEventAlgo(CompFactory.DerivationFramework.CommonAugmentation(
        name, AugmentationTools=listOfAugmTools))
    return acc

def ObserverTrackStateOnSurfaceDecoratorCfg(
        flags, name="ObserverTrackStateOnSurfaceDecorator", **kwargs):
    kwargs.setdefault("ContainerName", "InDetObservedTrackParticles")
    kwargs.setdefault("DecorationPrefix", "ObservedTrack_")
    kwargs.setdefault("PixelMsosName", "ObservedTrack_Pixel_MSOSs")
    kwargs.setdefault("SctMsosName", "ObservedTrack_SCT_MSOSs")
    kwargs.setdefault("TrtMsosName", "ObservedTrack_TRT_MSOSs")
    kwargs.setdefault("AddPRD", True)
    kwargs.setdefault("StoreHoles", False)
    return TrackStateOnSurfaceDecoratorCfg(flags, name, **kwargs)

def ObserverTSOS_CommonKernelCfg(flags, name="ObserverTSOS_CommonKernel"):
    acc = ComponentAccumulator()
    ObserverTrackStateOnSurfaceDecorator = acc.getPrimaryAndMerge(
        ObserverTrackStateOnSurfaceDecoratorCfg(flags))
    acc.addEventAlgo(CompFactory.DerivationFramework.CommonAugmentation(
        name, AugmentationTools=[ObserverTrackStateOnSurfaceDecorator]))
    return acc

def PseudoTrackStateOnSurfaceDecoratorCfg(
        flags, name="PseudoTrackStateOnSurfaceDecorator", **kwargs):
    kwargs.setdefault("ContainerName", "InDetPseudoTrackParticles")
    kwargs.setdefault("DecorationPrefix", "Pseudo_")
    kwargs.setdefault("PixelMsosName", "Pseudo_Pixel_MSOSs")
    kwargs.setdefault("SctMsosName", "Pseudo_SCT_MSOSs")
    kwargs.setdefault("TrtMsosName", "Pseudo_TRT_MSOSs")
    kwargs.setdefault("AddPRD", True)
    kwargs.setdefault("StoreHoles", False)
    return TrackStateOnSurfaceDecoratorCfg(flags, name, **kwargs)

def PseudoTSOS_CommonKernelCfg(flags, name="PseudoTSOS_CommonKernel"):
    acc = ComponentAccumulator()
    PseudoTrackStateOnSurfaceDecorator = acc.getPrimaryAndMerge(
        PseudoTrackStateOnSurfaceDecoratorCfg(flags))
    acc.addEventAlgo(CompFactory.DerivationFramework.CommonAugmentation(
        name, AugmentationTools=[PseudoTrackStateOnSurfaceDecorator]))
    return acc

def SiSPTrackStateOnSurfaceDecoratorCfg(
        flags, name="SiSPTrackStateOnSurfaceDecorator", **kwargs):
    kwargs.setdefault("ContainerName", "SiSPSeededTracksTrackParticles")
    kwargs.setdefault("DecorationPrefix", "SiSP_")
    kwargs.setdefault("PixelMsosName", "SiSP_Pixel_MSOSs")
    kwargs.setdefault("SctMsosName", "SiSP_SCT_MSOSs")
    kwargs.setdefault("TrtMsosName", "SiSP_TRT_MSOSs")
    kwargs.setdefault("AddPRD", True)
    kwargs.setdefault("StoreHoles", False)
    return TrackStateOnSurfaceDecoratorCfg(flags, name, **kwargs)

def SiSPTSOS_CommonKernelCfg(flags, name="SiSPTSOS_CommonKernel",
                             listOfExtensions=[]):
    acc = ComponentAccumulator()

    listOfAugmTools = []
    for extension in listOfExtensions:
        SiSPTrackStateOnSurfaceDecorator = acc.getPrimaryAndMerge(
            SiSPTrackStateOnSurfaceDecoratorCfg(
                flags, name = f"SiSP{extension}TrackStateOnSurfaceDecorator",
                ContainerName = f"SiSPSeededTracks{extension}TrackParticles",
                PixelMsosName = f"SiSP{extension}_Pixel_MSOSs",
                SctMsosName = f"SiSP{extension}_SCT_MSOSs",
                TrtMsosName = f"SiSP{extension}_TRT_MSOSs"))
        listOfAugmTools.append(SiSPTrackStateOnSurfaceDecorator)

    acc.addEventAlgo(CompFactory.DerivationFramework.CommonAugmentation(
        name, AugmentationTools=listOfAugmTools))
    return acc

def GSFTrackStateOnSurfaceDecoratorCfg(
        flags, name="GSFTrackStateOnSurfaceDecorator", **kwargs):
    kwargs.setdefault("ContainerName", "GSFTrackParticles")
    kwargs.setdefault("DecorationPrefix", "GSF_")
    kwargs.setdefault("PixelMsosName", "GSF_Pixel_MSOSs")
    kwargs.setdefault("SctMsosName", "GSF_SCT_MSOSs")
    kwargs.setdefault("TrtMsosName", "GSF_TRT_MSOSs")
    kwargs.setdefault("PRDtoTrackMap", "")
    return TrackStateOnSurfaceDecoratorCfg(flags, name, **kwargs)

def GSFTSOS_CommonKernelCfg(flags, name="GSFTSOS_CommonKernel"):
    acc = ComponentAccumulator()
    GSFTrackStateOnSurfaceDecorator = acc.getPrimaryAndMerge(
        GSFTrackStateOnSurfaceDecoratorCfg(flags))
    acc.addEventAlgo(CompFactory.DerivationFramework.CommonAugmentation(
        name, AugmentationTools=[GSFTrackStateOnSurfaceDecorator]))
    return acc

def ITkTrackStateOnSurfaceDecoratorCfg(
        flags, name="TrackStateOnSurfaceDecorator", **kwargs):
    """Configure the TSOS decorator"""
    # To produce ITkStripDetectorElementCollection
    from StripGeoModelXml.ITkStripGeoModelConfig import (
        ITkStripReadoutGeometryCfg)
    acc = ITkStripReadoutGeometryCfg(flags)

    from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
    AtlasExtrapolator = acc.popToolsAndMerge(AtlasExtrapolatorCfg(flags))
    acc.addPublicTool(AtlasExtrapolator)
    kwargs.setdefault("TrackExtrapolator", AtlasExtrapolator)

    from InDetConfig.InDetTrackHoleSearchConfig import (
        ITkTrackHoleSearchToolCfg)
    ITkHoleSearchTool = acc.popToolsAndMerge(ITkTrackHoleSearchToolCfg(flags))
    acc.addPublicTool(ITkHoleSearchTool)
    kwargs.setdefault("HoleSearch", ITkHoleSearchTool)

    kwargs.setdefault("DecorationPrefix", "")
    kwargs.setdefault("PixelMapName", "ITkPixelClustersOffsets")
    kwargs.setdefault("SctMapName", "ITkStripClustersOffsets")
    kwargs.setdefault("PixelClustersName", "ITkPixelClusters")
    kwargs.setdefault("SctClustersName", "ITkStripClusters")
    kwargs.setdefault("PRDtoTrackMap", "ITkPRDToTrackMapCombinedITkTracks")
    kwargs.setdefault("PixelMsosName", "ITkPixelMSOSs")
    kwargs.setdefault("SctMsosName", "ITkStripMSOSs")
    kwargs.setdefault("SCTDetEleCollKey", "ITkStripDetectorElementCollection")

    acc.addPublicTool(
        CompFactory.DerivationFramework.TrackStateOnSurfaceDecorator(
            name, **kwargs), primary=True)
    return acc

def ITkTSOS_CommonKernelCfg(flags, name="ITkTSOS_CommonKernel"):
    acc = ComponentAccumulator()
    ITkTrackStateOnSurfaceDecorator = acc.getPrimaryAndMerge(
        ITkTrackStateOnSurfaceDecoratorCfg(flags))
    acc.addEventAlgo(CompFactory.DerivationFramework.CommonAugmentation(
        name, AugmentationTools=[ITkTrackStateOnSurfaceDecorator]))
    return acc

def ITkSiSPTrackStateOnSurfaceDecoratorCfg(
        flags, name="SiSPTrackStateOnSurfaceDecorator", **kwargs):
    kwargs.setdefault("ContainerName", "SiSPSeededTracksTrackParticles")
    kwargs.setdefault("DecorationPrefix", "SiSP_")
    kwargs.setdefault("PixelMsosName", "SiSP_ITkPixel_MSOSs")
    kwargs.setdefault("SctMsosName", "SiSP_ITkStrip_MSOSs")
    kwargs.setdefault("AddPRD", True)
    kwargs.setdefault("StoreHoles", False)
    return ITkTrackStateOnSurfaceDecoratorCfg(flags, name, **kwargs)

def ITkSiSPTSOS_CommonKernelCfg(flags, name="ITkSiSPTSOS_CommonKernel"):
    acc = ComponentAccumulator()
    ITkSiSPTrackStateOnSurfaceDecorator = acc.getPrimaryAndMerge(
        ITkSiSPTrackStateOnSurfaceDecoratorCfg(flags))
    acc.addEventAlgo(CompFactory.DerivationFramework.CommonAugmentation(
        name, AugmentationTools=[ITkSiSPTrackStateOnSurfaceDecorator]))
    return acc

# Expression of Z0 at the primary vertex


def TrackParametersAtPVCfg(flags, name, **kwargs):
    """Configure the TrackParametersAtPV tool"""
    acc = ComponentAccumulator()
    acc.addPublicTool(CompFactory.DerivationFramework.TrackParametersAtPV(
        name, **kwargs), primary=True)
    return acc

# Pseudotrack selector


def PseudoTrackSelectorCfg(flags, name, **kwargs):
    """Configure the pseudotrack selector"""
    acc = ComponentAccumulator()

    if "trackTruthOriginTool" not in kwargs:
        from InDetTrackSystematicsTools.InDetTrackSystematicsToolsConfig import InDetTrackTruthOriginToolCfg
        kwargs.setdefault("trackTruthOriginTool", acc.popToolsAndMerge(
            InDetTrackTruthOriginToolCfg(flags)))

    acc.addPublicTool(
        CompFactory.DerivationFramework.PseudoTrackSelector(
            name, **kwargs), primary=True)
    return acc

# Tool for decorating tracks with the outcome of the track selector tool


def InDetTrackSelectionToolWrapperCfg(
        flags, name, CutLevel="TightPrimary", **kwargs):
    """Configure the InDetTrackSelectionToolWrapper"""
    acc = ComponentAccumulator()

    if "TrackSelectionTool" not in kwargs:
        from InDetConfig.InDetTrackSelectionToolConfig import (
            InDetTrackSelectionToolCfg)
        kwargs.setdefault("TrackSelectionTool", acc.popToolsAndMerge(
            InDetTrackSelectionToolCfg(
                flags, name="InDetTrackSelectionTool_"+CutLevel,
                CutLevel=CutLevel)))

    acc.addPublicTool(
        CompFactory.DerivationFramework.InDetTrackSelectionToolWrapper(
            name, **kwargs), primary=True)
    return acc

# Tool for thinning TrackParticle containers via string selection


def TrackParticleThinningCfg(flags, name, **kwargs):
    """Configure the TrackParticleThining tool"""
    acc = ComponentAccumulator()
    acc.addPublicTool(CompFactory.DerivationFramework.TrackParticleThinning(
        name, **kwargs), primary=True)
    return acc

# Tool for thinning TrackParticles that aren't associated with muons


def MuonTrackParticleThinningCfg(flags, name, **kwargs):
    """Configure the MuonTrackParticleThinning tool"""
    acc = ComponentAccumulator()
    acc.addPublicTool(
        CompFactory.DerivationFramework.MuonTrackParticleThinning(
            name, **kwargs), primary=True)
    return acc

# Tool for thinning TrackParticles that aren't associated with taus


def TauTrackParticleThinningCfg(flags, name, **kwargs):
    """Configure the TauTrackParticleThinning tool"""
    acc = ComponentAccumulator()
    acc.addPublicTool(CompFactory.DerivationFramework.TauTrackParticleThinning(
        name, **kwargs), primary=True)
    return acc

# Tool for thinning TrackParticles that aren't associated high-pt di-taus


def DiTauTrackParticleThinningCfg(flags, name, **kwargs):
    """Configure the DiTauTrackParticleThinning tool"""
    acc = ComponentAccumulator()
    acc.addPublicTool(
        CompFactory.DerivationFramework.DiTauTrackParticleThinning(
            name, **kwargs), primary=True)
    return acc

# Tool for thinning TrackParticles that are associated with jets


def JetTrackParticleThinningCfg(flags, name, **kwargs):
    """Configure the JetTrackParticleThinning tool"""
    acc = ComponentAccumulator()
    acc.addPublicTool(CompFactory.DerivationFramework.JetTrackParticleThinning(
        name, **kwargs), primary=True)
    return acc


def TauJetLepRMParticleThinningCfg(flags, name, **kwargs):
    """Configure the DiTauTrackParticleThinning tool"""
    acc = ComponentAccumulator()
    acc.addPublicTool(
        CompFactory.DerivationFramework.TauJets_LepRMParticleThinning(
            name, **kwargs), primary=True)
    return acc

# Tool for thinning TrackParticles that aren't associated with egamma objects


def EgammaTrackParticleThinningCfg(flags, name, **kwargs):
    """Configure the EgammaTrackParticleThinning tool"""
    acc = ComponentAccumulator()
    acc.addPublicTool(
        CompFactory.DerivationFramework.EgammaTrackParticleThinning(
            name, **kwargs), primary=True)
    return acc

# Track to vertex wrapper


def TrackToVertexWrapperCfg(flags, name, **kwargs):
    """Configure the TrackToVertexWrapper tool"""
    acc = ComponentAccumulator()
    acc.addPublicTool(CompFactory.DerivationFramework.TrackToVertexWrapper(
        name, **kwargs), primary=True)
    return acc
