/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "RNTupleAuxDynWriter.h"
#include "AthContainers/AuxStoreInternal.h"
#include "AthContainers/AuxTypeRegistry.h"
#include "AthContainers/normalizedTypeinfoName.h"

#include "TFile.h"

#include "TInterpreter.h"
#include <sstream>
#include <iostream>

#include "Compression.h"

#include "TClass.h"
#include "TVirtualCollectionProxy.h"


namespace RootAuxDynIO
{

   RNTupleAuxDynWriter::RNTupleAuxDynWriter(TFile* file, const std::string& ntupleName, int compression) :
         AthMessaging(std::string("RNTupleAuxDynWriter[")+ntupleName+"]"),
         m_ntupleName( ntupleName ),
         m_tfile( file )
      {
#if ROOT_VERSION_CODE < ROOT_VERSION( 6, 27, 0 )
         m_entry = std::make_unique<REntry>();
#endif
         m_opts.SetCompression( compression );
         m_model->SetDescription( ntupleName );
      }


   void  RNTupleAuxDynWriter::makeNewEntry() {
#if ROOT_VERSION_CODE < ROOT_VERSION( 6, 27, 0 )
      m_entry = std::make_unique<REntry>();
#else
      if( m_model ) {
         // prepare for writing of the first row
         if( !m_tfile ) {
            throw std::runtime_error( std::string("Attempt to write RNTuple ") + m_ntupleName + " without valid TFile ptr" );
         } else {
            // write into existing file
            ATH_MSG_DEBUG("Creating RNTuple " << m_tfile->GetName() << "/" << m_ntupleName);
            m_ntupleWriter = RNTupleWriter::Append(std::move(m_model), m_ntupleName, *m_tfile, m_opts);
         }
      }
      m_entry = m_ntupleWriter->GetModel()->CreateBareEntry();
#endif
   }


   /// handle writing of dynamic xAOD attributes of an object - called from RootTreeContainer::writeObject()
   //  throws exceptions
   int RNTupleAuxDynWriter::writeAuxAttributes( const std::string& base_name, SG::IAuxStoreIO* store, size_t /*rows_written*/ )
   {
      const SG::auxid_set_t selection = store->getSelectedAuxIDs();
      ATH_MSG_VERBOSE("Writing " << base_name << " with " << selection.size() << " Dynamic attributes");

      for(SG::auxid_t id : selection) {
         const std::type_info* attr_typeinfo = store->getIOType(id);
         const std::string attr_type = SG::normalizedTypeinfoName( *attr_typeinfo );
         const std::string attr_name = SG::AuxTypeRegistry::instance().getName(id);
         void* attr_data ATLAS_THREAD_SAFE = const_cast<void*>( store->getIOData(id) );
         const std::string field_name = RootAuxDynIO::auxFieldName( attr_name, base_name );

         // Ignore 'late' AuxDyn attributes until beckfilling is implemented
         static std::set<std::string> goodBranches ATLAS_THREAD_SAFE, badBranches ATLAS_THREAD_SAFE;
         if( m_model )  goodBranches.insert(field_name);
         else {
            if( goodBranches.count(field_name) == 0 ) {
               if( badBranches.count(field_name) == 0 ) {
                  badBranches.insert(field_name);
                  ATH_MSG_WARNING("ignoring late attribute " << field_name);
               }
               continue;
            }
         }
         addAttribute( field_name, attr_type, attr_data );
      }
      return 0;  // MN: can get bytes written only when calling Fill() at commit
   }


