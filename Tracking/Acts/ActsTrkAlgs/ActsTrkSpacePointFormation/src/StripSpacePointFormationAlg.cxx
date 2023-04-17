/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "StripSpacePointFormationAlg.h"

#include "InDetReadoutGeometry/SiDetectorElement.h"

#include "xAODInDetMeasurement/StripClusterAuxContainer.h"

#include "AthenaMonitoringKernel/Monitored.h"

namespace ActsTrk {

  //------------------------------------------------------------------------
  StripSpacePointFormationAlg::StripSpacePointFormationAlg(const std::string& name,
							   ISvcLocator* pSvcLocator)
    : AthReentrantAlgorithm(name, pSvcLocator)
    {}

  //-----------------------------------------------------------------------
  StatusCode StripSpacePointFormationAlg::initialize()
  {
    ATH_MSG_DEBUG( "Initializing " << name() << " ... " );

    ATH_CHECK( m_stripClusterContainerKey.initialize() );
    ATH_CHECK( m_stripSpacePointContainerKey.initialize() );
    ATH_CHECK( m_stripOverlapSpacePointContainerKey.initialize( m_processOverlapForStrip) );
    ATH_CHECK( m_stripDetEleCollKey.initialize() );
    ATH_CHECK( m_stripPropertiesKey.initialize() );

    ATH_CHECK( m_beamSpotKey.initialize() );

    if ( not m_monTool.empty() )
      ATH_CHECK( m_monTool.retrieve() );

    return StatusCode::SUCCESS;
  }

