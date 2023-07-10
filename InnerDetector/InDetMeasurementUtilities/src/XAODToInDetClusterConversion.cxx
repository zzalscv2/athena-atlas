/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "src/XAODToInDetClusterConversion.h"
#include "InDetMeasurementUtilities/ClusterConversionUtilities.h" 

#include "InDetIdentifier/PixelID.h"
#include "InDetIdentifier/SCT_ID.h"
#include "xAODInDetMeasurement/ContainerAccessor.h"

#include "PixelReadoutGeometry/PixelModuleDesign.h"
#include "SCT_ReadoutGeometry/StripStereoAnnulusDesign.h"

#include <map>
#include <cmath>

namespace InDet {
  
  XAODToInDetClusterConversion::XAODToInDetClusterConversion(const std::string &name,
							     ISvcLocator *pSvcLocator)
    : AthReentrantAlgorithm(name, pSvcLocator)
  {}
  
  StatusCode XAODToInDetClusterConversion::initialize() 
  {
    ATH_MSG_INFO( "Initializing " << name() << " ... " );
    
    // Pixel Clusters
    ATH_CHECK( m_pixelDetEleCollKey.initialize(m_processPixel) );
    ATH_CHECK( detStore()->retrieve(m_pixelID,"PixelID") );
    
    ATH_CHECK( m_inputPixelClusterContainerKey.initialize(m_processPixel) );
    ATH_CHECK( m_pixelClusterContainerLinkKey.initialize(m_processPixel) );
    ATH_CHECK( m_outputPixelClusterContainerKey.initialize(m_processPixel) );
    
    // Strip Clusters
    ATH_CHECK( m_stripDetEleCollKey.initialize(m_processStrip) );
    ATH_CHECK( detStore()->retrieve(m_stripID, "SCT_ID") );
    
    ATH_CHECK( m_inputStripClusterContainerKey.initialize(m_processStrip) );
    ATH_CHECK( m_stripClusterContainerLinkKey.initialize(m_processStrip) );
    ATH_CHECK( m_outputStripClusterContainerKey.initialize(m_processStrip) );
    
    ATH_CHECK( m_lorentzAngleTool.retrieve() );
    
    return StatusCode::SUCCESS;
  }
  
  StatusCode XAODToInDetClusterConversion::execute(const EventContext& ctx) const 
  {
    ATH_MSG_DEBUG( "Executing " << name() << " ... " );

    if (m_processPixel.value()) {    
      ATH_MSG_DEBUG("Converting Pixel Clusters: xAOD -> InDet");
      ATH_CHECK( convertPixelClusters(ctx) );
    }

    if (m_processStrip.value()) {
      ATH_MSG_DEBUG("Converting Strip Clusters: xAOD -> InDet");
      ATH_CHECK( convertStripClusters(ctx) );
    }

    return StatusCode::SUCCESS;
  }
  
  StatusCode XAODToInDetClusterConversion::convertPixelClusters(const EventContext& ctx) const
  {
    SG::ReadCondHandle<InDetDD::SiDetectorElementCollection> pixelDetEleHandle( m_pixelDetEleCollKey, ctx );
    const InDetDD::SiDetectorElementCollection* pixElements( *pixelDetEleHandle );
    if (not pixelDetEleHandle.isValid() or pixElements==nullptr) {
      ATH_MSG_FATAL(m_pixelDetEleCollKey.fullKey() << " is not available.");
      return StatusCode::FAILURE;
    }
    
    SG::ReadHandle<xAOD::PixelClusterContainer> inputPixelClusterContainer = SG::makeHandle( m_inputPixelClusterContainerKey, ctx );
    ATH_CHECK( inputPixelClusterContainer.isValid() );
    const xAOD::PixelClusterContainer *inputPixelClusters = inputPixelClusterContainer.cptr();
    
    SG::WriteHandle<InDet::PixelClusterContainer> outputPixelClusterContainer = SG::makeHandle(m_outputPixelClusterContainerKey, ctx);
    ATH_CHECK( outputPixelClusterContainer.record (std::make_unique<InDet::PixelClusterContainer>(m_pixelID->wafer_hash_max(), EventContainers::Mode::OfflineFast)) );
    ATH_CHECK( outputPixelClusterContainer.isValid() );
    ATH_MSG_DEBUG( "Container '" << m_outputPixelClusterContainerKey.key() << "' initialised" );
    
    ATH_CHECK( outputPixelClusterContainer.symLink( m_pixelClusterContainerLinkKey ) );
    
    // Conversion 
    // Access to the cluster from a given detector element is possible
    // via the ContainerAccessor.
    ContainerAccessor<xAOD::PixelCluster, IdentifierHash, 1>
      pixelAccessor ( *inputPixelClusters,
		      [] (const xAOD::PixelCluster& cl) -> IdentifierHash { return cl.identifierHash(); },
		      pixElements->size());
    
    const auto& allIdHashes = pixelAccessor.allIdentifiers();
    for (const auto& hashId : allIdHashes) {
      const InDetDD::SiDetectorElement *element = pixElements->getDetectorElement(hashId);
      if ( element == nullptr ) {
        ATH_MSG_FATAL( "Invalid pixel detector element for hash " << hashId);
        return StatusCode::FAILURE;
      }
      
      std::unique_ptr<InDet::PixelClusterCollection> collection = std::make_unique<InDet::PixelClusterCollection>(hashId);
      
      // Get the detector element and range for the idHash
      for (const auto& this_range : pixelAccessor.rangesForIdentifierDirect(hashId)) {
	for (auto start = this_range.first; start != this_range.second; ++start) {
	  const xAOD::PixelCluster* in_cluster = *start;
	  
	  InDet::PixelCluster* cluster = nullptr;
	  ATH_CHECK( TrackingUtilities::convertXaodToInDetCluster(*in_cluster, *element, *m_pixelID, cluster) );
	  cluster->setHashAndIndex(hashId, collection->size());

	  // Add to Collection
	  collection->push_back(cluster);
	}
      }

      InDet::PixelClusterContainer::IDC_WriteHandle lock = outputPixelClusterContainer->getWriteHandle(hashId);
      ATH_CHECK(lock.addOrDelete( std::move(collection) ));      

    } // loop on hashIds
    
    ATH_CHECK( outputPixelClusterContainer.setConst() );
    
    return StatusCode::SUCCESS;
  }

