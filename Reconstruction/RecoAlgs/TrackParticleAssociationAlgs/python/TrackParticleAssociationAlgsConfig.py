# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def TrackParticleCellAssociationAlgCfg(flags,
                                       name="TrackParticleCellAssociationAlg",
                                       **kwargs):
    result = ComponentAccumulator()

    from TrackToCalo.TrackToCaloConfig import ParticleCaloCellAssociationToolCfg
    kwargs.setdefault("ParticleCaloCellAssociationTool",
                      result.popToolsAndMerge(
                          ParticleCaloCellAssociationToolCfg(flags)))

    result.addEventAlgo(
        CompFactory.TrackParticleCellAssociationAlg(name, **kwargs))

    from OutputStreamAthenaPool.OutputStreamConfig import addToESD, addToAOD
    toAOD = [
        "xAOD::CaloClusterContainer#InDetTrackParticlesAssociatedClusters",
        "xAOD::CaloClusterAuxContainer#InDetTrackParticlesAssociatedClustersAux.",
        "CaloClusterCellLinkContainer#InDetTrackParticlesAssociatedClusters_links",
        "xAOD::TrackParticleClusterAssociationContainer#InDetTrackParticlesClusterAssociations",
        "xAOD::TrackParticleClusterAssociationAuxContainer#InDetTrackParticlesClusterAssociationsAux."
    ]
    from CaloRec.CaloThinCellsByClusterAlgConfig import CaloThinCellsByClusterAlgCfg
    result.merge(CaloThinCellsByClusterAlgCfg(
        flags,
        streamName="StreamAOD",
        clusters="InDetTrackParticlesAssociatedClusters",
        samplings=["TileGap1", "TileGap2", "TileGap3", "TileBar0", "TileExt0", "HEC0"],
        cells=flags.Egamma.Keys.Input.CaloCells))
    result.merge(addToESD(flags, toAOD))
    result.merge(addToAOD(flags, toAOD))

    return result


def LargeD0TrackParticleCellAssociationAlgCfg(flags,
                                              name="LargeD0TrackParticleCellAssociationAlg",
                                              **kwargs):
    result = ComponentAccumulator()
    from TrackToCalo.TrackToCaloConfig import ParticleCaloCellAssociationToolCfg
    kwargs.setdefault("ParticleCaloCellAssociationTool",
                      result.popToolsAndMerge(
                          ParticleCaloCellAssociationToolCfg(flags)))

    result.addEventAlgo(
        CompFactory.TrackParticleCellAssociationAlg(
            name,
            TrackParticleContainerName="InDetLargeD0TrackParticles",
            ClusterContainerName="InDetLargeD0TrackParticlesAssociatedClusters",
            CaloClusterCellLinkName="InDetLargeD0TrackParticlesAssociatedClusters_links",
            AssociationContainerName="InDetLargeD0TrackParticlesClusterAssociations",
            **kwargs))

    from OutputStreamAthenaPool.OutputStreamConfig import addToESD, addToAOD
    toAOD = [
        "xAOD::CaloClusterContainer#InDetLargeD0TrackParticlesAssociatedClusters",
        "xAOD::CaloClusterAuxContainer#InDetLargeD0TrackParticlesAssociatedClustersAux.",
        "CaloClusterCellLinkContainer#InDetLargeD0TrackParticlesAssociatedClusters_links",
        "xAOD::TrackParticleClusterAssociationContainer#InDetLargeD0TrackParticlesClusterAssociations",
        "xAOD::TrackParticleClusterAssociationAuxContainer#InDetLargeD0TrackParticlesClusterAssociationsAux."
    ]
    from CaloRec.CaloThinCellsByClusterAlgConfig import CaloThinCellsByClusterAlgCfg
    result.merge(CaloThinCellsByClusterAlgCfg(
        flags,
        streamName="StreamAOD",
        clusters="InDetLargeD0TrackParticlesAssociatedClusters",
        samplings=["TileGap1", "TileGap2", "TileGap3", "TileBar0", "TileExt0", "HEC0"]))
    result.merge(addToESD(flags, toAOD))
    result.merge(addToAOD(flags, toAOD))

    return result


def TrackParticleCellAssociationCfg(flags):
    acc = ComponentAccumulator()
    acc.merge(TrackParticleCellAssociationAlgCfg(flags))
    if (flags.Tracking.storeSeparateLargeD0Container and
            flags.Tracking.doLargeD0):
        acc.merge(LargeD0TrackParticleCellAssociationAlgCfg(flags))

    return acc
