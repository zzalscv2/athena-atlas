/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "src/PhysValTool.h"

namespace ActsTrk {

  PhysValTool::PhysValTool(const std::string & type,
			   const std::string& name,
			   const IInterface* parent)
    : ManagedMonitorToolBase(type, name, parent)
  {}
 
  StatusCode PhysValTool::initialize() 
  {
    ATH_MSG_DEBUG("Initializing " << name() << " ... ");
    
    ATH_CHECK(ManagedMonitorToolBase::initialize());
    ATH_CHECK(m_eventInfo.initialize());

    ATH_CHECK(m_pixelClusterContainerKey.initialize());
    ATH_CHECK(m_stripClusterContainerKey.initialize());

    ATH_CHECK(m_pixelSpacePointContainerKey.initialize());
    ATH_CHECK(m_stripSpacePointContainerKey.initialize());
    ATH_CHECK(m_stripOverlapSpacePointContainerKey.initialize());

    std::string folder = "SquirrelPlots/Acts"; 
    m_pixelClusterValidationPlots = 
      std::make_unique< ActsTrk::PixelClusterValidationPlots >(nullptr, 
							       Form("%s/%s/", 
								    folder.c_str(),
								    m_pixelClusterContainerKey.key().c_str()));
    m_stripClusterValidationPlots = 
      std::make_unique< ActsTrk::StripClusterValidationPlots >(nullptr, 
							       Form("%s/%s/", 
								    folder.c_str(),
								    m_stripClusterContainerKey.key().c_str()));

    m_pixelSpacePointValidationPlots =
      std::make_unique< ActsTrk::PixelSpacePointValidationPlots >(nullptr,
								  Form("%s/%s/",
								       folder.c_str(),
								       m_pixelSpacePointContainerKey.key().c_str()));
    m_stripSpacePointValidationPlots =
      std::make_unique< ActsTrk::StripSpacePointValidationPlots >(nullptr,
								  Form("%s/%s/",
								       folder.c_str(),
								       m_stripSpacePointContainerKey.key().c_str()),
								  "Strip");
    m_stripOverlapSpacePointValidationPlots = 
      std::make_unique< ActsTrk::StripSpacePointValidationPlots >(nullptr,
								  Form("%s/%s/",
								       folder.c_str(),
								       m_stripOverlapSpacePointContainerKey.key().c_str()),
								  "StripOverlap");
    
    ATH_CHECK(detStore()->retrieve(m_pixelID, "PixelID"));
    ATH_CHECK(detStore()->retrieve(m_stripID, "SCT_ID"));

    return StatusCode::SUCCESS;
  }

  StatusCode PhysValTool::bookHistograms() 
  {
    ATH_MSG_DEBUG("Booking histograms for " << name() << " ... " );

    ATH_CHECK(bookCollection(m_pixelClusterValidationPlots.get()));
    ATH_CHECK(bookCollection(m_stripClusterValidationPlots.get()));

    ATH_CHECK(bookCollection(m_pixelSpacePointValidationPlots.get()));
    ATH_CHECK(bookCollection(m_stripSpacePointValidationPlots.get()));
    ATH_CHECK(bookCollection(m_stripOverlapSpacePointValidationPlots.get()));

    return StatusCode::SUCCESS;
  }
  
