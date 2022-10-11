# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

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

    from InDetConfig.SiSPSeededTrackFinderConfig import SiSPSeededTrackFinderCfg
    acc.merge(SiSPSeededTrackFinderCfg(flags,
                                       TracksLocation = SiSPSeededTrackCollectionKey))

    # ------------------------------------------------------------
    #
    # ---------- Ambiguity solving
    #
    # ------------------------------------------------------------

    from TrkConfig.TrkAmbiguitySolverConfig import TrkAmbiguityScoreCfg, TrkAmbiguitySolverCfg
    acc.merge(TrkAmbiguityScoreCfg(flags,
                                   SiSPSeededTrackCollectionKey = SiSPSeededTrackCollectionKey,
                                   ClusterSplitProbContainer = ClusterSplitProbContainer))

    acc.merge(TrkAmbiguitySolverCfg(flags,
                                    ResolvedTrackCollectionKey = ResolvedTrackCollectionKey,
                                    ClusterSplitProbContainer = ClusterSplitProbContainer))

    return acc


