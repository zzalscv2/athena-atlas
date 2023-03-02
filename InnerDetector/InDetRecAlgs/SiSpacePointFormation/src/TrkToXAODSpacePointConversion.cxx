/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration   
*/

#include "SiSpacePointFormation/TrkToXAODSpacePointConversion.h"
#include "InDetPrepRawData/SiCluster.h"

namespace InDet {

  TrkToXAODSpacePointConversion::TrkToXAODSpacePointConversion(const std::string &name, 
							       ISvcLocator *pSvcLocator)
    : AthReentrantAlgorithm(name, pSvcLocator)
  {}

  StatusCode TrkToXAODSpacePointConversion::initialize()
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
  
  StatusCode TrkToXAODSpacePointConversion::execute(const EventContext& ctx) const
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

  StatusCode TrkToXAODSpacePointConversion::convertPixel(const EventContext& ctx) const
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
        unsigned int idHash = sp->elementIdList().first;
        const auto& globPos = sp->globalPosition();

        const InDet::SiCluster* c  = static_cast<const InDet::SiCluster*>(sp->clusterList().first);
        const InDetDD::SiDetectorElement *de = c->detectorElement();
        const Amg::Transform3D &Tp = de->surface().transform();

        float r_3 = static_cast<float>( Tp(0,2) );
        float r_4 = static_cast<float>( Tp(1,2) );
        float r_5 = static_cast<float>( Tp(2,2) );

        const Amg::MatrixX& v = c->localCovariance();
        float f22 = static_cast<float>( v(1,1) );
        float wid = static_cast<float>( c->width().z() );
        float cov = wid*wid*.08333;
        if(cov < f22) cov = f22;
        float covr = 6 * cov * (r_5*r_5);
        float covz = 6 * cov * (r_3*r_3 + r_4*r_4);

