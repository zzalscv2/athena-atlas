# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import Format

from InDetConfig.TrackRecoConfig import FTAG_AUXDATA

_flags_set = []  # For caching


def CombinedTrackingPassFlagSets(flags):

    global _flags_set
    if _flags_set:
        return _flags_set

    flags_set = []

    # Primary Pass
    flags = flags.cloneAndReplace("Tracking.ActiveConfig",
                                  flags.Tracking.ITkPrimaryPassConfig.value)
    flags_set += [flags]

    # LRT
    if flags.Tracking.doLargeD0:
        if flags.Tracking.doITkFastTracking:
            flagsLRT = flags.cloneAndReplace("Tracking.ActiveConfig",
                                             "Tracking.ITkLargeD0FastPass")
        else:
            flagsLRT = flags.cloneAndReplace("Tracking.ActiveConfig",
                                             "Tracking.ITkLargeD0Pass")
        flags_set += [flagsLRT]

    # Photon conversion tracking reco
    if flags.Detector.EnableCalo and flags.Tracking.doITkConversionFinding:
        flagsConv = flags.cloneAndReplace("Tracking.ActiveConfig",
                                          "Tracking.ITkConversionFindingPass")
        flags_set += [flagsConv]

    # LowPt
    if flags.Tracking.doLowPt:
        flagsLowPt = flags.cloneAndReplace("Tracking.ActiveConfig",
                                           "Tracking.ITkLowPt")
        flags_set += [flagsLowPt]

    _flags_set = flags_set  # Put into cache

    return flags_set


def ITkClusterSplitProbabilityContainerName(flags):
    flags_set = CombinedTrackingPassFlagSets(flags)
    extension = flags_set[-1].Tracking.ActiveConfig.extension
    ClusterSplitProbContainer = "ITkAmbiguityProcessorSplitProb" + extension
    return ClusterSplitProbContainer


##############################################################################
#####################     Main ITk tracking config       #####################
##############################################################################


