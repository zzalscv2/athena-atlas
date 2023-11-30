/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// Framework includes
#include "GaudiKernel/MsgStream.h"

// LArTPCnv includes
#include "LArTPCnv/LArTTL1ContainerCnv_p1.h"

// LArEventAthenaPool includes
#include "LArTTL1ContainerCnv.h"

LArTTL1Container_PERS* 
LArTTL1ContainerCnv::createPersistent( LArTTL1Container* transCont ) 
{
  MsgStream msg( msgSvc(), "LArTTL1ContainerCnv" );

  LArTTL1ContainerCnv_p1 cnv;
  LArTTL1Container_PERS *persObj = cnv.createPersistent( transCont, msg );

  if (msg.level()<=MSG::DEBUG)
    msg << MSG::DEBUG << "::createPersistent [Success]" << endmsg;
  return persObj;
}

LArTTL1Container* LArTTL1ContainerCnv::createTransient() {

  MsgStream msg( msgSvc(), "LArTTL1ContainerCnv" );

  LArTTL1Container *transObj = 0;

  static const pool::Guid tr_guid("38FAECC7-D0C5-4DD8-8FAE-8D35F0542ECD");
  static const pool::Guid p1_guid("b859a463-2ea4-4902-b46a-89e5fbc20132");

  if ( compareClassGuid(tr_guid) ) {

    // regular object from before the T/P separation
    return poolReadObject<LArTTL1Container>();

  } else if ( compareClassGuid(p1_guid) ) {

    // using unique_ptr ensures deletion of the persistent object
    std::unique_ptr<LArTTL1Container_p1> persObj( poolReadObject<LArTTL1Container_p1>() );
    LArTTL1ContainerCnv_p1 cnv;
    transObj = cnv.createTransient( persObj.get(), msg );
  } else {
    throw std::runtime_error("Unsupported persistent version of LArTTL1Container");
  }

  return transObj;
}