  StatusCode PhysValTool::fillHistograms() 
  {
    ATH_MSG_DEBUG("Filling histograms for " << name() << " ... ");
    
    const EventContext& ctx = Gaudi::Hive::currentContext();

    // Get Event Info
    SG::ReadHandle<xAOD::EventInfo> eventInfoHandle = SG::makeHandle(m_eventInfo, ctx);
    if (not eventInfoHandle.isValid()) {
      ATH_MSG_FATAL("Could not retrieve EventInfo with key " << m_eventInfo.key());
      return StatusCode::FAILURE;
    }
    const xAOD::EventInfo* eventInfo = eventInfoHandle.cptr();
    float beamSpotWeight = eventInfo->beamSpotWeight();

    // Get Input Collections :: CLUSTERS
    SG::ReadHandle< xAOD::PixelClusterContainer > inputPixelClusterContainer = SG::makeHandle( m_pixelClusterContainerKey, ctx );
    if (not inputPixelClusterContainer.isValid()) {
      ATH_MSG_FATAL("xAOD::PixelClusterContainer with key " << m_pixelClusterContainerKey.key() << " is not available...");
      return StatusCode::FAILURE;
    }
    const xAOD::PixelClusterContainer *pixelClusterContainer = inputPixelClusterContainer.cptr();


    SG::ReadHandle< xAOD::StripClusterContainer > inputStripClusterContainer = SG::makeHandle( m_stripClusterContainerKey, ctx );
    if (not inputStripClusterContainer.isValid()) {
      ATH_MSG_FATAL("xAOD::StripClusterContainer with key " << m_stripClusterContainerKey.key() << " is not available...");
      return StatusCode::FAILURE;
    }
    const xAOD::StripClusterContainer *stripClusterContainer = inputStripClusterContainer.cptr();    

    // Get Input Collections :: SPACE POINTS
    SG::ReadHandle< xAOD::SpacePointContainer > inputPixelSpacePointContainer = SG::makeHandle( m_pixelSpacePointContainerKey, ctx );
    if (not inputPixelSpacePointContainer.isValid()) {
      ATH_MSG_FATAL("xAOD::SpacePointContainer with key " << m_pixelSpacePointContainerKey.key() << " is not available...");
      return StatusCode::FAILURE;
    } 
    const xAOD::SpacePointContainer *pixelSpacePointContainer = inputPixelSpacePointContainer.cptr();

    SG::ReadHandle< xAOD::SpacePointContainer > inputStripSpacePointContainer = SG::makeHandle( m_stripSpacePointContainerKey, ctx );
    if (not inputStripSpacePointContainer.isValid()) {
      ATH_MSG_FATAL("xAOD::SpacePointContainer with key " << m_stripSpacePointContainerKey.key() << " is not available...");
      return StatusCode::FAILURE;
    }
    const xAOD::SpacePointContainer *stripSpacePointContainer = inputStripSpacePointContainer.cptr();

    SG::ReadHandle< xAOD::SpacePointContainer > inputStripOverlapSpacePointContainer = SG::makeHandle( m_stripOverlapSpacePointContainerKey, ctx );
    if (not inputStripOverlapSpacePointContainer.isValid()) {
      ATH_MSG_FATAL("xAOD::SpacePointContainer with key " << m_stripOverlapSpacePointContainerKey.key() << " is not available...");
      return StatusCode::FAILURE;
    }
    const xAOD::SpacePointContainer *stripOverlapSpacePointContainer = inputStripOverlapSpacePointContainer.cptr();

    // Fill histograms
    for (const xAOD::PixelCluster* cluster : *pixelClusterContainer) {
      m_pixelClusterValidationPlots->fill(cluster, beamSpotWeight, m_pixelID);
    }

    for (const xAOD::StripCluster* cluster : *stripClusterContainer) {
      m_stripClusterValidationPlots->fill(cluster, beamSpotWeight, m_stripID);
    }

    for (const xAOD::SpacePoint* spacePoint : *pixelSpacePointContainer) {
      m_pixelSpacePointValidationPlots->fill(spacePoint, beamSpotWeight, m_pixelID);
    }

    for (const xAOD::SpacePoint* spacePoint : *stripSpacePointContainer) {
      m_stripSpacePointValidationPlots->fill(spacePoint, beamSpotWeight, m_stripID);
    }

    for(const xAOD::SpacePoint* spacePoint : *stripOverlapSpacePointContainer) {
      m_stripOverlapSpacePointValidationPlots->fill(spacePoint, beamSpotWeight, m_stripID);
    }

    return StatusCode::SUCCESS;
  }
  
  StatusCode PhysValTool::procHistograms() 
  {
    ATH_MSG_DEBUG("Finalising hists for " << name() << "...");
    m_pixelClusterValidationPlots->finalize();
    m_stripClusterValidationPlots->finalize();
    return StatusCode::SUCCESS;
  }

}
