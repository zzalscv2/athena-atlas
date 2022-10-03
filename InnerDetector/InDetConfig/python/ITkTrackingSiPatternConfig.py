# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

# ------------------------------------------------------------
#
# ----------- Setup Si Pattern for New tracking
#
# ------------------------------------------------------------
def ITkTrackingSiPatternCfg(flags,
                            InputCollections = None,
                            ResolvedTrackCollectionKey = None,
                            SiSPSeededTrackCollectionKey = None ,
                            ClusterSplitProbContainer=''):
    acc = ComponentAccumulator()
    #
    # --- get list of already associated hits (always do this, even if no other tracking ran before)
    #
    if flags.ITk.Tracking.ActivePass.usePrdAssociationTool:
        from InDetConfig.InDetTrackPRD_AssociationConfig import ITkTrackPRD_AssociationCfg
        acc.merge(ITkTrackPRD_AssociationCfg(flags,
                                             name = 'ITkTrackPRD_Association' + flags.ITk.Tracking.ActivePass.extension,
                                             TracksName = list(InputCollections)))

    # ------------------------------------------------------------
    #
    # ----------- SiSPSeededTrackFinder
    #
    # ------------------------------------------------------------

    from InDetConfig.SiSPSeededTrackFinderConfig import ITkSiSPSeededTrackFinderCfg
    SiSPSeededTrackFinderCfg = ITkSiSPSeededTrackFinderCfg
    if flags.ITk.Tracking.ActivePass.extension == "ConversionFinding":
        from InDetConfig.SiSPSeededTrackFinderConfig import ITkSiSPSeededTrackFinderROIConvCfg
        SiSPSeededTrackFinderCfg = ITkSiSPSeededTrackFinderROIConvCfg
    acc.merge(SiSPSeededTrackFinderCfg(flags,
                                       TracksLocation = SiSPSeededTrackCollectionKey))
    # ------------------------------------------------------------
    #
    # ---------- Ambiguity solving
    #
    # ------------------------------------------------------------
    if flags.ITk.Tracking.doFastTracking:
        from TrkConfig.TrkCollectionAliasAlgConfig import ITkCopyAlgForAmbiCfg
        acc.merge(ITkCopyAlgForAmbiCfg(flags, "ITkCopyAlgForAmbi"+flags.ITk.Tracking.ActivePass.extension,
                                       CollectionName = SiSPSeededTrackCollectionKey, # Input
                                       AliasName = ResolvedTrackCollectionKey))       # Output

    else:
        from TrkConfig.TrkAmbiguitySolverConfig import ITkTrkAmbiguityScoreCfg, ITkTrkAmbiguitySolverCfg
        acc.merge(ITkTrkAmbiguityScoreCfg(flags,
                                          SiSPSeededTrackCollectionKey = SiSPSeededTrackCollectionKey,
                                          ClusterSplitProbContainer = ClusterSplitProbContainer))

        acc.merge(ITkTrkAmbiguitySolverCfg(flags,
                                           ResolvedTrackCollectionKey = ResolvedTrackCollectionKey))

    return acc

