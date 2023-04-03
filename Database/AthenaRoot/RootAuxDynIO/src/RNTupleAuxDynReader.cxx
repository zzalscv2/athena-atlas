/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "AthContainersInterfaces/IAuxStoreHolder.h"
#include "AthContainersInterfaces/AuxDataOption.h"
#include "AthContainers/AuxTypeRegistry.h"
#include "AthContainers/tools/error.h"
#include "AthContainers/exceptions.h"
#include "RootUtils/Type.h"

#include "RNTupleAuxDynReader.h"
#include "RNTupleAuxDynStore.h"
#include "AthContainersRoot/getDynamicAuxID.h"

#include "TBranch.h"
#include "TClass.h"
#include "TClassTable.h"
#include "TClassEdit.h"
#include "TVirtualCollectionProxy.h"
#include "TROOT.h"
#include "TDictAttributeMap.h"


#include <ROOT/RNTupleModel.hxx>
#include <ROOT/RNTuple.hxx>
#include <ROOT/REntry.hxx>
using RNTupleModel = ROOT::Experimental::RNTupleModel;
using RNTupleReader = ROOT::Experimental::RNTupleReader;
using REntry = ROOT::Experimental::REntry;
using RFieldBase = ROOT::Experimental::Detail::RFieldBase;
using std::string;

namespace {

/**
 * @brief Given RNTuple field typename, return the aux data type
 * @param standalone     Is this for a standalone object?
 * @param[out] elementTypeName Name of the type for one aux data element.
 *                             Same as storageTypeName if @c standalone is true.
 * @param      storageTypeName Name of the type used for I/O
 *
 * If standalone is true, then we return the extracted type
 * directly.  Otherwise, the type should be an STL vector;
 * we return the type of the vector's payload.
 *
 * Returns 0 on failure.
 */
const std::type_info*
getAuxElementType( bool standalone, std::string& elementTypeName, const std::string& storageTypeName)
{
   if (standalone) {
      elementTypeName = storageTypeName;
      // Need to stay with 'long long' for [mc]eventNumber as defined in the EventInfo class
      if( elementTypeName == "std::uint64_t") return &typeid(unsigned long long);
      return RootUtils::Type(storageTypeName).getTypeInfo();
   }

   // Not standalone - branch should be a vector.
   if( storageTypeName.rfind("vector<", 0) == 0
       || storageTypeName.rfind("std::vector<", 0) == 0 ) {
      const TClass *tclass = RootUtils::Type(storageTypeName).getClass();
      if( !tclass ) return nullptr;
      TVirtualCollectionProxy* proxy = tclass->GetCollectionProxy(); 
      if( !proxy ) return nullptr;
      if( proxy->GetValueClass() ) {
         elementTypeName = proxy->GetValueClass()->GetName();
         return proxy->GetValueClass()->GetTypeInfo();
      }
      RootUtils::Type elemtype( proxy->GetType() );
      elementTypeName = elemtype.getTypeName();
      return elemtype.getTypeInfo();
   }
   else
      if( storageTypeName.rfind("SG::PackedContainer<", 0) == 0) {
         elementTypeName.clear();
         {
            // Protect against data race inside TClassEdit.
            // https://github.com/root-project/root/issues/10353
            // Should be fixed in root 6.26.02.
            R__WRITE_LOCKGUARD(ROOT::gCoreMutex);
            TClassEdit::TSplitType split( storageTypeName.c_str() );
            if( split.fElements.size() > 1 ) {
               elementTypeName = split.fElements[1];
            }
         }
         if( !elementTypeName.empty() ) {
            return RootUtils::Type(elementTypeName).getTypeInfo();
         }
      }
   return 0;
}


SG::auxid_t
getAuxIdForAttribute(const std::string& attr_name, const std::string& attr_type, bool standalone)
{
   SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();
   SG::auxid_t auxid = r.findAuxID(attr_name);
   if(auxid != SG::null_auxid)
      return auxid;
   
   string element_type;
   const std::type_info* ti = getAuxElementType(standalone, element_type, attr_type);
   if( !ti )
      return auxid;

   return SG::getDynamicAuxID (*ti, attr_name, element_type, attr_type, standalone);
}

} // anonymous namespace


