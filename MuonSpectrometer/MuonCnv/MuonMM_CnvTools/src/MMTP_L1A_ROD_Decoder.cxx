/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include <memory>

#include "MuonNSWCommonDecode/NSWTriggerCommonDecoder.h"
#include "MuonNSWCommonDecode/NSWTriggerMML1AElink.h"
#include "MuonNSWCommonDecode/MMARTPacket.h"
#include "MuonNSWCommonDecode/MMTrigPacket.h"

#include "Identifier/IdentifierHash.h"
#include "eformat/Issue.h"
#include "eformat/SourceIdentifier.h"

#include "MMTP_L1A_ROD_Decoder.h"

Muon::MMTP_L1A_ROD_Decoder::MMTP_L1A_ROD_Decoder( const std::string& type, const std::string& name,const IInterface* parent )
: AthAlgTool(type, name, parent)
{
  declareInterface<IMMTP_L1A_ROD_Decoder>( this );
}


StatusCode Muon::MMTP_L1A_ROD_Decoder::fillCollection(const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment& fragment, NSW_MMTP_RawDataContainer& rdo) const
{

  try {
    fragment.check();
  } catch (eformat::Issue &ex) {
    ATH_MSG_WARNING(ex.what());
    return StatusCode::SUCCESS;
  }


  const Muon::nsw::NSWTriggerCommonDecoder decoder{fragment, "MML1A"};
  
  //once the trigger path is fully validated, need to handle NSWTriggerCommonDecoder exceptions properly
  if (!decoder.get_elinks().size()) {return StatusCode::SUCCESS;}

  //for all the 3 elinks expected, the MMTP header parameters should be the same by design; in case one could add a check on the differences
  std::shared_ptr<Muon::nsw::NSWTriggerMML1AElink> l0 = std::dynamic_pointer_cast<Muon::nsw::NSWTriggerMML1AElink>(decoder.get_elinks()[0]);
  uint32_t L1ID = l0->L1ID();
  uint16_t BCID = l0->head_BCID();
  uint16_t L1Aopen = l0->head_wdw_open();
  uint16_t L1Aclose = l0->head_wdw_close(); 
  uint16_t L1Areq = l0->head_l1a_req();

  NSW_MMTP_RawDataCollection* rdc = new NSW_MMTP_RawDataCollection(fragment.rob_source_id(), L1ID, BCID, L1Areq, L1Aopen, L1Aclose);  

  for (size_t i = 0; i < decoder.get_elinks().size(); ++i) {
    std::shared_ptr<Muon::nsw::NSWTriggerMML1AElink> l = std::dynamic_pointer_cast<Muon::nsw::NSWTriggerMML1AElink>(decoder.get_elinks()[i]);

    for (auto a: l->art_packets()) {
      for (auto c: a->channels()) {
	rdc->addHit(a->art_BCID(), c.first, c.second);
      }
    }
    for (auto t: l->trig_packets()) {
      rdc->addSegment(t->trig_BCID(), t->trig_dTheta(), t->trig_rBin(), t->trig_phiBin());
    }
  
  }
  
  ATH_CHECK(rdo.addCollection(rdc, rdo.numberOfCollections()));

  return StatusCode::SUCCESS;
}

