/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file   CaloCalibrationHitsTestTool.h
 * @author Chiara Debenedetti
 * @date    29/08/2011
 */

#ifndef G4ATLASTESTS_CALOCALIBRATIONHITSTESTTOOL_H
#define G4ATLASTESTS_CALOCALIBRATIONHITSTESTTOOL_H
 
#include "SimTestToolBase.h"
#include "CaloDetDescr/CaloDetDescrManager.h"
#include "StoreGate/ReadCondHandleKey.h"

class CaloCalibrationHitsTestTool : public SimTestToolBase {
 
 
 public:
 
  CaloCalibrationHitsTestTool(const std::string& type, const std::string& name, const IInterface* parent);
 
  virtual StatusCode initialize() override;
  virtual StatusCode processEvent() override;

 private:
  // keys
  std::string m_calibHitType;
  std::string m_hitcollkey;

  //--histos--//
  TH1 *m_eta, *m_phi, *m_eEM, *m_eNonEM, *m_eInv, *m_eEsc, *m_eTot;
  TH2 *m_rz, *m_etaphi;
  TH1 *m_eTot_partID, *m_eTot_eta, *m_eTot_phi;
  TH1 *m_partID_large, *m_partID_small;

  SG::ReadCondHandleKey<CaloDetDescrManager> m_caloMgrKey { this
      , "CaloDetDescrManager"
      , "CaloDetDescrManager"
      , "SG Key for CaloDetDescrManager in the Condition Store" };

};
 
#endif
