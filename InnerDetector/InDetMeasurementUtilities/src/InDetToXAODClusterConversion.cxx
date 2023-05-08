/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "src/InDetToXAODClusterConversion.h"
#include "InDetMeasurementUtilities/ClusterConversionUtilities.h"

#include "Identifier/Identifier.h"
#include "InDetIdentifier/PixelID.h"
#include "InDetIdentifier/SCT_ID.h"
#include "InDetPrepRawData/PixelClusterCollection.h"
#include "InDetPrepRawData/SCT_ClusterCollection.h"

#include "StoreGate/WriteHandle.h"
#include "StoreGate/ReadHandle.h"

#include "xAODInDetMeasurement/PixelClusterAuxContainer.h"
#include "xAODInDetMeasurement/StripClusterAuxContainer.h"

#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "SCT_ReadoutGeometry/StripStereoAnnulusDesign.h"

#include "AthContainers/DataVector.h"
#include <iterator>



using namespace InDet;

// Constructor with parameters:
InDetToXAODClusterConversion::InDetToXAODClusterConversion(const std::string &name, ISvcLocator *pSvcLocator) :
AthReentrantAlgorithm(name, pSvcLocator)
{}


//-----------------------------------------------------------------------------
// Initialize method:
StatusCode InDetToXAODClusterConversion::initialize() {
  ATH_MSG_INFO("Initializing " << name() << " ...");
  
  ATH_CHECK(detStore()->retrieve(m_pixelID,"PixelID"));
  ATH_CHECK(detStore()->retrieve(m_stripID,"SCT_ID"));
  
  ATH_CHECK( m_pixelDetEleCollKey.initialize() );
  ATH_CHECK( m_stripDetEleCollKey.initialize() );
  
  ATH_CHECK( m_inputPixelClusterContainerKey.initialize() );
  ATH_CHECK( m_inputStripClusterContainerKey.initialize() );
  ATH_CHECK( m_outputPixelClusterContainerKey.initialize() );
  ATH_CHECK( m_outputStripClusterContainerKey.initialize() );
  
  ATH_MSG_DEBUG( "Initialize done !" );
  return StatusCode::SUCCESS;
}

//----------------------------------------------------------------------------
//
StatusCode InDetToXAODClusterConversion::execute(const EventContext& ctx) const {
  ATH_MSG_DEBUG("Executing " << name() << " ...");

  ATH_MSG_DEBUG("Converting Pixel Clusters: InDet -> xAOD");
  ATH_CHECK( convertPixelClusters(ctx) );
  
  ATH_MSG_DEBUG("Converting Strip Clusters: InDet -> xAOD");
  ATH_CHECK( convertStripClusters(ctx) );

  return StatusCode::SUCCESS;
}

