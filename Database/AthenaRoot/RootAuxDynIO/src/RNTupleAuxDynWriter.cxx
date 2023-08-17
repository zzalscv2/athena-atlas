/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "RNTupleAuxDynWriter.h"
#include "AthContainers/AuxStoreInternal.h"
#include "AthContainers/AuxTypeRegistry.h"
#include "AthContainers/normalizedTypeinfoName.h"

#include "TFile.h"
#include "Compression.h"
#include "TClass.h"
#include "TVirtualCollectionProxy.h"

#include <sstream>
#include <iostream>


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
      ATH_MSG_DEBUG("Writing " << base_name << " with " << selection.size() << " Dynamic attributes");
      for(SG::auxid_t id : selection) {
         const std::string attr_type = SG::normalizedTypeinfoName( *store->getIOType(id) );
         const std::string attr_name = SG::AuxTypeRegistry::instance().getName(id);
         const std::string field_name = RootAuxDynIO::auxFieldName( attr_name, base_name );
         void* attr_data ATLAS_THREAD_SAFE = const_cast<void*>( store->getIOData(id) );

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
      if( m_attrDataMap.find(field_name) == m_attrDataMap.end() ) {
         addField(field_name, attr_type);
      }
      addFieldValue(field_name, attr_data);
#endif
   }

   /// Add a new field to the RNTuple - for now only allowed before the first write
   /// Used for data objects from RNTupleContainer, not for dynamic attributes
   void RNTupleAuxDynWriter::addField( const std::string& field_name, const std::string& attr_type )
   {
      if( m_attrDataMap.find(field_name) != m_attrDataMap.end() ) {
         throw std::runtime_error( std::string("Attempt to add existing field.  name: ")
                              + field_name + "new type: " + attr_type );
      }
      ATH_MSG_DEBUG("Adding new object column, name="<< field_name << " of type " << attr_type);
      auto field = RFieldBase::Create(field_name, attr_type).Unwrap();
      if( !m_model ) {
#if ROOT_VERSION_CODE > ROOT_VERSION( 6, 29, 0 )
         // first write was already done, need to update the model
         ATH_MSG_DEBUG("Adding late attribute " << field_name);
         auto updater = m_ntupleWriter->CreateModelUpdater();
         updater->BeginUpdate();
         updater->AddField( std::move(field) );
         updater->CommitUpdate();
#endif
      } else {
         m_model->AddField( std::move(field) );
      }
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
         // already started writing
#if ROOT_VERSION_CODE < ROOT_VERSION( 6, 27, 0 )
      if( !m_model ) {
         // MN: ROOT 6.26 version not tested
         if( !m_entry ) makeNewEntry();
         RFieldBase* field = m_ntupleFieldMap[ field_name ];
         m_entry->CaptureValue( field->CaptureValue( attr_data ) );
      }
#endif
      field_iter->second = attr_data;
      m_needsCommit = true;
   }   


   int RNTupleAuxDynWriter::commit()
   {
#if ROOT_VERSION_CODE >= ROOT_VERSION( 6, 29, 0 )
      // write only if there was data added, ignore empty commits
      if( !needsCommit() ) {
         ATH_MSG_DEBUG("Empty Commit");
         return 0;
      }
      ATH_MSG_DEBUG("Commit, row=" << m_rowN << " : " << m_ntupleName );
      if( !m_entry ) makeNewEntry();

      int num_bytes = 0;
      ATH_MSG_DEBUG(m_ntupleName << " has " <<  m_attrDataMap.size() << " attributes");
      int attrN = 0;
      for( auto& attr: m_attrDataMap ) {
         ATH_MSG_VERBOSE("Setting data ptr for field# " << ++attrN << ": " << attr.first << "  data=" << std::hex << attr.second << std::dec );
         if( !attr.second ) {
            if( m_generatedValues.find(attr.first) == m_generatedValues.end() ) {
               ATH_MSG_DEBUG("Generating default object for field: " << attr.first );
               for( auto val_i = m_entry->begin(); val_i != m_entry->end(); ++val_i ) {
                  if( val_i->GetField()->GetName() == attr.first ) {
                     m_generatedValues.insert( std::make_pair(attr.first, val_i->GetField()->GenerateValue()) );
                     break;
                  }
               }
            }
            attr.second = m_generatedValues.find(attr.first)->second.GetRawPtr();
         }
         // attach the attribute values rememberd internally
         m_entry->CaptureValueUnsafe( attr.first, attr.second );
      }
      num_bytes += m_ntupleWriter->Fill( *m_entry );
      ATH_MSG_DEBUG("Filled RNTuple Row, bytes written: " << num_bytes);

      m_entry.reset();
      // forget all values to see if any object is missing in a new commit
      for( auto& attr: m_attrDataMap ) { attr.second = nullptr; } 
      m_needsCommit = false;
      m_rowN++;

      return num_bytes;
#else
      ATH_MSG_WARNING("Commit not implemented for this ROOT version");
      return 0;
#endif
   }


   void RNTupleAuxDynWriter::close() {
      // delete the generated default fields (RField should delete the default data objest)
#if ROOT_VERSION_CODE >= ROOT_VERSION( 6, 29, 0 )
      m_generatedValues.clear();
#endif
      m_ntupleWriter.reset(); m_entry.reset(); m_model.reset(); m_rowN=0;
   }


   RNTupleAuxDynWriter::~RNTupleAuxDynWriter() {}

}// namespace



