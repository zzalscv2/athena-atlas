/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "InDetPrepRawDataFormation/XAODToInDetClusterConversion.h"
#include "InDetIdentifier/PixelID.h"
#include "xAODInDetMeasurement/ContainerAccessor.h"

#include <map>
#include <cmath>

namespace InDet {

  XAODToInDetClusterConversion::XAODToInDetClusterConversion(const std::string &name,
							     ISvcLocator *pSvcLocator)
    : AthReentrantAlgorithm(name, pSvcLocator)
  {}

  StatusCode XAODToInDetClusterConversion::initialize() 
  {
    ATH_MSG_DEBUG( "Initializing " << name() << " ... " );

    // Pixel Clusters
    ATH_CHECK( m_pixelDetEleCollKey.initialize());
    ATH_CHECK(detStore()->retrieve(m_pixelID,"PixelID"));

    ATH_CHECK( m_inputPixelClusterContainerKey.initialize() );
    ATH_CHECK( m_clusterContainerLinkKey.initialize() );

    ATH_CHECK( m_outputPixelClusterContainerKey.initialize() );

    return StatusCode::SUCCESS;
  }

  StatusCode XAODToInDetClusterConversion::execute(const EventContext& ctx) const 
  {
    ATH_MSG_DEBUG( "Executing " << name() << " ... " );

    ATH_MSG_DEBUG("Converting Pixel Clusters: xAOD -> InDet");
    ATH_CHECK( convertPixelClusters(ctx) );

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
    
    ATH_CHECK( outputPixelClusterContainer.symLink( m_clusterContainerLinkKey ) );
    
    // Conversion 
    // Access to the cluster from a given detector element is possible
    // via the ContainerAccessor.
    ContainerAccessor<xAOD::PixelCluster, IdentifierHash, 1>
      pixelAccessor ( *inputPixelClusters,
		      [] (const xAOD::PixelCluster& cl) -> IdentifierHash { return cl.identifierHash(); },
		      pixElements->size());
    
    const auto& allIdHashes = pixelAccessor.allIdentifiers();
    for (auto& hashId : allIdHashes) {
      const InDetDD::SiDetectorElement *element = pixElements->getDetectorElement(hashId);
      if ( element == nullptr ) {
        ATH_MSG_FATAL( "Invalid pixel detector element for hash " << hashId);
        return StatusCode::FAILURE;
      }
      
      const InDetDD::PixelModuleDesign* design(dynamic_cast<const InDetDD::PixelModuleDesign*>(&element->design()));
      if (design == nullptr) {
	ATH_MSG_ERROR("Could not retrieve InDetDD::PixelModuleDesign from element with hashId: " << hashId);
	return StatusCode::FAILURE;
      }      

      std::unique_ptr<InDet::PixelClusterCollection> collection = std::make_unique<InDet::PixelClusterCollection>(hashId);
      
      // Get the detector element and range for the idHash
      for (auto& this_range : pixelAccessor.rangesForIdentifierDirect(hashId)) {
	for (auto start = this_range.first; start != this_range.second; start++) {
	  const xAOD::PixelCluster* in_cluster = *start;
	  
	  const auto& locPos = in_cluster->localPosition<2>();
	  Amg::Vector2D localPosition(locPos(0,0), locPos(1,0));

	  InDetDD::SiLocalPosition centroid(localPosition);
	  const Identifier id = element->identifierOfPosition(centroid);
	  	  
	  const auto& globalPos = in_cluster->globalPosition();
	  Amg::Vector3D globalPosition(globalPos(0, 0), globalPos(1, 0), globalPos(2, 0));
	  
	  bool isSplit =  static_cast<bool>(in_cluster->isSplit());
	  
	  auto errorMatrix = Amg::MatrixX(2,2);
	  errorMatrix.setIdentity();
	  errorMatrix.fillSymmetric(0, 0, in_cluster->localCovariance<2>()(0, 0));
	  errorMatrix.fillSymmetric(1, 1, in_cluster->localCovariance<2>()(1, 1));
	  
	  int colmax = std::numeric_limits<int>::min();
	  int rowmax = std::numeric_limits<int>::min();
	  int colmin = std::numeric_limits<int>::max();
	  int rowmin = std::numeric_limits<int>::max();
	  
	  const std::vector<Identifier>& rod_list_cluster = in_cluster->rdoList();
	  for (const auto& this_rdo : rod_list_cluster) {
	    const int row = m_pixelID->phi_index(this_rdo);
	    if (row > rowmax)
	      rowmax = row;
	    if (row < rowmin)
	      rowmin = row;
	    
	    const int col = m_pixelID->eta_index(this_rdo);
	    if (col > colmax)
	      colmax = col;
	    if (col < colmin)
	      colmin = col;
	  }
	  
	  double etaWidth = design->widthFromColumnRange(colmin, colmax);
	  double phiWidth = design->widthFromRowRange(rowmin, rowmax);
	  InDet::SiWidth width( Amg::Vector2D(in_cluster->channelsInPhi(), in_cluster->channelsInEta()),
				Amg::Vector2D(phiWidth,etaWidth) );
	  
	  
	  
	  // Create Cluster
	  InDet::PixelCluster* cluster = new InDet::PixelCluster(id,
								 localPosition,
								 globalPosition,
								 in_cluster->rdoList(),
								 in_cluster->lvl1a(),
								 in_cluster->totList(),
								 in_cluster->chargeList(),
								 width,
								 element,
								 errorMatrix,
								 in_cluster->omegaX(),
								 in_cluster->omegaY(),
								 isSplit,
								 in_cluster->splitProbability1(),
								 in_cluster->splitProbability2());

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

}


