/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "LArDigitContainerCnv.h"
#include "LArTPCnv/LArDigitContainer_p1.h"
#include "LArTPCnv/LArDigitContainer_p2.h"
#include "LArTPCnv/LArDigitContainer_p3.h"
#include "LArTPCnv/LArDigitContainerCnv_p1.h"
#include "LArTPCnv/LArDigitContainerCnv_p2.h"
#include "LArTPCnv/LArDigitContainerCnv_p3.h"
#include "LArIdentifier/LArOnlineID.h"
#include "LArIdentifier/LArOnline_SuperCellID.h"
#include "StoreGate/StoreGateSvc.h"
#include <memory>

LArDigitContainerCnv::LArDigitContainerCnv(ISvcLocator* svcLoc) : 
  LArDigitContainerCnvBase(svcLoc),
  m_p0_guid("B15FFDA0-206D-4062-8B5F-582A1ECD5502"),
  m_p1_guid("F1876026-CDFE-4110-AA59-E441BAA5DE44"),
  m_p2_guid("66F5B7Af-595C-4F79-A2B7-56590777C313"),
  m_p3_guid("24480EBA-1AF1-4646-95A7-11285F09717C")
{}


StatusCode LArDigitContainerCnv::initialize() {

  MsgStream log(msgSvc(), "LArDigitContainerCnv");
  StoreGateSvc *detStore=nullptr;
  StatusCode sc=service("DetectorStore",detStore);
  if (sc.isFailure()) {
    log << MSG::FATAL << "DetectorStore service not found !" << endmsg;
    return StatusCode::FAILURE;
  }

  const LArOnlineID* idHelper=nullptr;
  sc=detStore->retrieve(idHelper,"LArOnlineID");
  if (sc.isFailure()) {
    log << MSG::FATAL << "Could not retrieve LArOnlineID helper from Detector Store" << endmsg;
    return StatusCode::FAILURE;
  }
  m_idHelper=idHelper;//cast to base-class

  const LArOnline_SuperCellID* idSCHelper=nullptr;
  sc=detStore->retrieve(idSCHelper,"LArOnline_SuperCellID");
  if (sc.isFailure()) {
    log << MSG::FATAL << "Could not retrieve LArOnline_SuperCellID helper from Detector Store" << endmsg;
    return StatusCode::FAILURE;
  }
  m_idSCHelper=idSCHelper;//cast to base-class

  sc=service("StoreGateSvc",m_storeGateSvc);
  if (sc.isFailure()) {
    log << MSG::FATAL << "StoreGate service not found !" << endmsg;
    return StatusCode::FAILURE;
  }

  return LArDigitContainerCnvBase::initialize();
}


LArDigitContainerPERS* LArDigitContainerCnv::createPersistent(LArDigitContainer* trans) {
    MsgStream log(msgSvc(), "LArDigitContainerCnv");
    log << MSG::DEBUG << "Writing LArDigitContainer_p3" << endmsg;
    LArDigitContainerPERS* pers=new LArDigitContainerPERS();
    LArDigitContainerCnv_p3 converter(m_idHelper, m_idSCHelper, m_storeGateSvc);
    converter.transToPers(trans,pers,log);
    return pers;
}


LArDigitContainer* LArDigitContainerCnv::createTransient() {
   MsgStream log(msgSvc(), "LArDigitContainerCnv" );
   if (compareClassGuid(m_p0_guid)) {
     log << MSG::DEBUG << "Read version p0 of LArDigitContainer. GUID=" 
	 << m_classID.toString() << endmsg;
     return poolReadObject<LArDigitContainer>();
   }
   else if (compareClassGuid(m_p1_guid)) {
     log << MSG::DEBUG << "Reading LArDigitContainer_p1. GUID=" 
	 << m_classID.toString() << endmsg;
     LArDigitContainer* trans=new LArDigitContainer();
     std::unique_ptr<LArDigitContainer_p1> pers(poolReadObject<LArDigitContainer_p1>());
     LArDigitContainerCnv_p1 converter;
     converter.persToTrans(pers.get(),trans, log);
     return trans;
   } 
   else if (compareClassGuid(m_p2_guid)) {
     log << MSG::DEBUG << "Reading LArDigitContainer_p2. GUID="
	 << m_classID.toString() << endmsg;
     LArDigitContainer* trans=new LArDigitContainer();
     std::unique_ptr<LArDigitContainer_p2> pers(poolReadObject<LArDigitContainer_p2>());
     LArDigitContainerCnv_p2 converter(m_idHelper);
     converter.persToTrans(pers.get(),trans, log);
     return trans;
   }
   else if (compareClassGuid(m_p3_guid)) {
     log << MSG::DEBUG << "Reading LArDigitContainer_p3. GUID="
	 << m_classID.toString() << endmsg;
     LArDigitContainer* trans=new LArDigitContainer();
     std::unique_ptr<LArDigitContainer_p3> pers(poolReadObject<LArDigitContainer_p3>());
     LArDigitContainerCnv_p3 converter(m_idHelper, m_idSCHelper, m_storeGateSvc);
     converter.persToTrans(pers.get(),trans, log);
     return trans;
   }
   log << MSG::ERROR << "Unsupported persistent version of LArDigitContainer. GUID="
       << m_classID.toString() << endmsg;
   throw std::runtime_error("Unsupported persistent version of Data Collection");
   // not reached
}