        pixel_xaod_container->push_back( new xAOD::SpacePoint() );
        pixel_xaod_container->back()->setSpacePoint(idHash,
                                                    globPos.cast<float>(),
                                                    covr,
                                                    covz,
                                                    {counter++});
      }
    }

    // Store
    SG::WriteHandle< xAOD::SpacePointContainer > pixel_xaod_handle = SG::makeHandle( m_outSpacepointsPixel, ctx );
    ATH_CHECK( pixel_xaod_handle.record( std::move(pixel_xaod_container), std::move(pixel_xaod_aux_container) ) );

    return StatusCode::SUCCESS;
  }

  StatusCode TrkToXAODSpacePointConversion::convertStrip(const EventContext& ctx,
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
	std::pair<unsigned int, unsigned int> idHashes = sp->elementIdList();
        const auto& globPos = sp->globalPosition();

	const InDet::SiCluster *c0 = static_cast<const InDet::SiCluster *>(sp->clusterList().first);
	const InDet::SiCluster *c1 = static_cast<const InDet::SiCluster *>(sp->clusterList().second);
	const InDetDD::SiDetectorElement *d0 = c0->detectorElement();
	const InDetDD::SiDetectorElement *d1 = c1->detectorElement();

	Amg::Vector2D lc0 = c0->localPosition();
	Amg::Vector2D lc1 = c1->localPosition();

	std::pair<Amg::Vector3D, Amg::Vector3D> e0 =
	  (d0->endsOfStrip(InDetDD::SiLocalPosition(lc0.y(), lc0.x(), 0.)));
	std::pair<Amg::Vector3D, Amg::Vector3D> e1 =
	  (d1->endsOfStrip(InDetDD::SiLocalPosition(lc1.y(), lc1.x(), 0.)));

	auto stripCenter_1 = 0.5 * (e0.first + e0.second);
        auto stripDir_1 = e0.first - e0.second;
        auto trajDir_1 = 2. * ( stripCenter_1 - vertex);

	auto stripCenter_2 = 0.5 * (e1.first + e1.second);
	auto stripDir_2 = e1.first - e1.second;

	float topHalfStripLength = 0.5 * stripDir_1.norm();
	Eigen::Matrix<double, 3, 1> topStripDirection = - stripDir_1 / (2. * topHalfStripLength);
	Eigen::Matrix<double, 3, 1> topStripCenter = 0.5 * trajDir_1;
        float bottomHalfStripLength = 0.5 * stripDir_2.norm();
	Eigen::Matrix<double, 3, 1> bottomStripDirection = - stripDir_2 / (2. * bottomHalfStripLength);
	Eigen::Matrix<double, 3, 1> stripCenterDistance = stripCenter_1 - stripCenter_2;

        const Amg::MatrixX& v = sp->localCovariance();
        float f22 = static_cast<float>( v(1,1) );

        float covr = d0->isBarrel() ? .1 : 8.*f22;
        float covz = d0->isBarrel() ? 8.*f22 : .1;

        strip_xaod_container->push_back( new xAOD::SpacePoint() );
        strip_xaod_container->back()->setSpacePoint({idHashes.first, idHashes.second},
                                                    globPos.cast<float>(),
                                                    covr,
                                                    covz,
                                                    {counter, counter++},
                                                    topHalfStripLength,
						    bottomHalfStripLength,
						    topStripDirection.cast<float>(),
						    bottomStripDirection.cast<float>(),
						    stripCenterDistance.cast<float>(),
						    topStripCenter.cast<float>());
      }
    }

    // Store
    SG::WriteHandle< xAOD::SpacePointContainer > strip_xaod_handle = SG::makeHandle( m_outSpacepointsStrip, ctx );
    ATH_CHECK( strip_xaod_handle.record( std::move(strip_xaod_container), std::move(strip_xaod_aux_container) ) );

    return StatusCode::SUCCESS;
  }


  StatusCode TrkToXAODSpacePointConversion::convertStripOverlap(const EventContext& ctx,
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
    std::size_t counter = 0;
    for (const Trk::SpacePoint *sp : *strip_overlap_container) {
      std::pair<unsigned int, unsigned int> idHashes = sp->elementIdList();
      const auto& globPos = sp->globalPosition();
      
      const InDet::SiCluster *c0 = static_cast<const InDet::SiCluster *>(sp->clusterList().first);
      const InDet::SiCluster *c1 = static_cast<const InDet::SiCluster *>(sp->clusterList().second);
      const InDetDD::SiDetectorElement *d0 = c0->detectorElement();
      const InDetDD::SiDetectorElement *d1 = c1->detectorElement();
      
      Amg::Vector2D lc0 = c0->localPosition();
      Amg::Vector2D lc1 = c1->localPosition();
      
      std::pair<Amg::Vector3D, Amg::Vector3D> e0 =
	(d0->endsOfStrip(InDetDD::SiLocalPosition(lc0.y(), lc0.x(), 0.)));
      std::pair<Amg::Vector3D, Amg::Vector3D> e1 =
	(d1->endsOfStrip(InDetDD::SiLocalPosition(lc1.y(), lc1.x(), 0.)));
      
      auto stripCenter_1 = 0.5 * (e0.first + e0.second);
      auto stripDir_1 = e0.first - e0.second;
      auto trajDir_1 = 2. * ( stripCenter_1 - vertex);
      
      auto stripCenter_2 = 0.5 * (e1.first + e1.second);
      auto stripDir_2 = e1.first - e1.second;
      
      float topHalfStripLength = 0.5 * stripDir_1.norm();
      Eigen::Matrix<double, 3, 1> topStripDirection = - stripDir_1 / (2. * topHalfStripLength);
      Eigen::Matrix<double, 3, 1> topStripCenter = 0.5 * trajDir_1;
      float bottomHalfStripLength = 0.5 * stripDir_2.norm();
      Eigen::Matrix<double, 3, 1> bottomStripDirection = - stripDir_2 / (2. * bottomHalfStripLength);
      Eigen::Matrix<double, 3, 1> stripCenterDistance = stripCenter_1 - stripCenter_2;
      
      const Amg::MatrixX& v = sp->localCovariance();
      float f22 = static_cast<float>( v(1,1) );
      
      float covr = d0->isBarrel() ? .1 : 8.*f22;
      float covz = d0->isBarrel() ? 8.*f22 : .1;
      
      strip_overlap_xaod_container->push_back( new xAOD::SpacePoint() );
      strip_overlap_xaod_container->back()->setSpacePoint({idHashes.first, idHashes.second},
							  globPos.cast<float>(),
							  covr,
							  covz,
							  {counter, counter++},
							  topHalfStripLength,
							  bottomHalfStripLength,
							  topStripDirection.cast<float>(),
							  bottomStripDirection.cast<float>(),
							  stripCenterDistance.cast<float>(),
							  topStripCenter.cast<float>());
    }

    // Store
    SG::WriteHandle< xAOD::SpacePointContainer > strip_overlap_xaod_handle = SG::makeHandle( m_outSpacepointsOverlap, ctx );
    ATH_CHECK( strip_overlap_xaod_handle.record( std::move(strip_overlap_xaod_container), std::move(strip_overlap_xaod_aux_container) ) );

    return StatusCode::SUCCESS;
  }
}
