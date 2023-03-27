/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "TrkSurfaces/Surface.h"
#include "InDetIdentifier/PixelID.h"
#include "PixelReadoutGeometry/PixelModuleDesign.h"

#include "PixelSpacePointFormationTool.h"

namespace ActsTrk {

    PixelSpacePointFormationTool::PixelSpacePointFormationTool(const std::string& type,
                                                               const std::string& name,
                                                               const IInterface* parent)
    : base_class(type, name, parent)
    {}

    StatusCode PixelSpacePointFormationTool::initialize() {

        ATH_CHECK(detStore()->retrieve(m_pixelId,"PixelID"));

        return StatusCode::SUCCESS;
    }

    StatusCode
    PixelSpacePointFormationTool::producePixelSpacePoint(const xAOD::PixelCluster& cluster,
                                                         xAOD::SpacePoint& sp,
                                                         const std::vector<std::size_t>& measIndexes,
                                                         const InDetDD::SiDetectorElement& element) const
    {
        // evaluate the cluster width for space point covariance evaluation
        const InDetDD::PixelModuleDesign* design
          (dynamic_cast<const InDetDD::PixelModuleDesign*>(&element.design()));
        if (not design){
            ATH_MSG_ERROR("Cast to InDetDD::PixelModuleDesign failed.");
            return StatusCode::FAILURE;
        }

        // Implementing space point covariance calculation as defined in
        // InnerDetector/InDetRecEvent/SiSPSeededTrackFinderData/src/ITkSiSpacePointForSeed.cxx
        // SiSpacePointForSeed::set(const Trk::SpacePoint*const& sp,const float* r)

        auto rdoList = cluster.rdoList();
        auto min_max =
          std::minmax_element(rdoList.begin(), rdoList.end(),
                              [this] (Identifier const& lh, Identifier const& rh) {
                                  return m_pixelId->eta_index(lh) < m_pixelId->eta_index(rh);
                              });

        // this is the width expressed in mm
        float width =
          design->widthFromColumnRange(m_pixelId->eta_index(*min_max.first),
                                       m_pixelId->eta_index(*min_max.second));

        // using width to scale the cluster covariance for space points
        float covTerm = width*width*s_oneOverTwelve;
        auto localCov = cluster.localCovariance<2>();
        if( covTerm < localCov(1, 1) )
            covTerm = localCov(1, 1);

        // use xz, yz, zz terms of rotation matrix to scale the covariance term
        const Amg::Transform3D &Tp = element.surface().transform();
        float cov_z = 6.*covTerm*static_cast<float>(Tp(0, 2)*Tp(0, 2)+Tp(1, 2)*Tp(1, 2));
        float cov_r = 6.*covTerm*static_cast<float>(Tp(2, 2)*Tp(2, 2));

	sp.setSpacePoint(cluster.identifierHash(),
			 cluster.globalPosition(),
			 cov_r, 
			 cov_z,
			 measIndexes);
	
        return StatusCode::SUCCESS;
    }
}
