/*
 *   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
 *   */

// Local implementation files
#include "StorageSvc/DbOption.h"
#include "RootDatabase.h"
#include "RootTreeIndexContainer.h"

// Root include files
#include "TTree.h"
#include "TBranch.h"
#include "TTreeIndex.h"

using namespace pool;

RootTreeIndexContainer::RootTreeIndexContainer() :
   m_indexBranch(nullptr), m_index_entries(0), m_index_multi( getpid() ), m_index(0), m_indexBump(0),
   m_firstRead(true)
{ }


DbStatus RootTreeIndexContainer::open( DbDatabase& dbH, 
                                       const std::string& nam,
                                       const DbTypeInfo* info,
                                       DbAccessMode mod)
{
   auto db = static_cast<const RootDatabase*>( dbH.info() );
   m_indexBump = db? db->currentIndexMasterID() : 0;
   return  RootTreeContainer::open( dbH, nam, info, mod );
}


long long int RootTreeIndexContainer::nextRecordId()
{
   long long int s = m_index_multi;
   s = s << 32;
   if (m_indexBranch == nullptr ) {
      s += RootTreeContainer::nextRecordId();
   } else {
      s += std::max(m_index_entries + DbContainerImp::size(), RootTreeContainer::nextRecordId());
   }
   return s + m_indexBump;
}

void RootTreeIndexContainer::useNextRecordId(long long int nextID)
{
   // Find out how this TTree index is behind the master index in the DB
   m_indexBump = m_indexBranch? nextID - m_rootDb->indexSize(m_indexBranch) : nextID;
   if( m_indexBump < 0 ) {
      // Seems this index is ahead of the master, cannot sync
      m_indexBump = 0;
   }
}


DbStatus RootTreeIndexContainer::writeObject(ActionList::value_type& action)
{
   // Prepare for writing - grab/create the index branch
   if( !m_indexBranch ) {
      m_indexBranch = m_tree->GetBranch("index_ref");
      if( !m_indexBranch ) {
         m_indexBranch = m_tree->Branch("index_ref", &m_index);
      }
   }
   if( m_indexBranch && m_index_entries >= m_rootDb->indexSize(m_indexBranch) ) {
      // need to update the index branch
      m_index = action.link.second;
      m_indexBranch->SetAddress(&m_index);
      if (isBranchContainer() && !m_treeFillMode) m_indexBranch->Fill();
      m_rootDb->indexSizeInc(m_indexBranch);
   }
   m_index_entries++;

   return RootTreeContainer::writeObject(action);
}


DbStatus RootTreeIndexContainer::loadObject(void** ptr, ShapeH shape, Token::OID_t& oid)
{
   if( (oid.second >> 32) > 0 ) {
      if( m_firstRead ) {
         // on the first read check if the index can and should be rebuilt
         if( m_tree->GetEntries()>0 and m_tree->GetBranch("index_ref")
             and !m_rootDb->wasIndexRebuilt(m_tree->GetName()) ) {
            delete m_tree->GetTreeIndex();
            m_tree->BuildIndex("index_ref");
            m_rootDb->markIndexRebuilt(m_tree->GetName());
         }
         m_firstRead = false;
      }
      long long int evt_id = m_tree->GetEntryNumberWithIndex(oid.second);
      if (evt_id == -1) {
         delete m_tree->GetTreeIndex();
         m_tree->BuildIndex("index_ref");
         evt_id = m_tree->GetEntryNumberWithIndex(oid.second);
      }
      if (evt_id >= 0) {
         oid.second = evt_id;
      }
   }
   return RootTreeContainer::loadObject(ptr, shape, oid);
}
