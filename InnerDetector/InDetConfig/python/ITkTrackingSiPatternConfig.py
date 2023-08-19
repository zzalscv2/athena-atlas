# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

# ------------------------------------------------------------
#
# ----------- Setup Si Pattern for New tracking
#
# ------------------------------------------------------------


def ITkTrackingSiPatternCfg(flags,
                            InputCollections=None,
                            ResolvedTrackCollectionKey=None,
                            SiSPSeededTrackCollectionKey=None,
                            ClusterSplitProbContainer=''):
    acc = ComponentAccumulator()
    #
    # --- get list of already associated hits (always do this, even if no other tracking ran before)
    #
    if flags.Tracking.ActiveConfig.usePrdAssociationTool:
        from InDetConfig.InDetTrackPRD_AssociationConfig import (
            ITkTrackPRD_AssociationCfg)
        acc.merge(ITkTrackPRD_AssociationCfg(
            flags,
            name=('ITkTrackPRD_Association' +
                  flags.Tracking.ActiveConfig.extension),
            TracksName=list(InputCollections)))

    is_acts_ambi=False
    # Can use FastTrackFinder instead of SiSPSeededTrackFinder
    if flags.Tracking.useITkFTF:

        # ------------------------------------------------------------
        #
        # ----------- FastTrackFinder
        #
        # ------------------------------------------------------------

        from TrigFastTrackFinder.ITkFastTrackFinderStandaloneConfig import (
            ITkFastTrackFinderStandaloneCfg)
        acc.merge(ITkFastTrackFinderStandaloneCfg(
            flags, SiSPSeededTrackCollectionKey))

    else:

        # ------------------------------------------------------------
        #
        # ----------- SiSPSeededTrackFinder
        #
        # ------------------------------------------------------------

        #
        # --- Deducing configuration from the flags
        #
        from ActsConfig.TrackingComponentConfigurer import (
            TrackingComponentConfigurer)
        configuration_settings = TrackingComponentConfigurer(flags)
        is_acts_ambi = TrackingComponentConfigurer(flags).doActsTrack and flags.Acts.doAmbiguityResolution

        # Athena Track
        if configuration_settings.doAthenaTrack:

            from InDetConfig.SiSPSeededTrackFinderConfig import (
                ITkSiSPSeededTrackFinderCfg)
            SiSPSeededTrackFinderCfg = ITkSiSPSeededTrackFinderCfg
            if flags.Tracking.ActiveConfig.extension == "Conversion":
                from InDetConfig.SiSPSeededTrackFinderConfig import (
                    ITkSiSPSeededTrackFinderROIConvCfg)
                SiSPSeededTrackFinderCfg = ITkSiSPSeededTrackFinderROIConvCfg

            acc.merge(SiSPSeededTrackFinderCfg(
                flags,
                TracksLocation=SiSPSeededTrackCollectionKey))

        # ACTS seed
        if configuration_settings.doActsSeed:

            from ActsConfig.ActsSeedingConfig import (
                ActsSeedingCfg)
            acc.merge(ActsSeedingCfg(flags))

            if flags.Tracking.ActiveConfig.extension == "Conversion":
                from AthenaCommon.Logging import logging
                log = logging.getLogger('ITkTrackingSiPattern')
                log.warning(
                    'ROI-based track-finding is not available yet in ACTS, so the default one is used')

        # ACTS track
        if configuration_settings.doActsTrack:
            from ActsConfig.ActsTrackFindingConfig import ActsTrackFindingCfg
            acc.merge(ActsTrackFindingCfg(flags))

            if flags.Acts.doAmbiguityResolution :
                from ActsConfig.ActsTrackFindingConfig import ActsAmbiguityResolutionCfg
                acc.merge(ActsAmbiguityResolutionCfg(flags))

        if configuration_settings.doActsToAthenaTrack:
            if not flags.Acts.doAmbiguityResolution :
                acts_athena_tracks_name = SiSPSeededTrackCollectionKey
            else :
                acts_athena_tracks_name = ResolvedTrackCollectionKey
            from ActsConfig.ActsEventCnvConfig import ActsToTrkConvertorAlgCfg
            acc.merge(ActsToTrkConvertorAlgCfg(flags,
                                               TracksLocation=acts_athena_tracks_name))

    from InDetConfig.ITkTrackTruthConfig import ITkTrackTruthCfg
    if flags.Tracking.doTruth and not is_acts_ambi:
        acc.merge(ITkTrackTruthCfg(
            flags,
            Tracks=SiSPSeededTrackCollectionKey,
            DetailedTruth=SiSPSeededTrackCollectionKey+"DetailedTruth",
            TracksTruth=SiSPSeededTrackCollectionKey+"TruthCollection"))

    # ------------------------------------------------------------
    #
    # ---------- Ambiguity solving
    #
    # ------------------------------------------------------------

    if flags.Tracking.doITkFastTracking:

        from TrkConfig.TrkCollectionAliasAlgConfig import CopyAlgForAmbiCfg
        acc.merge(CopyAlgForAmbiCfg(
            flags,
            "ITkCopyAlgForAmbi"+flags.Tracking.ActiveConfig.extension,
            CollectionName=SiSPSeededTrackCollectionKey,  # Input
            AliasName=ResolvedTrackCollectionKey))       # Output

    elif not is_acts_ambi:
        # with Acts.doAmbiguityResolution the converter will directly produce
        # tracks with the key ResolvedTrackCollectionKey
        from TrkConfig.TrkAmbiguitySolverConfig import (
            ITkTrkAmbiguityScoreCfg, ITkTrkAmbiguitySolverCfg)
        acc.merge(ITkTrkAmbiguityScoreCfg(
            flags,
            SiSPSeededTrackCollectionKey=SiSPSeededTrackCollectionKey,
            ClusterSplitProbContainer=ClusterSplitProbContainer))

        acc.merge(ITkTrkAmbiguitySolverCfg(
            flags,
            ResolvedTrackCollectionKey=ResolvedTrackCollectionKey))

    if flags.Tracking.doTruth:
        acc.merge(ITkTrackTruthCfg(
            flags,
            Tracks=ResolvedTrackCollectionKey,
            DetailedTruth=ResolvedTrackCollectionKey+"DetailedTruth",
            TracksTruth=ResolvedTrackCollectionKey+"TruthCollection"))

    return acc