  StatusCode XAODToInDetClusterConversion::convertStripClusters(const EventContext& ctx) const
  {
    SG::ReadCondHandle<InDetDD::SiDetectorElementCollection> stripDetEleHandle( m_stripDetEleCollKey, ctx );
    const InDetDD::SiDetectorElementCollection* stripElements( *stripDetEleHandle );
    if (not stripDetEleHandle.isValid() or stripElements==nullptr) {
      ATH_MSG_FATAL(m_stripDetEleCollKey.fullKey() << " is not available.");
      return StatusCode::FAILURE;
    }
    
    SG::ReadHandle<xAOD::StripClusterContainer> inputStripClusterContainer = SG::makeHandle( m_inputStripClusterContainerKey, ctx );
    ATH_CHECK( inputStripClusterContainer.isValid() );
    const xAOD::StripClusterContainer *inputStripClusters = inputStripClusterContainer.cptr();
    
    SG::WriteHandle<InDet::SCT_ClusterContainer> outputStripClusterContainer = SG::makeHandle( m_outputStripClusterContainerKey, ctx );
    ATH_CHECK( outputStripClusterContainer.record (std::make_unique<InDet::SCT_ClusterContainer>(m_stripID->wafer_hash_max(), EventContainers::Mode::OfflineFast)) );
    ATH_CHECK( outputStripClusterContainer.isValid() );
    ATH_MSG_DEBUG( "Container '" << m_outputStripClusterContainerKey.key() << "' initialised" );
    
    ATH_CHECK( outputStripClusterContainer.symLink( m_stripClusterContainerLinkKey ) );
    
    // Conversion
    // Access to the cluster from a given detector element is possible
    // via the ContainerAccessor.
    ContainerAccessor<xAOD::StripCluster, IdentifierHash, 1>
      stripAccessor ( *inputStripClusters,
		      [] (const xAOD::StripCluster& cl) -> IdentifierHash { return cl.identifierHash(); },
		      stripElements->size());



    const auto& allIdHashes = stripAccessor.allIdentifiers();
    for (const auto& hashId : allIdHashes) {
      const InDetDD::SiDetectorElement *element = stripElements->getDetectorElement(hashId);
      if ( element == nullptr ) {
        ATH_MSG_FATAL( "Invalid strip detector element for hash " << hashId);
        return StatusCode::FAILURE;
      }

      bool isBarrel = element->isBarrel();

      double shift = not isBarrel
	? m_lorentzAngleTool->getLorentzShift(hashId)
	: 0.;
      
      std::unique_ptr<InDet::SCT_ClusterCollection> collection = std::make_unique<InDet::SCT_ClusterCollection>(hashId);


      // Get the detector element and range for the idHash
      for (const auto& this_range : stripAccessor.rangesForIdentifierDirect(hashId)) {
        for (auto start = this_range.first; start != this_range.second; ++start) {
          const xAOD::StripCluster* in_cluster = *start;

	  InDet::SCT_Cluster* cluster = nullptr;
	  ATH_CHECK( TrackingUtilities::convertXaodToInDetCluster(*in_cluster, *element, *m_stripID, cluster, shift) );
	  cluster->setHashAndIndex(hashId, collection->size());

	  // Add to Collection
	  collection->push_back( cluster );
	}
      }

      InDet::SCT_ClusterContainer::IDC_WriteHandle lock = outputStripClusterContainer->getWriteHandle(hashId);
      ATH_CHECK(lock.addOrDelete( std::move(collection) ));

    }

    ATH_CHECK( outputStripClusterContainer.setConst() );
    
    return StatusCode::SUCCESS;
  }

}