def ITkTrackRecoCfg(flags):
    """Configures complete ITk tracking """
    result = ComponentAccumulator()

    if flags.Input.Format is Format.BS:
        # TODO: ITk BS providers
        raise RuntimeError("ByteStream inputs not supported")

    from InDetConfig.SiliconPreProcessing import ITkRecPreProcessingSiliconCfg
    result.merge(ITkRecPreProcessingSiliconCfg(flags))

    flags_set = CombinedTrackingPassFlagSets(flags)
    # Tracks to be ultimately merged in InDetTrackParticle collection
    InputCombinedITkTracks = []
    # Includes also tracks which end in standalone TrackParticle collections
    InputExtendedITkTracks = []
    ClusterSplitProbContainer = ""
    StatTrackCollections = []  # To be passed to the InDetRecStatistics alg
    StatTrackTruthCollections = []

    from InDetConfig.ITkTrackingSiPatternConfig import ITkTrackingSiPatternCfg
    from InDetConfig.ITkTrackTruthConfig import ITkTrackTruthCfg
    from xAODTrackingCnv.xAODTrackingCnvConfig import ITkTrackParticleCnvAlgCfg

    for current_flags in flags_set:

        extension = current_flags.Tracking.ActiveConfig.extension
        TrackContainer = "Resolved" + extension + "Tracks"
        SiSPSeededTracks = "SiSPSeeded" + extension + "Tracks"

        result.merge(ITkTrackingSiPatternCfg(
            current_flags,
            InputCollections=InputExtendedITkTracks,
            ResolvedTrackCollectionKey=TrackContainer,
            SiSPSeededTrackCollectionKey=SiSPSeededTracks,
            ClusterSplitProbContainer=ClusterSplitProbContainer))
        StatTrackCollections += [SiSPSeededTracks, TrackContainer]
        StatTrackTruthCollections += [SiSPSeededTracks+"TruthCollection",
                                      TrackContainer+"TruthCollection"]

        if current_flags.Tracking.ActiveConfig.storeSeparateContainer:
            if flags.Tracking.doTruth:
                result.merge(ITkTrackTruthCfg(
                    current_flags,
                    Tracks=TrackContainer,
                    DetailedTruth=TrackContainer+"DetailedTruth",
                    TracksTruth=TrackContainer+"TruthCollection"))

            result.merge(ITkTrackParticleCnvAlgCfg(
                current_flags,
                name=extension + "TrackParticleCnvAlg",
                TrackContainerName=TrackContainer,
                xAODTrackParticlesFromTracksContainerName=(
                    "InDet" + extension + "TrackParticles"),
                ClusterSplitProbabilityName=ClusterSplitProbContainer,
                AssociationMapName=""))

        else:
            ClusterSplitProbContainer = (
                "ITkAmbiguityProcessorSplitProb" + extension)
            InputCombinedITkTracks += [TrackContainer]

        InputExtendedITkTracks += [TrackContainer]

    from TrkConfig.TrkTrackCollectionMergerConfig import (
        ITkTrackCollectionMergerAlgCfg)
    result.merge(ITkTrackCollectionMergerAlgCfg(
        flags,
        InputCombinedTracks=InputCombinedITkTracks,
        OutputCombinedTracks="CombinedITkTracks",
        AssociationMapName=(
            "PRDtoTrackMapCombinedITkTracks"
            if not flags.Tracking.doITkFastTracking else "")))

    if flags.Tracking.doTruth:
        from InDetConfig.ITkTrackTruthConfig import ITkTrackTruthCfg
        result.merge(ITkTrackTruthCfg(
            flags,
            Tracks="CombinedITkTracks",
            DetailedTruth="CombinedITkTracksDetailedTruth",
            TracksTruth="CombinedITkTracksTruthCollection"))

    StatTrackCollections += ["CombinedITkTracks"]
    StatTrackTruthCollections += ["CombinedITkTracksTruthCollection"]

    if flags.Tracking.doSlimming:
        from TrkConfig.TrkTrackSlimmerConfig import TrackSlimmerCfg
        result.merge(TrackSlimmerCfg(
            flags,
            TrackLocation=["CombinedITkTracks"]))

    if flags.Tracking.doTruth:
        result.merge(ITkTrackTruthCfg(flags))

    result.merge(ITkTrackParticleCnvAlgCfg(
        flags,
        ClusterSplitProbabilityName=(
            ITkClusterSplitProbabilityContainerName(flags)
            if not flags.Tracking.doITkFastTracking else ""),
        AssociationMapName=(
            "PRDtoTrackMapCombinedITkTracks"
            if not flags.Tracking.doITkFastTracking else "")))

    if flags.Tracking.doVertexFinding:
        from InDetConfig.InDetPriVxFinderConfig import primaryVertexFindingCfg
        result.merge(primaryVertexFindingCfg(flags))

    if flags.Tracking.doStats:
        from InDetConfig.InDetRecStatisticsConfig import (
            ITkRecStatisticsAlgCfg)
        result.merge(ITkRecStatisticsAlgCfg(
            flags,
            TrackCollectionKeys=StatTrackCollections,
            TrackTruthCollectionKeys=(
                StatTrackTruthCollections if flags.Tracking.doTruth else [])))

        if flags.Tracking.doTruth:
            from InDetConfig.InDetTrackClusterAssValidationConfig import (
                ITkTrackClusterAssValidationCfg)
            result.merge(ITkTrackClusterAssValidationCfg(
                flags_set[0],  # Use cuts from primary pass
                TracksLocation=StatTrackCollections))

    if flags.Tracking.writeExtendedPRDInfo:
        from InDetConfig.InDetPrepRawDataToxAODConfig import (
            ITkPixelPrepDataToxAODCfg, ITkStripPrepDataToxAODCfg)
        result.merge(ITkPixelPrepDataToxAODCfg(
            flags,
            ClusterSplitProbabilityName=(
                ITkClusterSplitProbabilityContainerName(flags))))
        result.merge(ITkStripPrepDataToxAODCfg(flags))

        from DerivationFrameworkInDet.InDetToolsConfig import (
            ITkTrackStateOnSurfaceDecoratorCfg)
        TrackStateOnSurfaceDecorator = result.getPrimaryAndMerge(
            ITkTrackStateOnSurfaceDecoratorCfg(
                flags,
                name="ITkTrackStateOnSurfaceDecorator"))

        result.addEventAlgo(
            CompFactory.DerivationFramework.CommonAugmentation(
                "ITkCommonKernel",
                AugmentationTools=[TrackStateOnSurfaceDecorator]))

        if flags.Tracking.doStoreSiSPSeededTracks:
            from DerivationFrameworkInDet.InDetToolsConfig import (
                ITkSiSPTrackStateOnSurfaceDecoratorCfg)
            SiSPTrackStateOnSurfaceDecorator = result.getPrimaryAndMerge(
                ITkSiSPTrackStateOnSurfaceDecoratorCfg(flags))
            result.addEventAlgo(
                CompFactory.DerivationFramework.CommonAugmentation(
                    "SiSPITkCommonKernel",
                    AugmentationTools=[SiSPTrackStateOnSurfaceDecorator]))

        if flags.Input.isMC:
            from InDetPhysValMonitoring.InDetPhysValDecorationConfig import (
                InDetPhysHitDecoratorAlgCfg)
            result.merge(InDetPhysHitDecoratorAlgCfg(flags))

    if flags.Tracking.doStoreTrackSeeds:
        TrackContainer = "SiSPSeedSegments"
        result.merge(ITkTrackTruthCfg(
            flags,
            Tracks=TrackContainer,
            DetailedTruth=f"{TrackContainer}DetailedTruth",
            TracksTruth=f"{TrackContainer}TruthCollection"))

        result.merge(ITkTrackParticleCnvAlgCfg(
            flags,
            name=f"{TrackContainer}TrackParticleCnvAlg",
            TrackContainerName=TrackContainer,
            xAODTrackParticlesFromTracksContainerName=(
                f"{TrackContainer}TrackParticles")))

    if flags.Tracking.doStoreSiSPSeededTracks:
        result.merge(ITkTrackParticleCnvAlgCfg(
            flags,
            name = "ITkSiSPSeededTracksCnvAlg",
            TrackContainerName = "SiSPSeededTracks",
            xAODTrackParticlesFromTracksContainerName=(
                "SiSPSeededTracksTrackParticles"),
            AssociationMapName=(
                "PRDtoTrackMapCombinedITkTracks")))



    # output
    result.merge(ITkTrackRecoOutputCfg(flags))

    return result


