/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/


#include "GaudiKernel/MsgStream.h"

#include "JetEvent/JetKeyDescriptor.h"

#include <algorithm>

// DO NOT change these lines EVER!!!!!
const JetKeyConstants::key_t JetKeyConstants::ShapeCat = "JetShapes";
const JetKeyConstants::key_t JetKeyConstants::AssoCat = "JetAssociations";
const JetKeyConstants::key_t JetKeyConstants::TagCat = "JetTags";
const JetKeyConstants::key_t JetKeyConstants::InfoCat = "JetInfo";

const size_t                                          JetKeyDescriptorInstance::m_invalid  = size_t(-1);
const std::string                                     JetKeyDescriptorInstance::m_notFound = "unknown";
const std::vector<JetKeyDescriptorInstance::key_t>    JetKeyDescriptorInstance::m_invalidKeys;
const bool                                            JetKeyDescriptorInstance::m_persistified = false;

JetKeyDescriptorInstance::JetKeyDescriptorInstance(bool create) :
  m_Stores(nullptr),
  m_ConstStores(nullptr)
{
  // create a JetKeyDescriptorInstance :
  if (create) createKeyStore();
}

JetKeyDescriptorInstance::~JetKeyDescriptorInstance()
= default;

void JetKeyDescriptorInstance::createKeyStore() const {
   if (m_ConstStores) {
     throw std::runtime_error ("Attempt to modify const JetKeyDescriptor");
   }
   m_Stores = new JetKeyDescriptor();
   m_ConstStores = m_Stores;
}  

JetKeyDescriptor* JetKeyDescriptorInstance::getKeyStore(){
  if( ! bool(m_Stores) ) createKeyStore();
  return m_Stores;
}

size_t JetKeyDescriptorInstance::getIndex(const category_t& cat,
					  const key_t&      key,
					  bool createIfMissing)
{
  //std::cout <<" getIndex cat="<<cat<<"  key="<<key<<"   has store ="<< m_Stores << "  if missing="<< createIfMissing << std::endl;
  if ( ! bool(m_ConstStores) ) {
    if ( createIfMissing )
      createKeyStore();
    else
      return m_invalid;
  }

  //  std::cout <<& m_Stores->m_catStore << " Cat store size="<< m_Stores->m_catStore.size() << std::endl;
  // first find category
  catlist_t::const_iterator
    fCat(std::find(m_ConstStores->m_catStore.begin(),m_ConstStores->m_catStore.end(),cat));
  if (  fCat == m_ConstStores->m_catStore.end() ) {
    if ( createIfMissing )
      {
        if (!m_Stores) {
          createKeyStore();
        }
        // cppcheck-suppress nullPointerRedundantCheck; false positive
	m_Stores->m_catStore.push_back(cat);
	keystore_t kStore; kStore.push_back(key);
        // cppcheck-suppress nullPointerRedundantCheck; false positive
	m_Stores->m_keyStore.push_back(kStore);
	return 0;
      }
    else
      return m_invalid;
  }
  else
    {
      size_t iCat = fCat - m_ConstStores->m_catStore.begin();

      if ( iCat < m_ConstStores->m_keyStore.size() )  // really impossible!
	{
	  keystore_t::const_iterator fKey(std::find((m_ConstStores->m_keyStore[iCat]).begin(),
                                                    (m_ConstStores->m_keyStore[iCat]).end(),
                                                    key));
	  if ( fKey == (m_ConstStores->m_keyStore[iCat]).end() )
	    if ( createIfMissing )
	      {
                if (!m_Stores) {
                  createKeyStore();
                }
                // cppcheck-suppress nullPointerRedundantCheck; false positive
		(m_Stores->m_keyStore[iCat]).push_back(key);
                // cppcheck-suppress nullPointerRedundantCheck; false positive
		return (m_Stores->m_keyStore[iCat]).size()-1;
	      }
	    else
	      return m_invalid;
	  else
	    {
	      return fKey - (m_ConstStores->m_keyStore[iCat]).begin();
	    }
	}
      // should never be reached!
      return m_invalid;
    }
}

