/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "StripClusterizationAlg.h"

#include <AthenaMonitoringKernel/Monitored.h>
#include <xAODInDetMeasurement/StripClusterAuxContainer.h>
#include <xAODInDetMeasurement/ContainerAccessor.h>

namespace ActsTrk {

StripClusterizationAlg::StripClusterizationAlg(const std::string& name,
                                               ISvcLocator* pSvcLocator)
    : AthReentrantAlgorithm(name, pSvcLocator) {}

StatusCode StripClusterizationAlg::initialize() {
  ATH_MSG_DEBUG("Initializing " << name() << "... ");

  ATH_CHECK(m_rdoContainerKey.initialize());
  ATH_CHECK(m_stripDetElStatus.initialize(!m_stripDetElStatus.empty()));
  ATH_CHECK(m_stripDetEleCollKey.initialize());
  ATH_CHECK(m_clusterContainerKey.initialize());
  ATH_CHECK(m_clusteringTool.retrieve());
  ATH_CHECK(detStore()->retrieve(m_idHelper, "SCT_ID"));

  // Regional Tracking
  ATH_CHECK(m_roiCollectionKey.initialize());
  ATH_CHECK(m_regionSelector.retrieve());

  bool disableSmry =
      !m_stripDetElStatus.empty() && !VALIDATE_STATUS_ARRAY_ACTIVATED;
  ATH_CHECK(m_summaryTool.retrieve(DisableTool{disableSmry}));

  ATH_CHECK(m_monTool.retrieve(EnableTool{not m_monTool.empty()}));

  return StatusCode::SUCCESS;
}

StatusCode StripClusterizationAlg::execute(const EventContext& ctx) const {
  Monitored::Timer<std::chrono::milliseconds> timer("TIME_execute");
  Monitored::Group mon(m_monTool, timer);

  SG::ReadHandle<SCT_RDO_Container> rdoContainer =
      SG::makeHandle(m_rdoContainerKey, ctx);
  ATH_CHECK(rdoContainer.isValid());

  SG::WriteHandle<xAOD::StripClusterContainer> clusterHandle =
      SG::makeHandle(m_clusterContainerKey, ctx);
  ATH_CHECK(clusterHandle.record( std::make_unique<xAOD::StripClusterContainer>(),
				  std::make_unique<xAOD::StripClusterAuxContainer>() ));
  xAOD::StripClusterContainer *stripContainer = clusterHandle.ptr();
  // Reserve space, estimate of mean clusters to reduce re-allocations
  stripContainer->reserve(m_expectedClustersPerRDO.value() *
                          rdoContainer->size());

  SG::ReadHandle<InDet::SiDetectorElementStatus> status;
  if (!m_stripDetElStatus.empty()) {
    status = SG::makeHandle(m_stripDetElStatus, ctx);
    if (!status.isValid()) {
      ATH_MSG_FATAL("Invalid SiDetectorelementStatus");
      return StatusCode::FAILURE;
    }
  }

  SG::ReadCondHandle<InDetDD::SiDetectorElementCollection> stripDetEleHandle(
      m_stripDetEleCollKey, ctx);
  const InDetDD::SiDetectorElementCollection* stripDetEle(*stripDetEleHandle);
  if (!stripDetEleHandle.isValid() || stripDetEle == nullptr) {
    ATH_MSG_FATAL("Invalid SiDetectorElementCollection");
    return StatusCode::FAILURE;
  }

  // This will allow us to access the already processed hash ids
  // For now we are not using yet an update write handle, so the
  // cluster container is empty
  // Possibly, we need to find a more efficient solution
  ContainerAccessor<xAOD::StripCluster, IdentifierHash, 1>
    stripAccessor ( *stripContainer,
		    [] (const xAOD::StripCluster& cl) -> IdentifierHash 
		    { return cl.identifierHash(); },
		    stripDetEle->size());
  
  // retrieve the RoI as provided from upstream algos
  SG::ReadHandle<TrigRoiDescriptorCollection> roiCollectionHandle = SG::makeHandle( m_roiCollectionKey, ctx );
  ATH_CHECK(roiCollectionHandle.isValid());
  const TrigRoiDescriptorCollection *roiCollection = roiCollectionHandle.cptr();
  
  // Get list of Hash Ids from the RoI
  std::vector<IdentifierHash> listOfIds;
  for (const auto* roi : *roiCollection) {
    listOfIds.clear();
    m_regionSelector->HashIDList(*roi, listOfIds);
    // We'd need to first check the id hashes have not already been processed beforehand, and only then
    // add it to the list of ids to be processed.
    for (const IdentifierHash idHash : listOfIds) {
      if (stripAccessor.isIdentifierPresent(idHash)) continue;

      // If not already processed, do it now
      const InDetRawDataCollection<SCT_RDORawData>* rdos = rdoContainer->indexFindPtr(idHash);

      if (rdos == nullptr or rdos->empty()) {
	ATH_MSG_DEBUG("No input strip RDOs for this container element");
	continue;
      }
      
      bool goodModule = true;
      if (m_checkBadModules.value()) {
	if (!m_stripDetElStatus.empty()) {
	  goodModule = status->isGood(idHash);
	} else {
	  goodModule = m_summaryTool->isGood(idHash, ctx);
	}
      }
      VALIDATE_STATUS_ARRAY(
			    m_checkBadModules.value() && !m_stripDetElStatus.empty(),
			    status->isGood(idHash), m_summaryTool->isGood(idHash));
      
      if (!goodModule) {
	ATH_MSG_DEBUG("Strip module failed status check");
	continue;
      }
      
      // If more than a certain number of RDOs set module to bad
      // in this case we skip clusterization
      if (m_maxFiredStrips != 0u) {
	unsigned int nFiredStrips = 0u;
	for (const SCT_RDORawData* rdo : *rdos) {
	  nFiredStrips += rdo->getGroupSize();
	}
	if (nFiredStrips > m_maxFiredStrips)
	  continue;
      }
      
      ATH_CHECK(m_clusteringTool->clusterize(
					     *rdos, *m_idHelper, stripDetEle->getDetectorElement(idHash),
					     m_stripDetElStatus.empty() ? nullptr : status.cptr(),
					     *stripContainer));
    } // loop on ids 
  } // loop on RoIs

  return StatusCode::SUCCESS;
}

}  // namespace ActsTrk
