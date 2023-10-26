/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "LArRawSCCalibDataReadingAlg.h"
#include "LArIdentifier/LArOnlineID.h"
#include "ByteStreamCnvSvcBase/IROBDataProviderSvc.h" 
#include "LArRawEvent/LArDigitContainer.h"
#include "LArRawEvent/LArAccumulatedDigitContainer.h"
#include "LArRawEvent/LArAccumulatedCalibDigitContainer.h"
#include "LArRawEvent/LArLATOMEHeaderContainer.h"
#include "eformat/Version.h"
#include "eformat/index.h"



StatusCode LArRawSCCalibDataReadingAlg::initialize() {


  if (m_accDigitKey.key().size()>0) {
    ATH_CHECK(m_accDigitKey.initialize());
    m_doAccDigits=true;
  }
  else {
    m_doAccDigits=false;
  }

  if (m_accCalibDigitKey.key().size()>0) {
    ATH_CHECK(m_accCalibDigitKey.initialize());
    m_doAccCalibDigits=true;
  }
  else {
    m_doAccCalibDigits=false;
  }

  if (m_latomeHeaderCollKey.key().size()>0) {
    ATH_CHECK(m_latomeHeaderCollKey.initialize());
    m_doLATOMEHeader=true;
  }
  else {
    m_doLATOMEHeader=false;
  }


  if(!m_doAccDigits && !m_doAccCalibDigits) {
     ATH_MSG_FATAL("Needs ether Digits or CalibDigits or AccDigits  or AccCalibDigit Key");
     return StatusCode::FAILURE;
  }

  if(m_doAccDigits && m_doAccCalibDigits) {
     ATH_MSG_FATAL("Could not have both CalibDigits, AccCalibDigits Key");
     return StatusCode::FAILURE;
  }

  ATH_CHECK( m_mapKey.initialize() );
  ATH_CHECK( m_cablingKey.initialize() );
  ATH_CHECK( m_calibMapKey.initialize() );

  ATH_CHECK(m_robDataProviderSvc.retrieve());
  ATH_CHECK(detStore()->retrieve(m_onlineId,"LArOnlineID"));  

  ATH_CHECK(m_latomeDecoder.retrieve());


  return StatusCode::SUCCESS;
}     
  
StatusCode LArRawSCCalibDataReadingAlg::execute(const EventContext& ctx) const {
  LArAccumulatedDigitContainer* accdigits=nullptr;
  LArAccumulatedCalibDigitContainer* caccdigits=nullptr;
  LArLATOMEHeaderContainer* latome_header_coll = nullptr;


  if (m_doAccDigits) {
    SG::WriteHandle<LArAccumulatedDigitContainer> accdigitsHdl(m_accDigitKey,ctx);
    ATH_CHECK(accdigitsHdl.record(std::make_unique<LArAccumulatedDigitContainer>()));
    accdigits=accdigitsHdl.ptr();
    accdigits->reserve(35000); //Enough space for the full calo
  }

  if (m_doAccCalibDigits) {
    SG::WriteHandle<LArAccumulatedCalibDigitContainer> caccdigitsHdl(m_accCalibDigitKey,ctx);
    ATH_CHECK(caccdigitsHdl.record(std::make_unique<LArAccumulatedCalibDigitContainer>()));
    caccdigits=caccdigitsHdl.ptr();
    caccdigits->reserve(35000); //Enough space for the full calo
  }

  if (m_doLATOMEHeader) {
    SG::WriteHandle<LArLATOMEHeaderContainer> latomeHeaderHdl(m_latomeHeaderCollKey, ctx);
    ATH_CHECK(latomeHeaderHdl.record(std::make_unique<LArLATOMEHeaderContainer>()));
    latome_header_coll = latomeHeaderHdl.ptr();
    latome_header_coll->reserve(120);
  }

 // Get the mapping
  SG::ReadCondHandle<LArLATOMEMapping> mapHdl(m_mapKey, ctx);
  const LArLATOMEMapping *map=*mapHdl;
  if(!map) {
     ATH_MSG_ERROR("Do not have LATOME mapping with the key " << m_mapKey.key());
     return StatusCode::FAILURE;
  }

  SG::ReadCondHandle<LArOnOffIdMapping> cablingHdl{m_cablingKey, ctx};
  const LArOnOffIdMapping *onoffmap = *cablingHdl;
  if(!onoffmap) {
    ATH_MSG_ERROR( "Do not have mapping object " << m_cablingKey.key());
    return StatusCode::FAILURE;
  }

  SG::ReadCondHandle<LArCalibLineMapping> clmapHdl(m_calibMapKey, ctx);
  const LArCalibLineMapping *clmap=*clmapHdl;
  if(!clmap) {
     ATH_MSG_ERROR("Do not have calib line mapping with the key " << m_calibMapKey.key());
     return StatusCode::FAILURE;
  }

  //Get full events and filter out LAr ROBs
  const RawEvent* rawEvent=m_robDataProviderSvc->getEvent(ctx);
 
  StatusCode sc = m_latomeDecoder->convert(rawEvent, map, onoffmap, clmap,
                                           accdigits, caccdigits,
                                           latome_header_coll);
  if (sc != StatusCode::SUCCESS) 
    ATH_MSG_WARNING("ERROR LATOMEDecoder tool returned an error. LAr SC containers might be garbage");

  
  return StatusCode::SUCCESS;
}
