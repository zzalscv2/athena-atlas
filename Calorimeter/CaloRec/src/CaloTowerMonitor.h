/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CALOREC_CALOTOWERMONITOR_H
#define CALOREC_CALOTOWERMONITOR_H
///////////////////////////////////////////////////////////////////////////////
/// \brief simple monitor for CaloTower objects
///
/// The CaloTowerMonitor is an algorithm to produce a few histograms from 
/// CaloTowers to check the performance of the CaloTower algorithm.
///
/// \author Peter Loch <loch@physics.arizona.edu>
/// \date May 05, 2004 - first implementation
///
///////////////////////////////////////////////////////////////////////////////

#include "CaloGeoHelpers/CaloSampling.h"
#include "CaloEvent/CaloTowerContainer.h"
#include "StoreGate/ReadHandleKeyArray.h"
#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ITHistSvc.h"
#include "GaudiKernel/ServiceHandle.h"
#include "TH1.h"
#include "TH2.h"

#include <string>
#include <vector>
#include <map>


class CaloTowerMonitor : public AthAlgorithm
{
public:
  using AthAlgorithm::AthAlgorithm;

  virtual StatusCode initialize() override;
  virtual StatusCode execute() override;

 protected:

  SG::ReadHandleKeyArray<CaloTowerContainer> m_collectionNames
  { this, "InputTowerCollections", {} };

  ////////////////
  // Histograms //
  ////////////////

  // number of towers
  TH1* m_nTowersVsEta = nullptr;
  TH1* m_nTowersVsPhi = nullptr;

  // tower shape
  TH2* m_cellsInEtaVsPhi = nullptr;
  TH1* m_nCellsInTower = nullptr;
  TH2* m_nCellsInTowerVsEta = nullptr;
  TH2* m_nCellsInTowerVsPhi = nullptr;

  // tower energies
  TH1* m_eTowers = nullptr;
  TH2* m_eTowersVsEta = nullptr;
  TH2* m_eTowersVsPhi = nullptr;
  TH1* m_eLogTowers = nullptr;

  // tower transversal energies
  TH1* m_etTowers = nullptr;
  TH2* m_etTowersVsEta = nullptr;
  TH2* m_etTowersVsPhi = nullptr;
  TH1* m_etLogTowers = nullptr;

  // eta/phi matches
  TH2* m_etaTowerVsCell = nullptr;
  TH2* m_phiTowerVsCell = nullptr;
  std::map<CaloSampling::CaloSample,TH2*> m_etaTowerVsCellCalos;
  std::map<CaloSampling::CaloSample,TH2*> m_phiTowerVsCellCalos;

  ServiceHandle<ITHistSvc> m_histSvc
    { this, "THistSvc", "THistSvc" };

  StringProperty m_streamName
    { this, "StreamName", "ESD", "Histogram stream name" };
};
#endif
