# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

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

    pixel_spacepoint_shortlist = []
    strip_spacepoint_shortlist = ["topHalfStripLength", 
                                  "bottomHalfStripLength", 
                                  "topStripDirection",
                                  "bottomStripDirection",
                                  "stripCenterDistance",
                                  "topStripCenter"]

    pixel_spacepoint_variables = ".".join(pixel_spacepoint_shortlist)
    strip_spacepoint_variables = ".".join(strip_spacepoint_shortlist)

    toAOD += ["xAOD::SpacePointContainer#ITkPixelSpacePoints",
              "xAOD::SpacePointAuxContainer#ITkPixelSpacePointsAux." + pixel_spacepoint_variables,
              "xAOD::SpacePointContainer#ITkStripSpacePoints",
              "xAOD::SpacePointAuxContainer#ITkStripSpacePointsAux." + strip_spacepoint_variables,
              "xAOD::SpacePointContainer#ITkStripOverlapSpacePoints",
              "xAOD::SpacePointAuxContainer#ITkStripOverlapSpacePointsAux." + strip_spacepoint_variables]

    from OutputStreamAthenaPool.OutputStreamConfig import addToAOD    
    acc.merge(addToAOD(flags, toAOD))
    return acc
