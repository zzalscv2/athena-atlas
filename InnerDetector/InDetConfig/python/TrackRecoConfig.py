# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.Enums import BeamType, Format
from TrkConfig.TrackingPassFlags import printActiveConfig

_flags_set = []  # For caching
_extensions_list = [] # For caching

def CombinedTrackingPassFlagSets(flags):

    global _flags_set
    if _flags_set:
        return _flags_set

    flags_set = []

    # Primary Pass
    flags = flags.cloneAndReplace(
        "Tracking.ActiveConfig",
        f"Tracking.{flags.Tracking.PrimaryPassConfig.value}Pass")
    flags_set += [flags]

    # LRT pass
    if flags.Tracking.doLargeD0 or flags.Tracking.doLowPtLargeD0:

        if flags.Tracking.doLowPtLargeD0:
            flagsLRT = flags.cloneAndReplace("Tracking.ActiveConfig",
                                             "Tracking.LowPtLargeD0Pass")
        elif flags.Tracking.doLargeD0:
            flagsLRT = flags.cloneAndReplace("Tracking.ActiveConfig",
                                             "Tracking.R3LargeD0Pass")

        flags_set += [flagsLRT]

    # LowPtRoI pass (low-pt tracks in high pile-up enviroment)
    if flags.Tracking.doLowPtRoI:
        flagsLowPtRoI = flags.cloneAndReplace("Tracking.ActiveConfig",
                                              "Tracking.LowPtRoIPass")
        flags_set += [flagsLowPtRoI]

    # LowPt pass
    if flags.Tracking.doLowPt:
        flagsLowPt = flags.cloneAndReplace("Tracking.ActiveConfig",
                                           "Tracking.LowPtPass")
        flags_set += [flagsLowPt]

    # VeryLowPt pass
    if flags.Tracking.doVeryLowPt:
        flagsVeryLowPt = flags.cloneAndReplace("Tracking.ActiveConfig",
                                               "Tracking.VeryLowPtPass")
        flags_set += [flagsVeryLowPt]

    # TRT standalone pass
    if flags.Tracking.doTRTStandalone:
        flagsTRTStandalone = flags.cloneAndReplace("Tracking.ActiveConfig",
                                                   "Tracking.TRTStandalonePass")
        flags_set += [flagsTRTStandalone]

    # Forward tracklets
    if flags.Tracking.doForwardTracks:
        flagsForward = flags.cloneAndReplace("Tracking.ActiveConfig",
                                             "Tracking.ForwardPass")
        flags_set += [flagsForward]

    # Disappearing pixel tracklets
    if flags.Tracking.doTrackSegmentsDisappearing:
        flagsDisappearing = flags.cloneAndReplace("Tracking.ActiveConfig",
                                                  "Tracking.DisappearingPass")
        flags_set += [flagsDisappearing]

    # Beam gas
    if flags.Tracking.doBeamGas:
        flagsBeamGas = flags.cloneAndReplace("Tracking.ActiveConfig",
                                             "Tracking.BeamGasPass")
        flags_set += [flagsBeamGas]

    _flags_set = flags_set  # Put into cache

    return flags_set


def ClusterSplitProbabilityContainerName(flags):
    if flags.Detector.GeometryITk:
        from InDetConfig.ITkTrackRecoConfig import (
            ITkClusterSplitProbabilityContainerName)
        return ITkClusterSplitProbabilityContainerName(flags)

    flags_set = CombinedTrackingPassFlagSets(flags)
    extension = flags_set[-1].Tracking.ActiveConfig.extension

    # No ambi processing for TRT standalone, so pick the previous pass
    if extension == "TRTStandalone":
        extension = flags_set[-2].Tracking.ActiveConfig.extension

    ClusterSplitProbContainer = "InDetAmbiguityProcessorSplitProb" + extension

    # Only primary pass + back-tracking
    if len(flags_set) == 1 and flags.Tracking.doBackTracking:
        ClusterSplitProbContainer = (
            "InDetTRT_SeededAmbiguityProcessorSplitProb" + extension)
    return ClusterSplitProbContainer

# TODO find config to validate this


def InDetCosmicsTrackRecoPreProcessingCfg(flags):
    result = ComponentAccumulator()

    from InDetConfig.TrackingSiPatternConfig import TrackingSiPatternCfg
    result.merge(TrackingSiPatternCfg(
        flags,
        InputCollections=[],
        ResolvedTrackCollectionKey="ResolvedTracks",
        SiSPSeededTrackCollectionKey="SiSPSeededTracks"))

    from InDetConfig.TRTExtensionConfig import NewTrackingTRTExtensionPhaseCfg
    result.merge(NewTrackingTRTExtensionPhaseCfg(
        flags,
        SiTrackCollection="ResolvedTracks",
        ExtendedTrackCollection="ExtendedTracksPhase",
        ExtendedTracksMap="ExtendedTracksMapPhase"))

    from InDetConfig.TRTSegmentFindingConfig import TRTSegmentFinding_Phase_Cfg
    result.merge(TRTSegmentFinding_Phase_Cfg(flags))

    from InDetConfig.InDetTrackPRD_AssociationConfig import (
        InDetTrackPRD_AssociationCfg)
    result.merge(InDetTrackPRD_AssociationCfg(
        flags, name='InDetTRTonly_TrackPRD_AssociationPhase',
        AssociationMapName='InDetTRTonly_PRDtoTrackMapPhase',
        TracksName=[]))

    from InDetConfig.TRT_SegmentsToTrackConfig import (
        TRT_Cosmics_SegmentsToTrackCfg)
    result.merge(TRT_Cosmics_SegmentsToTrackCfg(
        flags, name='InDetTRT_Cosmics_SegmentsToTrack_Phase',
        InputSegmentsCollection="TRTSegments_Phase",
        OutputTrackCollection="TRT_Tracks_Phase"))

    from InDetConfig.InDetCosmicsEventPhaseConfig import (
        InDetCosmicsEventPhaseCfg)
    result.merge(InDetCosmicsEventPhaseCfg(
        flags,
        InputTracksNames=["TRT_Tracks_Phase"]))

    from InDetConfig.InDetPrepRawDataFormationConfig import (
        InDetTRT_Phase_RIO_MakerCfg)
    result.merge(InDetTRT_Phase_RIO_MakerCfg(flags))

    return result


def InDetPreProcessingCfg(flags):
    result = ComponentAccumulator()

    # Detector ByteStream pre-processing
    if flags.Input.Format is Format.BS:
        from PixelRawDataByteStreamCnv.PixelRawDataByteStreamCnvConfig import (
            PixelRawDataProviderAlgCfg)
        result.merge(PixelRawDataProviderAlgCfg(flags))

        from SCT_RawDataByteStreamCnv.SCT_RawDataByteStreamCnvConfig import (
            SCTRawDataProviderCfg, SCTEventFlagWriterCfg)
        result.merge(SCTRawDataProviderCfg(flags))
        result.merge(SCTEventFlagWriterCfg(flags))

        from TRT_RawDataByteStreamCnv.TRT_RawDataByteStreamCnvConfig import (
            TRTRawDataProviderCfg)
        result.merge(TRTRawDataProviderCfg(flags))

    from InDetConfig.SiliconPreProcessing import InDetRecPreProcessingSiliconCfg
    result.merge(InDetRecPreProcessingSiliconCfg(flags))
    from InDetConfig.TRTPreProcessing import TRTPreProcessingCfg
    result.merge(TRTPreProcessingCfg(flags))

    return result


