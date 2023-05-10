/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration   
*/

#include "src/InDetToXAODSpacePointConversion.h"
#include "InDetMeasurementUtilities/SpacePointConversionUtilities.h"

#include "InDetPrepRawData/SiCluster.h"

namespace InDet {

  InDetToXAODSpacePointConversion::InDetToXAODSpacePointConversion(const std::string &name, 
								   ISvcLocator *pSvcLocator)
    : AthReentrantAlgorithm(name, pSvcLocator)
  {}
  
  StatusCode InDetToXAODSpacePointConversion::initialize()
  { 
    ATH_MSG_DEBUG("Initializing " << name() << " ...");

    ATH_CHECK( m_beamSpotKey.initialize() );

    ATH_CHECK( m_inSpacepointsPixel.initialize() );
    ATH_CHECK( m_inSpacepointsStrip.initialize() );
    ATH_CHECK( m_inSpacepointsOverlap.initialize() );

    ATH_CHECK( m_outSpacepointsPixel.initialize() );
    ATH_CHECK( m_outSpacepointsStrip.initialize() );
    ATH_CHECK( m_outSpacepointsOverlap.initialize() );
    
    return StatusCode::SUCCESS; 
  }
  
  StatusCode InDetToXAODSpacePointConversion::execute(const EventContext& ctx) const
  { 
    ATH_MSG_DEBUG("Executing " << name() << " ...");
    
    // Conds
    SG::ReadCondHandle<InDet::BeamSpotData> beamSpotHandle { m_beamSpotKey, ctx };
    const InDet::BeamSpotData* beamSpot = *beamSpotHandle;
    auto vertex = beamSpot->beamVtx().position();

    // Convert
    ATH_CHECK( convertPixel(ctx) );
    ATH_CHECK( convertStrip(ctx, vertex) );
    ATH_CHECK( convertStripOverlap(ctx, vertex) );

    return StatusCode::SUCCESS; 
  }

  StatusCode InDetToXAODSpacePointConversion::convertPixel(const EventContext& ctx) const
  {
    // Input
    SG::ReadHandle< ::SpacePointContainer > pixel_handle = SG::makeHandle( m_inSpacepointsPixel, ctx );
    ATH_CHECK( pixel_handle.isValid() );
    const ::SpacePointContainer* pixel_container = pixel_handle.cptr();

    // Output
    std::unique_ptr< xAOD::SpacePointContainer > pixel_xaod_container = std::make_unique< xAOD::SpacePointContainer >();
    std::unique_ptr< xAOD::SpacePointAuxContainer > pixel_xaod_aux_container = std::make_unique< xAOD::SpacePointAuxContainer >();
    pixel_xaod_container->setStore( pixel_xaod_aux_container.get() );

    pixel_xaod_container->reserve(pixel_container->size());
    pixel_xaod_aux_container->reserve(pixel_container->size());

    // Conversion
    std::size_t counter = 0;
    for (const ::SpacePointCollection *spc : *pixel_container) {
      for (const Trk::SpacePoint *sp : *spc) {
	const InDet::PixelSpacePoint *indetSP = dynamic_cast<const InDet::PixelSpacePoint *>(sp);
	
	pixel_xaod_container->push_back( new xAOD::SpacePoint() );
	ATH_CHECK( TrackingUtilities::convertTrkToXaodPixelSpacePoint(*indetSP, *pixel_xaod_container->back()) );
	pixel_xaod_container->back()->setMeasurementIndexes({counter++});
      }
    }

    // Store
    SG::WriteHandle< xAOD::SpacePointContainer > pixel_xaod_handle = SG::makeHandle( m_outSpacepointsPixel, ctx );
    ATH_CHECK( pixel_xaod_handle.record( std::move(pixel_xaod_container), std::move(pixel_xaod_aux_container) ) );

    return StatusCode::SUCCESS;
  }

