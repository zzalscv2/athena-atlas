# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def SiSPSeededTrackFinderCfg(flags, name="InDetSiSpTrackFinder", **kwargs) :
    acc = ComponentAccumulator()

    from TrkConfig.TrkExRungeKuttaPropagatorConfig import InDetPropagatorCfg
    InDetPropagator = acc.popToolsAndMerge(InDetPropagatorCfg(flags))
    acc.addPublicTool(InDetPropagator)

    #
    # --- Setup Track finder using space points seeds
    #
    from InDetConfig.SiTrackMakerConfig import SiTrackMaker_xkCfg
    kwargs.setdefault("TrackTool", acc.popToolsAndMerge(SiTrackMaker_xkCfg(flags)))
    kwargs.setdefault("PropagatorTool", InDetPropagator)
    from TrkConfig.TrkTrackSummaryToolConfig import InDetTrackSummaryToolNoHoleSearchCfg
    kwargs.setdefault("TrackSummaryTool", acc.popToolsAndMerge(InDetTrackSummaryToolNoHoleSearchCfg(flags)))

    if "SeedsTool" not in kwargs:
        from InDetConfig.SiSpacePointsSeedToolConfig import SiSpacePointsSeedMakerCfg
        kwargs.setdefault("SeedsTool", acc.popToolsAndMerge(
            SiSpacePointsSeedMakerCfg(flags)))

    kwargs.setdefault("useMBTSTimeDiff", flags.Reco.EnableHI) # Heavy-ion config

    if flags.InDet.Tracking.ActivePass.usePrdAssociationTool:
        # not all classes have that property !!!
        kwargs.setdefault("PRDtoTrackMap", 'InDetPRDtoTrackMap'+ flags.InDet.Tracking.ActivePass.extension)

    if flags.InDet.Tracking.ActivePass.extension == "Forward":
        kwargs.setdefault("useZvertexTool", flags.Reco.EnableHI) # For heavy-ion
        kwargs.setdefault("useZBoundFinding", False)
    else:
        kwargs.setdefault("useZvertexTool", flags.Reco.EnableHI) # For heavy-ion
        kwargs.setdefault("useZBoundFinding", flags.InDet.Tracking.ActivePass.doZBoundary)
    
    #
    # --- Z-coordinates primary vertices finder (only for collisions)
    #
    if kwargs["useZvertexTool"] and "ZvertexTool" not in kwargs:
        from InDetConfig.SiZvertexToolConfig import SiZvertexMaker_xkCfg
        kwargs.setdefault("ZvertexTool", acc.popToolsAndMerge(
            SiZvertexMaker_xkCfg(flags)))

    if flags.Reco.EnableHI:
        kwargs.setdefault("FreeClustersCut",2) #Heavy Ion optimization from Igor

    InDetSiSPSeededTrackFinder = CompFactory.InDet.SiSPSeededTrackFinder(name = name+flags.InDet.Tracking.ActivePass.extension, **kwargs)
    acc.addEventAlgo(InDetSiSPSeededTrackFinder)
    return acc

# ------------------------------------------------------------
#
# ----------- Setup Si Pattern for New tracking
#
# ------------------------------------------------------------
def TrackingSiPatternCfg(flags,
                         InputCollections = None,
                         ResolvedTrackCollectionKey = None,
                         SiSPSeededTrackCollectionKey = None ,
                         ClusterSplitProbContainer=''):
    acc = ComponentAccumulator()
    #
    # --- get list of already associated hits (always do this, even if no other tracking ran before)
    #
    if flags.InDet.Tracking.ActivePass.usePrdAssociationTool:
        from InDetConfig.InDetTrackPRD_AssociationConfig import InDetTrackPRD_AssociationCfg
        acc.merge(InDetTrackPRD_AssociationCfg(flags,
                                               name = 'InDetTrackPRD_Association' + flags.InDet.Tracking.ActivePass.extension,
                                               TracksName = list(InputCollections)))

    # ------------------------------------------------------------
    #
    # ----------- SiSPSeededTrackFinder
    #
    # ------------------------------------------------------------

    acc.merge(SiSPSeededTrackFinderCfg( flags,
                                        TracksLocation = SiSPSeededTrackCollectionKey))

    # ------------------------------------------------------------
    #
    # ---------- Ambiguity solving
    #
    # ------------------------------------------------------------

    from TrkConfig.TrkAmbiguitySolverConfig import TrkAmbiguityScoreCfg, TrkAmbiguitySolverCfg
    acc.merge(TrkAmbiguityScoreCfg( flags,
                                    SiSPSeededTrackCollectionKey = SiSPSeededTrackCollectionKey,
                                    ClusterSplitProbContainer = ClusterSplitProbContainer))

    acc.merge(TrkAmbiguitySolverCfg(flags,
                                    ResolvedTrackCollectionKey = ResolvedTrackCollectionKey,
                                    ClusterSplitProbContainer = ClusterSplitProbContainer))

    return acc


