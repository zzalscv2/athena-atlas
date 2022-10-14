/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "InDetPrepRawDataFormation/XAODToInDetClusterConversion.h"
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
    ATH_MSG_DEBUG( "Initializing " << name() << " ... " );

    // Pixel Clusters
    ATH_CHECK( m_pixelDetEleCollKey.initialize() );
    ATH_CHECK( detStore()->retrieve(m_pixelID,"PixelID") );

    ATH_CHECK( m_inputPixelClusterContainerKey.initialize() );
    ATH_CHECK( m_pixelClusterContainerLinkKey.initialize() );
    ATH_CHECK( m_outputPixelClusterContainerKey.initialize() );

    // Strip Clusters
    ATH_CHECK( m_stripDetEleCollKey.initialize() );
    ATH_CHECK( detStore()->retrieve(m_stripID, "SCT_ID") );

    ATH_CHECK( m_inputStripClusterContainerKey.initialize() );
    ATH_CHECK( m_stripClusterContainerLinkKey.initialize() );
    ATH_CHECK( m_outputStripClusterContainerKey.initialize() );

    ATH_CHECK( m_lorentzAngleTool.retrieve() );

    return StatusCode::SUCCESS;
  }

  StatusCode XAODToInDetClusterConversion::execute(const EventContext& ctx) const 
  {
    ATH_MSG_DEBUG( "Executing " << name() << " ... " );

    ATH_MSG_DEBUG("Converting Pixel Clusters: xAOD -> InDet");
    ATH_CHECK( convertPixelClusters(ctx) );

    ATH_MSG_DEBUG("Converting Strip Clusters: xAOD -> InDet");
    ATH_CHECK( convertStripClusters(ctx) );

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
    for (auto& hashId : allIdHashes) {
      const InDetDD::SiDetectorElement *element = stripElements->getDetectorElement(hashId);
      if ( element == nullptr ) {
        ATH_MSG_FATAL( "Invalid strip detector element for hash " << hashId);
        return StatusCode::FAILURE;
      }

      bool isBarrel = element->isBarrel();

      const InDetDD::SCT_ModuleSideDesign* design = nullptr;
      if (not isBarrel) {
      	design = dynamic_cast<const InDetDD::StripStereoAnnulusDesign*>(&element->design());
      } else {
	design = dynamic_cast<const InDetDD::SCT_ModuleSideDesign*>(&element->design());
      }

      if (design == nullptr) {
	ATH_MSG_ERROR( "Could not retrieve InDetDD::SCT_ModuleSideDesign from element with hashId: " << hashId );
      	return StatusCode::FAILURE;
      }

      const auto designShape = design->shape();

      double shift = not isBarrel
	? m_lorentzAngleTool->getLorentzShift(hashId)
	: 0.;
      
      std::unique_ptr<InDet::SCT_ClusterCollection> collection = std::make_unique<InDet::SCT_ClusterCollection>(hashId);


      // Get the detector element and range for the idHash
      for (auto& this_range : stripAccessor.rangesForIdentifierDirect(hashId)) {
        for (auto start = this_range.first; start != this_range.second; start++) {
          const xAOD::StripCluster* in_cluster = *start;

	  const auto& rdoList = in_cluster->rdoList();
	  Identifier id = rdoList.front();

	  const auto& localPos = in_cluster->localPosition<1>();

	  double pos_x = localPos(0, 0);
	  double pos_y = 0;
	  if (not isBarrel) {
	    const Identifier firstStripId = rdoList.front();
	    int firstStrip = m_stripID->strip(firstStripId);
            int stripRow = m_stripID->row(firstStripId);
            int clusterSizeInStrips = in_cluster->channelsInPhi();
	    auto clusterPosition = design->localPositionOfCluster(design->strip1Dim(firstStrip, stripRow), clusterSizeInStrips);
	    pos_x = clusterPosition.xPhi() + shift;
	    pos_y = clusterPosition.xEta();
	  }

	  Amg::Vector2D locpos = Amg::Vector2D( pos_x, pos_y );

	  // Most of the following is taken from what is done in ClusterMakerTool

	  // Need to make this computation instead of using the local pos
	  // with local pos instead some differences w.r.t. reference are observed
	  const auto& firstStrip = m_stripID->strip(rdoList.front());
	  const auto& lastStrip = m_stripID->strip(rdoList.back());
	  const auto& row = m_stripID->row(rdoList.front());
	  const int firstStrip1D = design->strip1Dim (firstStrip, row );
	  const int lastStrip1D = design->strip1Dim( lastStrip, row );
	  const InDetDD::SiCellId cell1(firstStrip1D); 
	  const InDetDD::SiCellId cell2(lastStrip1D);  
	  const InDetDD::SiLocalPosition firstStripPos( element->rawLocalPositionOfCell(cell1 ));
	  const InDetDD::SiLocalPosition lastStripPos( element->rawLocalPositionOfCell(cell2) );
	  const InDetDD::SiLocalPosition centre( (firstStripPos+lastStripPos)*0.5 );
	  const double clusterWidth = design->stripPitch() * ( lastStrip - firstStrip + 1 );

	  const std::pair<InDetDD::SiLocalPosition, InDetDD::SiLocalPosition> ends( design->endsOfStrip(centre) );
	  const double stripLength( std::abs(ends.first.xEta() - ends.second.xEta()) );

	  InDet::SiWidth width( Amg::Vector2D(in_cluster->channelsInPhi(), 1), 
				Amg::Vector2D(clusterWidth, stripLength) );

	  const double col_x = width.colRow().x();
	  const double col_y = width.colRow().y();

	  double scale_factor = 1.;
	  if ( col_x  == 1 )
	    scale_factor = 1.05;
	  else if ( col_x == 2 )
	    scale_factor = 0.27;

	  auto errorMatrix = Amg::MatrixX(2,2);
          errorMatrix.setIdentity();
          errorMatrix.fillSymmetric(0, 0, scale_factor * scale_factor * width.phiR() * width.phiR() / 12.);
          errorMatrix.fillSymmetric(1, 1, width.z() * width.z() / col_y / col_y / 12.);

	  if( designShape == InDetDD::Trapezoid or
	      designShape == InDetDD::Annulus) {
	    // rotation for endcap SCT

	    // The following is being computed with the local position,
	    // without considering the lorentz shift
	    // So we remove it from the local position
	    Amg::Vector2D local(pos_x - shift, pos_y);
	    double sn = element->sinStereoLocal(local);
	    double sn2 = sn * sn;
	    double cs2 = 1. - sn2;
	    double w = element->phiPitch(local) / element->phiPitch(); 
	    double v0 = errorMatrix(0,0) * w * w;
	    double v1 = errorMatrix(1,1);
	    errorMatrix.fillSymmetric( 0, 0, cs2 * v0 + sn2 * v1 );
	    errorMatrix.fillSymmetric( 0, 1, sn * std::sqrt(cs2) * (v0 - v1) );
	    errorMatrix.fillSymmetric( 1, 1, sn2 * v0 + cs2 * v1 );
	  }

	  // Create Cluster
	  InDet::SCT_Cluster* cluster = new InDet::SCT_Cluster(id,
							       locpos,
							       rdoList,
							       width,
							       element,
							       errorMatrix);
	  cluster->setHitsInThirdTimeBin( in_cluster->hitsInThirdTimeBin() );

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


