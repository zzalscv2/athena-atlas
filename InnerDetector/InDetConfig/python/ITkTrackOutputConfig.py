# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from InDetConfig.InDetTrackOutputConfig import FTAG_AUXDATA

def ITkTrackRecoOutputCfg(flags, extensions_list=None):
    if extensions_list is None:
        extensions_list = []

    from OutputStreamAthenaPool.OutputStreamConfig import addToESD, addToAOD
    toAOD = []
    toESD = []

    # excluded track aux data
    excludedAuxData = ('-clusterAssociation.-TTVA_AMVFVertices_forReco'
                       '.-TTVA_AMVFWeights_forReco')
    # remove track decorations used internally by FTAG software
    excludedAuxData += '.-'.join([''] + FTAG_AUXDATA)

    # exclude TTVA decorations
    excludedAuxData += '.-TTVA_AMVFVertices.-TTVA_AMVFWeights'

    # exclude IDTIDE/IDTRKVALID decorations
    excludedAuxData += ('.-TrkBLX.-TrkBLY.-TrkBLZ.-TrkIBLX.-TrkIBLY.-TrkIBLZ'
                        '.-TrkL1X.-TrkL1Y.-TrkL1Z.-TrkL2X.-TrkL2Y.-TrkL2Z')
    if not flags.Tracking.writeExtendedSi_PRDInfo:
        excludedAuxData += '.-msosLink'

    # Save PRD
    toESD += [
        "InDet::SCT_ClusterContainer#ITkStripClusters",
        "InDet::PixelClusterContainer#ITkPixelClusters",
        "InDet::PixelGangedClusterAmbiguities#ITkPixelClusterAmbiguitiesMap",
    ]
    if flags.Tracking.doPixelClusterSplitting:
        toESD += [
            "InDet::PixelGangedClusterAmbiguities#ITkSplitClusterAmbiguityMap"]

    from InDetConfig.ITkTrackRecoConfig import ITkClusterSplitProbabilityContainerName
    toESD += ["Trk::ClusterSplitProbabilityContainer#" +
              ITkClusterSplitProbabilityContainerName(flags)]

    # Save (Detailed) Track Truth
    if flags.Tracking.doTruth:
        toESD += [
            "TrackTruthCollection#CombinedITkTracksTrackTruthCollection",
            "DetailedTrackTruthCollection#CombinedITkTracksDetailedTrackTruth"]

    if flags.Tracking.doStoreSiSPSeededTracks:
        # get list of extensions requesting track candidates. Add always the Primary Pass.
        listOfExtensionsRequesting = [
            e for e in extensions_list
            if (e == '' or flags.Tracking.__getattr__(e+'Pass').storeSiSPSeededTracks) ]
        for extension in listOfExtensionsRequesting:
            toAOD += [
                f"xAOD::TrackParticleContainer#SiSPSeededTracks{extension}TrackParticles",
                f"xAOD::TrackParticleAuxContainer#SiSPSeededTracks{extension}TrackParticlesAux.{excludedAuxData}"]

    if flags.Tracking.doStoreTrackSeeds:
        listOfExtensionsRequesting = [
            e for e in extensions_list
            if (e == '' or flags.Tracking.__getattr__(e+'Pass').storeTrackSeeds) ]
        for extension in listOfExtensionsRequesting:
            toESD += ["TrackCollection#SiSPSeedSegments"+extension]

    toESD += ["TrackCollection#CombinedITkTracks"]

    ##### AOD #####
    toAOD += [
        "xAOD::TrackParticleContainer#InDetTrackParticles",
        f"xAOD::TrackParticleAuxContainer#InDetTrackParticlesAux.{excludedAuxData}"
    ]

    if flags.Tracking.writeExtendedSi_PRDInfo:
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

        if flags.Tracking.doStoreSiSPSeededTracks:
            toAOD += [
                "xAOD::TrackStateValidationContainer#SiSP_ITkPixel_MSOSs",
                "xAOD::TrackStateValidationAuxContainer#SiSP_ITkPixel_MSOSsAux.",
                "xAOD::TrackStateValidationContainer#SiSP_ITkStrip_MSOSs",
                "xAOD::TrackStateValidationAuxContainer#SiSP_ITkStrip_MSOSsAux."
            ]

    if (flags.Tracking.doLargeD0 and
            flags.Tracking.storeSeparateLargeD0Container):
        toAOD += [
            "xAOD::TrackParticleContainer#InDetLargeD0TrackParticles",
            f"xAOD::TrackParticleAuxContainer#InDetLargeD0TrackParticlesAux.{excludedAuxData}"
        ]


    result = ComponentAccumulator()
    result.merge(addToESD(flags, toAOD+toESD))
    result.merge(addToAOD(flags, toAOD))
    return result