  //-------------------------------------------------------------------------
  StatusCode StripSpacePointFormationAlg::execute (const EventContext& ctx) const
  {
    auto timer = Monitored::Timer<std::chrono::milliseconds>( "TIME_execute" );
    auto nReceivedSPsStrip = Monitored::Scalar<int>( "numStripSpacePoints" , 0 );
    auto nReceivedSPsStripOverlap = Monitored::Scalar<int>( "numStripOverlapSpacePoints" , 0 );
    auto mon = Monitored::Group( m_monTool, timer, nReceivedSPsStrip, nReceivedSPsStripOverlap );

    SG::ReadCondHandle<InDet::BeamSpotData> beamSpotHandle { m_beamSpotKey, ctx };
    const InDet::BeamSpotData* beamSpot = *beamSpotHandle;
    auto vertex = beamSpot->beamVtx().position();

    auto stripSpacePointContainer = SG::WriteHandle<xAOD::SpacePointContainer>( m_stripSpacePointContainerKey, ctx );
    ATH_MSG_DEBUG( "--- Strip Space Point Container `" << m_stripSpacePointContainerKey.key() << "` created ..." );


    SG::ReadCondHandle<InDetDD::SiDetectorElementCollection> stripDetEleHandle(m_stripDetEleCollKey, ctx);
    const InDetDD::SiDetectorElementCollection* stripElements(*stripDetEleHandle);
    if (not stripDetEleHandle.isValid() or stripElements==nullptr) {
      ATH_MSG_FATAL(m_stripDetEleCollKey.fullKey() << " is not available.");
      return StatusCode::FAILURE;
    }

    SG::ReadCondHandle<InDet::SiElementPropertiesTable> stripProperties(m_stripPropertiesKey, ctx);
    const InDet::SiElementPropertiesTable* properties = stripProperties.retrieve();
    if (properties==nullptr) {
      ATH_MSG_FATAL("Pointer of SiElementPropertiesTable (" << m_stripPropertiesKey.fullKey() << ") could not be retrieved");
      return StatusCode::FAILURE;
    }


    SG::ReadHandle<xAOD::StripClusterContainer> inputStripClusterContainer( m_stripClusterContainerKey, ctx );
    if (!inputStripClusterContainer.isValid()){
        ATH_MSG_FATAL("xAOD::StripClusterContainer with key " << m_stripClusterContainerKey.key() << " is not available...");
        return StatusCode::FAILURE;
    }

    std::vector<StripSP> sps;
    std::vector<StripSP> osps;
    sps.reserve(inputStripClusterContainer->size() * 0.5);
    osps.reserve(inputStripClusterContainer->size() * 0.5);

    ATH_CHECK( m_spacePointMakerTool->produceSpacePoints(ctx,
							 *inputStripClusterContainer.cptr(),
							 *properties,
							 *stripElements,
							 vertex,
							 sps,
							 osps,
							 m_processOverlapForStrip) );

    // using trick for fast insertion
    std::unique_ptr<xAOD::SpacePointContainer> spacePoints =
        std::make_unique<xAOD::SpacePointContainer>();
    std::unique_ptr<xAOD::SpacePointAuxContainer> spacePointsAux =
      std::make_unique<xAOD::SpacePointAuxContainer>();
    spacePoints->setStore(spacePointsAux.get());

    spacePoints->reserve(sps.size());
    spacePointsAux->reserve(sps.size());

    std::vector<xAOD::SpacePoint*> sp_collection;
    sp_collection.reserve(sps.size());
    for (std::size_t i(0); i<sps.size(); ++i)
      sp_collection.push_back(new xAOD::SpacePoint());
    spacePoints->insert(spacePoints->end(), sp_collection.begin(), sp_collection.end());

    // fill
    for (std::size_t i(0); i<sps.size(); ++i) {
      auto& toAdd = sps.at(i);
      spacePoints->at(i)->setSpacePoint(toAdd.idHashes, 
					toAdd.globPos,
					toAdd.cov_r,
					toAdd.cov_z,
					toAdd.measurementIndexes,
					toAdd.topHalfStripLength,
					toAdd.bottomHalfStripLength,
					toAdd.topStripDirection,
					toAdd.bottomStripDirection,
					toAdd.stripCenterDistance,
					toAdd.topStripCenter);
    }



    std::unique_ptr<xAOD::SpacePointContainer> overlapSpacePoints =
        std::make_unique<xAOD::SpacePointContainer>();
    std::unique_ptr<xAOD::SpacePointAuxContainer> overlapSpacePointsAux =
      std::make_unique<xAOD::SpacePointAuxContainer>();
    overlapSpacePoints->setStore(overlapSpacePointsAux.get());

    overlapSpacePoints->reserve(osps.size());
    overlapSpacePointsAux->reserve(osps.size());

    std::vector<xAOD::SpacePoint*> sp_overlap_collection;
    sp_overlap_collection.reserve(osps.size());
    if (m_processOverlapForStrip) {
      for (std::size_t i(0); i<osps.size(); ++i)
	sp_overlap_collection.push_back(new xAOD::SpacePoint());
      overlapSpacePoints->insert(overlapSpacePoints->end(), sp_overlap_collection.begin(), sp_overlap_collection.end());
      
      for (std::size_t i(0); i<osps.size(); ++i) {
	auto& toAdd = osps.at(i);
	overlapSpacePoints->at(i)->setSpacePoint(toAdd.idHashes, 
						 toAdd.globPos,
						 toAdd.cov_r,
						 toAdd.cov_z,
						 toAdd.measurementIndexes,
						 toAdd.topHalfStripLength,
						 toAdd.bottomHalfStripLength,
						 toAdd.topStripDirection,
						 toAdd.bottomStripDirection,
						 toAdd.stripCenterDistance,
						 toAdd.topStripCenter);
      }
    }
    
    ATH_CHECK( stripSpacePointContainer.record( std::move( spacePoints ), std::move( spacePointsAux ) ) );

    nReceivedSPsStrip = stripSpacePointContainer->size();

    if (m_processOverlapForStrip) {
      auto stripOverlapSpacePointContainer = SG::WriteHandle<xAOD::SpacePointContainer>( m_stripOverlapSpacePointContainerKey, ctx );
      ATH_MSG_DEBUG( "--- Strip Overlap Space Point Container `" << m_stripOverlapSpacePointContainerKey.key() << "` created ..." );
      ATH_CHECK( stripOverlapSpacePointContainer.record( std::move( overlapSpacePoints ), std::move( overlapSpacePointsAux ) ) );

      nReceivedSPsStripOverlap = stripOverlapSpacePointContainer->size();
    }

    return StatusCode::SUCCESS;
  }

}