namespace RootAuxDynIO
{
// New RNTupleReader for attributes of an AuxContainer stored in Field 'field_name'
   RNTupleAuxDynReader::RNTupleAuxDynReader(const std::string& field_name, RNTupleReader* rntReader) :
              AthMessaging(std::string("RNTupleAuxDynReader[")+field_name+"]"),
              m_storeFieldName(field_name),
              m_ntupleReader( rntReader )
   {
      const RFieldBase *cont_field = m_ntupleReader->GetModel()->GetField(field_name);
      if( cont_field ) {
         std::string field_type = cont_field->GetType();
         std::string field_prefix = field_type + "_";
         if( field_name.rfind( field_type, 0 ) != std::string::npos ) {
            m_key = field_name.substr( field_type.size()+1 );
         }
         ATH_MSG_VERBOSE("field name=" << field_name << "  field_prefix=" << field_prefix << "  key=" << m_key);
         TClass *tc = TClass::GetClass( cont_field->GetType().c_str() );
         if( tc ) {
            TClass *storeTC = tc->GetBaseClass("SG::IAuxStoreHolder");
            if( storeTC ) {
               m_storeHolderOffset = tc->GetBaseClassOffset( storeTC );
            } else {
               throw std::runtime_error(string("Class ") + tc->GetName() +" does not implement SG::IAuxStoreHolder");
            }
         } else {
            throw std::runtime_error(string("Class ") + field_type +" could not be found");
         }
      } else {
         throw std::runtime_error( string("Field ") + field_name + " could not be found in RNTuple: "
                                   + m_ntupleReader->GetModel()->GetDescription() );
      }
   }


// Has to be a separate method because 'standalone' status is not know at construction time
// Prepare all Field infos for dynamic attributes (auxids and types)
   void RNTupleAuxDynReader::init( bool standalone )
   {
      if( m_initialized )  return;
   
      const SG::AuxTypeRegistry& reg = SG::AuxTypeRegistry::instance();
      const string field_prefix = m_storeFieldName + ':';
      REntry *entry = m_ntupleReader->GetModel()->GetDefaultEntry();
      for( auto iValue = entry->begin(); iValue != entry->end(); ++iValue ) {
         string field_name = iValue->GetField()->GetName();
         if( field_name.rfind(field_prefix,0) == 0 ) {
            const string attr_infile = field_name.substr(field_prefix.size());
            const string attr_name = reg.inputRename(m_key, attr_infile);
            string field_type = iValue->GetField()->GetType();

            SG::auxid_t auxid = getAuxIdForAttribute(attr_name, field_type, standalone);
            // add AuxID to the list
            // May still be null if we don't have a dictionary for the branch.
            if( auxid != SG::null_auxid ) {
               m_auxids.insert(auxid);
            } else {
               errorcheck::ReportMessage msg (MSG::WARNING, ERRORCHECK_ARGS, "RNTupleAuxDynReader::init");
               msg << "Could not find auxid for " << attr_infile << " type: " << field_type
                   << "  standalone=" << standalone;
            }
         } 
      }
      m_initialized = true;
   }
  
// Called by the AuxStore when it is reading new attribute data from the file
// All information is cached in a FieldInfo object for better performance
   const RNTupleAuxDynReader::FieldInfo&
   RNTupleAuxDynReader::getFieldInfo(const SG::auxid_t& auxid, const SG::AuxStoreInternal& store)
   {
      FieldInfo& fieldInfo = m_fieldInfos[auxid];
      if( fieldInfo.status == FieldInfo::NotInitialized )
      {
         SG::AuxTypeRegistry& reg = SG::AuxTypeRegistry::instance();
         fieldInfo.auxid = auxid;
         fieldInfo.attribName = reg.getName(auxid);
         // Don't match this attribute if it's been renamed.
         // For example, suppose we've renamed attribute `foo' to `foo_old',
         // and someone then comes and asks for `foo'.
         // `foo' will not be found in the m_fieldMap test below
         // (`foo_old' will be in this map).  However, in the following
         // else clause, we'll recreate the field name from `foo'.
         // This field exists (renaming is only in the transient store),
         // so if we didn't have the condition here, then we'd then
         // make a `foo' attribute from that field.
         if (reg.inputRename (m_key, fieldInfo.attribName) != fieldInfo.attribName) {
            fieldInfo.status = FieldInfo::NotFound;
            return fieldInfo;
         }

         const string field_prefix = m_storeFieldName + ':';
         REntry *entry = m_ntupleReader->GetModel()->GetDefaultEntry();
         for( auto iValue = entry->begin(); iValue != entry->end(); ++iValue ) {
            auto fieldName  = iValue->GetField()->GetName();
            if( fieldName.rfind(field_prefix,0) == 0 ) {
               const string attr_infile = fieldName.substr(field_prefix.size());
               const string attr_name = reg.inputRename(m_key, attr_infile);
               if( attr_infile == fieldInfo.attribName ) {
                  fieldInfo.fieldName = fieldName;
                  fieldInfo.fieldType = iValue->GetField()->GetType();
                  break;
               }
            }
         }
         if( fieldInfo.fieldName.empty() ) {
            // mark initialized here so it remembers this field was not found
            fieldInfo.status = FieldInfo::NotFound;
            return fieldInfo;
         }

         if( !store.standalone() and fieldInfo.fieldType.rfind("SG::PackedContainer<", 0) == 0 )
            fieldInfo.isPackedContainer = true;

         /*
           string elem_tname, branch_tname;
           const type_info* ti = getAuxElementType( fieldInfo.tclass, typ, store.standalone(),
           elem_tname, branch_tname );
           const type_info* reg_ti = reg.getType(auxid);
           if( ti && ti != reg_ti && strcmp(ti->name(), reg_ti->name()) != 0 )
           {
           // type in registry is different than type in the file.
           // will need to use ROOT auto schema evolution
           fieldInfo.needsSE = true;
           errorcheck::ReportMessage msg (MSG::INFO, ERRORCHECK_ARGS, "RNTupleAuxDynReader");
           msg << "attribute " << fieldInfo.attribName << " (id=" << auxid <<
           " typename=" << SG::AuxTypeRegistry::instance().getType(auxid)->name()
           << ") has different type than the branch " << branch_tname;

           const std::type_info *tinf =  store.getIOType(auxid);
           fieldInfo.SE_tclass  = TClass::GetClass(*tinf);
           fieldInfo.SE_edt = kOther_t;
           if( !fieldInfo.SE_tclass  ) {
           fieldInfo.SE_edt = TDataType::GetType(*tinf);
           if( fieldInfo.SE_edt <=0 ) {
           fieldInfo.status = FieldInfo::TypeError;
           throw string("Error getting ROOT type for AUX branch ") + fieldInfo.branch->GetName()
           + " typeinfo=" + tinf->name();
           }
           }
           }
         */
      
         fieldInfo.status = FieldInfo::Initialized;
      }
      return fieldInfo;
   }


   void RNTupleAuxDynReader::addReaderToObject(void* object, size_t ttree_row, std::recursive_mutex* iomtx)
   {
      auto store_holder = reinterpret_cast<SG::IAuxStoreHolder*>((char*)object + m_storeHolderOffset);
      bool standalone { store_holder->getStoreType()==SG::IAuxStoreHolder::AST_ObjectStore };
      if( !m_initialized )
         init(standalone);
      store_holder->setStore( new RNTupleAuxDynStore(*this, m_ntupleReader, m_storeFieldName, ttree_row, standalone, iomtx) );
   }

}// namespace