# Returns CA + ClusterSplitProbContainer
def SiSubDetTrackRecoCfg(flags, detector="",
                         extensions_list=[],
                         ClusterSplitProbContainer=""):
    result = ComponentAccumulator()

    flagsDet = flags.cloneAndReplace("Tracking.ActiveConfig",
                                     f"Tracking.{detector}Pass")
    TrackContainer = f"Resolved{detector}Tracks"
    extensions_list.append(flagsDet.Tracking.ActiveConfig.extension)
    printActiveConfig(flagsDet)

    from InDetConfig.TrackingSiPatternConfig import TrackingSiPatternCfg
    result.merge(TrackingSiPatternCfg(
        flagsDet,
        InputCollections=[],
        ResolvedTrackCollectionKey=TrackContainer,
        SiSPSeededTrackCollectionKey=f"SiSPSeeded{detector}Tracks",
        ClusterSplitProbContainer=ClusterSplitProbContainer))

    ClusterSplitProbContainer = (
        "InDetAmbiguityProcessorSplitProb" +
        flagsDet.Tracking.ActiveConfig.extension)

    if flags.Tracking.doTruth:
        from InDetConfig.TrackTruthConfig import InDetTrackTruthCfg
        result.merge(InDetTrackTruthCfg(
            flagsDet,
            Tracks=TrackContainer,
            DetailedTruth=TrackContainer+"DetailedTruth",
            TracksTruth=TrackContainer+"TruthCollection"))

    from xAODTrackingCnv.xAODTrackingCnvConfig import (
        TrackParticleCnvAlgNoPIDCfg)
    result.merge(TrackParticleCnvAlgNoPIDCfg(
        flags,
        name=TrackContainer+"CnvAlg",
        TrackContainerName=TrackContainer,
        xAODTrackParticlesFromTracksContainerName=(
            f"InDet{detector}TrackParticles")))

    return result, ClusterSplitProbContainer


def TRTTrackRecoCfg(flags, extensions_list=[]):
    result = ComponentAccumulator()

    flagsTRT = flags.cloneAndReplace("Tracking.ActiveConfig",
                                     "Tracking.TRTPass")
    extensions_list.append(flagsTRT.Tracking.ActiveConfig.extension)
    printActiveConfig(flagsTRT)

    from InDetConfig.TRTSegmentFindingConfig import (
        TRTSegmentFinding_TrackSegments_Cfg)
    result.merge(TRTSegmentFinding_TrackSegments_Cfg(flagsTRT))

    from InDetConfig.TRTStandaloneConfig import TRT_TrackSegment_Cfg
    result.merge(TRT_TrackSegment_Cfg(flagsTRT))

    return result


def TRTStandalonePassRecoCfg(flags,
                             InputCombinedInDetTracks=[],
                             InputExtendedInDetTracks=[],
                             StatTrackCollections=[],
                             StatTrackTruthCollections=[]):
    result = ComponentAccumulator()

    from InDetConfig.TRTStandaloneConfig import TRTStandaloneCfg
    result.merge(TRTStandaloneCfg(flags,
                                  InputCollections=InputCombinedInDetTracks))

    TRTTrackContainer = "TRTStandaloneTracks"
    InputCombinedInDetTracks += [TRTTrackContainer]
    InputExtendedInDetTracks += [TRTTrackContainer]
    StatTrackCollections += [TRTTrackContainer]
    StatTrackTruthCollections += [TRTTrackContainer+"TruthCollection"]

    if flags.Tracking.doTrackSegmentsTRT:
        from xAODTrackingCnv.xAODTrackingCnvConfig import (
            TrackParticleCnvAlgNoPIDCfg)
        result.merge(TrackParticleCnvAlgNoPIDCfg(
            flags, TRTTrackContainer+"CnvAlg",
            TrackContainerName=TRTTrackContainer,
            xAODTrackParticlesFromTracksContainerName=(
                "InDetTRTTrackParticles")))

    return result


# Return CA + TrackContainer
def StoreTrackSeparateContainerCfg(flags, TrackContainer="",
                                   ClusterSplitProbContainer=""):
    result = ComponentAccumulator()

    # Dummy Merger to fill additional info
    # for PRD-associated pixel tracklets
    # Can also run on all separate collections like R3LargeD0
    # but kept consistent with legacy config

    AssociationMapName = ""
    extension = flags.Tracking.ActiveConfig.extension

    if extension == "Disappearing" or flags.Overlay.doTrackOverlay:
        if extension == "Disappearing":
            InputTracks = [TrackContainer]
            if flags.Overlay.doTrackOverlay:
                InputTracks += [flags.Overlay.BkgPrefix +
                                extension + "Tracks"]
            TrackContainer = extension+"Tracks"
            AssociationMapName = "PRDtoTrackMap" + TrackContainer
            MergerOutputTracks = TrackContainer
        elif flags.Overlay.doTrackOverlay:
            # schedule merger to combine signal and background tracks
            InputTracks = [flags.Overlay.SigPrefix+TrackContainer,
                           flags.Overlay.BkgPrefix+TrackContainer]
            AssociationMapName = ("PRDtoTrackMapResolved" +
                                  extension + "Tracks")
            MergerOutputTracks = TrackContainer

        from TrkConfig.TrkTrackCollectionMergerConfig import (
            TrackCollectionMergerAlgCfg)
        result.merge(TrackCollectionMergerAlgCfg(
            flags, "InDetTrackCollectionMerger"+extension,
            InputCombinedTracks=InputTracks,
            OutputCombinedTracks=MergerOutputTracks,
            AssociationMapName=AssociationMapName))

    if flags.Tracking.doTruth:
        from InDetConfig.TrackTruthConfig import InDetTrackTruthCfg
        result.merge(InDetTrackTruthCfg(
            flags,
            Tracks=TrackContainer,
            DetailedTruth=TrackContainer+"DetailedTruth",
            TracksTruth=TrackContainer+"TruthCollection"))

    # Need specific handling with TrackParticles for R3LargeD0
    # not to break downstream configs
    xAODTrackParticlesName = (
        "InDetLargeD0TrackParticles" if "LargeD0" in extension else
        "InDet" + extension + "TrackParticles")

    from xAODTrackingCnv.xAODTrackingCnvConfig import (
        TrackParticleCnvAlgPIDCheckCfg)
    result.merge(TrackParticleCnvAlgPIDCheckCfg(
        flags, extension + "TrackParticleCnvAlg",
        TrackContainerName=TrackContainer,
        xAODTrackParticlesFromTracksContainerName=xAODTrackParticlesName,
        ClusterSplitProbabilityName=ClusterSplitProbContainer,
        AssociationMapName=AssociationMapName))

    return result, TrackContainer


