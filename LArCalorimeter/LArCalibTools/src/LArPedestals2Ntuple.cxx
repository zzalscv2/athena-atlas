/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "LArCalibTools/LArPedestals2Ntuple.h"
#include "LArRawConditions/LArPedestalComplete.h"
#include "CaloIdentifier/CaloGain.h"


LArPedestals2Ntuple::LArPedestals2Ntuple(const std::string& name, ISvcLocator* pSvcLocator): 
  LArCond2NtupleBase(name, pSvcLocator),
  m_pedKey("Pedestal"){
 
  declareProperty("ContainerKey",m_pedKey);
  m_ntTitle="Pedestals";
  m_ntpath="/NTUPLES/FILE1/PEDESTALS";
    
}

LArPedestals2Ntuple::~LArPedestals2Ntuple() 
= default;

StatusCode LArPedestals2Ntuple::initialize() {
  ATH_CHECK(m_pedKey.initialize());
  return LArCond2NtupleBase::initialize();
}

StatusCode LArPedestals2Ntuple::stop()
{
  // For compatibility with existing configurations, look in the detector
  // store first, then in conditions.
  const ILArPedestal* larPedestal =
    detStore()->tryConstRetrieve<ILArPedestal> (m_pedKey.key());
  if (!larPedestal) {
    SG::ReadCondHandle<ILArPedestal> pedHandle{m_pedKey};
    larPedestal = *pedHandle;
  }

  if (larPedestal==nullptr) {
    ATH_MSG_ERROR( "Unable to retrieve ILArPedestal with key " 
		   << m_pedKey.key() << " from DetectorStore");
    return StatusCode::FAILURE;
  } 

  NTuple::Item<long> cellIndex,gain;
  NTuple::Item<double> ped;
  NTuple::Item<double> rms;


  StatusCode sc=m_nt->addItem("icell",cellIndex,0,200000);
  if (sc!=StatusCode::SUCCESS)
    {ATH_MSG_ERROR( "addItem 'Cell Index' failed" );
    return StatusCode::FAILURE;
  }

  sc=m_nt->addItem("gain",gain,0,3);
  if (sc!=StatusCode::SUCCESS) {
    ATH_MSG_ERROR( "addItem 'gain' failed" );
    return StatusCode::FAILURE;
  }


  sc=m_nt->addItem("ped",ped,-1000.,5000.);
  if (sc!=StatusCode::SUCCESS) {
    ATH_MSG_ERROR( "addItem 'ped' failed" );
    return StatusCode::FAILURE;
  }

  sc=m_nt->addItem("rms",rms,0.,1e12);
  if (sc!=StatusCode::SUCCESS) {
    ATH_MSG_ERROR( "addItem 'rms' failed" );
    return StatusCode::FAILURE;
  }


 unsigned cellCounter=0;
 for(long igain=CaloGain::LARHIGHGAIN; igain<CaloGain::LARNGAIN; igain++) {
   for (const HWIdentifier hwid: m_onlineId->channel_range()) {
     if (larPedestal->pedestal(hwid,igain)>ILArPedestal::ERRORCODE) {
       fillFromIdentifier(hwid);       
       cellIndex = cellCounter;
                     
       ped=larPedestal->pedestal(hwid,igain);
       rms=larPedestal->pedestalRMS(hwid,igain);
       gain=igain;

       sc=ntupleSvc()->writeRecord(m_nt);
       if (sc!=StatusCode::SUCCESS) {
	 ATH_MSG_ERROR( "writeRecord failed" );
	 return StatusCode::FAILURE;
       }
     }// end if Pedestal exists for this channel
     cellCounter++;
   }//end loop over gains
 }//end loop over online ID

 ATH_MSG_INFO( "LArPedestals2Ntuple has finished." );
 return StatusCode::SUCCESS;
}// end finalize-method.
   
