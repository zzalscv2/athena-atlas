/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGT1CALOFEXSIM_JGTOWERMAKER_H
#define TRIGT1CALOFEXSIM_JGTOWERMAKER_H
#include "CaloDetDescr/CaloDetDescrManager.h"
#include "StoreGate/WriteHandle.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "AthenaBaseComps/AthAlgorithm.h"
#include "CaloTriggerTool/JTowerSCMap.h"
#include "CaloTriggerTool/GTowerSCMap.h"
#include "CaloIdentifier/TTOnlineID.h"
#include "CaloIdentifier/CaloCell_SuperCell_ID.h"
#include "CaloIdentifier/CaloCell_ID.h"
#include "CaloIdentifier/JTower_ID.h"
#include "CaloIdentifier/GTower_ID.h"
#include "CaloEvent/CaloCellContainer.h"
#include "xAODTrigL1Calo/JGTower.h"
#include "xAODTrigL1Calo/JGTowerContainer.h"
#include "xAODTrigL1Calo/JGTowerAuxContainer.h"
#include "Identifier/IdentifierHash.h"
#include "TrigT1CaloFexSim/JGTower.h"
class JGTowerMaker: public ::AthAlgorithm { 
 public: 
  JGTowerMaker( const std::string& name, ISvcLocator* pSvcLocator );
  virtual ~JGTowerMaker(); 

  virtual StatusCode  initialize();
  virtual StatusCode  execute();
  virtual StatusCode  finalize();
  virtual StatusCode  JFexAlg(xAOD::JGTowerContainer*towerContainer);   
  virtual StatusCode  GFexAlg(xAOD::JGTowerContainer*towerContainer);
  virtual StatusCode  beginInputFile();
  virtual StatusCode  SCTowerMapping();

  virtual StatusCode  AllCaloMapping(); //temporary solution

  std::vector<JGTower*> jT;
  std::vector<JGTower*> gT;

  GTowerSCMap *myGMap = new GTowerSCMap();
  JTowerSCMap *myJMap = new JTowerSCMap();
 private: 
  const DataHandle<CaloSuperCellDetDescrManager> m_sem_mgr;
//  const DataHandle<CaloTTDescrManager> m_tt_mgr; //kept temporarily to use trigger tower id for for ID mapping in the futher
  const CaloCell_SuperCell_ID* m_scid;
  const JTower_ID* m_jTowerId;
  const GTower_ID* m_gTowerId;
//  const TTOnlineID* m_TTID; // same reason as above
  const Tile_SuperCell_ID* m_tid;
  const CaloCell_ID* m_ccIdHelper;
  bool  m_useSCQuality;
  bool  m_useAllCalo;
  bool  m_TileMapped;
  bool  JMatched;
  std::map<TString, TH1*> hName;
  std::map<TString, TH2*> h2Name;
}; 

#endif //> !XAODL1CALO_XAODL1CALOALG_H
