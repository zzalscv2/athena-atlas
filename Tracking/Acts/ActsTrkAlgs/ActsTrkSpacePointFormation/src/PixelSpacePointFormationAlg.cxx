/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "PixelSpacePointFormationAlg.h"

#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "PixelReadoutGeometry/PixelModuleDesign.h"

#include "xAODInDetMeasurement/PixelClusterAuxContainer.h"

#include "AthenaMonitoringKernel/Monitored.h"

namespace ActsTrk {

  //------------------------------------------------------------------------
  PixelSpacePointFormationAlg::PixelSpacePointFormationAlg(const std::string& name,
							   ISvcLocator* pSvcLocator)
    : AthReentrantAlgorithm(name, pSvcLocator)
    {}

  //-----------------------------------------------------------------------
  StatusCode PixelSpacePointFormationAlg::initialize()
  {
    ATH_MSG_DEBUG( "Initializing " << name() << " ... " );

    ATH_CHECK( m_pixelClusterContainerKey.initialize() );
    ATH_CHECK( m_pixelSpacePointContainerKey.initialize() );
    ATH_CHECK( m_pixelDetEleCollKey.initialize() );
    ATH_CHECK( m_spacePointMakerTool.retrieve() );

    if ( not m_monTool.empty() )
      ATH_CHECK( m_monTool.retrieve() );

    return StatusCode::SUCCESS;
  }

  //-------------------------------------------------------------------------
  StatusCode PixelSpacePointFormationAlg::execute (const EventContext& ctx) const
  {
    auto timer = Monitored::Timer<std::chrono::milliseconds>( "TIME_execute" );
    auto nReceivedSPsPixel = Monitored::Scalar<int>( "numPixSpacePoints" , 0 );
    auto mon = Monitored::Group( m_monTool, timer, nReceivedSPsPixel );

    auto pixelSpacePointContainer = SG::WriteHandle<xAOD::SpacePointContainer>( m_pixelSpacePointContainerKey, ctx );
    ATH_MSG_DEBUG( "--- Pixel Space Point Container `" << m_pixelSpacePointContainerKey.key() << "` created ..." );

    SG::ReadCondHandle<InDetDD::SiDetectorElementCollection> pixelDetEleHandle(m_pixelDetEleCollKey, ctx);
    const InDetDD::SiDetectorElementCollection* pixelElements(*pixelDetEleHandle);
    if (not pixelDetEleHandle.isValid() or pixelElements==nullptr) {
      ATH_MSG_FATAL(m_pixelDetEleCollKey.fullKey() << " is not available.");
      return StatusCode::FAILURE;
    }

    std::unique_ptr<xAOD::SpacePointContainer> pixelSpacePoints = std::make_unique<xAOD::SpacePointContainer>();
    std::unique_ptr<xAOD::SpacePointAuxContainer> pixelSpacePointsAux = std::make_unique<xAOD::SpacePointAuxContainer>();
    pixelSpacePoints->setStore( pixelSpacePointsAux.get() );



    SG::ReadHandle<xAOD::PixelClusterContainer> inputPixelClusterContainer( m_pixelClusterContainerKey, ctx );
    if (!inputPixelClusterContainer.isValid()){
        ATH_MSG_FATAL("xAOD::PixelClusterContainer with key " << m_pixelClusterContainerKey.key() << " is not available...");
        return StatusCode::FAILURE;
    }
    const xAOD::PixelClusterContainer *pixelClusters = inputPixelClusterContainer.cptr();

    // Reserve space
    pixelSpacePoints->reserve(pixelClusters->size());
    pixelSpacePointsAux->reserve(pixelClusters->size());

    // using trick for fast insertion
    std::vector< xAOD::SpacePoint* > preCollection;
    preCollection.reserve(pixelClusters->size());
    for ( std::size_t i(0); i<inputPixelClusterContainer->size(); ++i)
      preCollection.push_back( new xAOD::SpacePoint() );
    pixelSpacePoints->insert(pixelSpacePoints->end(), preCollection.begin(), preCollection.end());

    /// production of ActsTrk::SpacePoint from pixel clusters
    /// Pixel space points are created directly from the clusters global position
    for (std::size_t idx(0); idx<inputPixelClusterContainer->size(); ++idx) {
      auto cluster = inputPixelClusterContainer->at(idx);
      std::vector<std::size_t> measIndexes({idx});
      const InDetDD::SiDetectorElement* pixelElement = pixelElements->getDetectorElement(cluster->identifierHash());
      if (pixelElement == nullptr) {
        ATH_MSG_FATAL("Element pointer is nullptr");
        return StatusCode::FAILURE;
      }

      ATH_CHECK( m_spacePointMakerTool->producePixelSpacePoint(*cluster,
      							       *pixelSpacePoints->at(idx),
      							       measIndexes,
      							       *pixelElement ) );
    }

    ATH_CHECK( pixelSpacePointContainer.record( std::move( pixelSpacePoints ), std::move(pixelSpacePointsAux) ) );
    nReceivedSPsPixel = pixelSpacePointContainer->size();

    return StatusCode::SUCCESS;
  }

} //namespace