# Returns CA + ClusterSplitProbContainer
def BackTrackingRecoCfg(flags,
                        InputCombinedInDetTracks=[],
                        InputExtendedInDetTracks=[],
                        StatTrackCollections=[],
                        StatTrackTruthCollections=[],
                        ClusterSplitProbContainer=""):
    result = ComponentAccumulator()

    from InDetConfig.BackTrackingConfig import BackTrackingCfg
    result.merge(BackTrackingCfg(
        flags,
        InputCollections=InputCombinedInDetTracks,
        ClusterSplitProbContainer=ClusterSplitProbContainer))

    ClusterSplitProbContainer = (
        "InDetTRT_SeededAmbiguityProcessorSplitProb" +
        flags.Tracking.ActiveConfig.extension)
    TRTSeededTracks = "TRTSeededTracks"
    ResolvedTRTSeededTracks = "ResolvedTRTSeededTracks"
    InputCombinedInDetTracks += [ResolvedTRTSeededTracks]
    InputExtendedInDetTracks += [ResolvedTRTSeededTracks]
    StatTrackCollections += [TRTSeededTracks,
                             ResolvedTRTSeededTracks]
    StatTrackTruthCollections += [TRTSeededTracks+"TruthCollection",
                                  ResolvedTRTSeededTracks+"TruthCollection"]

    return result, ClusterSplitProbContainer


# Returns CA + ClusterSplitProbContainer
def TrackRecoPassCfg(flags, extension="",
                     doTrackingSiPattern=True,
                     InputCombinedInDetTracks=[],
                     InputExtendedInDetTracks=[],
                     StatTrackCollections=[],
                     StatTrackTruthCollections=[],
                     ClusterSplitProbContainer=""):
    result = ComponentAccumulator()

    ResolvedTracks = "Resolved" + extension + "Tracks"

    # for track overlay, save resolved track name
    # for final merged track collection
    if (flags.Overlay.doTrackOverlay and
        flags.Tracking.ActiveConfig.storeSeparateContainer and
        not flags.Tracking.ActiveConfig.useTRTExtension):
        ResolvedTracks = flags.Overlay.SigPrefix + ResolvedTracks

    # Tweak to match old config key
    if "LargeD0" in extension:
        ResolvedTracks = "ResolvedLargeD0Tracks"

    # ---------------------------------------
    # --- Si pattern, if not done in the cosmic preprocessing
    # ---------------------------------------

    if doTrackingSiPattern:
        SiSPSeededTracks = "SiSPSeeded" + extension + "Tracks"
        from InDetConfig.TrackingSiPatternConfig import TrackingSiPatternCfg
        result.merge(TrackingSiPatternCfg(
            flags,
            InputCollections=InputExtendedInDetTracks,
            ResolvedTrackCollectionKey=ResolvedTracks,
            SiSPSeededTrackCollectionKey=SiSPSeededTracks,
            ClusterSplitProbContainer=ClusterSplitProbContainer))
        StatTrackCollections += [SiSPSeededTracks, ResolvedTracks]
        StatTrackTruthCollections += [SiSPSeededTracks+"TruthCollection",
                                      ResolvedTracks+"TruthCollection"]

    TrackContainer = ResolvedTracks
    if (flags.Overlay.doTrackOverlay and
        flags.Tracking.ActiveConfig.storeSeparateContainer):
        TrackContainer = "Resolved" + extension + "Tracks"

    # ---------------------------------------
    # --- TRT extension
    # ---------------------------------------

    if flags.Tracking.ActiveConfig.useTRTExtension:
        ExtendedTracks = "Extended" + extension + "Tracks"
        # Tweaks to match old config key
        if extension == "Disappearing":
            ExtendedTracks = "ExtendedTracksDisappearing"
        elif "LargeD0" in extension:
            ExtendedTracks = "ExtendedLargeD0Tracks"
            if flags.Overlay.doTrackOverlay:
                ExtendedTracks = flags.Overlay.SigPrefix+"ExtendedLargeD0Tracks"
        ExtendedTracksMap = "ExtendedTracksMap" + extension

        from InDetConfig.TRTExtensionConfig import NewTrackingTRTExtensionCfg
        result.merge(NewTrackingTRTExtensionCfg(
            flags,
            SiTrackCollection=ResolvedTracks,
            ExtendedTrackCollection=ExtendedTracks,
            ExtendedTracksMap=ExtendedTracksMap))

        TrackContainer = ExtendedTracks
        if flags.Overlay.doTrackOverlay and "LargeD0" in extension:
            TrackContainer = "ExtendedLargeD0Tracks"
        StatTrackCollections += [ExtendedTracks]
        StatTrackTruthCollections += [ExtendedTracks+"TruthCollection"]

    # ---------------------------------------
    # --- Store separate container if needed
    # ---------------------------------------

    if flags.Tracking.ActiveConfig.storeSeparateContainer:
        acc, TrackContainer = StoreTrackSeparateContainerCfg(
            flags,
            TrackContainer=TrackContainer,
            ClusterSplitProbContainer=ClusterSplitProbContainer)
        result.merge(acc)

    else:  # Do not store separate track container
        ClusterSplitProbContainer = (
            "InDetAmbiguityProcessorSplitProb" +
            flags.Tracking.ActiveConfig.extension)
        InputCombinedInDetTracks += [TrackContainer]

    InputExtendedInDetTracks += [TrackContainer]

    return result, ClusterSplitProbContainer

def TrackFinalCfg(flags,
                  InputCombinedInDetTracks=[],
                  StatTrackCollections=[],
                  StatTrackTruthCollections=[]):
    result = ComponentAccumulator()

    if flags.Overlay.doTrackOverlay:
        InputCombinedInDetTracks += [flags.Overlay.BkgPrefix +
                                     "CombinedInDetTracks"]

    TrackContainer = "CombinedInDetTracks"

    from TrkConfig.TrkTrackCollectionMergerConfig import (
        TrackCollectionMergerAlgCfg)
    result.merge(TrackCollectionMergerAlgCfg(
        flags,
        InputCombinedTracks=InputCombinedInDetTracks,
        OutputCombinedTracks=TrackContainer,
        AssociationMapName=f"PRDtoTrackMap{TrackContainer}"))

    if flags.Tracking.doTruth:
        from InDetConfig.TrackTruthConfig import InDetTrackTruthCfg
        result.merge(InDetTrackTruthCfg(
            flags,
            Tracks=TrackContainer,
            DetailedTruth=f"{TrackContainer}DetailedTruth",
            TracksTruth=f"{TrackContainer}TruthCollection"))

    StatTrackCollections += [TrackContainer]
    StatTrackTruthCollections += [f"{TrackContainer}TruthCollection"]

    if flags.Tracking.doSlimming:
        from TrkConfig.TrkTrackSlimmerConfig import TrackSlimmerCfg
        result.merge(TrackSlimmerCfg(
            flags,
            TrackLocation=[TrackContainer]))

    if flags.Tracking.perigeeExpression == "BeamLine":
        from xAODTrackingCnv.xAODTrackingCnvConfig import TrackParticleCnvAlgCfg
        result.merge(TrackParticleCnvAlgCfg(
            flags,
            ClusterSplitProbabilityName=ClusterSplitProbabilityContainerName(
                flags),
            AssociationMapName=f"PRDtoTrackMap{TrackContainer}"))

    return result

