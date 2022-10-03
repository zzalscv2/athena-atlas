# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import Format

from InDetConfig.TrackRecoConfig import FTAG_AUXDATA

_flags_set = [] # For caching

def CombinedTrackingPassFlagSets(flags):

    global _flags_set
    if _flags_set:
        return _flags_set

    flags_set = []

    # Primary Pass
    if flags.ITk.Tracking.doFastTracking:
        flags = flags.cloneAndReplace("ITk.Tracking.ActivePass", "ITk.Tracking.FastPass")
    else:
        flags = flags.cloneAndReplace("ITk.Tracking.ActivePass", "ITk.Tracking.MainPass")
    flags_set += [flags]

    # LRT
    if flags.ITk.Tracking.doLargeD0:
        flagsLRT = flags.cloneAndReplace("ITk.Tracking.ActivePass", "ITk.Tracking.LargeD0Pass")
        if flags.ITk.Tracking.doFastTracking:
            flagsLRT = flags.cloneAndReplace("ITk.Tracking.ActivePass", "ITk.Tracking.LargeD0FastPass")
        flags_set += [flagsLRT]

    # Photon conversion tracking reco
    if flags.Detector.EnableCalo and flags.ITk.Tracking.doConversionFinding:
        flagsConv = flags.cloneAndReplace("ITk.Tracking.ActivePass", "ITk.Tracking.ConversionFindingPass")
        flags_set += [flagsConv]

    _flags_set = flags_set # Put into cache 

    return flags_set

def ITkClusterSplitProbabilityContainerName(flags):
    flags_set = CombinedTrackingPassFlagSets(flags)
    extension = flags_set[-1].ITk.Tracking.ActivePass.extension
    ClusterSplitProbContainer = "ITkAmbiguityProcessorSplitProb" + extension
    return ClusterSplitProbContainer

def ITkTrackRecoCfg(flags):
    """Configures complete ID tracking """
    result = ComponentAccumulator()

    if flags.Input.Format is Format.BS:
        # TODO: ITk BS providers
        raise RuntimeError("ByteStream inputs not supported")

    from InDetConfig.SiliconPreProcessing import ITkRecPreProcessingSiliconCfg
    result.merge(ITkRecPreProcessingSiliconCfg(flags))

    flags_set = CombinedTrackingPassFlagSets(flags)
    InputCombinedITkTracks = [] # Tracks to be ultimately merged in InDetTrackParticle collection
    InputExtendedITkTracks = [] # Includes also tracks which end in standalone TrackParticle collections
    ClusterSplitProbContainer = ""

    from InDetConfig.ITkTrackingSiPatternConfig import ITkTrackingSiPatternCfg
    from InDetConfig.ITkTrackTruthConfig import ITkTrackTruthCfg
    from xAODTrackingCnv.xAODTrackingCnvConfig import ITkTrackParticleCnvAlgCfg

    for current_flags in flags_set:

        extension = current_flags.ITk.Tracking.ActivePass.extension
        TrackContainer = "Resolved" + extension + "Tracks"
        SiSPSeededTracks = "SiSPSeeded" + extension + "Tracks"

        result.merge(ITkTrackingSiPatternCfg(current_flags,
                                             InputCollections = InputExtendedITkTracks,
                                             ResolvedTrackCollectionKey = TrackContainer,
                                             SiSPSeededTrackCollectionKey = SiSPSeededTracks,
                                             ClusterSplitProbContainer = ClusterSplitProbContainer))

        if current_flags.ITk.Tracking.ActivePass.storeSeparateContainer:
            if flags.ITk.Tracking.doTruth:
                result.merge(ITkTrackTruthCfg(current_flags,
                                              Tracks = TrackContainer,
                                              DetailedTruth = TrackContainer+"DetailedTruth",
                                              TracksTruth = TrackContainer+"TruthCollection"))

            result.merge(ITkTrackParticleCnvAlgCfg(current_flags,
                                                   name = extension + "TrackParticleCnvAlg",
                                                   TrackContainerName = TrackContainer,
                                                   xAODTrackParticlesFromTracksContainerName = "InDet" + extension + "TrackParticles")) # Need specific handling for R3LargeD0 not to break downstream configs
        else:
            ClusterSplitProbContainer = "ITkAmbiguityProcessorSplitProb" + extension
            InputCombinedITkTracks += [TrackContainer]

        InputExtendedITkTracks += [TrackContainer]

    from TrkConfig.TrkTrackCollectionMergerConfig import ITkTrackCollectionMergerAlgCfg
    result.merge(ITkTrackCollectionMergerAlgCfg(flags,
                                                InputCombinedTracks = InputCombinedITkTracks,
                                                CombinedITkClusterSplitProbContainer = ITkClusterSplitProbabilityContainerName(flags) if not flags.ITk.Tracking.doFastTracking else ""))

    if flags.ITk.Tracking.doTruth:
        result.merge(ITkTrackTruthCfg(flags))

    result.merge(ITkTrackParticleCnvAlgCfg(flags))

    if flags.ITk.PriVertex.doVertexFinding:
        from InDetConfig.VertexFindingConfig import primaryVertexFindingCfg
        result.merge(primaryVertexFindingCfg(flags))

    if flags.ITk.Tracking.writeExtendedPRDInfo:
        from InDetConfig.InDetPrepRawDataToxAODConfig import ITkPixelPrepDataToxAODCfg, ITkStripPrepDataToxAODCfg
        result.merge(ITkPixelPrepDataToxAODCfg(flags, ClusterSplitProbabilityName = ITkClusterSplitProbabilityContainerName(flags)))
        result.merge(ITkStripPrepDataToxAODCfg(flags))

        from DerivationFrameworkInDet.InDetToolsConfig import ITkTrackStateOnSurfaceDecoratorCfg
        TrackStateOnSurfaceDecorator = result.getPrimaryAndMerge(ITkTrackStateOnSurfaceDecoratorCfg(flags, name="ITkTrackStateOnSurfaceDecorator"))
        result.addEventAlgo(CompFactory.DerivationFramework.CommonAugmentation("ITkCommonKernel", AugmentationTools = [TrackStateOnSurfaceDecorator]))

        if flags.Input.isMC:
            from InDetPhysValMonitoring.InDetPhysValDecorationConfig import InDetPhysHitDecoratorAlgCfg
            result.merge(InDetPhysHitDecoratorAlgCfg(flags))

    # output
    result.merge(ITkTrackRecoOutputCfg(flags))

    return result

