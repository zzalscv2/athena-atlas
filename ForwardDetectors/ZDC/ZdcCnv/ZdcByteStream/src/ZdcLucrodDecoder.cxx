/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "ZdcByteStream/ZdcLucrodDecoder.h"
#include "ZdcByteStream/ZdcLucrodData.h"

#define endreq endmsg

StatusCode ZdcLucrodDecoder::decode(const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment* robFragment, ZdcLucrodData* zld) {
  
  if (msgLevel(MSG::DEBUG)) msg(MSG::DEBUG) << " ZdcLucrodDecoder::decode" << endreq;
  
  uint32_t ROD_run_no            = robFragment->rod_run_no();
  uint32_t ROD_lvl1_id           = robFragment->rod_lvl1_id();
  uint32_t ROD_bc_id             = robFragment->rod_bc_id();
  uint32_t ROD_lvl1_trigger_type = robFragment->rod_lvl1_trigger_type();
  uint32_t ROD_detev_type        = robFragment->rod_detev_type();
  uint32_t ROD_marker            = robFragment->rod_marker();
  uint32_t ROD_fragment_size     = robFragment->rod_fragment_size_word();
  uint32_t ROD_header_size       = robFragment->rod_header_size_word();
  uint32_t ROD_trailer_size      = robFragment->rod_trailer_size_word();
  uint32_t ROD_version           = robFragment->rod_version() >> 16;     // extract the upper 16 bits
  uint32_t ROD_sourceID          = robFragment->rod_source_id();
  uint32_t ROD_nstatus           = robFragment->rod_nstatus();
  uint32_t ROD_ndata             = robFragment->rod_ndata(); 
  uint32_t ROD_status_pos        = robFragment->rod_status_position();

  uint32_t ROD_sourceID_high = ROD_sourceID >> 16; // extract the upper 16 bits

  if (msgLevel(MSG::DEBUG)) 
    msg(MSG::DEBUG) 
      << " ROD_run_no:            "   << std::dec << ROD_run_no            << endreq
      << " ROD_lvl1_id:           0x" << std::hex << ROD_lvl1_id           << endreq
      << " ROD_bc_id:             "   << std::dec << ROD_bc_id             << endreq
      << " ROD_lvl1_trigger_type: "   << std::dec << ROD_lvl1_trigger_type << endreq
      << " ROD_detev_type:        "   << std::dec << ROD_detev_type        << endreq
      << " ROD_marker:            0x" << std::hex << ROD_marker            << endreq
      << " ROD_fragment_size:     "   << std::dec << ROD_fragment_size     << endreq
      << " ROD_header_size:       "   << std::dec << ROD_header_size       << endreq
      << " ROD_trailer_size:      "   << std::dec << ROD_trailer_size      << endreq
      << " ROD_version:           0x" << std::hex << ROD_version           << endreq
      << " ROD_source_ID:         0x" << std::hex << ROD_sourceID         << endreq
      << " ROD_nstatus:           "   << std::dec << ROD_nstatus           << endreq
      << " ROD_ndata:             "   << std::dec << ROD_ndata             << endreq
      << " ROD_status_pos:        "   << std::dec << ROD_status_pos        << endreq
      << std::dec;

  // Check for  valid marker
  //
  if (ROD_marker        != ROD_MARKER)        {
    msg(MSG::ERROR) << " ERROR: Invalid ROD_marker " <<  std::hex << ROD_marker << endreq;
    return StatusCode::FAILURE;
  }
  
  // Check for expected ROD format version
  //
  if (ROD_version      != m_rodVersion)   {
    msg(MSG::ERROR) << " ERROR: Incorrect ROD_version, expected  " << m_rodVersion
		    << ", found " << (ROD_version>>16) << endreq;
    return StatusCode::FAILURE; 
  }

  // Check for ZDC detector ID in upper bits
  //
  if (ROD_sourceID_high  != m_sourceIdHigh)
  {
    msg(MSG::ERROR) << " ERROR: invalid source ID detector word, " << ROD_sourceID << endreq;
    return StatusCode::FAILURE;
  }
  
  unsigned int nwordsNoTrig = ROD_ndata - 5; // Five words for per-module amplitudes and per-side sums/4

  // There should be 32 words per bunch crossing: 1/2 word for each of eight samples * 8 channels
  //
  if (nwordsNoTrig % 32 != 0) {
    msg(MSG::WARNING) << " WARNING: incorrect number of words in ROD data: " << ROD_ndata << endreq;
    return StatusCode::FAILURE;
  }

  unsigned int numBCRead = ROD_ndata/32;
  
  if (msgLevel(MSG::DEBUG)) msg(MSG::DEBUG) << " Fill ZdcLucrodData, with " << numBCRead << " bunch crossings" << endreq; 
  
  zld->SetBCID     (ROD_bc_id);
  zld->SetRunNumber(ROD_run_no);
  zld->SetLevel1ID (ROD_lvl1_id);
  zld->SetNumBCs(numBCRead);

  OFFLINE_FRAGMENTS_NAMESPACE::PointerType vintData;
  OFFLINE_FRAGMENTS_NAMESPACE::PointerType vintStat;
  
  robFragment->rod_data  (vintData);
  robFragment->rod_status(vintStat);
  
  uint32_t            channel = 0xffff;  
  std::vector<uint16_t> waveform;

  uint32_t wordIt=0;
  uint32_t word  =0;

  while (wordIt<ROD_ndata) {
    
    word = vintData[wordIt]; wordIt++;

    if ((word & 0xf0000000) >> 28 == 0xc) { // start of trigger data

      if (waveform.size()) {
	if (!zld->AddChanData(channel, waveform)) {
	  msg(MSG::ERROR) << " ERROR: inconsistency in LUCROD data" << endreq;
	  return StatusCode::FAILURE;
	}
      }
      break;      
    }
    
    // ADCs are all shorts
    uint16_t adc1 =  word & 0xfff;
    uint16_t adc2 = (word & 0xfff0000)  >> 16;
    uint16_t cid1 = (word & 0x7000)     >> 12;
    uint16_t cid2 = (word & 0x70000000) >> 28;

    if (msgLevel(MSG::DEBUG)) 
      msg(MSG::DEBUG)
	<< " adc1: " << adc1
	<< " adc2: " << adc2
	<< " cid1: " << cid1
	<< " cid2: " << cid2 
	<< endreq;

    if (cid1 != cid2) return StatusCode::FAILURE;
    
    if (channel == cid1) {
      
      waveform.push_back(adc1);
      waveform.push_back(adc2);
    }
    else { // new channel
      if (waveform.size()) {
	if (!zld->AddChanData(channel, waveform)) {
	  msg(MSG::ERROR) << " ERROR: inconsistency in LUCROD data" << endreq;
	  return StatusCode::FAILURE;
	} 
      }
      
      channel = cid1; // shouldn't this go before the line above?
      
      waveform.clear();
      waveform.push_back(adc1);
      waveform.push_back(adc2);
    }
  }
  
  
  // this can adapt
  if ((word & 0xf000) >> 12 != 0xa) return StatusCode::FAILURE;
  
  uint16_t trigAvgA =  (word & 0xfff);
  uint16_t trigAvgC = ((word & 0xfff0000)>>16);

  if (msgLevel(MSG::DEBUG)) 
    msg(MSG::DEBUG)
      << " trigAvgA: " << trigAvgA
      << " trigAvgC: " << trigAvgC
      << endreq;

  zld->SetTrigAvgA(trigAvgA);
  zld->SetTrigAvgC(trigAvgC);

  // Loop over all data words in the fragment
  //
  while (wordIt<ROD_ndata) { 
    
    word = vintData[wordIt]; wordIt++;

    if ((word & 0xf0000000) >> 28 != 0xb) return StatusCode::FAILURE;
    
    uint16_t amp1 = (word & 0xfff);         // 12 bit ADC
    uint16_t amp2 = (word & 0xfff0000)>>16;
    
    if (msgLevel(MSG::DEBUG)) 
      msg(MSG::DEBUG)
	<< " amp1: " << amp1
	<< " amp2: " << amp2
	<< endreq;
    
    zld->AddTrigData(amp1);
    zld->AddTrigData(amp2);
  }
  
  if (msgLevel(MSG::DEBUG)) 
    msg(MSG::DEBUG) 
      << " ChanDataSize: " << zld->GetChanDataSize() 
      << " TrigDataSize: " << zld->GetTrigDataSize() 
      << endreq;
			    
  if (zld->GetChanDataSize() != ROD_NCHANNELS) { if (msgLevel(MSG::DEBUG)) msg(MSG::DEBUG) << " ERROR: ChanDataSize " << endreq; return StatusCode::FAILURE; }
  if (zld->GetTrigDataSize() != ROD_NCHANNELS) { if (msgLevel(MSG::DEBUG)) msg(MSG::DEBUG) << " ERROR: TrigDataSize " << endreq; return StatusCode::FAILURE; }

  zld->SetStatus(vintStat[0]);
  
  return StatusCode::SUCCESS;
}
