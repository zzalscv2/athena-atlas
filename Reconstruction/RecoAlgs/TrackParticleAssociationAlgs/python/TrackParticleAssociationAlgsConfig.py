# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def TrackParticleCellAssociationAlgCfg(flags, name="TrackParticleCellAssociationAlg", **kwargs):
    result = ComponentAccumulator()

    from TrackToCalo.TrackToCaloConfig import ParticleCaloCellAssociationToolCfg
    kwargs.setdefault("ParticleCaloCellAssociationTool", result.popToolsAndMerge(
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
    result.merge(CaloThinCellsByClusterAlgCfg(flags, streamName="StreamAOD",
                                              clusters="InDetTrackParticlesAssociatedClusters",
                                              samplings=["TileGap1", "TileGap2", "TileGap3", "TileBar0", "TileExt0", "HEC0"]))
    result.merge(addToESD(flags, toAOD))
    result.merge(addToAOD(flags, toAOD))

    return result