def PseudoTrackFinalCfg(flags):
    result = ComponentAccumulator()

    TrackContainer = "InDetPseudoTracks"
    if flags.Tracking.doTruth:
        from InDetConfig.TrackTruthConfig import InDetTrackTruthCfg
        result.merge(InDetTrackTruthCfg(
            flags,
            Tracks=TrackContainer,
            DetailedTruth=TrackContainer + "DetailedTruth",
            TracksTruth=TrackContainer + "TruthCollection"))

    from xAODTrackingCnv.xAODTrackingCnvConfig import TrackParticleCnvAlgCfg
    result.merge(TrackParticleCnvAlgCfg(
        flags,
        name="PseudoTrackParticleCnvAlg",
        TrackContainerName=TrackContainer,
        xAODTrackParticlesFromTracksContainerName=(
            "InDetPseudoTrackParticles"),
        AssociationMapName=(
            "PRDtoTrackMapCombinedInDetTracks")))

    return result


def ObserverTrackFinalCfg(flags):
    result = ComponentAccumulator()

    TrackContainer = "ObservedTracksCollection"
    if flags.Tracking.doTruth:
        from InDetConfig.TrackTruthConfig import InDetTrackTruthCfg
        result.merge(InDetTrackTruthCfg(
            flags,
            Tracks=TrackContainer,
            DetailedTruth=TrackContainer + "DetailedTruth",
            TracksTruth=TrackContainer + "TruthCollection"))

    from xAODTrackingCnv.xAODTrackingCnvConfig import (
        ObserverTrackParticleCnvAlgCfg)
    result.merge(ObserverTrackParticleCnvAlgCfg(
        flags,
        AssociationMapName = (
            "PRDtoTrackMapCombinedInDetTracks")))

    return result


##############################################################################
#####################     Main ID tracking config       #####################
##############################################################################

