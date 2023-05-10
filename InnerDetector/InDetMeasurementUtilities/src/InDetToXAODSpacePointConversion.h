/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTOXAOD_SPACEPOINT_CONVERSION_H
#define INDETTOXAOD_SPACEPOINT_CONVERSION_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "xAODInDetMeasurement/SpacePointContainer.h"
#include "xAODInDetMeasurement/SpacePointAuxContainer.h"

#include "TrkSpacePoint/SpacePointContainer.h" 
#include "TrkSpacePoint/SpacePointOverlapCollection.h"

#include "BeamSpotConditionsData/BeamSpotData.h"

namespace InDet {

  class InDetToXAODSpacePointConversion :
    public AthReentrantAlgorithm {
  public:
    /// Constructor with parameters:
    InDetToXAODSpacePointConversion(const std::string &name, ISvcLocator *pSvcLocator);
    
    //@name Usual algorithm methods
    //@{
    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;
    //@{

  private:
    StatusCode convertPixel(const EventContext& ctx) const;
    StatusCode convertStrip(const EventContext& ctx, 
			    const Amg::Vector3D& vertex) const;
    StatusCode convertStripOverlap(const EventContext& ctx, 
				   const Amg::Vector3D& vertex) const;

  private:
    SG::ReadCondHandleKey<InDet::BeamSpotData> m_beamSpotKey { this, "BeamSpotKey", "BeamSpotData", 
	"SG key for beam spot" };

    SG::ReadHandleKey< ::SpacePointContainer > m_inSpacepointsPixel {this, "InputPixelSpacePointsName", "ITkPixelSpacePoints", 
	"Input Pixel space points container"};
    SG::ReadHandleKey< ::SpacePointContainer > m_inSpacepointsStrip {this, "InputStripSpacePointsName", "ITkStripSpacePoints", 
	"Input Strip space points container"};
    SG::ReadHandleKey< ::SpacePointOverlapCollection > m_inSpacepointsOverlap {this, "InputStripOverlapSpacePointsName", "ITkOverlapSpacePoints",
	"Input Strip overlap space points container"};

    SG::WriteHandleKey< xAOD::SpacePointContainer > m_outSpacepointsPixel {this, "OutputPixelSpacePointsName", "ITkPixelSpacePoints",
	"Output Pixel space points container"};
    SG::WriteHandleKey< xAOD::SpacePointContainer > m_outSpacepointsStrip {this, "OutputStripSpacePointsName", "ITkStripSpacePoints",
	"Output Strip space points container"};
    SG::WriteHandleKey< xAOD::SpacePointContainer > m_outSpacepointsOverlap {this, "OutputStripOverlapSpacePointsName", "ITkStripOverlapSpacePoints",
	"Output Strip Overlap space points container"};
  };

}

#endif

