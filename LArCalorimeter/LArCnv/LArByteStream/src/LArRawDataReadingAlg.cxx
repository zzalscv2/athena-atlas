/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "LArRawDataReadingAlg.h"
#include "LArIdentifier/LArOnlineID.h"
#include "ByteStreamCnvSvcBase/IROBDataProviderSvc.h" 
#include "LArRawEvent/LArRawChannelContainer.h"
#include "LArRawEvent/LArDigitContainer.h"
#include "LArRawEvent/LArFebHeaderContainer.h"
#include "eformat/Version.h"
#include "eformat/index.h"

#include "LArByteStream/LArRodBlockTransparentV0.h"
#include "LArByteStream/LArRodBlockCalibrationV3.h"
#include "LArByteStream/LArRodBlockStructure.h"
#include "LArByteStream/LArRodBlockPhysicsV5.h"
#include "LArByteStream/LArRodBlockPhysicsV6.h"

#include "LArFebHeaderReader.h"
#include "LArElecCalib/LArProvenance.h"

LArRawDataReadingAlg::LArRawDataReadingAlg(const std::string& name, ISvcLocator* pSvcLocator) :  
  AthReentrantAlgorithm(name, pSvcLocator) {}

  StatusCode LArRawDataReadingAlg::initialize() {
  m_doRawChannels = !m_rawChannelKey.empty();
  ATH_CHECK(m_rawChannelKey.initialize(m_doRawChannels));

  m_doDigits = !m_digitKey.empty();
  ATH_CHECK(m_digitKey.initialize(m_doDigits));

  m_doFebHeaders = !m_febHeaderKey.empty();
  ATH_CHECK(m_febHeaderKey.initialize(m_doFebHeaders));  

  ATH_CHECK(m_eventInfoKey.initialize() );

  ATH_CHECK(m_robDataProviderSvc.retrieve());
  ATH_CHECK(detStore()->retrieve(m_onlineId,"LArOnlineID"));  
  return StatusCode::SUCCESS;
}     
  
