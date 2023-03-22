"""Define methods to construct configured Tracking conversion algorithms

Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

######################################
###   RecTrackParticleContainerCnvTool
######################################

def RecTrackParticleContainerCnvToolCfg(flags, name="RecTrackParticleContainerCnvTool", **kwargs):
    result = ComponentAccumulator()

    if "TrackParticleCreator" not in kwargs:
        from TrkConfig.TrkParticleCreatorConfig import (
            TrackParticleCreatorToolCfg)
        TrackParticleCreator = result.popToolsAndMerge(
            TrackParticleCreatorToolCfg(flags))
        result.addPublicTool(TrackParticleCreator)
        kwargs.setdefault("TrackParticleCreator", TrackParticleCreator)

    result.setPrivateTools(
        CompFactory.xAODMaker.RecTrackParticleContainerCnvTool(name, **kwargs))
    return result

def MuonRecTrackParticleContainerCnvToolCfg(flags, name = "MuonRecTrackParticleContainerCnvTool", **kwargs):
    result = ComponentAccumulator()

    if "TrackParticleCreator" not in kwargs:
        from TrkConfig.TrkParticleCreatorConfig import (
            MuonCombinedParticleCreatorCfg)
        TrackParticleCreator = result.popToolsAndMerge(
            MuonCombinedParticleCreatorCfg(flags))
        result.addPublicTool(TrackParticleCreator)
        kwargs.setdefault("TrackParticleCreator", TrackParticleCreator)

    result.setPrivateTools(
        CompFactory.xAODMaker.RecTrackParticleContainerCnvTool(name, **kwargs))
    return result

######################################
###   TrackCollectionCnvTool
######################################

def TrackCollectionCnvToolCfg(flags, name="TrackCollectionCnvTool", **kwargs):
    if flags.Detector.GeometryITk:
        name = name.replace("InDet", "ITk")
        from InDetConfig.ITkTrackRecoConfig import ITkTrackCollectionCnvToolCfg
        return ITkTrackCollectionCnvToolCfg(flags, name, **kwargs)

    result = ComponentAccumulator()

    if "TrackParticleCreator" not in kwargs:
        from TrkConfig.TrkParticleCreatorConfig import (
            TrackParticleCreatorToolCfg)
        TrackParticleCreator = result.popToolsAndMerge(
            TrackParticleCreatorToolCfg(flags))
        result.addPublicTool(TrackParticleCreator)
        kwargs.setdefault("TrackParticleCreator", TrackParticleCreator)

    result.setPrivateTools(
        CompFactory.xAODMaker.TrackCollectionCnvTool(name, **kwargs))
    return result

def ITkTrackCollectionCnvToolCfg(flags, name="ITkTrackCollectionCnvTool", **kwargs):
    result = ComponentAccumulator()

    if "TrackParticleCreator" not in kwargs:
        from TrkConfig.TrkParticleCreatorConfig import ITkTrackParticleCreatorToolCfg
        TrackParticleCreator = result.popToolsAndMerge(
            ITkTrackParticleCreatorToolCfg(flags))
        result.addPublicTool(TrackParticleCreator)
        kwargs.setdefault("TrackParticleCreator", TrackParticleCreator)

    result.setPrivateTools(CompFactory.xAODMaker.TrackCollectionCnvTool(name, **kwargs))
    return result

def MuonTrackCollectionCnvToolCfg(flags, name = "MuonTrackCollectionCnvTool", **kwargs):
    result = ComponentAccumulator()

    if "TrackParticleCreator" not in kwargs:
        from TrkConfig.TrkParticleCreatorConfig import MuonCombinedParticleCreatorCfg
        TrackParticleCreator = result.popToolsAndMerge(
            MuonCombinedParticleCreatorCfg(flags))
        result.addPublicTool(TrackParticleCreator)
        kwargs.setdefault("TrackParticleCreator", TrackParticleCreator)

    result.setPrivateTools(CompFactory.xAODMaker.TrackCollectionCnvTool(name, **kwargs))
    return result

######################################
###   TrackParticleCnvAlg
######################################

def TrackParticleCnvAlgCfg(flags, name="TrackParticleCnvAlg",
                           ClusterSplitProbabilityName = "",
                           AssociationMapName = "",
                           **kwargs):
    if flags.Detector.GeometryITk:
        name = name.replace("InDet", "ITk")
        from InDetConfig.ITkTrackRecoConfig import ITkTrackParticleCnvAlgCfg
        return ITkTrackParticleCnvAlgCfg(flags, name,
                                         ClusterSplitProbabilityName,
                                         AssociationMapName,
                                         **kwargs)

    result = ComponentAccumulator()

    kwargs.setdefault("ConvertTracks", True)
    kwargs.setdefault("ConvertTrackParticles", False)
    kwargs.setdefault("TrackContainerName", "CombinedInDetTracks")
    kwargs.setdefault("xAODTrackParticlesFromTracksContainerName", "InDetTrackParticles")

    if "TrackParticleCreator" not in kwargs:
        from TrkConfig.TrkParticleCreatorConfig import TrackParticleCreatorToolCfg
        kwargs.setdefault("TrackParticleCreator", result.popToolsAndMerge(
            TrackParticleCreatorToolCfg(
                flags,
                name = kwargs["xAODTrackParticlesFromTracksContainerName"] \
                + "CreatorTool",
                ClusterSplitProbabilityName = ClusterSplitProbabilityName,
                AssociationMapName = AssociationMapName)))

    if "TrackCollectionCnvTool" not in kwargs:
        result.addPublicTool(kwargs["TrackParticleCreator"])
        kwargs.setdefault("TrackCollectionCnvTool", result.popToolsAndMerge(
            TrackCollectionCnvToolCfg(
                flags,
                TrackParticleCreator = kwargs["TrackParticleCreator"])))

    if flags.Tracking.doTruth:
        kwargs.setdefault("TrackTruthContainerName", "TrackTruthCollection" \
                          if kwargs["TrackContainerName"]=="CombinedInDetTracks" \
                          else kwargs["TrackContainerName"]+"TruthCollection")
        kwargs.setdefault("AddTruthLink", True)
        if "MCTruthClassifier" not in kwargs:
            from MCTruthClassifier.MCTruthClassifierConfig import MCTruthClassifierCfg
            kwargs.setdefault("MCTruthClassifier", result.popToolsAndMerge(
                MCTruthClassifierCfg(flags)))
    else:
        kwargs.setdefault("AddTruthLink", False)

    if flags.Tracking.perigeeExpression == "Vertex":
        kwargs.setdefault("PrimaryVerticesName", "PrimaryVertices")

    result.addEventAlgo(CompFactory.xAODMaker.TrackParticleCnvAlg(name, **kwargs))
    return result

def BeamLineTrackParticleCnvAlgCfg(flags, name="BeamLineTrackParticleCnvAlg",
                                   ClusterSplitProbabilityName = "",
                                   AssociationMapName = "",
                                   **kwargs):
    result = ComponentAccumulator()

    if "TrackParticleCreator" not in kwargs:
        from TrkConfig.TrkParticleCreatorConfig import TrackParticleCreatorToolCfg
        kwargs.setdefault("TrackParticleCreator", result.popToolsAndMerge(
              TrackParticleCreatorToolCfg(
                  flags,
                  name="InDetxAODParticleCreatorToolBeamLine",
                  ClusterSplitProbabilityName = ClusterSplitProbabilityName,
                  AssociationMapName = AssociationMapName,
                  PerigeeExpression = "BeamLine")))
    
    if flags.Tracking.perigeeExpression == "Vertex":
        kwargs.setdefault("PrimaryVerticesName", "")

    result.merge(TrackParticleCnvAlgCfg(flags, name, **kwargs))
    return result

def TrackParticleCnvAlgPIDCheckCfg(flags, name,
                                   ClusterSplitProbabilityName = "",
                                   AssociationMapName = "",
                                   **kwargs):
    result = ComponentAccumulator()

    if "TrackParticleCreator" not in kwargs:
        from TrkConfig.TrkParticleCreatorConfig import TrackParticleCreatorToolPIDCheckCfg
        kwargs.setdefault("TrackParticleCreator", result.popToolsAndMerge(
            TrackParticleCreatorToolPIDCheckCfg(
                flags,
                name = kwargs["xAODTrackParticlesFromTracksContainerName"] \
                + "CreatorTool",
                ClusterSplitProbabilityName = ClusterSplitProbabilityName,
                AssociationMapName = AssociationMapName)))

    result.merge(TrackParticleCnvAlgCfg(flags, name, **kwargs))
    return result

def TrackParticleCnvAlgNoPIDCfg(flags, name,
                                ClusterSplitProbabilityName = "",
                                AssociationMapName = "",
                                **kwargs):
    result = ComponentAccumulator()

    if "TrackParticleCreator" not in kwargs:
        from TrkConfig.TrkParticleCreatorConfig import TrackParticleCreatorToolNoPIDCfg
        kwargs.setdefault("TrackParticleCreator", result.popToolsAndMerge(
            TrackParticleCreatorToolNoPIDCfg(
                flags,
                name = kwargs["xAODTrackParticlesFromTracksContainerName"] \
                + "CreatorTool",
                ClusterSplitProbabilityName = ClusterSplitProbabilityName,
                AssociationMapName = AssociationMapName)))

    result.merge(TrackParticleCnvAlgCfg(flags, name,
                                        ClusterSplitProbabilityName,
                                        AssociationMapName, **kwargs))
    return result

def ObserverTrackParticleCnvAlgCfg(flags, name="ObserverTrackParticleCnvAlg",
                                   ClusterSplitProbabilityName = "",
                                   AssociationMapName = "",
                                   **kwargs):
     kwargs.setdefault("TrackContainerName", "ObservedTracksCollection")
     kwargs.setdefault("xAODTrackParticlesFromTracksContainerName", "InDetObservedTrackParticles")
     kwargs.setdefault("AugmentObservedTracks", True)
     kwargs.setdefault("TracksMapName", "ObservedTracksCollectionMap")

     return TrackParticleCnvAlgCfg(flags, name, ClusterSplitProbabilityName, AssociationMapName, **kwargs)

def ITkTrackParticleCnvAlgCfg(flags, name="ITkTrackParticleCnvAlg",
                              ClusterSplitProbabilityName = "",
                              AssociationMapName = "",
                              **kwargs):
    result = ComponentAccumulator()

    kwargs.setdefault("ConvertTracks", True)
    kwargs.setdefault("ConvertTrackParticles", False)
    kwargs.setdefault("TrackContainerName", "CombinedITkTracks")
    kwargs.setdefault("xAODTrackParticlesFromTracksContainerName", "InDetTrackParticles")

    if "TrackParticleCreator" not in kwargs:
        from TrkConfig.TrkParticleCreatorConfig import ITkTrackParticleCreatorToolCfg
        kwargs.setdefault("TrackParticleCreator", result.popToolsAndMerge(
            ITkTrackParticleCreatorToolCfg(
                flags,
                name = kwargs["xAODTrackParticlesFromTracksContainerName"] \
                + "CreatorTool",
                ClusterSplitProbabilityName = ClusterSplitProbabilityName,
                AssociationMapName = AssociationMapName)))

    if "TrackCollectionCnvTool" not in kwargs:
        result.addPublicTool(kwargs["TrackParticleCreator"])
        kwargs.setdefault("TrackCollectionCnvTool", result.popToolsAndMerge(
            ITkTrackCollectionCnvToolCfg(
                flags,
                TrackParticleCreator = kwargs["TrackParticleCreator"])))

    if flags.Tracking.doTruth:
        kwargs.setdefault("TrackTruthContainerName",
                          kwargs["TrackContainerName"]+"TruthCollection")
        kwargs.setdefault("AddTruthLink", True)
        if "MCTruthClassifier" not in kwargs:
            from MCTruthClassifier.MCTruthClassifierConfig import MCTruthClassifierCfg
            kwargs.setdefault("MCTruthClassifier", result.popToolsAndMerge(
                MCTruthClassifierCfg(flags)))
    else:
        kwargs.setdefault("AddTruthLink", False)

    result.addEventAlgo(CompFactory.xAODMaker.TrackParticleCnvAlg(name, **kwargs))
    return result

def MuonStandaloneTrackParticleCnvAlgCfg(flags, name = "MuonStandaloneTrackParticleCnvAlg",**kwargs):
    from TrkConfig.TrkParticleCreatorConfig import MuonParticleCreatorToolCfg
    from BeamSpotConditions.BeamSpotConditionsConfig import BeamSpotCondAlgCfg

    result=ComponentAccumulator()
    result.merge(BeamSpotCondAlgCfg(flags))

    muonparticlecreatortool = result.popToolsAndMerge(MuonParticleCreatorToolCfg(flags))
    result.addPublicTool(muonparticlecreatortool) # Public in TrackCollectionCnvTool

    kwargs.setdefault("TrackParticleCreator", muonparticlecreatortool)
    kwargs.setdefault("RecTrackParticleContainerCnvTool", result.popToolsAndMerge(
        MuonRecTrackParticleContainerCnvToolCfg(flags, TrackParticleCreator = muonparticlecreatortool)))
    muontrackcollectioncnvtool = result.popToolsAndMerge(
        MuonTrackCollectionCnvToolCfg(flags, TrackParticleCreator = muonparticlecreatortool))
    kwargs.setdefault("TrackCollectionCnvTool", muontrackcollectioncnvtool)

    kwargs.setdefault("TrackContainerName", "MuonSpectrometerTracks")
    kwargs.setdefault("xAODTrackParticlesFromTracksContainerName", "MuonSpectrometerTrackParticles")
    kwargs.setdefault("AODContainerName", "")
    kwargs.setdefault("AODTruthContainerName", "")
    kwargs.setdefault("xAODTruthLinkVector",  "")
    kwargs.setdefault("ConvertTrackParticles", False)
    kwargs.setdefault("ConvertTracks", True)
    kwargs.setdefault("AddTruthLink", False)

    result.addEventAlgo(CompFactory.xAODMaker.TrackParticleCnvAlg(name, **kwargs))
    return result

def MuonTrackParticleCnvCfg(flags, name = "MuonTrackParticleCnvAlg",**kwargs):
    result=ComponentAccumulator()
    from BeamSpotConditions.BeamSpotConditionsConfig import BeamSpotCondAlgCfg
    result.merge(BeamSpotCondAlgCfg(flags))


    if "TrackParticleCreator" not in kwargs:
        from TrkConfig.TrkParticleCreatorConfig import MuonCombinedParticleCreatorCfg
        kwargs.setdefault("TrackParticleCreator", result.popToolsAndMerge(MuonCombinedParticleCreatorCfg(flags)))

    if "TrackCollectionCnvTool" not in kwargs:
        from xAODTrackingCnv.xAODTrackingCnvConfig import MuonTrackCollectionCnvToolCfg
        kwargs.setdefault("TrackCollectionCnvTool", result.popToolsAndMerge(MuonTrackCollectionCnvToolCfg(flags)))

    kwargs.setdefault("TrackContainerName", "MuonSpectrometerTracks")
    kwargs.setdefault("xAODTrackParticlesFromTracksContainerName", "MuonSpectrometerTrackParticles")
    kwargs.setdefault("ConvertTrackParticles", False)
    kwargs.setdefault("ConvertTracks", True)
    kwargs.setdefault("AddTruthLink", False)

    result.addEventAlgo(CompFactory.xAODMaker.TrackParticleCnvAlg(name, **kwargs))
    return result