  StatusCode InDetToXAODSpacePointConversion::convertStrip(const EventContext& ctx,
							   const Amg::Vector3D& vertex) const
  {
    // Input
    SG::ReadHandle< ::SpacePointContainer > strip_handle = SG::makeHandle( m_inSpacepointsStrip, ctx );
    ATH_CHECK( strip_handle.isValid() );
    const ::SpacePointContainer* strip_container = strip_handle.cptr();
    
    // Output
    std::unique_ptr< xAOD::SpacePointContainer > strip_xaod_container = std::make_unique< xAOD::SpacePointContainer >();
    std::unique_ptr< xAOD::SpacePointAuxContainer > strip_xaod_aux_container = std::make_unique< xAOD::SpacePointAuxContainer >();
    strip_xaod_container->setStore( strip_xaod_aux_container.get() );
    
    strip_xaod_container->reserve(strip_container->size());
    strip_xaod_aux_container->reserve(strip_container->size());
    
    // Conversion
    std::size_t counter = 0;
    for (const ::SpacePointCollection *spc : *strip_container) {
      for (const Trk::SpacePoint *sp : *spc) {
	const InDet::SCT_SpacePoint *indetSP = dynamic_cast<const InDet::SCT_SpacePoint *>(sp);

	strip_xaod_container->push_back( new xAOD::SpacePoint() );	
	ATH_CHECK( TrackingUtilities::convertTrkToXaodStripSpacePoint(*indetSP, vertex, *strip_xaod_container->back()) );
	strip_xaod_container->back()->setMeasurementIndexes({counter, counter++}); 
      }
    }

    // Store
    SG::WriteHandle< xAOD::SpacePointContainer > strip_xaod_handle = SG::makeHandle( m_outSpacepointsStrip, ctx );
    ATH_CHECK( strip_xaod_handle.record( std::move(strip_xaod_container), std::move(strip_xaod_aux_container) ) );

    return StatusCode::SUCCESS;
  }


  StatusCode InDetToXAODSpacePointConversion::convertStripOverlap(const EventContext& ctx,
								  const Amg::Vector3D& vertex) const
  {
    // Inputs
    SG::ReadHandle< ::SpacePointOverlapCollection > strip_overlap_handle = SG::makeHandle( m_inSpacepointsOverlap, ctx );
    ATH_CHECK( strip_overlap_handle.isValid() );
    const ::SpacePointOverlapCollection* strip_overlap_container = strip_overlap_handle.cptr();

    // Outputs
    std::unique_ptr< xAOD::SpacePointContainer > strip_overlap_xaod_container = std::make_unique< xAOD::SpacePointContainer >();
    std::unique_ptr< xAOD::SpacePointAuxContainer > strip_overlap_xaod_aux_container = std::make_unique< xAOD::SpacePointAuxContainer >();
    strip_overlap_xaod_container->setStore( strip_overlap_xaod_aux_container.get() );

    strip_overlap_xaod_container->reserve(strip_overlap_container->size());
    strip_overlap_xaod_aux_container->reserve(strip_overlap_container->size());

    // Conversion
    static const SG::AuxElement::Accessor< ElementLink< ::SpacePointOverlapCollection > > stripSpacePointLinkAcc("stripOverlapSpacePointLink");
    std::size_t counter = 0;
    for (const Trk::SpacePoint *sp : *strip_overlap_container) {
      const InDet::SCT_SpacePoint *indetSP = dynamic_cast<const InDet::SCT_SpacePoint *>(sp);
      
      strip_overlap_xaod_container->push_back( new xAOD::SpacePoint() );
      ATH_CHECK( TrackingUtilities::convertTrkToXaodStripSpacePoint(*indetSP, vertex, *strip_overlap_xaod_container->back()) );
      strip_overlap_xaod_container->back()->setMeasurementIndexes({counter, counter++});

      ElementLink< ::SpacePointOverlapCollection > TrkLink(sp, *strip_overlap_container);
      stripSpacePointLinkAcc( *strip_overlap_xaod_container->back() ) = TrkLink;
    }

    // Store
    SG::WriteHandle< xAOD::SpacePointContainer > strip_overlap_xaod_handle = SG::makeHandle( m_outSpacepointsOverlap, ctx );
    ATH_CHECK( strip_overlap_xaod_handle.record( std::move(strip_overlap_xaod_container), std::move(strip_overlap_xaod_aux_container) ) );

    return StatusCode::SUCCESS;
  }

}
