/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef RNTUPLEAUXDYNREADER_H
#define RNTUPLEAUXDYNREADER_H

#include "AthenaBaseComps/AthMessaging.h"
#include "AthContainers/AuxStoreInternal.h" 
#include "RootAuxDynIO/RootAuxDynIO.h" 

#include <map>
#include <string>

class TClass;

namespace RootAuxDynIO
{

   class RNTupleAuxDynReader : public AthMessaging, public IRootAuxDynReader
   {
   public :

      struct FieldInfo
      {
         enum Status { NotInitialized, Initialized, TypeError, NotFound };

         TClass*       tclass = 0;
         TClass*       SE_tclass = 0;
    
         bool          isPackedContainer = false;
         bool          needsSE = false;
         enum Status   status = NotInitialized;

         SG::auxid_t   auxid;
         std::string   attribName;
         std::string   fieldName;
         std::string   fieldType;
      };

   
      RNTupleAuxDynReader(const std::string& store_key, RNTupleReader* rntReader);

      void init(bool standalone);

      virtual void addReaderToObject(void* object, size_t ttree_row, std::recursive_mutex* iomtx = nullptr ) override final;

      virtual const SG::auxid_set_t& auxIDs() const override final;

      void addBytes(size_t bytes);

      virtual size_t getBytesRead() const override final;

      virtual void resetBytesRead() override final;

      const FieldInfo& getFieldInfo(const SG::auxid_t& auxid, const SG::AuxStoreInternal& store);

      virtual ~RNTupleAuxDynReader() {}

   protected:
      // auxids that could be found in registry for attribute names from the file
      SG::auxid_set_t                       m_auxids;
  
      std::string                           m_storeFieldName;
      // counter for bytes read
      size_t                                m_bytesRead = 0;
      // offset of the AxuStoreHolder base class in the objects read by the Reader
      int                                   m_storeHolderOffset = -1;
      bool                                  m_initialized = false;
      std::string                           m_key;

      // map auxid -> branch info. not sure if it can be different from m_branchMap
      std::map<SG::auxid_t, FieldInfo>     m_fieldInfos;

      RNTupleReader*                       m_ntupleReader;
   };



   inline void  RNTupleAuxDynReader::addBytes(size_t bytes) {
      m_bytesRead += bytes;
   }

   inline size_t RNTupleAuxDynReader::getBytesRead() const {
      return m_bytesRead;
   }

   inline void RNTupleAuxDynReader::resetBytesRead() {
      m_bytesRead = 0;
   }

   inline const SG::auxid_set_t& RNTupleAuxDynReader::auxIDs() const {
      return m_auxids;
   }

} //namespace
#endif
