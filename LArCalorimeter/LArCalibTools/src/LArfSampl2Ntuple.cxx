/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "LArCalibTools/LArfSampl2Ntuple.h"
#include "LArRawConditions/LArfSamplMC.h"
#include "CaloIdentifier/CaloGain.h"


LArfSampl2Ntuple::LArfSampl2Ntuple(const std::string& name, ISvcLocator* pSvcLocator): 
  LArCond2NtupleBase(name, pSvcLocator) { 
  m_ntTitle="fSampl";
  m_ntpath="/NTUPLES/FILE1/FSAMPL";

}

LArfSampl2Ntuple::~LArfSampl2Ntuple() 
{}

StatusCode LArfSampl2Ntuple::initialize(){
  ATH_CHECK(m_contKey.initialize());
  return LArCond2NtupleBase::initialize();
}

StatusCode LArfSampl2Ntuple::stop() {

  // For compatibility with existing configurations, look in the detector
  // store first, then in conditions store
  const ILArfSampl* larfSampl = detStore()->tryConstRetrieve<ILArfSampl>(m_contKey.key());
  if (!larfSampl){
    SG::ReadCondHandle<ILArfSampl> fsamplHdl(m_contKey);
    larfSampl=*fsamplHdl;
  }

  StatusCode sc;

 NTuple::Item<long> cellIndex;
 NTuple::Item<float> fsampl;

 sc=m_nt->addItem("icell",cellIndex,0,2000);
 if (sc!=StatusCode::SUCCESS)
   {ATH_MSG_ERROR( "addItem 'Cell Index' failed" );
    return StatusCode::FAILURE;
   }

 sc=m_nt->addItem("fsampl",fsampl);
 if (sc!=StatusCode::SUCCESS)
   {ATH_MSG_ERROR( "addItem 'fsampl' failed" );
    return StatusCode::FAILURE;
   }

 SG::ReadCondHandle<LArOnOffIdMapping> cablingHdl{m_cablingKey};
 const LArOnOffIdMapping* cabling=*cablingHdl;
 if(!cabling) {
     ATH_MSG_WARNING( "Do not have cabling object LArOnOffIdMapping" );
     return StatusCode::FAILURE;
 }

 unsigned cellCounter=0;
 for (const HWIdentifier hwid: m_onlineId->channel_range()) {
   if ( cabling->isOnlineConnected(hwid)) {
       fillFromIdentifier(hwid);       
       cellIndex = cellCounter;
       fsampl = larfSampl->FSAMPL(hwid);
       sc=ntupleSvc()->writeRecord(m_nt);
       if (sc!=StatusCode::SUCCESS) {
         ATH_MSG_ERROR( "writeRecord failed" );
         return StatusCode::FAILURE;
       }
   }//end if isConnected
   cellCounter++;
 }//end loop over online ID

 ATH_MSG_INFO( "LArfSampl2Ntuple has finished." );
 return StatusCode::SUCCESS;
}// end finalize-method.
   
