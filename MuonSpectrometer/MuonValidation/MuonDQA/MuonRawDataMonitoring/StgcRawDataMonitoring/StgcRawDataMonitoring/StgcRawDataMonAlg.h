/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
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

// stl includes                                                                                 
#include <string>

namespace Muon {
  class sTgcPrepData;
}

namespace GeometricSectors {
  static const std::array<std::string, 2> sTgc_Side = {"CSide", "ASide"};
}

class sTgcRawDataMonAlg: public AthMonitorAlgorithm {
 public:
  sTgcRawDataMonAlg(const std::string& name, ISvcLocator* pSvcLocator);
  
  virtual ~sTgcRawDataMonAlg()=default;
  virtual StatusCode initialize() override;
  virtual StatusCode fillHistograms(const EventContext& ctx) const override;
 private:  
  ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc {this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
    
  void fillsTgcOverviewHistograms(const Muon::sTgcPrepData*, const Muon::MuonPrepDataCollection<Muon::sTgcPrepData> &prd) const;
  void fillsTgcSummaryHistograms(const Muon::sTgcPrepData*, const MuonGM::MuonDetectorManager*) const;

  int getSectors(const Identifier& id) const;
  
  SG::ReadHandleKey<Muon::sTgcPrepDataContainer> m_sTgcContainerKey{this,"sTgcPrepDataContainerName", "STGC_Measurements"};
  SG::ReadCondHandleKey<MuonGM::MuonDetectorManager> m_detectorManagerKey{this, "DetectorManagerKey", "MuonDetectorManager","Key of input MuonDetectorManager condition data"};

  Gaudi::Property<bool> m_dosTgcESD{this,"dosTgcESD", true};
  Gaudi::Property<bool> m_dosTgcOverview{this,"dosTgcOverview", true};
};    
#endif
