/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonRDO/MdtCsm.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/ISvcLocator.h"
#include "StoreGate/StoreGateSvc.h"


#include "MdtCsmCnv_p1.h"
#include <sstream>

MdtCsmCnv_p1::MdtCsmCnv_p1(){
    StoreGateSvc* detStore = nullptr;
    if (!Gaudi::svcLocator()->existsService("DetectorStore")) return;
    StatusCode sc = Gaudi::svcLocator()->service("DetectorStore", detStore);
    if (sc != StatusCode::SUCCESS) { return;}
    sc = detStore->retrieve(m_idHelper, "MDTIDHELPER");
    if (sc != StatusCode::SUCCESS) {return; }
    m_2CSM_Mod = m_idHelper->stationNameIndex("BME") != -1;   
}
void
MdtCsmCnv_p1::persToTrans(const MdtCsm_p1* persColl, MdtCsm* transColl, MsgStream &log) 
{
  const Identifier mod_id{Identifier32(persColl->m_Id)};
  IdentifierHash mod_hash{persColl->m_idHash};
  if (m_idHelper) {
    if (m_2CSM_Mod) m_idHelper->get_detectorElement_hash(mod_id,mod_hash);
    else m_idHelper->get_module_hash(mod_id,mod_hash);
  }
  *transColl = MdtCsm (mod_id,
                       mod_hash,
                       persColl->m_SubDetId,
                       persColl->m_MrodId,
                       persColl->m_CsmId);
  // The assignment above will leave *transColl as a view container.
  // But it should own it's elements, so convert it back
  // to an owning container.
  transColl->clear (SG::OWN_ELEMENTS);
  
  // Invoke vector converter from the base template
  MdtCsm_Cnvp1_base_t::persToTrans( persColl, transColl, log );
}


void
MdtCsmCnv_p1::transToPers(const MdtCsm* transColl, MdtCsm_p1* persColl, MsgStream &log) 
{
    // log <<  MSG::DEBUG << " ***  Writing out MdtCsm" << endmsg;

    persColl->m_Id       = transColl->identify().get_identifier32().get_compact();
    persColl->m_idHash   = transColl->identifyHash();

    persColl->m_SubDetId = transColl->SubDetId();
    persColl->m_MrodId   = transColl->MrodId();
    persColl->m_CsmId    = transColl->CsmId();
    
    // Invoke vector converter from the base template
   MdtCsm_Cnvp1_base_t::transToPers( transColl, persColl, log );

}






