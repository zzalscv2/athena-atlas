/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "LArCalibTools/LArMinBias2Ntuple.h"
#include "LArRawConditions/LArMinBiasMC.h"
#include "LArRawConditions/LArMinBiasAverageMC.h"
#include "CaloIdentifier/CaloGain.h"
/*
#include "GaudiKernel/INTupleSvc.h"
#include "GaudiKernel/NTuple.h"
#include "GaudiKernel/SmartDataPtr.h"
*/

//#include <fstream>

LArMinBias2Ntuple::LArMinBias2Ntuple(const std::string& name, ISvcLocator* pSvcLocator): 
  LArCond2NtupleBase(name, pSvcLocator),
  m_isPileup(false) { 
  declareProperty("ContainerKey",m_contKey="LArMinBias");
  //declareProperty("IsMC",m_isMC = false);

  declareProperty("NtupleTitle",m_ntTitle="MinBias");
  declareProperty("NtupleName",m_ntpath="/NTUPLES/FILE1/MINBIAS");

}

LArMinBias2Ntuple::~LArMinBias2Ntuple() 
= default;

StatusCode LArMinBias2Ntuple::stop() {
   
  const ILArMinBias* LArMinBias = nullptr;
  const ILArMinBiasAverage* LArMinBiasAv = nullptr;
  
  m_isPileup = m_contKey.find("Pileup") != std::string::npos;
  if(!m_isPileup) ATH_CHECK( m_detStore->retrieve(LArMinBias,m_contKey) );
  ATH_CHECK( m_detStore->retrieve(LArMinBiasAv,m_contKey+"Average") );

 NTuple::Item<float> minbias;
 NTuple::Item<float> minbias_av;

 SG::ReadCondHandle<LArOnOffIdMapping> cablingHdl{m_cablingKey};
 const LArOnOffIdMapping* cabling=*cablingHdl;
 if(!cabling) {
     ATH_MSG_WARNING( "Do not have cabling object LArOnOffIdMapping" );
     return StatusCode::FAILURE;
 }

 if(!m_isPileup) ATH_CHECK( m_nt->addItem("MinBias",minbias) );
 ATH_CHECK( m_nt->addItem("MinBiasAv",minbias_av) );

 unsigned cellCounter=0;
 for (const HWIdentifier hwid: m_onlineId->channel_range()) {
   if ( cabling->isOnlineConnected(hwid)) {
     fillFromIdentifier(hwid);       
     if(!m_isPileup) minbias = LArMinBias->minBiasRMS(hwid);
     minbias_av = LArMinBiasAv->minBiasAverage(hwid);
     ATH_CHECK( ntupleSvc()->writeRecord(m_nt) );
   }//end if isConnected
   cellCounter++;
 }//end loop over online ID

 ATH_MSG_INFO(  "LArMinBias2Ntuple has finished, " << cellCounter << " cells written." );
 return StatusCode::SUCCESS;
}// end finalize-method.
   