   void RNTupleAuxDynWriter::addAttribute( const std::string& field_name, const std::string& attr_type, void* attr_data )
   {
#if ROOT_VERSION_CODE < ROOT_VERSION( 6, 27, 0 )
      if( !m_entry ) makeNewEntry();
      if( m_model ) {
         // first event - create Fields and update NTuple Model
         ATH_MSG_VERBOSE( "MN: adding attribute to NTuple, name: " <<  field_name << " type: " << attr_type << "   |ptr=" << attr_data);
         auto field = RFieldBase::Create(field_name, attr_type).Unwrap();
         m_entry->CaptureValue( field->CaptureValue( attr_data ) );
         m_ntupleFieldMap[ field_name ] = field.get();
         m_model->AddField( std::move(field) );
      }
      else {
         // NTupleWriter and Fields already created
         RFieldBase* field = m_ntupleFieldMap[ field_name ];
         m_entry->CaptureValue( field->CaptureValue( attr_data ) );
      }
#else
      if( m_model ) {
         // first event - only update NTuple Model and store tha data pointer locally
         // fill the RNTuple entry when the entire model is defined (in writeEntry)
         m_attrDataMap[ field_name ] = attr_data;
         auto field = RFieldBase::Create(field_name, attr_type).Unwrap();
         m_model->AddField( move(field) );
      }
      else {
         addFieldValue(field_name, attr_data);
      }
#endif
   }


   /// Add a new field to the RNTuple - for now only allowed before the first write
   void RNTupleAuxDynWriter::addField( const std::string& field_name, const std::string& attr_type )
   {
      if( !m_model ) {
         // first write was already done, cannot add new fields any more
         throw std::runtime_error( std::string("Attempt to add new field to RNTuple after the first write. name: ")
                              + field_name + " type: " + attr_type );
      }
      if( m_attrDataMap.find(field_name) != m_attrDataMap.end() ) {
         throw std::runtime_error( std::string("Attempt to add existing field.  name: ")
                              + field_name + "new type: " + attr_type );
      }
      auto field = RFieldBase::Create(field_name, attr_type).Unwrap();
      m_model->AddField( move(field) );
      m_attrDataMap[ field_name ] = nullptr;
   }


   /// Supply data address for a given field
   void RNTupleAuxDynWriter::addFieldValue( const std::string& field_name, void* attr_data )
   {
      auto field_iter = m_attrDataMap.find(field_name);
      if( field_iter == m_attrDataMap.end() ) {
         std::stringstream msg;
         msg <<"Attempt to write unknown Field with name: '" << field_name << std::ends;
         throw std::runtime_error( msg.str() );
      }
      if( !m_model ) {
         // already started writing
         if( !m_entry ) makeNewEntry();
#if ROOT_VERSION_CODE >= ROOT_VERSION( 6, 27, 0 )
         m_entry->CaptureValueUnsafe( field_name, attr_data );
#else
         // MN: ROOT 6.26 version not tested
         RFieldBase* field = m_ntupleFieldMap[ field_name ];
         m_entry->CaptureValue( field->CaptureValue( attr_data ) );
#endif
      }
      field_iter->second = attr_data;
   }   


   int RNTupleAuxDynWriter::commit()
   {
      ATH_MSG_VERBOSE("Commit");
#if ROOT_VERSION_CODE >= ROOT_VERSION( 6, 27, 0 )
      if( m_model ) {
         makeNewEntry();
         // attach the attribute values rememberd internally during model creation
         for( const auto& attr: m_attrDataMap ) {
            m_entry->CaptureValueUnsafe( attr.first, attr.second );
         }
      }

      // write only if there was data added, ignore empty commits
      int num_bytes = 0;
      if( m_entry ) {
         for( auto& attr: m_attrDataMap ) {
            if( !attr.second ) {
               // MN: the default object created here needs to be deleted - should use REntry::AddValue()
               attr.second = m_entry->GetValue(attr.first).GetField()->GenerateValue().GetRawPtr();
               m_entry->CaptureValueUnsafe( attr.first, attr.second );
            } else {
               // the value here was already added in addFieldValue, now just clear the "flag"
               attr.second = nullptr;
            }
         }
         num_bytes += m_ntupleWriter->Fill( *m_entry );
         m_entry.reset();
         for( auto& field : m_attrDataMap ) field.second = nullptr;
         m_rowN++;
      }

      return num_bytes;
#else
      return 0;
#endif
   }


   void RNTupleAuxDynWriter::close() {
      m_ntupleWriter.reset(); m_entry.reset(); m_model.reset(); m_rowN=0;
   }

   RNTupleAuxDynWriter::~RNTupleAuxDynWriter() {}

}// namespace
