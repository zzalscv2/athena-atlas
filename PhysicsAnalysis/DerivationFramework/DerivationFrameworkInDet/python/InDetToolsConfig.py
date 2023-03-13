# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

#==============================================================================
# Provides configs for the tools used for building/thinning tracking related
# object containers and decorations in the DAODs
#==============================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

# Track collection merger
def TrackParticleMergerCfg(ConfigFlags, name, **kwargs):
    """Configure the track particle merger tool"""
    acc = ComponentAccumulator()
    TrackParticleMerger = CompFactory.DerivationFramework.TrackParticleMerger
    acc.addPublicTool(TrackParticleMerger(name, **kwargs),
                      primary = True)
    return acc

# Used in vertex fit track decorator
def UsedInVertexFitTrackDecoratorCfg(ConfigFlags, name, **kwargs):
    """Configure the UsedInVertexFitTrackDecorator"""
    acc = ComponentAccumulator()
    UsedInVertexFitTrackDecorator = CompFactory.DerivationFramework.UsedInVertexFitTrackDecorator
    acc.addPublicTool(UsedInVertexFitTrackDecorator(name, **kwargs),
                      primary = True)
    return acc

# Hard scatter vertex decorator
def HardScatterVertexDecoratorCfg(ConfigFlags, name, **kwargs):
    """Configure the hard process vertex decorator"""
    acc = ComponentAccumulator()
    HardScatterVertexDecorator = CompFactory.DerivationFramework.HardScatterVertexDecorator
    acc.addPublicTool(HardScatterVertexDecorator(name, **kwargs),
                       primary = True)
    return acc

# TrackStateOnSurface decorator
def TrackStateOnSurfaceDecoratorCfg(ConfigFlags, name, **kwargs):
    """Configure the TSOS decorator"""
    from SCT_GeoModel.SCT_GeoModelConfig import SCT_ReadoutGeometryCfg
    acc = SCT_ReadoutGeometryCfg(ConfigFlags) # To produce SCT_DetectorElementCollection

    kwargs.setdefault("DecorationPrefix", "notSet")

    from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
    AtlasExtrapolator = acc.popToolsAndMerge(AtlasExtrapolatorCfg(ConfigFlags))
    acc.addPublicTool(AtlasExtrapolator)
    kwargs.setdefault("TrackExtrapolator", AtlasExtrapolator)

    from InDetConfig.InDetTrackHoleSearchConfig import InDetTrackHoleSearchToolCfg
    InDetHoleSearchTool = acc.popToolsAndMerge(InDetTrackHoleSearchToolCfg(ConfigFlags))
    acc.addPublicTool(InDetHoleSearchTool)
    kwargs.setdefault("HoleSearch", InDetHoleSearchTool)

    kwargs.setdefault("DecorationPrefix", "")
    kwargs.setdefault("PRDtoTrackMap", "PRDtoTrackMapCombinedInDetTracks")

    TrackStateOnSurfaceDecorator = CompFactory.DerivationFramework.TrackStateOnSurfaceDecorator
    acc.addPublicTool(TrackStateOnSurfaceDecorator(name, **kwargs),
                      primary = True)
    return acc

def ObserverTrackStateOnSurfaceDecoratorCfg(ConfigFlags,
                                            name="ObserverTrackStateOnSurfaceDecorator",
                                            **kwargs):
    kwargs.setdefault("ContainerName", "InDetObservedTrackParticles")
    kwargs.setdefault("DecorationPrefix", "ObservedTrack_")
    kwargs.setdefault("PixelMsosName", "ObservedTrack_Pixel_MSOSs")
    kwargs.setdefault("SctMsosName", "ObservedTrack_SCT_MSOSs")
    kwargs.setdefault("TrtMsosName", "ObservedTrack_TRT_MSOSs")
    kwargs.setdefault("AddPRD", True)
    kwargs.setdefault("StoreHoles", False)
    return TrackStateOnSurfaceDecoratorCfg(ConfigFlags, name, **kwargs)