def ITkTrackRecoOutputCfg(flags):
    from OutputStreamAthenaPool.OutputStreamConfig import addToESD,addToAOD
    toAOD = []
    toESD = []

    # excluded track aux data
    excludedAuxData = "-clusterAssociation.-TTVA_AMVFVertices_forReco.-TTVA_AMVFWeights_forReco"
    # remove track decorations used internally by FTAG software
    excludedAuxData += '.-'.join([''] + FTAG_AUXDATA)

    # exclude IDTIDE/IDTRKVALID decorations
    excludedAuxData += '.-TrkBLX.-TrkBLY.-TrkBLZ.-TrkIBLX.-TrkIBLY.-TrkIBLZ.-TrkL1X.-TrkL1Y.-TrkL1Z.-TrkL2X.-TrkL2Y.-TrkL2Z'
    if not flags.ITk.Tracking.writeExtendedPRDInfo:
        excludedAuxData += '.-msosLink'

    # Save PRD
    toESD += [
        "InDet::SCT_ClusterContainer#ITkStripClusters",
        "InDet::PixelClusterContainer#ITkPixelClusters",
        "InDet::PixelGangedClusterAmbiguities#ITkPixelClusterAmbiguitiesMap",
        "InDet::PixelGangedClusterAmbiguities#ITkSplitClusterAmbiguityMap"
    ]
    toESD += ["Trk::ClusterSplitProbabilityContainer#" + ITkClusterSplitProbabilityContainerName(flags)]

    # add tracks
    if flags.ITk.Tracking.doStoreTrackSeeds:
        toESD += ["TrackCollection#SiSPSeedSegments"]

    toESD += ["TrackCollection#SiSPSeededTracks"]
    toESD += ["TrackCollection#CombinedInDetTracks"]

    ##### AOD #####
    toAOD += ["xAOD::TrackParticleContainer#InDetTrackParticles"]
    toAOD += [f"xAOD::TrackParticleAuxContainer#InDetTrackParticlesAux.{excludedAuxData}"]

    if flags.ITk.Tracking.writeExtendedPRDInfo:
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

    result = ComponentAccumulator()
    result.merge(addToESD(flags, toAOD+toESD))
    result.merge(addToAOD(flags, toAOD))
    return result


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import ConfigFlags

    # Disable calo for this test
    ConfigFlags.Detector.EnableCalo = False

    from AthenaConfiguration.TestDefaults import defaultTestFiles
    ConfigFlags.Input.Files = defaultTestFiles.RDO_RUN2
    ConfigFlags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    top_acc = MainServicesCfg(ConfigFlags)

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    top_acc.merge(PoolReadCfg(ConfigFlags))

    if ConfigFlags.Input.isMC:
        from xAODTruthCnv.xAODTruthCnvConfig import GEN_AOD2xAODCfg
        top_acc.merge(GEN_AOD2xAODCfg(ConfigFlags))

    top_acc.merge(ITkTrackRecoCfg(ConfigFlags))

    from AthenaCommon.Constants import DEBUG
    top_acc.foreach_component("AthEventSeq/*").OutputLevel=DEBUG
    top_acc.printConfig(withDetails=True, summariseProps=True)
    top_acc.store(open("ITkTrackReco.pkl", "wb"))

    import sys
    if "--norun" not in sys.argv:
        sc = top_acc.run(5)
        if sc.isFailure():
            sys.exit(-1)
