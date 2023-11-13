/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARPULSESHAPE_H
#define LARPULSESHAPE_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/NTuple.h"

#include "LArElecCalib/ILArFEBTempTool.h"
#include "LArCalibTools/LArCond2NtupleBase.h"

#include "CaloIdentifier/CaloCell_ID.h"
#include <iostream>
#include "TProfile.h"
#include "TH1.h"

#include "LumiBlockData/LuminosityCondData.h"
#include "LArRawEvent/LArDigitContainer.h"
#include "LArCabling/LArOnOffIdMapping.h"
#include "LArRecConditions/LArBadChannelCont.h"
#include "LArElecCalib/ILArPedestal.h"
#include "StoreGate/ReadCondHandleKey.h"

class HWIdentifier;
class LArOnlineID_Base;
class StoreGateSvc;
class LArEM_Base_ID;
class LArHEC_Base_ID;
class LArFCAL_Base_ID;
class CaloCell_ID;

namespace Trig{
   class TrigDecisionTool;
}

class LArPulseShape : public AthAlgorithm {

 public:
  LArPulseShape(const std::string & name, ISvcLocator * pSvcLocator);
  ~LArPulseShape();

  //Standard algo methods
  virtual StatusCode initialize() override;
  virtual StatusCode execute() override; 
  //Finalize needs to be implemented by the deriving class
  virtual StatusCode stop() override;

 std::map<std::string, TProfile*>::const_iterator TEffbegin() const { return m_effhistos.begin(); }
 std::map<std::string, TProfile*>::const_iterator TEffend()   const { return m_effhistos.end(); }
 std::map<int, TProfile*>::const_iterator TCellbegin() const { return m_cellHistos.begin(); }
 std::map<int, TProfile*>::const_iterator TCellend()   const { return m_cellHistos.end(); } 
 
 std::map<std::string, TH1*>::const_iterator THbegin() const { return m_histos.begin(); }
 std::map<std::string, TH1*>::const_iterator THend()   const { return m_histos.end(); }

 private:
   bool m_initialized;
   std::string m_ntname; 
   std::string m_folder;
   const CaloCell_ID*       m_calo_id;
   NTuple::Item<short> m_peakSample;
   NTuple::Item<short> m_useHgIntercept;
   NTuple::Item<short> m_useMgIntercept;
   NTuple::Item<short> m_useLgIntercept;
   int m_OffId = 0;  
   int m_OffId_conv = 0;     
   const char * m_id_char = nullptr;  
   int m_isample = 0;
   int m_mindist = 0;
   int m_closestBC = 0;

   PublicToolHandle<Trig::TrigDecisionTool>m_trigDec{this, "TrigDecisionTool", "", "Handle to the TrigDecisionTool"};  

   SG::ReadCondHandleKey<LuminosityCondData> m_lumiDataKey{this,"LumiKey", "LuminosityCondData","SG Key of LuminosityCondData object"};  
   SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingKey{this,"CablingKey","LArOnOffIdMap","SG Key of LArOnOffIdMapping object"};
   SG::ReadCondHandleKey<LArBadChannelCont> m_BCKey{this, "BadChanKey", "LArBadChannel", "SG bad channels key"};

   const ILArPedestal* m_larPedestal = nullptr;

   std::map<std::string, TProfile*> m_effhistos;
   std::map<int, TProfile*> m_cellHistos;     
   TProfile* m_TProfpulse_diff = nullptr;    
   std::map<std::string, TH1*> m_histos;
   TH1D* m_BCID = nullptr;
   TH1D* m_diffBCID = nullptr;   
   TH1D* m_actualInt = nullptr;
  void addHistogram( TH1* h ) {
    std::string name = h->GetName();
    m_histos.insert( std::map<std::string, TH1*>::value_type( name, h) );
  } 
  void addHistogram( TProfile* h ) {
    std::string name = h->GetName();
    m_effhistos.insert( std::map<std::string, TProfile*>::value_type( name, h) );
  }
  
 protected:   
  bool  m_addBC = false, m_addFEBTemp = false;
  enum {NOT_VALID = -999};

  std::string m_ntpath, m_ntTitle;

  //Ntuple pointer
  NTuple::Tuple* m_nt;

  //Ntuple variables:
  NTuple::Item<long> m_detector, m_region, m_layer, m_eta, m_phi, m_onlChanId, m_oflChanId;
  NTuple::Item<long> m_pos_neg, m_barrel_ec, m_FT, m_slot, m_channel;
  NTuple::Item<long> m_calibLine,m_badChanWord;
  NTuple::Item<long> m_isConnected;
  NTuple::Item<long> m_chanHash, m_febHash, m_oflHash;
  
  NTuple::Item<float> m_FEBTemp1, m_FEBTemp2;


  const LArEM_Base_ID* m_emId = nullptr;
  const LArHEC_Base_ID* m_hecId = nullptr;
  const LArFCAL_Base_ID* m_fcalId = nullptr;
  const LArOnlineID_Base* m_onlineId = nullptr;
  const CaloCell_ID* m_caloId = nullptr;

  bool m_isSC = false;
  bool m_isFlat = false;

  bool m_addHash = false;

};
#endif
