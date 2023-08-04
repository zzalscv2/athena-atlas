/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef RNTUPLEAUXDYNWRITER_H
#define RNTUPLEAUXDYNWRITER_H

#include "AthenaBaseComps/AthMessaging.h"
#include "RootAuxDynIO/RootAuxDynIO.h"

#include "ROOT/RNTuple.hxx"
#include "ROOT/RNTupleModel.hxx"
#include "ROOT/RNTupleOptions.hxx"
#include "ROOT/RField.hxx"
#include "ROOT/REntry.hxx"

namespace SG { class IAuxStoreIO; }


namespace RootAuxDynIO
{
   using RFieldBase    = ROOT::Experimental::Detail::RFieldBase;
   using RNTupleWriter = ROOT::Experimental::RNTupleWriter;
   using RNTupleModel  = ROOT::Experimental::RNTupleModel;
   using REntry        = ROOT::Experimental::REntry;
   using RNTupleWriteOptions = ROOT::Experimental::RNTupleWriteOptions;

   
   class RNTupleAuxDynWriter : public AthMessaging, public IRNTupleWriter
   {
   public:

#if ROOT_VERSION_CODE < ROOT_VERSION( 6, 27, 0 )
      std::map<std::string, RFieldBase*>  m_ntupleFieldMap;
#endif
      // store data ptr for the first row, when only creating the model
      std::map<std::string, void*>        m_attrDataMap;

      std::unique_ptr<RNTupleModel>       m_model = RNTupleModel::Create();
      std::unique_ptr<REntry>             m_entry;
      std::unique_ptr<RNTupleWriter>      m_ntupleWriter;

      std::string          m_ntupleName;
      TFile*               m_tfile;
      RNTupleWriteOptions  m_opts;
      int                  m_rowN = 0;
      /// Count how many APR Containers are writing to this RNTuple (more than one makes a Group)
      int                  m_clients = 0;
      bool                 m_needsCommit = false;

#if ROOT_VERSION_CODE >= ROOT_VERSION( 6, 29, 0 )
      /// default-constructed objects to fill out blanks
      std::map<std::string, RFieldBase::RValue> m_generatedValues;
#endif

      RNTupleAuxDynWriter(TFile* file, const std::string& ntupleName, int compression);

      /// Create a new empty RNTuple row with the current model (fields)
      void  makeNewEntry();

      /// handle writing of dynamic xAOD attributes of an object - called from Container::writeObject()
      //  throws exceptions
      virtual int writeAuxAttributes( const std::string& base_branch, SG::IAuxStoreIO* store, size_t /*rows_written*/ ) override final;

      /// Add a new field to the RNTuple, collect the data pointer for the commit
      void addAttribute( const std::string& field_name, const std::string& attr_type, void* attr_data );

      /// Add a new field to the RNTuple
      virtual void addField( const std::string& field_name, const std::string& attr_type ) override;

      /// Supply data address for a given field
      virtual void addFieldValue( const std::string& field_name, void* attr_data ) override;

      virtual int commit() override final;

      virtual const std::string& getName() const override { return m_ntupleName; }

      virtual size_t size() const override { return m_rowN; }

      /// Check if any data needs to be committed
      virtual bool needsCommit() const override final { return m_needsCommit; }

      /// Is this RNTuple used by more than one APR container?
      virtual bool isGrouped() const override final { return m_clients > 1; }

      /// Keep track of how many APR containers are writing to this RNTuple
      virtual void increaseClientCount() override final { m_clients++; }

      virtual void close() override;
      
      virtual ~RNTupleAuxDynWriter();
   };

}// namespace
#endif

