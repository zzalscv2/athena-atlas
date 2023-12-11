///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// JetCollectionCnv_p4.cxx 
// Implementation file for class JetCollectionCnv_p4
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 

// DataModel includes
#include "AthAllocators/DataPool.h"

// JetEvent includes
#include "JetEvent/Jet.h"
#include "JetEvent/JetCollection.h"


// JetEventTPCnv includes
#include "JetEventTPCnv/JetCnv_p4.h"
#include "JetEventTPCnv/JetCollection_p4.h"
#include "JetEventTPCnv/JetCollectionCnv_p4.h"
#include "JetEventTPCnv/JetKeyDescriptorCnv_p1.h"

// DataModelAthenaPool includes
#include "DataModelAthenaPool/DataLinkCnv_p1.h"

#include <sstream>

// preallocate converters
static const JetCnv_p4 jetCnv;
static const JetKeyDescriptorCnv_p1 jetkeyCnv;


void 
JetCollectionCnv_p4::persToTrans( const JetCollection_p4* pers, 
                                  JetCollection* trans, 
                                  MsgStream& msg ) const
{
  msg << MSG::DEBUG << "Loading JetCollection from persistent state..."
      << endmsg;
  
  // make sure to first read the JetKeyDescriptor
  DataLinkCnv_p1<DataLink<JetKeyDescriptor> > JetKeyStoreCnv;
  JetKeyStoreCnv.persToTrans( &pers->m_keyStore, &trans->m_keyStore, msg );
  // link the JetKeyDescriptorInstance to the store:
  if( trans->m_keyStore.isValid() ){
    trans->keyDesc()->m_Stores  = nullptr;
    trans->keyDesc()->m_ConstStores  = trans->m_keyStore.cptr();
    // make sure the global instance is pointing to this jetkey store
    JetKeyDescriptorInstance::instance()->m_Stores =  nullptr;
    JetKeyDescriptorInstance::instance()->m_ConstStores =  trans->keyDesc()->m_ConstStores;
  }
  else if (trans->m_keyStore.isDefault()) {
    DataLink<JetKeyDescriptor> dl ("JetKeyMap");
    if (dl.isValid()) {
      trans->keyDesc()->m_Stores  = nullptr;
      trans->keyDesc()->m_ConstStores  = dl.cptr();
      // make sure the global instance is pointing to this jetkey store
      JetKeyDescriptorInstance::instance()->m_Stores =  nullptr;
      JetKeyDescriptorInstance::instance()->m_ConstStores =  trans->keyDesc()->m_ConstStores;
    }
    else {
      trans->keyDesc()->m_Stores  =
        JetKeyDescriptorInstance::instance()->m_Stores;
      trans->keyDesc()->m_ConstStores  =
        JetKeyDescriptorInstance::instance()->m_ConstStores;
    }
  }
  else {
    trans->keyDesc()->m_Stores  = JetKeyDescriptorInstance::instance()->m_Stores;
    trans->keyDesc()->m_ConstStores  = JetKeyDescriptorInstance::instance()->m_ConstStores;
  }

  msg <<  MSG::DEBUG << "attached JetKeyDescriptor to its instance" << endmsg;
  
  /// Individual items

  trans->setOrdered (static_cast<JetCollection::OrderedVar>(pers->m_ordered));
  // the transient version does not have this data member any more,
  // each jet knows its ROI
  //  trans->m_ROIauthor = //pers->m_roiAuthor;

  /// The jets themselves. Taken care of behind our backs.

  trans->clear();
  trans->reserve(pers->size());

  for (const TPObjRef& ref : *pers) {
    trans->push_back(createTransFromPStore((ITPConverterFor<Jet>**)nullptr, ref, msg));
  }
  
  // now each jet knows its ROI
  unsigned int RoIWord;
  std::stringstream strm(pers->m_roiAuthor);
  strm >> RoIWord;
  if( strm.good() )
    {
      msg <<  MSG::DEBUG << "Note: This jet collection uses RoIWords!" << endmsg;
      for( JetCollection::iterator itr = trans->begin(); itr != trans->end(); ++itr )
	(*itr)->set_RoIword( RoIWord );
    }
  
  msg << MSG::DEBUG << "Loading JetCollection from persistent state [OK]"
      << endmsg;
}

void 
JetCollectionCnv_p4::transToPers( const JetCollection* trans, 
                                  JetCollection_p4* pers, 
                                  MsgStream& msg ) const
{
//   msg << MSG::DEBUG << "Creating persistent state of JetCollection..."
//       << endmsg;

  pers->m_ordered   = static_cast<short>(trans->ordered());
  //pers->m_roiAuthor = trans->m_ROIauthor;

  pers->clear();
  pers->reserve(trans->size());

  for (const Jet* jet : *trans) {
    pers->push_back(toPersistent((ITPConverterFor<Jet>**)nullptr, jet, msg));
  }

  // RS now deal with the JetKeyDescriptor
  DataLinkCnv_p1<DataLink<JetKeyDescriptor> > JetKeyStoreCnv;
  JetKeyStoreCnv.transToPers( &trans->m_keyStore, &pers->m_keyStore, msg );
}

