/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "FPGATrackSimClusteringOfflineTool.h"

FPGATrackSimClusteringOfflineTool::FPGATrackSimClusteringOfflineTool(const std::string& algname, const std::string &name, const IInterface *ifc) :
  base_class(algname, name, ifc)
{
}


StatusCode FPGATrackSimClusteringOfflineTool::DoClustering(FPGATrackSimLogicalEventInputHeader &header, std::vector<FPGATrackSimCluster> &clusters) const
{

    clusters = header.optional().getOfflineClusters();
    //fill the multitruth
    for( auto& cluster:clusters){
        FPGATrackSimHit clusterEquiv = cluster.getClusterEquiv();
        FPGATrackSimMultiTruth mt;
        FPGATrackSimMultiTruth::Barcode uniquecode(clusterEquiv.getEventIndex(),clusterEquiv.getBarcode());
        mt.maximize(uniquecode,clusterEquiv.getBarcodePt());
        clusterEquiv.setTruth(mt);
        cluster.setClusterEquiv(clusterEquiv);
    }
    return StatusCode::SUCCESS;
}
