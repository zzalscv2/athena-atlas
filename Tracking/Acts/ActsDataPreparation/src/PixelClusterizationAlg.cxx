/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "PixelClusterizationAlg.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "xAODInDetMeasurement/PixelClusterAuxContainer.h"
#include "xAODInDetMeasurement/ContainerAccessor.h"
#include <math.h>

namespace ActsTrk {

PixelClusterizationAlg::PixelClusterizationAlg(const std::string& name,
							   ISvcLocator* pSvcLocator)
    : AthReentrantAlgorithm(name, pSvcLocator) {}


StatusCode PixelClusterizationAlg::initialize()
{
    ATH_MSG_DEBUG("Initializing " << name() << " ...");

    ATH_CHECK(m_pixelDetEleCollKey.initialize());
    ATH_CHECK(m_rdoContainerKey.initialize());
    ATH_CHECK(m_clusterContainerKey.initialize());
    ATH_CHECK(m_roiCollectionKey.initialize());

    ATH_CHECK(m_clusteringTool.retrieve());
    ATH_CHECK(m_regionSelector.retrieve());

    ATH_CHECK(detStore()->retrieve(m_idHelper,"PixelID"));

    // Monitoring
    ATH_CHECK(m_monTool.retrieve(EnableTool{not m_monTool.empty()}));
    
    return StatusCode::SUCCESS;
}


StatusCode PixelClusterizationAlg::execute(const EventContext& ctx) const
{
    auto timer = Monitored::Timer<std::chrono::milliseconds>( "TIME_execute" );
    auto mon = Monitored::Group( m_monTool, timer );

    SG::ReadCondHandle<InDetDD::SiDetectorElementCollection> pixelDetEleHandle( m_pixelDetEleCollKey, ctx );
    const InDetDD::SiDetectorElementCollection* pixElements( *pixelDetEleHandle );
    if (pixElements == nullptr) {
      ATH_MSG_FATAL(m_pixelDetEleCollKey.fullKey() << " is not available.");
      return StatusCode::FAILURE;
    }

    SG::ReadHandle<PixelRDO_Container> rdoContainer = SG::makeHandle(m_rdoContainerKey, ctx);
    ATH_CHECK(rdoContainer.isValid());

    SG::WriteHandle<xAOD::PixelClusterContainer> clusterHandle
	= SG::makeHandle(m_clusterContainerKey, ctx);
    ATH_CHECK(clusterHandle.record( std::make_unique<xAOD::PixelClusterContainer>(), 
				    std::make_unique<xAOD::PixelClusterAuxContainer>() ));
    xAOD::PixelClusterContainer *pixelContainer = clusterHandle.ptr();
    // Reserve space, estimate of mean clusters to reduce re-allocations
    pixelContainer->reserve( m_expectedClustersPerRDO.value() * rdoContainer->size() );

    // This will allow us to access the already processed hash ids
    // For now we are not using yet an update write handle, so the 
    // cluster container is empty    
    // Possibly, we need to find a more efficient solution
    ContainerAccessor<xAOD::PixelCluster, IdentifierHash, 1>
      pixelAccessor ( *pixelContainer,
		      [] (const xAOD::PixelCluster& cl) -> IdentifierHash 
		      { return cl.identifierHash(); },
		      pixElements->size());

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
      for (const IdentifierHash id : listOfIds) {
	if (pixelAccessor.isIdentifierPresent(id)) continue;

	// If not already processed, do it now
	const InDetRawDataCollection<PixelRDORawData>* rdos = rdoContainer->indexFindPtr(id);
	if (rdos != nullptr && !rdos->empty()) {
	  ATH_CHECK(m_clusteringTool->clusterize(*rdos, *m_idHelper, ctx, *pixelContainer));
	} else {
	  ATH_MSG_DEBUG("No input RDOs for this container element");
	}
      } // loop on ids
    } // loop on rois
    
    return StatusCode::SUCCESS;
}

}

