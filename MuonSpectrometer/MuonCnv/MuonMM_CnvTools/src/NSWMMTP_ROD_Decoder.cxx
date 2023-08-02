/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/


#include "NSWMMTP_ROD_Decoder.h"
#include "Identifier/Identifier.h"
#include "eformat/Issue.h"
#include "eformat/SourceIdentifier.h"

#include "MuonNSWCommonDecode/NSWTriggerCommonDecoder.h"
#include "MuonNSWCommonDecode/NSWTriggerMML1AElink.h"
#include "MuonNSWCommonDecode/MMARTPacket.h"
#include "MuonNSWCommonDecode/MMTrigPacket.h"


Muon::NSWMMTP_ROD_Decoder::NSWMMTP_ROD_Decoder( const std::string& type, const std::string& name,const IInterface* parent )
: AthAlgTool(type, name, parent)
{
  declareInterface<INSWMMTP_ROD_Decoder>( this );
}


StatusCode Muon::NSWMMTP_ROD_Decoder::initialize() {
  ATH_CHECK(m_idHelperSvc.retrieve());
  return StatusCode::SUCCESS;
}

StatusCode Muon::NSWMMTP_ROD_Decoder::fillCollection(const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment& fragment, xAOD::NSWMMTPRDOContainer& rdoContainer) const
{

  try {
    fragment.check();
  } catch (eformat::Issue &ex) {
    ATH_MSG_WARNING(ex.what());
    return StatusCode::SUCCESS;
  }


  const Muon::nsw::NSWTriggerCommonDecoder decoder{fragment, "MML1A"};

  if (decoder.has_error()) {
    ATH_MSG_WARNING("NSW MMTP Common Decoder found exceptions while reading this MML1A fragment. Skipping. Error id: "+std::to_string(decoder.error_id()));
    return StatusCode::SUCCESS;
  }

  if (decoder.get_elinks().size()!=3) {
    ATH_MSG_WARNING("NSW MMTP Common Decoder didn't give 3 elinks in output: something off with this fragment. Skipping.");
    return StatusCode::SUCCESS;
  }

  //TODO add checks: engines, timeouts, constants and elinks ID vs stream IDs

  xAOD::NSWMMTPRDO* rdo = new xAOD::NSWMMTPRDO();
  rdoContainer.push_back(rdo);

  //for all the 3 elinks expected, the MMTP header parameters should be the same by design; in case one could add a check on the differences
  const auto l0 = std::dynamic_pointer_cast<Muon::nsw::NSWTriggerMML1AElink>(decoder.get_elinks()[0]);

  //this info is redundant with elinkID in the decoded packet! Wanna add a check in the future?
  uint32_t sid = fragment.rob_source_id ();
  eformat::helper::SourceIdentifier source_id (sid);
  //eformat::SubDetector s = source_id.subdetector_id ();
  uint16_t m = source_id.module_id ();


  //ROD info                  
  rdo->set_sourceID(sid);
  rdo->set_moduleID(m);
  rdo->set_ROD_L1ID(fragment.rod_lvl1_id ());
  rdo->set_ROD_BCID(fragment.rod_bc_id ());

    
  //TP head
  rdo->set_EC(l0->head_EC());
  rdo->set_sectID(l0->head_sectID());
  rdo->set_L1ID(l0->head_BCID());
  rdo->set_BCID(l0->L1ID());

  //TP L1A head
  rdo->set_l1a_request_BCID(l0->l1a_local_req_BCID());
  rdo->set_l1a_release_BCID(l0->l1a_local_rel_BCID());
  rdo->set_l1a_window_open(l0->l1a_open_BCID());
  rdo->set_l1a_window_center(l0->l1a_req_BCID());
  rdo->set_l1a_window_close(l0->l1a_close_BCID());
  rdo->set_l1a_window_open_offset(l0->l1a_open_BCID_offset());
  rdo->set_l1a_window_center_offset(l0->l1a_req_BCID_offset());
  rdo->set_l1a_window_close_offset(l0->l1a_close_BCID_offset());

  //L1A data quality
  rdo->set_l1a_timeout(l0->l1a_timeout());
  rdo->set_l1a_engines(l0->l1a_engine_snapshot());


  for(const auto& baseLink: decoder.get_elinks()){
    const auto l = std::dynamic_pointer_cast<Muon::nsw::NSWTriggerMML1AElink>(baseLink);
    
    if (l->l1a_timeout()==0) {ATH_MSG_WARNING("NSW MMTP Common Decoder reporting timeout condition: unclear if current event can be trusted");}

    for (const auto& a: l->art_packets()) {
      for (const auto& c: a->channels()) {
	rdo->art_BCID().push_back(a->art_BCID());
	rdo->art_layer().push_back(c.first);
	rdo->art_channel().push_back(c.second);
      }
    }
    for (const auto& t: l->trig_packets()) {
      rdo->trig_BCID().push_back(t->trig_BCID());
      rdo->trig_dTheta().push_back(t->trig_dTheta());
      rdo->trig_ROI_rID().push_back(t->trig_rBin());
      rdo->trig_ROI_phiID().push_back(t->trig_phiBin());
    }
  
  }
  

  return StatusCode::SUCCESS;
}

