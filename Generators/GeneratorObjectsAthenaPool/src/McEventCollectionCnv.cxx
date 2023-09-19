///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// McEventCollectionCnv.cxx 
// Implementation file for class McEventCollectionCnv
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 

// Framework includes
#include "GaudiKernel/MsgStream.h"

// GeneratorObjectsTPCnv includes
#include "GeneratorObjectsTPCnv/McEventCollectionCnv_p1.h"
#include "GeneratorObjectsTPCnv/McEventCollectionCnv_p2.h"
#include "GeneratorObjectsTPCnv/McEventCollectionCnv_p3.h"
#include "GeneratorObjectsTPCnv/McEventCollectionCnv_p4.h"
#include "GeneratorObjectsTPCnv/McEventCollectionCnv_p5.h"
#include "GeneratorObjectsTPCnv/McEventCollectionCnv_p6.h"

// GeneratorObjectsAthenaPool includes
#include "McEventCollectionCnv.h"

McEventCollection_PERS* 
McEventCollectionCnv::createPersistent( McEventCollection* transCont ) 
{
  MsgStream msg( msgSvc(), "McEventCollectionCnv" );

  McEventCollectionCnv_p6 cnv;
  McEventCollection_PERS *persObj = cnv.createPersistent( transCont, msg );

  msg << MSG::DEBUG << "::createPersistent [Success]" << endmsg;
  return persObj; 
}

McEventCollection* McEventCollectionCnv::createTransient() 
{
   MsgStream msg( msgSvc(), "McEventCollectionConverter" );

   McEventCollection *transObj = 0;

   static const pool::Guid tr_guid("6DE62B45-7C72-4539-92F2-3A8E739A4AC3");
   static const pool::Guid p1_guid("BF93438C-D1D3-4F1C-8850-EB690AB7C416");
   static const pool::Guid p2_guid("851BB1D2-1964-4B0A-B83A-6BD596CFB5E2");
   static const pool::Guid p3_guid("6FC41599-64D6-4DB9-973E-9493166F6291");
   static const pool::Guid p4_guid("C517102A-94DE-407C-B07F-09BD81F6172E");
   static const pool::Guid p5_guid("D52391A4-F951-46BF-A0D5-E407698D2917");
   static const pool::Guid p6_guid("6B78A751-B31A-4597-BFB6-DDCE62646CF9");

   // Hook to disable datapool if we are doing pileup
   bool isPileup(false);
   if(serviceLocator()->existsService("PileUpEventLoopMgr")) {
      isPileup=true;
      msg << MSG::DEBUG << "Pile run, disable datapool for McEventCollection " << endmsg;
   }

   if ( compareClassGuid(tr_guid) ) {

     // regular object from before the T/P separation
     return poolReadObject<McEventCollection>();

   } else if ( compareClassGuid(p1_guid) ) {

      std::unique_ptr<McEventCollection_p1> persObj( poolReadObject<McEventCollection_p1>() );
      McEventCollectionCnv_p1 cnv;
      transObj = cnv.createTransient( persObj.get(), msg );
   } else if ( compareClassGuid(p2_guid) ) {

      std::unique_ptr<McEventCollection_p2> persObj( poolReadObject<McEventCollection_p2>() );
      McEventCollectionCnv_p2 cnv;
      transObj = cnv.createTransient( persObj.get(), msg );
   } else if ( compareClassGuid(p3_guid) ) {

      std::unique_ptr<McEventCollection_p3> persObj( poolReadObject<McEventCollection_p3>() );
      McEventCollectionCnv_p3 cnv;
      transObj = cnv.createTransient( persObj.get(), msg ); 
   } else if ( compareClassGuid(p4_guid) ) {

      std::unique_ptr<McEventCollection_p4> persObj( poolReadObject<McEventCollection_p4>() );
      McEventCollectionCnv_p4 cnv;
      if(isPileup) cnv.setPileup();
      transObj = cnv.createTransient( persObj.get(), msg );
   } else if ( compareClassGuid(p5_guid) ) {

      std::unique_ptr<McEventCollection_p5> persObj( poolReadObject<McEventCollection_p5>() );
      McEventCollectionCnv_p5 cnv;
      if(isPileup) cnv.setPileup();
      transObj = cnv.createTransient( persObj.get(), msg );
   } else if ( compareClassGuid(p6_guid) ) {

      std::unique_ptr<McEventCollection_p6> persObj( poolReadObject<McEventCollection_p6>() );
      McEventCollectionCnv_p6 cnv;
      if(isPileup) cnv.setPileup();
      transObj = cnv.createTransient( persObj.get(), msg );
  } else {
     throw std::runtime_error("Unsupported persistent version of McEventCollection");
   }

   return transObj;
}