def PseudoTrackStateOnSurfaceDecoratorCfg(ConfigFlags,
                                          name="PseudoTrackStateOnSurfaceDecorator",
                                          **kwargs):
    kwargs.setdefault("ContainerName", "InDetPseudoTrackParticles")
    kwargs.setdefault("DecorationPrefix", "Pseudo_")
    kwargs.setdefault("PixelMsosName", "Pseudo_Pixel_MSOSs")
    kwargs.setdefault("SctMsosName", "Pseudo_SCT_MSOSs")
    kwargs.setdefault("TrtMsosName", "Pseudo_TRT_MSOSs")
    kwargs.setdefault("AddPRD", True)
    kwargs.setdefault("StoreHoles", False)
    return TrackStateOnSurfaceDecoratorCfg(ConfigFlags, name, **kwargs)

def SiSPTrackStateOnSurfaceDecoratorCfg(ConfigFlags,
                                        name="SiSPTrackStateOnSurfaceDecorator",
                                        **kwargs):
    kwargs.setdefault("ContainerName", "SiSPSeededTracksTrackParticles")
    kwargs.setdefault("DecorationPrefix", "SiSP_")
    kwargs.setdefault("PixelMsosName", "SiSP_Pixel_MSOSs")
    kwargs.setdefault("SctMsosName", "SiSP_SCT_MSOSs")
    kwargs.setdefault("TrtMsosName", "SiSP_TRT_MSOSs")
    kwargs.setdefault("AddPRD", True)
    kwargs.setdefault("StoreHoles", False)
    return TrackStateOnSurfaceDecoratorCfg(ConfigFlags, name, **kwargs)

def ITkTrackStateOnSurfaceDecoratorCfg(ConfigFlags, name, **kwargs):
    """Configure the TSOS decorator"""
    from StripGeoModelXml.ITkStripGeoModelConfig import ITkStripReadoutGeometryCfg
    acc = ITkStripReadoutGeometryCfg(ConfigFlags) # To produce ITkStripDetectorElementCollection

    from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
    AtlasExtrapolator = acc.popToolsAndMerge(AtlasExtrapolatorCfg(ConfigFlags))
    acc.addPublicTool(AtlasExtrapolator)
    kwargs.setdefault("TrackExtrapolator", AtlasExtrapolator)

    from InDetConfig.InDetTrackHoleSearchConfig import ITkTrackHoleSearchToolCfg
    ITkHoleSearchTool = acc.popToolsAndMerge(ITkTrackHoleSearchToolCfg(ConfigFlags))
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

    TrackStateOnSurfaceDecorator = CompFactory.DerivationFramework.TrackStateOnSurfaceDecorator
    acc.addPublicTool(TrackStateOnSurfaceDecorator(name, **kwargs),
                      primary = True)
    return acc

# Expression of Z0 at the primary vertex
def TrackParametersAtPVCfg(ConfigFlags, name, **kwargs):
    """Configure the TrackParametersAtPV tool"""
    acc = ComponentAccumulator()
    TrackParametersAtPV = CompFactory.DerivationFramework.TrackParametersAtPV
    acc.addPublicTool(TrackParametersAtPV(name, **kwargs),
                      primary = True)
    return acc

# Pseudotrack selector
def PseudoTrackSelectorCfg(ConfigFlags, name, **kwargs):
    """Configure the pseudotrack selector"""
    acc = ComponentAccumulator()

    if "trackTruthOriginTool" not in kwargs:
        from InDetTrackSystematicsTools.InDetTrackSystematicsToolsConfig import InDetTrackTruthOriginToolCfg
        kwargs.setdefault("trackTruthOriginTool", acc.popToolsAndMerge(
            InDetTrackTruthOriginToolCfg(ConfigFlags)))

    acc.addPublicTool(CompFactory.DerivationFramework.PseudoTrackSelector(name, **kwargs),
                      primary = True)
    return acc

