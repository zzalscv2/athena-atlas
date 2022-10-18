/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRKSEEDING_SEEDINGFROMATHENAALG_H
#define ACTSTRKSEEDING_SEEDINGFROMATHENAALG_H 1
  
// Base Class
#include "AthenaBaseComps/AthReentrantAlgorithm.h"

// Gaudi includes
#include "GaudiKernel/ToolHandle.h"

// Tools
#include "ActsTrkToolInterfaces/ISeedingTool.h"
#include "ActsTrkToolInterfaces/ITrackParamsEstimationTool.h"
#include "ActsGeometryInterfaces/IActsTrackingGeometryTool.h"
#include "ActsGeometryInterfaces/IActsATLASConverterTool.h"

// Athena
#include "BeamSpotConditionsData/BeamSpotData.h"
#include "MagFieldConditions/AtlasFieldCacheCondObj.h"
#include "MagFieldElements/AtlasFieldCache.h"
#include "TrkSpacePoint/SpacePointContainer.h"
#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "xAODInDetMeasurement/StripClusterContainer.h"
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "AthenaMonitoringKernel/GenericMonitoringTool.h"

// Handle Keys
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadHandleKeyArray.h"
#include "StoreGate/WriteHandleKey.h"

#include "ActsTrkEvent/TrackParameters.h"

namespace ActsTrk {

  class SeedingFromAthenaAlg :
    public AthReentrantAlgorithm {
    
  public:
    SeedingFromAthenaAlg(const std::string &name,
				   ISvcLocator *pSvcLocator);
    virtual ~SeedingFromAthenaAlg() = default;

    
    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;

  private:
    // Tool Handles
    ToolHandle< ActsTrk::ISeedingTool > m_seedsTool {this, "SeedTool", "","Seed Tool"};
    ToolHandle< ActsTrk::ITrackParamsEstimationTool > m_paramEstimationTool {this, "TrackParamsEstimationTool", "", "Track Param Estimation from Seeds"};
    PublicToolHandle< IActsTrackingGeometryTool > m_trackingGeometryTool {this, "TrackingGeometryTool", "ActsTrackingGeometryTool"};
    ToolHandle< IActsATLASConverterTool > m_ATLASConverterTool{this, "ATLASConverterTool", "ActsATLASConverterTool"};
    ToolHandle< GenericMonitoringTool > m_monTool {this, "MonTool", "", "Monitoring tool"};

    // Handle Keys
    SG::ReadCondHandleKey< InDet::BeamSpotData > m_beamSpotKey{this, "BeamSpotKey", "BeamSpotData", "SG key for beam spot"};
    SG::ReadCondHandleKey< AtlasFieldCacheCondObj > m_fieldCondObjInputKey {this, "AtlasFieldCacheCondObj", "fieldCondObj",
        "Name of the Magnetic Field conditions object key"};
    SG::ReadCondHandleKey< InDetDD::SiDetectorElementCollection > m_detEleCollKey {this, "DetectorElements", "", "Key of input SiDetectorElementCollection"};

    SG::ReadHandleKey< xAOD::PixelClusterContainer > m_pixelClusterContainerKey {this, "PixelClusterContainerKey", "ITkPixelClusters", "Key of input pixel clusters"};
    SG::ReadHandleKey< xAOD::StripClusterContainer > m_stripClusterContainerKey {this, "StripClusterContainerKey", "ITkStripClusters", "Key of input strip clusters"};
    SG::ReadHandleKeyArray< ActsTrk::SpacePointContainer > m_spacePointKey {this,"InputSpacePoints",{},"Input Space Points"};
    SG::WriteHandleKey< ActsTrk::SeedContainer > m_seedKey {this,"OutputSeeds","","Output Seeds"};    
    SG::WriteHandleKey< ActsTrk::BoundTrackParametersContainer > m_actsTrackParamsKey {this, "OutputEstimatedTrackParameters", "", ""};

    Gaudi::Property< bool > m_usePixel {this, "UsePixel", true};
  };
  
} // namespace

#endif
