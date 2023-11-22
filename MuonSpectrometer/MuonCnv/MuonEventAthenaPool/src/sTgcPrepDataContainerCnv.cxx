/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "sTgcPrepDataContainerCnv.h"

#include "GaudiKernel/StatusCode.h"
#include "GaudiKernel/MsgStream.h"
#include "StoreGate/StoreGateSvc.h"
#include "MuonPrepRawData/sTgcPrepDataContainer.h"

sTgcPrepDataContainerCnv::sTgcPrepDataContainerCnv(ISvcLocator* svcloc) :
sTgcPrepDataContainerCnvBase(svcloc)
{
}

sTgcPrepDataContainerCnv::~sTgcPrepDataContainerCnv() {
}

StatusCode sTgcPrepDataContainerCnv::initialize() {
   // Call base clase initialize
    if( !sTgcPrepDataContainerCnvBase::initialize().isSuccess() )
       return StatusCode::FAILURE;
    
   // Get the messaging service, print where you are
    MsgStream log(msgSvc(), "sTgcPrepDataContainerCnv");
    if (log.level() <= MSG::INFO) log << MSG::INFO << "sTgcPrepDataContainerCnv::initialize()" << endmsg;

    return StatusCode::SUCCESS;
}

sTgcPrepDataContainer_PERS*    sTgcPrepDataContainerCnv::createPersistent (Muon::sTgcPrepDataContainer* transCont) {
    MsgStream log(msgSvc(), "sTgcPrepDataContainerCnv" );
    if (log.level() <= MSG::DEBUG) log<<MSG::DEBUG<<"createPersistent(): main converter"<<endmsg;
    sTgcPrepDataContainer_PERS *pers= m_converter_p3.createPersistent( transCont, log );
    return pers;
}

Muon::sTgcPrepDataContainer* sTgcPrepDataContainerCnv::createTransient() {
    MsgStream log(msgSvc(), "sTgcPrepDataContainerCnv" );
    static const pool::Guid   p1_guid("7AB87DDE-8D7C-11E2-AA7C-001517648C14"); 
    static const pool::Guid   p2_guid("9E1B8028-D22C-4E02-BA82-D0EC79DB4F6C"); 
    static const pool::Guid   p3_guid("F06F048C-878D-11EE-AFB6-5811229BAA38"); 

    if (log.level() <= MSG::DEBUG) log<<MSG::DEBUG<<"createTransient(): main converter"<<endmsg;
    Muon::sTgcPrepDataContainer* p_collection(nullptr);
    if( compareClassGuid(p3_guid) ) {
        if (log.level() <= MSG::DEBUG) log<<MSG::DEBUG<<"createTransient(): T/P version 3 detected"<<endmsg;
        std::unique_ptr< Muon::sTgcPrepDataContainer_p3 >  p_coll( poolReadObject< Muon::sTgcPrepDataContainer_p3 >() );
        p_collection = m_converter_p3.createTransient( p_coll.get(), log );
    } else if( compareClassGuid(p2_guid) ) {
        if (log.level() <= MSG::DEBUG) log<<MSG::DEBUG<<"createTransient(): T/P version 2 detected"<<endmsg;
        std::unique_ptr< Muon::sTgcPrepDataContainer_p2 >  p_coll( poolReadObject< Muon::sTgcPrepDataContainer_p2 >() );
        p_collection = m_converter_p2.createTransient( p_coll.get(), log );
    } else if( compareClassGuid(p1_guid) ) {
        if (log.level() <= MSG::DEBUG) log<<MSG::DEBUG<<"createTransient(): T/P version 1 detected"<<endmsg;
        std::unique_ptr< Muon::sTgcPrepDataContainer_p1 >  p_coll( poolReadObject< Muon::sTgcPrepDataContainer_p1 >() );
        p_collection = m_converter_p1.createTransient( p_coll.get(), log );
    } else {
        throw std::runtime_error("Unsupported persistent version of sTgcPrepDataContainer");
    }
    return p_collection;
}