def InDetTrackRecoCfg(flags):

    # Bypass to ITk config
    if flags.Detector.GeometryITk:
        from InDetConfig.ITkTrackRecoConfig import ITkTrackRecoCfg
        return ITkTrackRecoCfg(flags)

    """Configures complete ID tracking """
    result = ComponentAccumulator()
    result.merge(InDetPreProcessingCfg(flags))

    ClusterSplitProbContainer = ''

    from xAODTrackingCnv.xAODTrackingCnvConfig import (
        TrackParticleCnvAlgNoPIDCfg)

    # ------------------------------------------------------------
    #
    # ----------- Subdetector pattern from New Tracking
    #
    # ------------------------------------------------------------

    # Pixel track segment finding
    if flags.Tracking.doTrackSegmentsPixel:
        acc, ClusterSplitProbContainer = (
            SiSubDetTrackRecoCfg(flags, detector="Pixel",
                                 extensions_list=_extensions_list,
                                 ClusterSplitProbContainer=ClusterSplitProbContainer))
        result.merge(acc)

    # SCT track segment finding
    if flags.Tracking.doTrackSegmentsSCT:
        acc, ClusterSplitProbContainer = (
            SiSubDetTrackRecoCfg(flags, detector="SCT",
                                 extensions_list=_extensions_list,
                                 ClusterSplitProbContainer=ClusterSplitProbContainer))
        result.merge(acc)

    # TRT track segment finding
    if flags.Tracking.doTrackSegmentsTRT:
        result.merge(TRTTrackRecoCfg(flags, extensions_list=_extensions_list))

    flags_set = CombinedTrackingPassFlagSets(flags)

    # Pre-processing for TRT phase in cosmics
    if flags.Beam.Type is BeamType.Cosmics:
        flagsCosmics = flags_set[0]
        result.merge(InDetCosmicsTrackRecoPreProcessingCfg(flagsCosmics))

    # ------------------------------------------------------------
    #
    # ----------- Main passes for standard reconstruction
    #
    # ------------------------------------------------------------

    # Tracks to be ultimately merged in InDetTrackParticle collection
    InputCombinedInDetTracks = []
    # Includes also tracks which end in standalone TrackParticle collections
    InputExtendedInDetTracks = []
    ClusterSplitProbContainer = ""
    StatTrackCollections = []  # To be passed to the InDetRecStatistics alg
    StatTrackTruthCollections = []
    isPrimaryPass = True

    for current_flags in flags_set:
        printActiveConfig(current_flags)

        extension = (
            "" if isPrimaryPass else
            current_flags.Tracking.ActiveConfig.extension)
        _extensions_list.append(extension)

        # ---------------------------------------
        # ----   TRTStandalone pass
        # ---------------------------------------

        if flags.Tracking.doTRTStandalone and extension == "TRTStandalone":
            result.merge(TRTStandalonePassRecoCfg(
                current_flags,
                InputCombinedInDetTracks=InputCombinedInDetTracks,
                InputExtendedInDetTracks=InputExtendedInDetTracks,
                StatTrackCollections=StatTrackCollections,
                StatTrackTruthCollections=StatTrackTruthCollections))

            continue  # Skip rest of config for the TRTStandalone pass

        # ---------------------------------------
        # ----   All the passes but TRTStandalone
        # ---------------------------------------

        acc, ClusterSplitProbContainer = TrackRecoPassCfg(
            current_flags, extension=extension,
            doTrackingSiPattern=not(isPrimaryPass and
                                    flags.Beam.Type is BeamType.Cosmics),
            InputCombinedInDetTracks=InputCombinedInDetTracks,
            InputExtendedInDetTracks=InputExtendedInDetTracks,
            StatTrackCollections=StatTrackCollections,
            StatTrackTruthCollections=StatTrackTruthCollections,
            ClusterSplitProbContainer=ClusterSplitProbContainer)
        result.merge(acc)

        # ---------------------------------------
        # --- A few passes, only after primary pass
        # ---------------------------------------

        if isPrimaryPass:

            # ---------------------------------------
            # --- TRT Segments
            # ---------------------------------------

            if flags.Tracking.doTRTSegments:
                from InDetConfig.TRTSegmentFindingConfig import (
                    TRTSegmentFindingCfg)
                result.merge(TRTSegmentFindingCfg(
                    current_flags,
                    InputCollections=InputCombinedInDetTracks))

            # ---------------------------------------
            # --- BackTracking
            # ---------------------------------------

            if flags.Tracking.doBackTracking:
                acc, ClusterSplitProbContainer = BackTrackingRecoCfg(
                    current_flags,
                    InputCombinedInDetTracks=InputCombinedInDetTracks,
                    InputExtendedInDetTracks=InputExtendedInDetTracks,
                    StatTrackCollections=StatTrackCollections,
                    StatTrackTruthCollections=StatTrackTruthCollections,
                    ClusterSplitProbContainer=ClusterSplitProbContainer)
                result.merge(acc)

            # ---------------------------------------
            # --- PseudoTracking
            # ---------------------------------------

            if (flags.Tracking.doTruth and
                (flags.Tracking.doPseudoTracking or
                 flags.Tracking.doIdealPseudoTracking)):

                from TrkConfig.TrkTruthTrackAlgsConfig import TruthTrackingCfg
                result.merge(TruthTrackingCfg(current_flags))

            isPrimaryPass = False

    # ----------------------------------------------------
    # --- Loop over tracking passes is done, final configs
    # ----------------------------------------------------

    result.merge(
        TrackFinalCfg(flags,
                      InputCombinedInDetTracks=InputCombinedInDetTracks,
                      StatTrackCollections=StatTrackCollections,
                      StatTrackTruthCollections=StatTrackTruthCollections))

    if (flags.Tracking.doPseudoTracking or
        flags.Tracking.doIdealPseudoTracking):
        result.merge(PseudoTrackFinalCfg(flags))

    if flags.Tracking.doTIDE_AmbiTrackMonitoring:
        result.merge(ObserverTrackFinalCfg(flags))

    if flags.Tracking.doStoreTrackSeeds:
        # get list of extensions requesting track seeds. Add always the Primary Pass.
        listOfExtensionsRequesting = [ e for e in _extensions_list if (e == '' or flags.Tracking.__getattr__(e+'Pass').storeTrackSeeds) ]
        for extension in listOfExtensionsRequesting:
            TrackContainer = "SiSPSeedSegments"+extension

            if flags.Tracking.doTruth:
                from InDetConfig.TrackTruthConfig import InDetTrackTruthCfg
                result.merge(InDetTrackTruthCfg(
                    flags,
                    Tracks=TrackContainer,
                    DetailedTruth=f"{TrackContainer}DetailedTruth",
                    TracksTruth=f"{TrackContainer}TruthCollection"))

            result.merge(TrackParticleCnvAlgNoPIDCfg(
                flags,
                name=f"SiSPSeedSegments{extension}CnvAlg",
                TrackContainerName=TrackContainer,
                xAODTrackParticlesFromTracksContainerName=(
                    f"{TrackContainer}TrackParticles")))

    if flags.Tracking.doStoreSiSPSeededTracks:
        from xAODTrackingCnv.xAODTrackingCnvConfig import (
            TrackParticleCnvAlgNoPIDCfg)
        # get list of extensions requesting track candidates. Add always the Primary Pass.
        listOfExtensionsRequesting = [ e for e in _extensions_list if (e == '' or flags.Tracking.__getattr__(e+'Pass').storeSiSPSeededTracks) ]
        for extension in listOfExtensionsRequesting:
            AssociationMapNameKey="PRDtoTrackMapCombinedInDetTracks"
            if extension=='Disappearing': AssociationMapNameKey = "PRDtoTrackMapDisappearingTracks"
            elif not (extension == ''): AssociationMapNameKey = f"InDetPRDtoTrackMap{extension}"
            result.merge(TrackParticleCnvAlgNoPIDCfg(
                flags,
                name = f"SiSPSeededTracks{extension}CnvAlg",
                TrackContainerName = f"SiSPSeeded{extension}Tracks",
                xAODTrackParticlesFromTracksContainerName=(
                    f"SiSPSeededTracks{extension}TrackParticles"),
                AssociationMapName=AssociationMapNameKey))

    # ---------------------------------------
    # --- Primary vertexing
    # ---------------------------------------

    if flags.Tracking.doVertexFinding:
        from InDetConfig.InDetPriVxFinderConfig import primaryVertexFindingCfg
        result.merge(primaryVertexFindingCfg(flags))

    if flags.Tracking.doStats:
        from InDetConfig.InDetRecStatisticsConfig import (
            InDetRecStatisticsAlgCfg)
        result.merge(InDetRecStatisticsAlgCfg(
            flags,
            TrackCollectionKeys=StatTrackCollections,
            TrackTruthCollectionKeys=(
                StatTrackTruthCollections if flags.Tracking.doTruth else [])))

        if flags.Tracking.doTruth:
            from InDetConfig.InDetTrackClusterAssValidationConfig import (
                InDetTrackClusterAssValidationCfg)
            result.merge(InDetTrackClusterAssValidationCfg(
                flags_set[0],  # Use cuts from primary pass
                TracksLocation=StatTrackCollections))

    # ---------------------------------------
    # --- Extra optional decorations
    # ---------------------------------------

    if flags.Tracking.doV0Finder:
        from InDetConfig.InDetV0FinderConfig import InDetV0FinderCfg
        result.merge(InDetV0FinderCfg(flags))

    if (flags.Tracking.writeExtendedSi_PRDInfo or
        flags.Tracking.writeExtendedTRT_PRDInfo):

        if (flags.Tracking.doTIDE_AmbiTrackMonitoring or
                flags.Tracking.doPseudoTracking):

            if flags.Tracking.writeExtendedSi_PRDInfo:
                from InDetConfig.InDetPrepRawDataToxAODConfig import (
                    InDetPixelPrepDataToxAOD_ExtraTruthCfg,
                    InDetSCT_PrepDataToxAOD_ExtraTruthCfg)
                result.merge(InDetPixelPrepDataToxAOD_ExtraTruthCfg(
                    flags,
                    ClusterSplitProbabilityName=(
                        ClusterSplitProbabilityContainerName(flags))))
                result.merge(InDetSCT_PrepDataToxAOD_ExtraTruthCfg(flags))

            if flags.Tracking.writeExtendedTRT_PRDInfo:
                from InDetConfig.InDetPrepRawDataToxAODConfig import (
                    InDetTRT_PrepDataToxAOD_ExtraTruthCfg)
                result.merge(InDetTRT_PrepDataToxAOD_ExtraTruthCfg(flags))

        else:

            if flags.Tracking.writeExtendedSi_PRDInfo:
                from InDetConfig.InDetPrepRawDataToxAODConfig import (
                    InDetPixelPrepDataToxAODCfg,
                    InDetSCT_PrepDataToxAODCfg)
                result.merge(InDetPixelPrepDataToxAODCfg(
                    flags,
                    ClusterSplitProbabilityName=(
                        ClusterSplitProbabilityContainerName(flags))))
                result.merge(InDetSCT_PrepDataToxAODCfg(flags))

            if flags.Tracking.writeExtendedTRT_PRDInfo:
                from InDetConfig.InDetPrepRawDataToxAODConfig import (
                    InDetTRT_PrepDataToxAODCfg)
                result.merge(InDetTRT_PrepDataToxAODCfg(flags))

        from DerivationFrameworkInDet.InDetToolsConfig import (
            TSOS_CommonKernelCfg)
        # Setup one algorithm for each output tracking container
        listOfExtensionsRequesting = [
            e for e in _extensions_list if (e == '') or
            (flags.Tracking.__getattr__(e+'Pass').storeSiSPSeededTracks and
             flags.Tracking.__getattr__(e+'Pass').storeSeparateContainer) ]
        result.merge(TSOS_CommonKernelCfg(
            flags, listOfExtensions = listOfExtensionsRequesting))

        if flags.Tracking.doTIDE_AmbiTrackMonitoring:
            from DerivationFrameworkInDet.InDetToolsConfig import (
                ObserverTSOS_CommonKernelCfg)
            result.merge(ObserverTSOS_CommonKernelCfg(flags))

        if flags.Tracking.doPseudoTracking:
            from DerivationFrameworkInDet.InDetToolsConfig import (
                PseudoTSOS_CommonKernelCfg)
            result.merge(PseudoTSOS_CommonKernelCfg(flags))

        if flags.Tracking.doStoreSiSPSeededTracks:
            from DerivationFrameworkInDet.InDetToolsConfig import (
                SiSPTSOS_CommonKernelCfg)
            # Setup one algorithm for each output tracking container
            listOfExtensionsRequesting = [
                e for e in _extensions_list if (e == '') or
                flags.Tracking.__getattr__(e+'Pass').storeSiSPSeededTracks ]
            result.merge(SiSPTSOS_CommonKernelCfg(
                flags, listOfExtensions = listOfExtensionsRequesting))

        if flags.Input.isMC:
            #check if we want to add it for other passes
            listOfExtensionsRequesting = [ e for e in _extensions_list if (e == '') or (flags.Tracking.__getattr__(e+'Pass').storeSiSPSeededTracks and flags.Tracking.__getattr__(e+'Pass').storeSeparateContainer) ]
            from InDetPhysValMonitoring.InDetPhysValDecorationConfig import (
                InDetPhysHitDecoratorAlgCfg)
            for extension in listOfExtensionsRequesting:
                result.merge(InDetPhysHitDecoratorAlgCfg(flags, name=f"InDetPhysHit{extension}DecoratorAlg", 
                                                         TrackParticleContainerName=f"InDet{extension}TrackParticles"))

    # output
    result.merge(InDetTrackRecoOutputCfg(flags))

    return result


