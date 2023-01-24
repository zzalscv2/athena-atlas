# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
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
    if flags.ITk.Tracking.ActiveConfig.usePrdAssociationTool:
        from InDetConfig.InDetTrackPRD_AssociationConfig import ITkTrackPRD_AssociationCfg
        acc.merge(ITkTrackPRD_AssociationCfg(flags,
                                             name = 'ITkTrackPRD_Association' + flags.ITk.Tracking.ActiveConfig.extension,
                                             TracksName = list(InputCollections)))

    # ------------------------------------------------------------
    #
    # ----------- SiSPSeededTrackFinder
    #
    # ------------------------------------------------------------

    #
    # --- Deducing flags
    #
    doSeedingActs = False
    doTrackFindingAthena = False
    doTrackFindingActs = False

    from InDetConfig.ITkConfigFlags import TrackingComponent
    if TrackingComponent.AthenaChain in flags.ITk.Tracking.recoChain:
        doTrackFindingAthena = True
    if TrackingComponent.ActsChain in flags.ITk.Tracking.recoChain:
        doSeedingActs = True
        doTrackFindingActs = True
    if TrackingComponent.ValidateActsClusters in flags.ITk.Tracking.recoChain:
        doTrackFindingAthena = True        
    if TrackingComponent.ValidateActsSpacePoints in flags.ITk.Tracking.recoChain:
        doSeedingActs = True
        doTrackFindingAthena = True
    if TrackingComponent.ValidateActsSeeds in flags.ITk.Tracking.recoChain:
        doSeedingActs = True
        doTrackFindingAthena = True
    if TrackingComponent.ValidateActsTracks in flags.ITk.Tracking.recoChain:
        doSeedingActs = True
        doTrackFindingActs = True
        doTrackFindingAthena = False

    # Seeding does not have a real EDM converter (nor we want it!)
    # There is however an Acts-based SiSpacePointSeedMaker that acts the same way (Acts -> Athena EDM converter)
    # this is used in ITkSiSPSeededTrackFinderCfg
    # No Athena -> Acts EDM converter is possible

    if doTrackFindingAthena:
        from InDetConfig.SiSPSeededTrackFinderConfig import ITkSiSPSeededTrackFinderCfg
        SiSPSeededTrackFinderCfg = ITkSiSPSeededTrackFinderCfg
        if flags.ITk.Tracking.ActiveConfig.extension == "ConversionFinding":
            from InDetConfig.SiSPSeededTrackFinderConfig import ITkSiSPSeededTrackFinderROIConvCfg
            SiSPSeededTrackFinderCfg = ITkSiSPSeededTrackFinderROIConvCfg
        acc.merge(SiSPSeededTrackFinderCfg(flags,
                                           TracksLocation = SiSPSeededTrackCollectionKey))

    # Not schedule the following if doTrackFindingActs is False
    # this is needed in case we are scheduling the Acts-based SiSpacePointSeedMaker but not the Acts track finding
    doSeedingActs = doSeedingActs and doTrackFindingActs
    if doSeedingActs:
        from ActsTrkSeeding.ActsTrkSeedingConfig import ActsTrkSeedingFromAthenaCfg
        acc.merge(ActsTrkSeedingFromAthenaCfg(flags))

        if flags.ITk.Tracking.ActiveConfig.extension == "ConversionFinding":
            from AthenaCommon.Logging import logging 
            log = logging.getLogger( 'ITkTrackingSiPattern' )
            log.warning('ROI-based track-finding is not available yet in ACTS, so the default one is used')

    if doTrackFindingActs:
        from ActsTrkFinding.ActsTrkFindingConfig import ActsTrkFindingCfg
        if doTrackFindingAthena:
            acc.merge(ActsTrkFindingCfg(flags))
        else: # send output TrackCollection to Athena ambiguity scorer etc
            acc.merge(ActsTrkFindingCfg(flags, TracksLocation=SiSPSeededTrackCollectionKey))

    # ------------------------------------------------------------
    #
    # ---------- Ambiguity solving
    #
    # ------------------------------------------------------------
    if flags.ITk.Tracking.doFastTracking:
        from TrkConfig.TrkCollectionAliasAlgConfig import CopyAlgForAmbiCfg
        acc.merge(CopyAlgForAmbiCfg(flags, "ITkCopyAlgForAmbi"+flags.ITk.Tracking.ActiveConfig.extension,
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

