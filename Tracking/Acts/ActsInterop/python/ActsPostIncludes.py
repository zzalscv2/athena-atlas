# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def PersistifyActsEDMCfg(flags) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    
    pixel_cluster_shortlist = ['-pixelClusterLink']
    strip_cluster_shortlist = ['-sctClusterLink']

    pixel_cluster_variables = '.'.join(pixel_cluster_shortlist)
    strip_cluster_variables = '.'.join(strip_cluster_shortlist)

    toAOD = ["xAOD::PixelClusterContainer#ITkPixelClusters",
             "xAOD::PixelClusterAuxContainer#ITkPixelClustersAux." + pixel_cluster_variables,
             "xAOD::StripClusterContainer#ITkStripClusters",
             "xAOD::StripClusterAuxContainer#ITkStripClustersAux." + strip_cluster_variables]

    from OutputStreamAthenaPool.OutputStreamConfig import addToAOD    
    acc.merge(addToAOD(flags, toAOD))
    return acc
