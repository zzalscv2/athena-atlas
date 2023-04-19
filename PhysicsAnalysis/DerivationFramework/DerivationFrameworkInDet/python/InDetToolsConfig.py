# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# ==============================================================================
# Provides configs for the tools used for building/thinning tracking related
# object containers and decorations in the DAODs
# ==============================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

# Track collection merger


def TrackParticleMergerCfg(flags, name, **kwargs):
    """Configure the track particle merger tool"""
    acc = ComponentAccumulator()
    acc.addPublicTool(CompFactory.DerivationFramework.TrackParticleMerger(
        name, **kwargs), primary=True)
    return acc

# Used in vertex fit track decorator


def UsedInVertexFitTrackDecoratorCfg(flags, name, **kwargs):
    """Configure the UsedInVertexFitTrackDecorator"""
    acc = ComponentAccumulator()
    acc.addPublicTool(
        CompFactory.DerivationFramework.UsedInVertexFitTrackDecorator(
            name, **kwargs), primary=True)
    return acc

# Hard scatter vertex decorator


def HardScatterVertexDecoratorCfg(flags, name, **kwargs):
    """Configure the hard process vertex decorator"""
    acc = ComponentAccumulator()
    acc.addPublicTool(
        CompFactory.DerivationFramework.HardScatterVertexDecorator(
            name, **kwargs), primary=True)
    return acc

# TrackStateOnSurface decorator


def TrackStateOnSurfaceDecoratorCfg(flags, name, **kwargs):
    """Configure the TSOS decorator"""
    # To produce SCT_DetectorElementCollection
    from SCT_GeoModel.SCT_GeoModelConfig import SCT_ReadoutGeometryCfg
    acc = SCT_ReadoutGeometryCfg(flags)

    kwargs.setdefault("DecorationPrefix", "notSet")

    from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
    AtlasExtrapolator = acc.popToolsAndMerge(AtlasExtrapolatorCfg(flags))
    acc.addPublicTool(AtlasExtrapolator)
    kwargs.setdefault("TrackExtrapolator", AtlasExtrapolator)

    from InDetConfig.InDetTrackHoleSearchConfig import (
        InDetTrackHoleSearchToolCfg)
    InDetHoleSearchTool = acc.popToolsAndMerge(
        InDetTrackHoleSearchToolCfg(flags))
    acc.addPublicTool(InDetHoleSearchTool)
    kwargs.setdefault("HoleSearch", InDetHoleSearchTool)

    kwargs.setdefault("DecorationPrefix", "")
    kwargs.setdefault("PRDtoTrackMap", "PRDtoTrackMapCombinedInDetTracks")

    acc.addPublicTool(
        CompFactory.DerivationFramework.TrackStateOnSurfaceDecorator(
            name, **kwargs), primary=True)
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


def ITkTrackStateOnSurfaceDecoratorCfg(flags, name, **kwargs):
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


def InDetTrackSelectionToolWrapperCfg(flags, name, **kwargs):
    """Configure the InDetTrackSelectionToolWrapper"""
    acc = ComponentAccumulator()
    InDetTrackSelectionTool = CompFactory.InDet.InDetTrackSelectionTool(
        name=f"{name}Tool", CutLevel="TightPrimary")
    acc.addPublicTool(InDetTrackSelectionTool, primary=False)
    InDetTrackSelectionToolWrapper = (
        CompFactory.DerivationFramework.InDetTrackSelectionToolWrapper)
    kwargs["TrackSelectionTool"] = InDetTrackSelectionTool
    acc.addPublicTool(InDetTrackSelectionToolWrapper(
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
