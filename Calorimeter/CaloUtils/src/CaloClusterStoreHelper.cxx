/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "CaloUtils/CaloClusterStoreHelper.h"
#include "CaloEvent/CaloClusterCellLink.h"
#include "xAODCaloEvent/CaloClusterAuxContainer.h"
#include "StoreGate/StoreGateSvc.h"
#include "AthenaKernel/errorcheck.h"

xAOD::CaloCluster* CaloClusterStoreHelper::makeCluster(const CaloCellContainer* cellCont) {
  xAOD::CaloCluster* cluster=new xAOD::CaloCluster();
  cluster->makePrivateStore();
  if (cellCont) cluster->addCellLink(new CaloClusterCellLink(cellCont));
  return cluster;
}
 

xAOD::CaloCluster* CaloClusterStoreHelper::makeCluster(const CaloCellContainer* cellCont,
						       const double eta0, const double phi0,
						       const xAOD::CaloCluster_v1::ClusterSize clusterSize) {
  xAOD::CaloCluster* cluster=CaloClusterStoreHelper::makeCluster(cellCont);
  cluster->setEta0(eta0);
  cluster->setPhi0(phi0);
  cluster->setClusterSize(clusterSize);
  return cluster;
}


xAOD::CaloCluster* CaloClusterStoreHelper::makeCluster(xAOD::CaloClusterContainer* cont, 
						       const CaloCellContainer* cellCont) {
  xAOD::CaloCluster* cluster=new xAOD::CaloCluster();
  cont->push_back(cluster);
  if (cellCont) cluster->addCellLink(new CaloClusterCellLink(cellCont));
  return cluster;
}

xAOD::CaloClusterContainer* CaloClusterStoreHelper::makeContainer(StoreGateSvc* pStoreGate,
								  const std::string& clusCollKey,
								  MsgStream& msg) {
  // Create the xAOD container and its auxiliary store:
  xAOD::CaloClusterContainer* clusColl = new xAOD::CaloClusterContainer();
  if (pStoreGate->overwrite(clusColl, clusCollKey).isFailure()) {
    msg << MSG::ERROR << "Failed to record xAOD::CaloClusterContainer with key" << clusCollKey <<endreq;
    delete clusColl;
    return NULL;
  }

  xAOD::CaloClusterAuxContainer* aux = new xAOD::CaloClusterAuxContainer();
  if(pStoreGate->overwrite(aux, clusCollKey+"Aux.").isFailure()) {
    msg << MSG::ERROR << "Failed to record xAOD::CaloClusterAuxContainer with key " << clusCollKey+"Aux." << endreq;
    delete aux;
    delete clusColl;
    return NULL;
  }
  clusColl->setStore( aux );
  
  return clusColl;
}

StatusCode CaloClusterStoreHelper::finalizeClusters(StoreGateSvc* pStoreGate,
						    xAOD::CaloClusterContainer* pClusterColl,
						    const std::string& clusCollKey,
						    MsgStream& msg) {

  CaloClusterCellLinkContainer* cellLinks= new CaloClusterCellLinkContainer();
  if(pStoreGate->overwrite(cellLinks, clusCollKey + "_links").isFailure()) {
    msg << MSG::ERROR << "Failed to record CaloClusterCellLinkContainer with key " << clusCollKey + "Links" << endreq;
    return StatusCode::FAILURE;
  }

  //Loop on clusters and call setLink to transfer ownership of CaloClusterCellLink object to 
  //CaloClusterCellLinkContainer
  xAOD::CaloClusterContainer::iterator cluIt=pClusterColl->begin();
  xAOD::CaloClusterContainer::iterator cluIt_e=pClusterColl->end();
  for(;cluIt!=cluIt_e;++cluIt) {
    (*cluIt)->setLink(cellLinks);
  }


  if (pStoreGate->setConst(pClusterColl).isFailure()) {
    msg << MSG::ERROR << "Failed to lock CaloClusterContainer" << endreq;
  }
  if (pStoreGate->setConst(cellLinks).isFailure())  {
    msg << MSG::ERROR << "Failed to lock CaloClusterCellLinkContainer" << endreq;
  }
  return StatusCode::SUCCESS;
}


 //Moved to here from CaloRunClusterCorrection
void CaloClusterStoreHelper::copyContainer (const xAOD::CaloClusterContainer* oldColl, xAOD::CaloClusterContainer* newColl) {
  // make a Cluster Container 
  newColl->clear();
  newColl->reserve (oldColl->size());
  for (const xAOD::CaloCluster* oldCluster : *oldColl) { 
    newColl->push_back (new xAOD::CaloCluster (*oldCluster)); //Copy c'tor creates a private AuxStore and a private ClusterCellLink obj
  }
  return;
}