StatusCode LArRawDataReadingAlg::execute(const EventContext& ctx) const {
  LArRawChannelContainer* rawChannels=nullptr;
  LArDigitContainer* digits=nullptr;
  LArFebHeaderContainer* febHeaders=nullptr;

  if (m_doRawChannels) {
    SG::WriteHandle<LArRawChannelContainer> rawChannelsHdl(m_rawChannelKey,ctx);
    ATH_CHECK(rawChannelsHdl.record(std::make_unique<LArRawChannelContainer>()));
    rawChannels=rawChannelsHdl.ptr();
    rawChannels->reserve(182468); //Total number of LAr readout channels
  }

  if (m_doDigits) {
    SG::WriteHandle<LArDigitContainer> digitsHdl(m_digitKey,ctx);
    ATH_CHECK(digitsHdl.record(std::make_unique<LArDigitContainer>()));
    digits=digitsHdl.ptr();
    digits->reserve(1000); //Approximate number of Digits above threshold
  }

  if (m_doFebHeaders) {
    SG::WriteHandle<LArFebHeaderContainer> febHeadersHdl(m_febHeaderKey,ctx);
    ATH_CHECK(febHeadersHdl.record(std::make_unique<LArFebHeaderContainer>()));
    febHeaders=febHeadersHdl.ptr();
    febHeaders->reserve(1524); //Total number of LAr Front End Boards
  }

  //Get full events and filter out LAr ROBs
  const RawEvent* fullEvent=m_robDataProviderSvc->getEvent(ctx);
  std::map<eformat::SubDetectorGroup, std::vector<const uint32_t*> > rawEventTOC;
  eformat::helper::build_toc(*fullEvent, rawEventTOC);
  auto larRobs=rawEventTOC.find(eformat::LAR);
  if (larRobs==rawEventTOC.end()) {
     ATH_MSG_DEBUG("No LAr data found in this event. Recording empty LArRawChannelContainer");
     return StatusCode::SUCCESS;
  } 
  
  
  std::unique_ptr<LArRodBlockStructure> rodBlock;
  uint16_t rodMinorVersion=0x0;
  uint32_t rodBlockType=0x0;


  for (const uint32_t* robPtr : larRobs->second) {
    OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment rob(robPtr);
    ATH_MSG_VERBOSE("Decoding ROB fragment 0x" << std::hex << rob.rob_source_id () << " with " << std::dec << rob.rod_fragment_size_word() << " ROB words");

    if (rob.rod_fragment_size_word() <3) {
      if (m_failOnCorruption) {
        ATH_MSG_ERROR("Encountered corrupt ROD fragment, less than 3 words!");
	return StatusCode::FAILURE;
      }else { 
	continue;
      }
    } else if(rob.rob_source_id()& 0x1000 ){
         //ATH_MSG_DEBUG(" skip Latome fragment with source ID "<< std::hex << rob.rob_source_id()
         rodBlock=nullptr;
         continue;
    } else if(!(rob.rod_source_id()>>12& 0x0F) //0xnn0nnn must be
             && !((rob.rod_source_id()>>20) == 4) ){ //0x4nnnnn must be
     
         ATH_MSG_WARNING("Found not LAr fragment " << " event: "<<ctx.eventID().event_number());
         SG::ReadHandle<xAOD::EventInfo> eventInfo (m_eventInfoKey, ctx);
         ATH_MSG_WARNING("Rob source id.: 0x"<<  std::hex << rob.rob_source_id () <<std::dec  <<" ROD Source id: 0x"<<std::hex<<rob.rod_source_id()<<std::dec<<" Lvl1ID: "<<eventInfo->extendedLevel1ID());
         continue;
    }

 
     eformat::helper::Version ver(rob.rod_version());
    //(re-)init rodBlock only once per event or if (very unlikly or even impossible) some FEBs have a differnt firmware
    if (rodBlock==nullptr || rodMinorVersion !=ver.minor_version() || rodBlockType!=(rob.rod_detev_type()&0xff)) {
      rodMinorVersion=ver.minor_version();
      rodBlockType=rob.rod_detev_type()&0xff;
      ATH_MSG_VERBOSE("Found version " << rodMinorVersion <<  " of Rod Block Type  " <<  rodBlockType);
      if (rodBlockType==4) { //Physics mode
	switch(rodMinorVersion) {
	case 12: //Physics mode v6 09.03.2011 for LHC
	  rodBlock.reset(new LArRodBlockPhysicsV6);
	  break;
	case 11: //Physics mode v5 16.06.2008 for LHC 
	case 10: //Physics mode v5 16.06.2008 for LHC 
	  rodBlock.reset(new LArRodBlockPhysicsV5);
	  break;
	default: // Unknown version of rod block type 4 (Physics mode)
	  if (m_failOnCorruption) {
	    ATH_MSG_ERROR("Found unsupported ROD Block version " << rodMinorVersion 
			  << " of ROD block type " << rodBlockType << ". ROD Source id: 0x" <<std::hex<<rob.rod_source_id());
	    return StatusCode::FAILURE;
	  }
	  else {
	    ATH_MSG_WARNING("Found unsupported ROD Block version " << rodMinorVersion 
			    << " of ROD block type " << rodBlockType << ". ROD Source id: 0x" <<std::hex<<rob.rod_source_id());
	    continue;
	  }
	}// end switch(rodMinorVersion)
      }//end rodBlockType==4 (physics mode)
      else if (rodBlockType==2) { //Transparent mode
	switch(rodMinorVersion) {
           case 4:  
	     rodBlock.reset(new LArRodBlockTransparentV0<LArRodBlockHeaderTransparentV0>);
             break;
           case 12:
             rodBlock.reset(new LArRodBlockCalibrationV3);
             break;
           default:  
	     ATH_MSG_WARNING("Found unsupported ROD Block version " << rodMinorVersion 
			<< " of ROD block type " << rodBlockType);
	     return m_failOnCorruption ? StatusCode::FAILURE : StatusCode::SUCCESS;
        }
      } 
    }//End if need to re-init RodBlock

    const uint32_t* pData=rob.rod_data();
    const uint32_t  nData=rob.rod_ndata();
    if (nData==0) {
      if (m_failOnCorruption) {
	ATH_MSG_ERROR("ROD 0x"<<std::hex<<rob.rod_source_id() << std::dec << " reports data block size 0");
	return StatusCode::FAILURE;
      }
      else {
	ATH_MSG_WARNING("ROD 0x"<<std::hex<<rob.rod_source_id() << std::dec << " reports data block size 0");
	continue; //Jump to next ROD
      }
    }

    if (!rodBlock->setFragment(pData,nData)) {
      ATH_MSG_ERROR("Failed to assign fragment pointer to LArRodBlockStructure");
      return StatusCode::FAILURE;
    }

    if(m_verifyChecksum) {
      const uint32_t onsum  = rodBlock->onlineCheckSum();
      const uint32_t offsum = rodBlock->offlineCheckSum();
      if(onsum!=offsum) {
	if (m_failOnCorruption) {
	  ATH_MSG_ERROR("Checksum error:");
	  ATH_MSG_ERROR("online checksum  = 0x" << MSG::hex << onsum);
	  ATH_MSG_ERROR("offline checksum = 0x" << MSG::hex << offsum << MSG::dec);
	  return StatusCode::FAILURE;
        } else {
	   continue; //Jump to the next ROD-block
        }
      }
    }

    //Loop over FEBs in ROD:
    do {
      HWIdentifier fId(Identifier32(rodBlock->getFEBID()));
      if (!m_onlineId->isValidId(fId)) {
	if (m_failOnCorruption){ 
	  ATH_MSG_ERROR("Invalid FEB identifer 0x" << std::hex << fId.get_identifier32().get_compact()); 
	  return StatusCode::FAILURE;
        } else {
	  ATH_MSG_WARNING("Invalid FEB identifer 0x" << std::hex << fId.get_identifier32().get_compact()); 
	  continue; //Jump to next FEB
        }
      }
      const int NthisFebChannel=m_onlineId->channelInSlotMax(fId);

      //Decode RawChanels (if requested)
      if (m_doRawChannels) {
	int32_t energy;
	int32_t time;
	int32_t quality;
	uint32_t gain;
	int fcNb;
	while (rodBlock->getNextEnergy(fcNb,energy,time,quality,gain)) {
	  if (fcNb>=NthisFebChannel)
	    continue;

	  HWIdentifier cId = m_onlineId->channel_Id(fId,fcNb);
	  uint16_t iquality = 0;
	  uint16_t iprovenance = LArProv::DSPCALC; //0x1000
	  if (quality>0) {
            iprovenance |= LArProv::QTPRESENT; //0x2000
            iquality = (quality & 0xFFFF);
	  } 
	rawChannels->emplace_back(cId, energy, time, iquality, iprovenance, (CaloGain::CaloGain)gain);
	}//end getNextEnergyLoop
      }//end if m_doRawChannels 

      //Decode LArDigits (if requested)
      if (m_doDigits) {
	uint32_t gain;
	int fcNb;
	std::vector<short> samples;
	while (rodBlock->getNextRawData(fcNb,samples,gain)) {
	  if (fcNb>=NthisFebChannel)
	    continue;
	  if (samples.size()==0) continue; // Ignore missing cells
	  HWIdentifier cId = m_onlineId->channel_Id(fId,fcNb);
	  digits->emplace_back(new LArDigit(cId, (CaloGain::CaloGain)gain, std::move(samples)));
	  samples.clear();
	}//end getNextRawData loop
      }//end if m_doDigits

      //Decode FebHeaders (if requested)
      if (m_doFebHeaders) {
	std::unique_ptr<LArFebHeader> larFebHeader(new LArFebHeader(fId));
	LArFebHeaderReader::fillFebHeader(larFebHeader.get(),rodBlock.get(),rob);
	febHeaders->push_back(std::move(larFebHeader));
      }//end if m_doFebHeaders

    }while (rodBlock->nextFEB()); //Get NextFeb
  } //end loop over ROBs
  return StatusCode::SUCCESS;
}
