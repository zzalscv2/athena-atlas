/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRKSEEDING_SEEDINGALG_H
#define ACTSTRKSEEDING_SEEDINGALG_H
  
// Base Class
#include "AthenaBaseComps/AthReentrantAlgorithm.h"

// Gaudi includes
#include "GaudiKernel/ToolHandle.h"

// Tools
#include "ActsToolInterfaces/ISeedingTool.h"
#include "ActsToolInterfaces/ITrackParamsEstimationTool.h"
#include "ActsGeometryInterfaces/IActsTrackingGeometryTool.h"
#include "ActsEventCnv/IActsToTrkConverterTool.h"

// Athena
#include "BeamSpotConditionsData/BeamSpotData.h"
#include "MagFieldConditions/AtlasFieldCacheCondObj.h"
#include "MagFieldElements/AtlasFieldCache.h"
#include "TrkSpacePoint/SpacePointContainer.h"
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "AthenaMonitoringKernel/GenericMonitoringTool.h"

// Handle Keys
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadHandleKeyArray.h"
#include "StoreGate/WriteHandleKey.h"

#include "ActsEvent/TrackParameters.h"

namespace ActsTrk {

  class SeedingAlg :
    public AthReentrantAlgorithm {
    
  public:
    SeedingAlg(const std::string &name,
	       ISvcLocator *pSvcLocator);
    virtual ~SeedingAlg() = default;

    
    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;

  private:
    // Tool Handles
    ToolHandle< ActsTrk::ISeedingTool > m_seedsTool {this, "SeedTool", "","Seed Tool"};
    ToolHandle< ActsTrk::ITrackParamsEstimationTool > m_paramEstimationTool {this, "TrackParamsEstimationTool", "", "Track Param Estimation from Seeds"};
    PublicToolHandle< IActsTrackingGeometryTool > m_trackingGeometryTool {this, "TrackingGeometryTool", "ActsTrackingGeometryTool"};
    ToolHandle< ActsTrk::IActsToTrkConverterTool > m_ATLASConverterTool{this, "ATLASConverterTool", "ActsToTrkConverterTool"};
    ToolHandle< GenericMonitoringTool > m_monTool {this, "MonTool", "", "Monitoring tool"};

    // Handle Keys
    SG::ReadCondHandleKey< InDet::BeamSpotData > m_beamSpotKey{this, "BeamSpotKey", "BeamSpotData", "SG key for beam spot"};
    SG::ReadCondHandleKey< AtlasFieldCacheCondObj > m_fieldCondObjInputKey {this, "AtlasFieldCacheCondObj", "fieldCondObj",
        "Name of the Magnetic Field conditions object key"};
    SG::ReadCondHandleKey< InDetDD::SiDetectorElementCollection > m_detEleCollKey {this, "DetectorElements", "", "Key of input SiDetectorElementCollection"};

    SG::ReadHandleKeyArray< xAOD::SpacePointContainer > m_spacePointKey {this,"InputSpacePoints",{},"Input Space Points"};
    SG::WriteHandleKey< ActsTrk::SeedContainer > m_seedKey {this,"OutputSeeds","","Output Seeds"};    
    SG::WriteHandleKey< ActsTrk::BoundTrackParametersContainer > m_actsTrackParamsKey {this, "OutputEstimatedTrackParameters", "", ""};

    Gaudi::Property< bool > m_usePixel {this, "UsePixel", true};
  };
  
} // namespace

#endif
