/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "LArCalibTools/LArGainThresholds2Ntuple.h"
#include "LArIdentifier/LArOnlineID.h"

LArGainThresholds2Ntuple::LArGainThresholds2Ntuple(const std::string& name, ISvcLocator* pSvcLocator): 
  LArCond2NtupleBase(name, pSvcLocator) {

  m_ntTitle="Gain Thresholds";
  m_ntpath="/NTUPLES/FILE1/GAINTH";
}


StatusCode LArGainThresholds2Ntuple::initialize() {

  ATH_CHECK(m_configKey.initialize());

  return LArCond2NtupleBase::initialize();
}

LArGainThresholds2Ntuple::~LArGainThresholds2Ntuple() 
= default;

StatusCode LArGainThresholds2Ntuple::stop() {

   ATH_MSG_DEBUG(" trying stop");

   NTuple::Item<long> lower;
   NTuple::Item<long> upper;
 
   SG::ReadCondHandle<LArFebConfig> configHdl{m_configKey};
   const LArFebConfig* febConfig = *configHdl;
   if (febConfig==nullptr) {
     ATH_MSG_ERROR( "Unable to retrieve LArFebConfig with key " << m_configKey.key());
     return StatusCode::FAILURE;
   }
   StatusCode sc=m_nt->addItem("lower",lower,-1000,5000);
   if (sc!=StatusCode::SUCCESS) {
     ATH_MSG_ERROR( "addItem 'lower' failed" );
     return StatusCode::FAILURE;
   }
   
   sc=m_nt->addItem("upper",upper,-1000.,5000.);
   if (sc!=StatusCode::SUCCESS) {
     ATH_MSG_ERROR( "addItem 'upper' failed" );
     return StatusCode::FAILURE;
   }

   ATH_CHECK(m_nt->addItem("lower",lower,-1000,5000));
   ATH_CHECK(m_nt->addItem("upper",upper,-1000.,5000.));
   
   for (const HWIdentifier hwid: m_onlineId->channel_range()) {
     short lower_v, upper_v;
     febConfig->thresholds (hwid, lower_v, upper_v);
     lower = lower_v;
     upper = upper_v;

     fillFromIdentifier(hwid);
     
     ATH_CHECK(ntupleSvc()->writeRecord(m_nt));      
   }
 
   ATH_MSG_INFO("LArGainThresholds2Ntuple has finished.");
   return StatusCode::SUCCESS;
   
}