# Tool for decorating tracks with the outcome of the track selector tool
def InDetTrackSelectionToolWrapperCfg(ConfigFlags, name, **kwargs):
    """Configure the InDetTrackSelectionToolWrapper"""
    acc = ComponentAccumulator()
    InDetTrackSelectionTool = CompFactory.InDet.InDetTrackSelectionTool(name=f"{name}Tool",CutLevel = "TightPrimary")
    acc.addPublicTool(InDetTrackSelectionTool, primary=False) 
    InDetTrackSelectionToolWrapper = CompFactory.DerivationFramework.InDetTrackSelectionToolWrapper
    kwargs["TrackSelectionTool"] = InDetTrackSelectionTool
    acc.addPublicTool(InDetTrackSelectionToolWrapper(name, **kwargs),
                      primary = True)
    return acc

# Tool for thinning TrackParticle containers via string selection
def TrackParticleThinningCfg(ConfigFlags, name, **kwargs):
    """Configure the TrackParticleThining tool"""
    acc = ComponentAccumulator()
    TrackParticleThinning = CompFactory.DerivationFramework.TrackParticleThinning
    acc.addPublicTool(TrackParticleThinning(name,**kwargs),
                      primary = True)
    return acc

# Tool for thinning TrackParticles that aren't associated with muons
def MuonTrackParticleThinningCfg(ConfigFlags, name, **kwargs):
    """Configure the MuonTrackParticleThinning tool"""
    acc = ComponentAccumulator()
    MuonTrackParticleThinning = CompFactory.DerivationFramework.MuonTrackParticleThinning
    acc.addPublicTool(MuonTrackParticleThinning(name,**kwargs),
                      primary = True)
    return acc

# Tool for thinning TrackParticles that aren't associated with taus
def TauTrackParticleThinningCfg(ConfigFlags, name, **kwargs):
    """Configure the TauTrackParticleThinning tool"""
    acc = ComponentAccumulator()
    TauTrackParticleThinning = CompFactory.DerivationFramework.TauTrackParticleThinning
    acc.addPublicTool(TauTrackParticleThinning(name, **kwargs),
                      primary = True)
    return acc

# Tool for thinning TrackParticles that aren't associated high-pt di-taus
def DiTauTrackParticleThinningCfg(ConfigFlags, name, **kwargs):
    """Configure the DiTauTrackParticleThinning tool"""
    acc = ComponentAccumulator()
    DiTauTrackParticleThinning = CompFactory.DerivationFramework.DiTauTrackParticleThinning
    acc.addPublicTool(DiTauTrackParticleThinning(name, **kwargs),
                      primary = True)
    return acc

# Tool for thinning TrackParticles that are associated with jets
def JetTrackParticleThinningCfg(ConfigFlags, name, **kwargs):
    """Configure the JetTrackParticleThinning tool"""
    acc = ComponentAccumulator()
    JetTrackParticleThinning = CompFactory.DerivationFramework.JetTrackParticleThinning
    acc.addPublicTool(JetTrackParticleThinning(name,**kwargs),
            primary = True)
    return acc

def TauJetLepRMParticleThinningCfg(ConfigFlags, name, **kwargs):
    """Configure the DiTauTrackParticleThinning tool"""
    acc = ComponentAccumulator()
    TauJetLepRMParticleThinningTool = CompFactory.DerivationFramework.TauJets_LepRMParticleThinning
    acc.addPublicTool(TauJetLepRMParticleThinningTool(name, **kwargs), primary = True)
    return acc

# Tool for thinning TrackParticles that aren't associated with egamma objects
def EgammaTrackParticleThinningCfg(ConfigFlags, name, **kwargs):
    """Configure the EgammaTrackParticleThinning tool"""
    acc = ComponentAccumulator()
    EgammaTrackParticleThinning = CompFactory.DerivationFramework.EgammaTrackParticleThinning
    acc.addPublicTool(EgammaTrackParticleThinning(name, **kwargs),
                      primary = True)
    return acc

# Track to vertex wrapper
def TrackToVertexWrapperCfg(ConfigFlags, name, **kwargs):
    """Configure the TrackToVertexWrapper tool"""
    acc = ComponentAccumulator()
    TrackToVertexWrapper = CompFactory.DerivationFramework.TrackToVertexWrapper
    acc.addPublicTool(TrackToVertexWrapper(name, **kwargs),
                      primary = True)
    return acc
