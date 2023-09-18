
#include "PFClusterWidthDecorator.h"

#include "CaloEvent/CaloCluster.h"

PFClusterWidthDecorator::PFClusterWidthDecorator(const std::string& name, ISvcLocator* pSvcLocator) :
  AthAlgorithm(name, pSvcLocator) 
{}

StatusCode PFClusterWidthDecorator::initialize() {
  ATH_CHECK(m_clusterContainerWidthEtaKey.initialize());
  ATH_CHECK(m_clusterContainerWidthPhiKey.initialize());
  return StatusCode::SUCCESS;
}

StatusCode PFClusterWidthDecorator::execute() {

    SG::WriteDecorHandle<xAOD::CaloClusterContainer,float> clusterContainerWidthEta(m_clusterContainerWidthEtaKey);
    if (!clusterContainerWidthEta.isValid()) {
      ATH_MSG_WARNING("Invalid cluster container with name " << m_clusterContainerWidthEtaKey.key());
      return StatusCode::SUCCESS;
    }

    SG::WriteDecorHandle<xAOD::CaloClusterContainer,float> clusterContainerWidthPhi(m_clusterContainerWidthPhiKey);
    if (!clusterContainerWidthPhi.isValid()) {
      ATH_MSG_WARNING("Invalid cluster container with name " << m_clusterContainerWidthPhiKey.key());
      return StatusCode::SUCCESS;
    }

    for (const auto thisCluster : *clusterContainerWidthEta) {
        const CaloClusterCellLink* theCellLinks = thisCluster->getCellLinks();
        if (!theCellLinks) {
          ATH_MSG_WARNING("No cell links found for cluster");
          continue;
        }

        std::vector<double> eta,phi;
        for (CaloClusterCellLink::const_iterator it=theCellLinks->begin(); it!=theCellLinks->end(); ++it){
            const CaloCell* cell = *it;
            eta.push_back(cell->eta());
            phi.push_back(cell->phi());
        }

        std::pair<double,double> width = m_clusterWidthCalculator.getPFClusterCoordinateWidth(eta, phi, thisCluster->eta(), thisCluster->phi(), theCellLinks->size());                
        clusterContainerWidthEta(*thisCluster) = width.first;
        clusterContainerWidthPhi(*thisCluster) = width.second;
        
  }

  return StatusCode::SUCCESS;
}