size_t JetKeyDescriptorInstance::getIndex(const category_t& cat,
					  const key_t&      key) const
{
  if ( ! bool(m_ConstStores) ) {
    return m_invalid;
  }

  // first find category
  catlist_t::const_iterator
    fCat(std::find(m_ConstStores->m_catStore.begin(),m_ConstStores->m_catStore.end(),cat));
  if (  fCat == m_ConstStores->m_catStore.end() )
    return m_invalid;
  else
    {
      size_t iCat = fCat - m_ConstStores->m_catStore.begin();

      if ( iCat < m_ConstStores->m_keyStore.size() )  // really impossible!
	{
	  keystore_t::const_iterator fKey(std::find((m_ConstStores->m_keyStore[iCat]).begin(),
                                                    (m_ConstStores->m_keyStore[iCat]).end(),
                                                    key));
	  if ( fKey == (m_ConstStores->m_keyStore[iCat]).end() )
	      return m_invalid;
	  else
	    {
	      return fKey - (m_ConstStores->m_keyStore[iCat]).begin();
	    }
	}
      // should never be reached!
      return m_invalid;
    }
}

void JetKeyDescriptorInstance::printOut(MsgStream& msgStream) const
{
  if ( ! bool(m_ConstStores) ) return;

  msgStream << "Number of categories: " << m_ConstStores->m_catStore.size() << endmsg;
  for ( size_t i=0;i<m_ConstStores->m_catStore.size();i++ )
    {
      msgStream << m_ConstStores->m_catStore[i] << " Number of keys: "
        	<< (m_ConstStores->m_keyStore[i]).size() << endmsg;
      for ( size_t j=0;j<(m_ConstStores->m_keyStore[i]).size();j++ )
        {
          msgStream << "    index: " << j << " key <" << (m_ConstStores->m_keyStore[i])[j]
        	    << ">" << endmsg;
        } 	      
    }
}

const JetKeyDescriptorInstance::key_t& JetKeyDescriptorInstance::getKey(const category_t& cat,
									size_t index) const
{
  if ( ! bool(m_ConstStores) ) createKeyStore();
  // find category
  catlist_t::const_iterator fCat(std::find(m_ConstStores->m_catStore.begin(),
					   m_ConstStores->m_catStore.end(),
					   cat));
  if ( fCat == m_ConstStores->m_catStore.end() ) return m_notFound;
  // find key
  size_t iCat(fCat-m_ConstStores->m_catStore.begin());
  return index < (m_ConstStores->m_keyStore[iCat]).size()
    ? (m_ConstStores->m_keyStore[iCat])[index]
    : m_notFound;
}

const std::vector<JetKeyDescriptorInstance::key_t>& 
JetKeyDescriptorInstance::getKeys(const category_t& cat) const
{
  if ( ! bool(m_ConstStores) )
    return m_invalidKeys;
  catlist_t::const_iterator fCat(std::find(m_ConstStores->m_catStore.begin(),
					   m_ConstStores->m_catStore.end(),
					   cat));
  if ( fCat != m_ConstStores->m_catStore.end() )
    {
      size_t aInd(fCat-m_ConstStores->m_catStore.begin());
      return m_ConstStores->m_keyStore[aInd];
    }
  else
    {
      return m_invalidKeys;
    }
}

// empty constructor for the converter
JetKeyDescriptor::JetKeyDescriptor()
{   
  // this constructor is called only from deregister
  //(JetKeyDescriptorInstance::instance())->m_Stores = this;

  // Set the first Author entry to be NoAuthor
  JetKeyDescriptorInstance descInst(false);
  descInst.m_Stores = this; // connect to this store
  descInst.m_ConstStores = this; // connect to this store
  descInst.getIndex(JetKeyConstants::InfoCat,"NoAuthor",true);

  // notes about the above lines :
  // The line is safe w.r.t T/P when reading file written before it is implemented
  // (because the Cnv clears everything before reading the pers object).
  //  However in such cases, jets created after such reading will have their jetAuthor()
  //  pointing to an arbitrary string (usualy "AntiKt6H1Tower") rather than the now expected "NoAuthor"
  
}


JetKeyDescriptor::~JetKeyDescriptor()
{
  // here, now delete the pointer to the data of the  instance...
  if ( (JetKeyDescriptorInstance::instance())->m_Stores == this ) 
    (JetKeyDescriptorInstance::instance())->m_Stores = nullptr;
  if ( (JetKeyDescriptorInstance::instance())->m_ConstStores == this ) 
    (JetKeyDescriptorInstance::instance())->m_ConstStores = nullptr;
}