# these are used internally by the FTAG software. We generally don't
# want to save them since they can be reconstructed by rerunning
# flavor tagging.
FTAG_AUXDATA = [
    'VxTrackAtVertex',
    'TrackCompatibility',
    'JetFitter_TrackCompatibility_antikt4emtopo',
    'JetFitter_TrackCompatibility_antikt4empflow',
    'btagIp_d0Uncertainty',
    'btagIp_z0SinThetaUncertainty',
    'btagIp_z0SinTheta',
    'btagIp_d0',
    'btagIp_trackMomentum',
    'btagIp_trackDisplacement',
    'btagIp_invalidIp',
]


def InDetTrackRecoOutputCfg(flags):
    from OutputStreamAthenaPool.OutputStreamConfig import addToESD, addToAOD
    toAOD = []
    toESD = []

    # FIXME: special branches without flags
    special = False

    # excluded track aux data
    excludedAuxData = "-clusterAssociation.-TTVA_AMVFVertices_forReco.-TTVA_AMVFWeights_forReco"
    # remove track decorations used internally by FTAG software
    excludedAuxData += '.-'.join([''] + FTAG_AUXDATA)

    if not special:  # not flags.InDet.keepFirstParameters or flags.InDet.keepAdditionalHitsOnTrackParticle
        excludedAuxData += '.-trackParameterCovarianceMatrices.-parameterX.-parameterY.-parameterZ.-parameterPX.-parameterPY.-parameterPZ.-parameterPosition'

    # exclude TTVA decorations
    excludedAuxData += '.-TTVA_AMVFVertices.-TTVA_AMVFWeights'

    # exclude IDTIDE/IDTRKVALID decorations
    excludedAuxData += '.-TrkBLX.-TrkBLY.-TrkBLZ.-TrkIBLX.-TrkIBLY.-TrkIBLZ.-TrkL1X.-TrkL1Y.-TrkL1Z.-TrkL2X.-TrkL2Y.-TrkL2Z'
    if not (flags.Tracking.writeExtendedSi_PRDInfo or
            flags.Tracking.writeExtendedTRT_PRDInfo):
        excludedAuxData += '.-msosLink'

    # exclude IDTIDE decorations
    excludedAuxData += ('.-IDTIDE1_biased_PVd0Sigma.-IDTIDE1_biased_PVz0Sigma.-IDTIDE1_biased_PVz0SigmaSinTheta.-IDTIDE1_biased_d0.-IDTIDE1_biased_d0Sigma'
                        '.-IDTIDE1_biased_z0.-IDTIDE1_biased_z0Sigma.-IDTIDE1_biased_z0SigmaSinTheta.-IDTIDE1_biased_z0SinTheta.-IDTIDE1_unbiased_PVd0Sigma.-IDTIDE1_unbiased_PVz0Sigma'
                        '.-IDTIDE1_unbiased_PVz0SigmaSinTheta.-IDTIDE1_unbiased_d0.-IDTIDE1_unbiased_d0Sigma.-IDTIDE1_unbiased_z0.-IDTIDE1_unbiased_z0Sigma.-IDTIDE1_unbiased_z0SigmaSinTheta'
                        '.-IDTIDE1_unbiased_z0SinTheta')

    ##### ESD #####
    # Save full and zero-suppressed BCM rdos
    # (the latter is needed to allow writting to AOD and the former will hopefully be removed in future):
    toESD += [
        "BCM_RDO_Container#BCM_RDOs",
        "BCM_RDO_Container#BCM_CompactDOs",
    ]

    # In case of cosmics we save the RDOs as well
    if special:  # flags.InDet.writeRDOs:
        toESD += [
            "PixelRDO_Container#PixelRDOs",
            "SCT_RDO_Container#SCT_RDOs",
            # "TRT_RDO_Container#TRT_RDOs",
        ]

    # write phase calculation into ESD
    if flags.InDet.doTRTPhase:
        toESD += ["ComTime#TRT_Phase"]

    # Save PRD
    toESD += [
        "InDet::SCT_ClusterContainer#SCT_Clusters",
        "InDet::PixelClusterContainer#PixelClusters",
        "InDet::TRT_DriftCircleContainer#TRT_DriftCircles",
        "InDet::PixelGangedClusterAmbiguities#PixelClusterAmbiguitiesMap",
    ]
    if flags.Tracking.doPixelClusterSplitting:
        toESD += ["InDet::PixelGangedClusterAmbiguities#SplitClusterAmbiguityMap"]
    toESD += ["IDCInDetBSErrContainer#SCT_FlaggedCondData"]
    toESD += ["Trk::ClusterSplitProbabilityContainer#" +
              ClusterSplitProbabilityContainerName(flags)]

    # add tracks
    if flags.Tracking.doStoreTrackSeeds:
        listOfExtensionsRequesting = [ e for e in _extensions_list if (e == '' or flags.Tracking.__getattr__(e+'Pass').storeTrackSeeds) ]
        for extension in listOfExtensionsRequesting:
            toESD += ["TrackCollection#SiSPSeedSegments"+extension]

    if flags.Tracking.doTrackSegmentsPixel:
        toESD += ["TrackCollection#ResolvedPixelTracks"]
        if flags.Tracking.doTruth:
            toESD += ["TrackTruthCollection#ResolvedPixelTracksTruthCollection"]
            toESD += ["DetailedTrackTruthCollection#ResolvedPixelTracksDetailedTruth"]

    if flags.Tracking.doTrackSegmentsSCT:
        toESD += ["TrackCollection#ResolvedSCTTracks"]
        if flags.Tracking.doTruth:
            toESD += ["TrackTruthCollection#ResolvedSCTTracksTruthCollection"]
            toESD += ["DetailedTrackTruthCollection#ResolvedSCTTracksDetailedTruth"]

    if flags.Tracking.doTrackSegmentsTRT:
        toESD += ["TrackCollection#StandaloneTRTTracks"]
        if flags.Tracking.doTruth:
            toESD += ["TrackTruthCollection#StandaloneTRTTracksTruthCollection"]
            toESD += ["DetailedTrackTruthCollection#StandaloneTRTTracksDetailedTruth"]

    if flags.Tracking.doPseudoTracking:
        toESD += ["TrackCollection#InDetPseudoTracks"]
        if flags.Tracking.doTruth:
            toESD += ["TrackTruthCollection#InDetPseudoTracksTruthCollection"]
            toESD += ["DetailedTrackTruthCollection#InDetPseudoTracksDetailedTruth"]

    if flags.Tracking.doTIDE_AmbiTrackMonitoring:
        toESD += ["TrackCollection#ObservedTracksCollection"]

    # add the forward tracks for combined muon reconstruction
    if flags.Tracking.doForwardTracks:
        toESD += ["TrackCollection#ResolvedForwardTracks"]
        if flags.Tracking.doTruth:
            toESD += ["TrackTruthCollection#ResolvedForwardTracksTruthCollection"]
            toESD += ["DetailedTrackTruthCollection#ResolvedForwardTracksDetailedTruth"]

    if flags.Tracking.doTrackSegmentsDisappearing:
        toESD += ["TrackCollection#DisappearingTracks"]
        if flags.Tracking.doTruth:
            toESD += ["TrackTruthCollection#DisappearingTracksTruthCollection"]
            toESD += ["DetailedTrackTruthCollection#DisappearingTracksDetailedTruth"]

    if flags.Tracking.doLowPtRoI:
        toESD += ["xAOD::VertexContainer#RoIVerticesLowPtRoI", "xAOD::VertexAuxContainer#RoIVerticesLowPtRoIAux."]
        if flags.Tracking.LowPtRoIPass.storeSeparateContainer:
            toESD += ["TrackCollection#ExtendedLowPtRoITracks"]
            if flags.Tracking.doTruth:
                toESD += ["TrackTruthCollection#ExtendedLowPtRoITracksTruthCollection"]
                toESD += ["DetailedTrackTruthCollection#ExtendedLowPtRoITracksDetailedTruth"] 

    # Add TRT Segments (only if standalone is off).
    # TODO: no TP converter?
    # if not flags.doTRTStandalone:
    #    toESD += ["Trk::SegmentCollection#TRTSegments"]

    # Save (Detailed) Track Truth
    if flags.Tracking.doTruth:
        toESD += ["TrackTruthCollection#CombinedInDetTracksTruthCollection"]
        toESD += ["DetailedTrackTruthCollection#CombinedInDetTracksDetailedTruth"]

        # save PRD MultiTruth
        toESD += [
            "PRD_MultiTruthCollection#PRD_MultiTruthPixel",
            "PRD_MultiTruthCollection#PRD_MultiTruthSCT",
            "PRD_MultiTruthCollection#PRD_MultiTruthTRT",
        ]

    if not flags.Input.isMC:
        toESD += [
            "InDetBSErrContainer#PixelByteStreamErrs",
            "TRT_BSErrContainer#TRT_ByteStreamErrs",
            "TRT_BSIdErrContainer#TRT_ByteStreamIdErrs",
            "IDCInDetBSErrContainer#SCT_ByteStreamErrs",
        ]

    toESD += ["TrackCollection#CombinedInDetTracks"]

    ##### AOD #####
    toAOD += ["xAOD::TrackParticleContainer#InDetTrackParticles"]
    toAOD += [
        f"xAOD::TrackParticleAuxContainer#InDetTrackParticlesAux.{excludedAuxData}"]
    toAOD += ["xAOD::TrackParticleContainer#InDetForwardTrackParticles"]
    toAOD += [
        f"xAOD::TrackParticleAuxContainer#InDetForwardTrackParticlesAux.{excludedAuxData}"]
    toAOD += ["xAOD::TrackParticleContainer#InDetLargeD0TrackParticles"]
    toAOD += [
        f"xAOD::TrackParticleAuxContainer#InDetLargeD0TrackParticlesAux.{excludedAuxData}"]
    if flags.Tracking.doTrackSegmentsPixel:
        toAOD += ["xAOD::TrackParticleContainer#InDetPixelTrackParticles"]
        toAOD += [
            f"xAOD::TrackParticleAuxContainer#InDetPixelTrackParticlesAux.{excludedAuxData}"]
    if flags.Tracking.doTrackSegmentsDisappearing:
        toAOD += ["xAOD::TrackParticleContainer#InDetDisappearingTrackParticles"]
        toAOD += [
            f"xAOD::TrackParticleAuxContainer#InDetDisappearingTrackParticlesAux.{excludedAuxData}"]
    if flags.Tracking.doLowPtRoI:
        toAOD += ["xAOD::VertexContainer#RoIVerticesLowPtRoI", "xAOD::VertexAuxContainer#RoIVerticesLowPtRoIAux."]
        if flags.Tracking.LowPtRoIPass.storeSeparateContainer:
            toAOD += ["xAOD::TrackParticleContainer#InDetLowPtRoITrackParticles"]
            toAOD += [
                f"xAOD::TrackParticleAuxContainer#InDetLowPtRoITrackParticlesAux.{excludedAuxData}"]    
    if flags.Tracking.doTrackSegmentsSCT:
        toAOD += ["xAOD::TrackParticleContainer#InDetSCTTrackParticles"]
        toAOD += [
            f"xAOD::TrackParticleAuxContainer#InDetSCTTrackParticlesAux.{excludedAuxData}"]
    if flags.Tracking.doTrackSegmentsTRT:
        toAOD += ["xAOD::TrackParticleContainer#InDetTRTTrackParticles"]
        toAOD += [
            f"xAOD::TrackParticleAuxContainer#InDetTRTTrackParticlesAux.{excludedAuxData}"]
    if flags.Tracking.doPseudoTracking:
        toAOD += ["xAOD::TrackParticleContainer#InDetPseudoTrackParticles"]
        toAOD += [
            f"xAOD::TrackParticleAuxContainer#InDetPseudoTrackParticlesAux.{excludedAuxData}"]
        if flags.Tracking.doTruth:
            toAOD += ["TrackTruthCollection#InDetPseudoTrackTruthCollection"]
            toAOD += ["DetailedTrackTruthCollection#InDetPseudoTrackDetailedTruth"]
    if flags.Tracking.doTIDE_AmbiTrackMonitoring:
        toAOD += ["xAOD::TrackParticleContainer#InDetObservedTrackParticles"]
        toAOD += [
            f"xAOD::TrackParticleAuxContainer#InDetObservedTrackParticlesAux.{excludedAuxData}"]
        if flags.Tracking.doTruth:
            toAOD += ["TrackTruthCollection#InDetObservedTrackTruthCollection"]
            toAOD += ["DetailedTrackTruthCollection#ObservedDetailedTracksTruth"]
    if flags.Tracking.doStoreSiSPSeededTracks:
        # get list of extensions requesting track candidates. Add always the Primary Pass.
        listOfExtensionsRequesting = [ e for e in _extensions_list if (e == '' or flags.Tracking.__getattr__(e+'Pass').storeSiSPSeededTracks) ]
        for extension in listOfExtensionsRequesting:
            toAOD += [f"xAOD::TrackParticleContainer#SiSPSeededTracks{extension}TrackParticles"]
            toAOD += [
                f"xAOD::TrackParticleAuxContainer#SiSPSeededTracks{extension}TrackParticlesAux.{excludedAuxData}"]

    if flags.Tracking.doStoreTrackSeeds:
        # get list of extensions requesting track seeds. Add always the Primary Pass.
        listOfExtensionsRequesting = [ e for e in _extensions_list if (e == '' or flags.Tracking.__getattr__(e+'Pass').storeTrackSeeds) ]
        for extension in listOfExtensionsRequesting:
            toAOD += [
                f"xAOD::TrackParticleContainer#SiSPSeedSegments{extension}TrackParticles",
                f"xAOD::TrackParticleAuxContainer#SiSPSeedSegments{extension}TrackParticlesAux."
            ]
    if (flags.Tracking.writeExtendedSi_PRDInfo or
        flags.Tracking.writeExtendedTRT_PRDInfo):
        toAOD += [
            "xAOD::TrackMeasurementValidationContainer#PixelClusters",
            "xAOD::TrackMeasurementValidationAuxContainer#PixelClustersAux.",
            "xAOD::TrackMeasurementValidationContainer#SCT_Clusters",
            "xAOD::TrackMeasurementValidationAuxContainer#SCT_ClustersAux.",
            "xAOD::TrackMeasurementValidationContainer#TRT_DriftCircles",
            "xAOD::TrackMeasurementValidationAuxContainer#TRT_DriftCirclesAux."
        ]
        # get list of extensions requesting track seeds. Add always the Primary Pass.
        listOfExtensionsRequesting = [ e for e in _extensions_list if (e == '' or flags.Tracking.__getattr__(e+'Pass').storeTrackSeeds) ]
        for extension in listOfExtensionsRequesting:
            toAOD += [
                f"xAOD::TrackStateValidationContainer#{extension}Pixel_MSOSs",
                f"xAOD::TrackStateValidationAuxContainer#{extension}Pixel_MSOSsAux.",
                f"xAOD::TrackStateValidationContainer#{extension}SCT_MSOSs",
                f"xAOD::TrackStateValidationAuxContainer#{extension}SCT_MSOSsAux.",
                f"xAOD::TrackStateValidationContainer#{extension}TRT_MSOSs",
                f"xAOD::TrackStateValidationAuxContainer#{extension}TRT_MSOSsAux."
            ]
        if flags.Tracking.doTIDE_AmbiTrackMonitoring:
            toAOD += [
                "xAOD::TrackStateValidationContainer#ObservedTrack_Pixel_MSOSs",
                "xAOD::TrackStateValidationAuxContainer#ObservedTrack_Pixel_MSOSsAux.",
                "xAOD::TrackStateValidationContainer#ObservedTrack_SCT_MSOSs",
                "xAOD::TrackStateValidationAuxContainer#ObservedTrack_SCT_MSOSsAux.",
                "xAOD::TrackStateValidationContainer#ObservedTrack_TRT_MSOSs",
                "xAOD::TrackStateValidationAuxContainer#ObservedTrack_TRT_MSOSsAux."
            ]
        if flags.Tracking.doPseudoTracking:
            toAOD += [
                "xAOD::TrackStateValidationContainer#Pseudo_Pixel_MSOSs",
                "xAOD::TrackStateValidationAuxContainer#Pseudo_Pixel_MSOSsAux.",
                "xAOD::TrackStateValidationContainer#Pseudo_SCT_MSOSs",
                "xAOD::TrackStateValidationAuxContainer#Pseudo_SCT_MSOSsAux.",
                "xAOD::TrackStateValidationContainer#Pseudo_TRT_MSOSs",
                "xAOD::TrackStateValidationAuxContainer#Pseudo_TRT_MSOSsAux."
            ]
        if flags.Tracking.doStoreSiSPSeededTracks:
            # get list of extensions requesting track seeds. Add always the Primary Pass.
            listOfExtensionsRequesting = [ e for e in _extensions_list if (e == '' or flags.Tracking.__getattr__(e+'Pass').storeTrackSeeds) ]
            for extension in listOfExtensionsRequesting:
                toAOD += [
                    f"xAOD::TrackStateValidationContainer#SiSP{extension}_Pixel_MSOSs",
                    f"xAOD::TrackStateValidationAuxContainer#SiSP{extension}_Pixel_MSOSsAux.",
                    f"xAOD::TrackStateValidationContainer#SiSP{extension}_SCT_MSOSs",
                    f"xAOD::TrackStateValidationAuxContainer#SiSP{extension}_SCT_MSOSsAux.",
                    f"xAOD::TrackStateValidationContainer#SiSP{extension}_TRT_MSOSs",
                    f"xAOD::TrackStateValidationAuxContainer#SiSP{extension}_TRT_MSOSsAux."
                ]

    if flags.Tracking.doV0Finder:
        excludedVtxAuxData = "-vxTrackAtVertex.-MvfFitInfo.-isInitialized.-VTAV"
        V0Vertices = ["V0UnconstrVertices", "V0KshortVertices",
                      "V0LambdaVertices", "V0LambdabarVertices"]
        for v0 in V0Vertices:
            toAOD += [
                f"xAOD::VertexContainer#{v0}",
                f"xAOD::VertexAuxContainer#{v0}Aux.{excludedVtxAuxData}",
            ]

    result = ComponentAccumulator()
    result.merge(addToESD(flags, toESD + toAOD))
    result.merge(addToAOD(flags, toAOD))

    return result


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()

    # Disable calo for this test
    flags.Detector.EnableCalo = False

    from AthenaConfiguration.TestDefaults import defaultTestFiles
    flags.Input.Files = defaultTestFiles.RDO_RUN2
    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    top_acc = MainServicesCfg(flags)

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    top_acc.merge(PoolReadCfg(flags))

    #######################################################################
    #################### Additional Configuration  ########################
    if "EventInfo" not in flags.Input.Collections:
        from xAODEventInfoCnv.xAODEventInfoCnvConfig import EventInfoCnvAlgCfg
        top_acc.merge(EventInfoCnvAlgCfg(flags))

    if flags.Input.isMC:
        from xAODTruthCnv.xAODTruthCnvConfig import GEN_AOD2xAODCfg
        top_acc.merge(GEN_AOD2xAODCfg(flags))

    top_acc.merge(InDetTrackRecoCfg(flags))
    from AthenaCommon.Constants import DEBUG
    top_acc.foreach_component("AthEventSeq/*").OutputLevel = DEBUG
    top_acc.printConfig(withDetails=True, summariseProps=True)
    top_acc.store(open("TrackRecoConfig.pkl", "wb"))

    import sys
    if "--norun" not in sys.argv:
        sc = top_acc.run(1)
        if sc.isFailure():
            sys.exit(-1)
