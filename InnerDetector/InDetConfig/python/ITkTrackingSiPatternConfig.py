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

    runTruth = True
    runActsTrackTruth = False

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

        runTruth = flags.Tracking.ActiveConfig.doAthenaTrack or flags.Tracking.ActiveConfig.doActsToAthenaTrack

        # Athena Track
        if flags.Tracking.ActiveConfig.doAthenaTrack:

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
        if flags.Tracking.ActiveConfig.doActsSeed:

            from ActsConfig.ActsSeedingConfig import (
                ActsSeedingCfg)
            acc.merge(ActsSeedingCfg(flags))

            if flags.Tracking.ActiveConfig.extension == "Conversion":
                from AthenaCommon.Logging import logging
                log = logging.getLogger('ITkTrackingSiPattern')
                log.warning(
                    'ROI-based track-finding is not available yet in ACTS, so the default one is used')

        # ACTS track
        if flags.Tracking.ActiveConfig.doActsTrack:
            from ActsConfig.ActsTrackFindingConfig import ActsTrackFindingCfg
            acc.merge(ActsTrackFindingCfg(flags))
            runActsTrackTruth = flags.Tracking.doTruth


        # Convert Tracks Acts -> Athena (before ambi)
        if flags.Tracking.ActiveConfig.doActsToAthenaTrack:
            from ActsConfig.ActsEventCnvConfig import ActsToTrkConvertorAlgCfg
            acc.merge(ActsToTrkConvertorAlgCfg(flags,
                                               TracksLocation=SiSPSeededTrackCollectionKey))

        # Convert tracks Athena -> Acts (before ambi)
        if flags.Tracking.ActiveConfig.doAthenaToActsTrack:
            from ActsConfig.ActsEventCnvConfig import TrkToActsConvertorAlgCfg
            acc.merge(TrkToActsConvertorAlgCfg(flags,
                                               TrackContainerLocation="ActsTracks",
                                               TrackCollectionKeys=[SiSPSeededTrackCollectionKey]))
            
    from InDetConfig.ITkTrackTruthConfig import ITkTrackTruthCfg
    if flags.Tracking.doTruth and runTruth:
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
    runTruth = True

    if flags.Tracking.doITkFastTracking:

        from TrkConfig.TrkCollectionAliasAlgConfig import CopyAlgForAmbiCfg
        acc.merge(CopyAlgForAmbiCfg(
            flags,
            "ITkCopyAlgForAmbi"+flags.Tracking.ActiveConfig.extension,
            CollectionName=SiSPSeededTrackCollectionKey,  # Input
            AliasName=ResolvedTrackCollectionKey))       # Output

    else:
        # If we run Athena tracking we also want CTIDE ambi
        if flags.Tracking.ActiveConfig.doAthenaAmbiguityResolution:
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
            
            runTruth = True

        # If we run Acts tracking we may want Acts ambi, depending on the flag
        if flags.Tracking.ActiveConfig.doActsAmbiguityResolution:
            # Schedule ACTS ambi. resolution and eventually the track convertions  
            from ActsConfig.ActsTrackFindingConfig import ActsAmbiguityResolutionCfg
            acc.merge(ActsAmbiguityResolutionCfg(flags))
            runTruth = False

        if flags.Tracking.ActiveConfig.doActsToAthenaResolvedTrack:
            from ActsConfig.ActsEventCnvConfig import ActsToTrkConvertorAlgCfg
            acc.merge(ActsToTrkConvertorAlgCfg(flags,
                                               TracksLocation=ResolvedTrackCollectionKey))                
            runTruth = False
        

    if flags.Tracking.doTruth and runTruth:
        acc.merge(ITkTrackTruthCfg(
            flags,
            Tracks=ResolvedTrackCollectionKey,
            DetailedTruth=ResolvedTrackCollectionKey+"DetailedTruth",
            TracksTruth=ResolvedTrackCollectionKey+"TruthCollection"))

    if runActsTrackTruth :
        from ActsConfig.ActsTruthConfig import TrackToTruthAssociationCfg
        acts_tracks='ActsTracks' if not flags.Acts.doAmbiguityResolution else 'ResolvedActsTracks'
        acc.merge(TrackToTruthAssociationCfg(flags,
                                             ACTSTracksLocation=acts_tracks,
                                             AssociationMapOut=acts_tracks+"ToTruthParticleAssociation"))


    return acc