def ITkTrackRecoOutputCfg(flags):
    from OutputStreamAthenaPool.OutputStreamConfig import addToESD, addToAOD
    toAOD = []
    toESD = []

    # excluded track aux data
    excludedAuxData = "-clusterAssociation.-TTVA_AMVFVertices_forReco.-TTVA_AMVFWeights_forReco"
    # remove track decorations used internally by FTAG software
    excludedAuxData += '.-'.join([''] + FTAG_AUXDATA)

    # exclude TTVA decorations
    excludedAuxData += '.-TTVA_AMVFVertices.-TTVA_AMVFWeights'

    # exclude IDTIDE/IDTRKVALID decorations
    excludedAuxData += '.-TrkBLX.-TrkBLY.-TrkBLZ.-TrkIBLX.-TrkIBLY.-TrkIBLZ.-TrkL1X.-TrkL1Y.-TrkL1Z.-TrkL2X.-TrkL2Y.-TrkL2Z'
    if not flags.Tracking.writeExtendedPRDInfo:
        excludedAuxData += '.-msosLink'

    # Save PRD
    toESD += [
        "InDet::SCT_ClusterContainer#ITkStripClusters",
        "InDet::PixelClusterContainer#ITkPixelClusters",
        "InDet::PixelGangedClusterAmbiguities#ITkPixelClusterAmbiguitiesMap",
    ]
    if flags.Tracking.doPixelClusterSplitting:
        toESD += ["InDet::PixelGangedClusterAmbiguities#ITkSplitClusterAmbiguityMap"]
    toESD += ["Trk::ClusterSplitProbabilityContainer#" +
              ITkClusterSplitProbabilityContainerName(flags)]

    # Save (Detailed) Track Truth
    if flags.Tracking.doTruth:
        toESD += ["TrackTruthCollection#TrackTruthCollection"]
        toESD += ["DetailedTrackTruthCollection#DetailedTrackTruth"]

    # add tracks
    if flags.Tracking.doStoreTrackSeeds:
        toESD += ["TrackCollection#SiSPSeedSegments"]

    toESD += ["TrackCollection#CombinedITkTracks"]

    ##### AOD #####
    toAOD += ["xAOD::TrackParticleContainer#InDetTrackParticles"]
    toAOD += [
        f"xAOD::TrackParticleAuxContainer#InDetTrackParticlesAux.{excludedAuxData}"]

    if flags.Tracking.writeExtendedPRDInfo:
        toAOD += [
            "xAOD::TrackMeasurementValidationContainer#ITkPixelClusters",
            "xAOD::TrackMeasurementValidationAuxContainer#ITkPixelClustersAux.",
            "xAOD::TrackMeasurementValidationContainer#ITkStripClusters",
            "xAOD::TrackMeasurementValidationAuxContainer#ITkStripClustersAux.",
            "xAOD::TrackStateValidationContainer#ITkPixelMSOSs",
            "xAOD::TrackStateValidationAuxContainer#ITkPixelMSOSsAux.",
            "xAOD::TrackStateValidationContainer#ITkStripMSOSs",
            "xAOD::TrackStateValidationAuxContainer#ITkStripMSOSsAux."
        ]

    if (flags.Tracking.doLargeD0 and
            flags.Tracking.storeSeparateLargeD0Container):
        toAOD += [
            "xAOD::TrackParticleContainer#InDetLargeD0TrackParticles",
            f"xAOD::TrackParticleAuxContainer#InDetLargeD0TrackParticlesAux.{excludedAuxData}"
        ]

    if flags.Tracking.doStoreSiSPSeededTracks:
        toAOD += [
            "xAOD::TrackStateValidationContainer#SiSP_ITkPixel_MSOSs",
            "xAOD::TrackStateValidationAuxContainer#SiSP_ITkPixel_MSOSsAux.",
            "xAOD::TrackStateValidationContainer#SiSP_ITkStrip_MSOSs",
            "xAOD::TrackStateValidationAuxContainer#SiSP_ITkStrip_MSOSsAux.",
            "xAOD::TrackParticleContainer#SiSPSeededTracksTrackParticles",
            f"xAOD::TrackParticleAuxContainer#SiSPSeededTracksTrackParticlesAux.{excludedAuxData}"
        ]


    result = ComponentAccumulator()
    result.merge(addToESD(flags, toAOD+toESD))
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

    if flags.Input.isMC:
        from xAODTruthCnv.xAODTruthCnvConfig import GEN_AOD2xAODCfg
        top_acc.merge(GEN_AOD2xAODCfg(flags))

    top_acc.merge(ITkTrackRecoCfg(flags))

    from AthenaCommon.Constants import DEBUG
    top_acc.foreach_component("AthEventSeq/*").OutputLevel = DEBUG
    top_acc.printConfig(withDetails=True, summariseProps=True)
    top_acc.store(open("ITkTrackReco.pkl", "wb"))

    import sys
    if "--norun" not in sys.argv:
        sc = top_acc.run(5)
        if sc.isFailure():
            sys.exit(-1)
