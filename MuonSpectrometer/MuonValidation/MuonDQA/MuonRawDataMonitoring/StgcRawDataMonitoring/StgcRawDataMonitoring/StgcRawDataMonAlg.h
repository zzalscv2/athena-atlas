/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Package : sTgcRawDataMonAlg
// Author: Sebastian Fuenzalida Garrido
// Local supervisor: Edson Carquin Lopez
// Technical supervisor: Gerardo Vasquez
//
// DESCRIPTION:
// Subject: sTgc --> sTgc raw data monitoring
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef sTgcRawDataMonAlg_H
#define sTgcRawDataMonAlg_H

// Core Include
#include "AthenaMonitoring/AthMonitorAlgorithm.h"

// Helper Includes
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonPrepRawData/sTgcPrepDataContainer.h"
#include "StoreGate/ReadHandleKey.h"
#include "MuonPrepRawData/sTgcPrepData.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonIdHelpers/sTgcIdHelper.h"
#include "MuonRIO_OnTrack/sTgcClusterOnTrack.h"

#include "TrkToolInterfaces/IResidualPullCalculator.h"
#include "TrkParameters/TrackParameters.h"

#include "MuonRDO/NSW_PadTriggerDataContainer.h"

// stl includes                                                                                 
#include <string>

namespace Muon {
  class sTgcPrepData;
}

namespace GeometricSectors {
  static const std::array<std::string, 2> sTgcSide = {"C", "A"};
  static const std::array<std::string, 2> sTgcSize = {"S", "L"};
}

class sTgcRawDataMonAlg: public AthMonitorAlgorithm {
 public:
  sTgcRawDataMonAlg(const std::string& name, ISvcLocator* pSvcLocator);
  
  virtual ~sTgcRawDataMonAlg()=default;
  virtual StatusCode initialize() override;
  virtual StatusCode fillHistograms(const EventContext& ctx) const override;
 private:  
  ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc {this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
  ToolHandle<Trk::IResidualPullCalculator> m_residualPullCalculator {this, "ResPullCalc", "Trk::ResidualPullCalculator/ResidualPullCalculator"};
  
  void fillsTgcOccupancyHistograms(const Muon::sTgcPrepData*, const MuonGM::MuonDetectorManager*) const;
  void fillsTgcLumiblockHistograms(const Muon::sTgcPrepData*, const int lb) const;
  void fillsTgcClusterFromTrackHistograms(const xAOD::TrackParticleContainer*) const;  
  void fillsTgcPadTriggerDataHistograms(const Muon::NSW_PadTriggerDataContainer*, const int lb) const;

  int getSectors(const Identifier& id) const;
  int getLayer(const int multiplet, const int gasGap) const;
  int32_t sourceidToSector(uint32_t sourceid, bool isSideA) const;
  
  SG::ReadHandleKey<Muon::sTgcPrepDataContainer> m_sTgcContainerKey{this,"sTgcPrepDataContainerName", "STGC_Measurements"};
  SG::ReadCondHandleKey<MuonGM::MuonDetectorManager> m_detectorManagerKey{this, "DetectorManagerKey", "MuonDetectorManager","Key of input MuonDetectorManager condition data"}; 
  SG::ReadHandleKey<xAOD::TrackParticleContainer> m_meTrkKey{this, "METrkContainer", "ExtrapolatedMuonTrackParticles"};
  SG::ReadHandleKey<Muon::NSW_PadTriggerDataContainer> m_rdoKey{this, "NSW_PadTriggerDataKey", "NSW_PadTrigger_RDO"};


  Gaudi::Property<unsigned int> m_clusterSizeCut{this, "clusterSizeCut", 3};
};    
#endif