StatusCode InDetToXAODClusterConversion::convertPixelClusters(const EventContext& ctx) const {
  SG::ReadCondHandle<InDetDD::SiDetectorElementCollection> pixelDetEleHandle(m_pixelDetEleCollKey, ctx);
  const InDetDD::SiDetectorElementCollection* pixElements(*pixelDetEleHandle);
  if (not pixelDetEleHandle.isValid() or pixElements==nullptr) {
    ATH_MSG_FATAL(m_pixelDetEleCollKey.fullKey() << " is not available.");
    return StatusCode::FAILURE;
  }
  
  SG::WriteHandle<xAOD::PixelClusterContainer> outputPixelClusterContainer(m_outputPixelClusterContainerKey, ctx);
  ATH_CHECK( outputPixelClusterContainer.record (std::make_unique<xAOD::PixelClusterContainer>(),
						 std::make_unique<xAOD::PixelClusterAuxContainer>()) );
  ATH_MSG_DEBUG( "Recorded xAOD::PixelClusterContainer with key: " << m_outputPixelClusterContainerKey.key()  );
  
  SG::ReadHandle<InDet::PixelClusterContainer> inputPixelClusterContainer(m_inputPixelClusterContainerKey, ctx);
  
  static const SG::AuxElement::Accessor< ElementLink< InDet::PixelClusterCollection > > pixelLinkAcc("pixelClusterLink");
  for (const auto *const clusterCollection : *inputPixelClusterContainer) {
    if (!clusterCollection) continue;
    for(const auto *const theCluster : *clusterCollection)  {
      Identifier clusterId = theCluster->identify();
      
      const InDetDD::SiDetectorElement *element=pixElements->getDetectorElement(m_pixelID->wafer_hash(m_pixelID->wafer_id(clusterId)));
      if ( element==nullptr ) {
	ATH_MSG_FATAL( "Invalid pixel detector element for cluster identifier " << clusterId );
	return StatusCode::FAILURE;
      }
      
      xAOD::PixelCluster * pixelCl = new xAOD::PixelCluster();
      outputPixelClusterContainer->push_back(pixelCl);
      ATH_CHECK( TrackingUtilities::convertInDetToXaodCluster(*theCluster, *element, *pixelCl) );
      
      // Create auxiliary branches accessors
      ElementLink<InDet::PixelClusterCollection> pixelLink(theCluster, *clusterCollection);
      pixelLinkAcc( *pixelCl ) = pixelLink;
    }
  }
  
  ATH_MSG_DEBUG("xAOD::PixelClusterContainer with size: " << outputPixelClusterContainer->size());
  return StatusCode::SUCCESS;
}

StatusCode InDetToXAODClusterConversion::convertStripClusters(const EventContext& ctx) const {
  SG::ReadCondHandle<InDetDD::SiDetectorElementCollection> stripDetEleHandle(m_stripDetEleCollKey, ctx);
  const InDetDD::SiDetectorElementCollection* stripElements(*stripDetEleHandle);
  if (not stripDetEleHandle.isValid() or stripElements==nullptr) {
    ATH_MSG_FATAL(m_stripDetEleCollKey.fullKey() << " is not available.");
    return StatusCode::FAILURE;
  }
  
  SG::WriteHandle<xAOD::StripClusterContainer> outputStripClusterContainer(m_outputStripClusterContainerKey, ctx);
  ATH_CHECK( outputStripClusterContainer.record (std::make_unique<xAOD::StripClusterContainer>(),
						 std::make_unique<xAOD::StripClusterAuxContainer>()) );
  ATH_MSG_DEBUG( "Recorded xAOD::StripClusterContainer with key: " << m_outputStripClusterContainerKey.key()  );
  
  SG::ReadHandle<InDet::SCT_ClusterContainer> inputStripClusterContainer(m_inputStripClusterContainerKey, ctx);
  
  static const SG::AuxElement::Accessor< ElementLink< InDet::SCT_ClusterCollection > > stripLinkAcc("sctClusterLink");
  for (const auto *const clusterCollection : *inputStripClusterContainer) {
    if (!clusterCollection) continue;
    for(const auto *const theCluster : *clusterCollection)  {
      Identifier clusterId = theCluster->identify();
      
      const InDetDD::SiDetectorElement *element=stripElements->getDetectorElement(m_stripID->wafer_hash(m_stripID->wafer_id(clusterId)));
      if ( element==nullptr ) {
	ATH_MSG_FATAL( "Invalid strip detector element for cluster with identifier " << clusterId );
	return StatusCode::FAILURE;
      }
      
      
      xAOD::StripCluster * stripCl = new xAOD::StripCluster();
      outputStripClusterContainer->push_back(stripCl);
      ATH_CHECK( TrackingUtilities::convertInDetToXaodCluster(*theCluster, *element, *stripCl) );
      
      // Create auxiliary branches accessors
      ElementLink<InDet::SCT_ClusterCollection> stripLink(theCluster, *clusterCollection);
      stripLinkAcc( *stripCl ) = stripLink;
    }
  }
  
  ATH_MSG_DEBUG("xAOD::StripClusterContainer with size: " << outputStripClusterContainer->size());
  
  return StatusCode::SUCCESS;
}
