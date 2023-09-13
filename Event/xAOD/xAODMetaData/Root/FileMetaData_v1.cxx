/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// System include(s):
#include <cmath>
#include <cstdlib>

// Core include(s):
#include "AthContainers/AuxTypeRegistry.h"
#include "AthContainers/normalizedTypeinfoName.h"

// Local include(s):
#include "xAODMetaData/versions/FileMetaData_v1.h"
#include "FileMetaDataAccessors_v1.h"
#include <iostream>

namespace xAOD {

   FileMetaData_v1::FileMetaData_v1()  : SG::AuxElement()
   { }


   bool FileMetaData_v1::compareWith( const FileMetaData_v1& rhs, const std::set<std::string>& ignore ) const
   {
      // Get the variable types from both objects:
      SG::auxid_set_t auxids1 = this->getAuxIDs();
      SG::auxid_set_t auxids2 = rhs.getAuxIDs();
      SG::AuxTypeRegistry& reg = SG::AuxTypeRegistry::instance();
      for( auto var : ignore ) {
         SG::auxid_t varid = reg.findAuxID(var);
         auxids1.erase(varid);
         auxids2.erase(varid);
      }

      // They need to be the same. If the two objects have different variables,
      // that's bad. Unfortunately there's no equivalency operator for
      // auxid_set_t, so this check needs to be spelled out. :-(
      if( auxids1.size() != auxids2.size() ) {
         return false;
      }
      for( SG::auxid_t auxid : auxids1 ) {
         if( auxids2.find( auxid ) == auxids2.end() ) {
            return false;
         }
      }

      // Now, compare all elements:
      for( SG::auxid_t auxid : auxids1 ) {

         // Check the type of the variable:
         const std::type_info* ti = reg.getType( auxid );
         if( ! ti ) {
            // This is weird, but there's not much that we can do about it
            // here...
            continue;
         }
         if( ( *ti != typeid( std::string ) ) &&
             ( *ti != typeid( uint32_t ) ) &&
             ( *ti != typeid( float ) ) &&
             ( *ti != typeid( char ) ) &&
             ( *ti != typeid( std::vector< uint32_t > ) ) ) {
            // We just ignore every other type. Still, this is strange, let's
            // warn the user about it.
            std::cerr << "xAOD::FileMetaData::operator==  WARNING  Unsupported "
                      << "variable (\"" << reg.getName( auxid ) << "\"/"
                      << SG::normalizedTypeinfoName( *ti )
                      << ") encountered" << std::endl;
            continue;
         }

         // The variable name:
         const std::string name = reg.getName( auxid );

         // Treat different types separately:
         if( *ti == typeid( std::string ) ) {

            // Retrieve the values:
            const std::string& value1 = this->auxdata< std::string >( name );
            const std::string& value2 = rhs.auxdata< std::string >( name );
            // And simply compare them:
            if( value1 != value2 ) {
               return false;
            }

         } else if( *ti == typeid( uint32_t ) ) {

            // Retrieve the values:
            const uint32_t& value1 = this->auxdata< uint32_t >( name );
            const uint32_t& value2 = rhs.auxdata< uint32_t >( name );
            // And simply compare them:
            if( value1 != value2 ) {
               return false;
            }

         } else if( *ti == typeid( float ) ) {

            // Retrieve the values:
            const float& value1 = this->auxdata< float >( name );
            const float& value2 = rhs.auxdata< float >( name );
            // And (not so simply) compare them:
            if( std::abs( value1 - value2 ) > 0.001 ) {
               return false;
            }

         } else if( *ti == typeid( char ) ) {

            // Retrieve the values:
            const char& value1 = this->auxdata< char >( name );
            const char& value2 = rhs.auxdata< char >( name );
            // And (not so simply) compare them:
            if( value1 != value2 ) {
               return false;
            }
         } else if ( *ti == typeid( std::vector<uint32_t> ) ) {
            // One code to retrieve them
            const std::vector<uint32_t>& value1 =
               this->auxdata< std::vector<uint32_t> >(name);
            const std::vector<uint32_t>& value2 =
               rhs.auxdata< std::vector<uint32_t> >(name);
            // and in simplicity compare them
            if( value1 != value2 ) {
               return false;
            }
         } else {
            // We should really never end up here unless a coding mistake was
            // made upstream.
            std::abort();
         }
      }

      /*
      // Compare the string properties:
      std::array< MetaDataType, 8 > stringTypes{ { productionRelease, amiTag,
         AODFixVersion, AODCalibVersion, dataType, geometryVersion,
         conditionsTag, beamType } };
      for( MetaDataType type : stringTypes ) {
         // (Try to) Retrieve the properties:
         std::string val1, val2;
         const bool found1 = this->value( type, val1 );
         const bool found2 = rhs.value( type, val2 );
         // If both of them failed, then let's continue. If both of them are
         // missing this variable, that's fine.
         if( ( ! found1 ) && ( ! found2 ) ) {
            continue;
         }
         // If the variable is only available on one of them, then we already
         // have a difference. Although this point could be fine-tuned later on.
         if( ( found1 && ( ! found2 ) ) || ( ( ! found1 ) && found2 ) ) {
            return false;
         }
         // If both values were found, then let's compare them:
         if( val1 != val2 ) {
            return false;
         }
      }

      // Compare the float propery/properties:
      std::array< MetaDataType, 1 > floatTypes{ { beamEnergy } };
      for( MetaDataType type : floatTypes ) {
         // (Try to) Retrieve the properties:
         float val1 = 0.0, val2 = 0.0;
         const bool found1 = this->value( type, val1 );
         const bool found2 = rhs.value( type, val2 );
         // If both of them failed, then let's continue. If both of them are
         // missing this variable, that's fine.
         if( ( ! found1 ) && ( ! found2 ) ) {
            continue;
         }
         // If the variable is only available on one of them, then we already
         // have a difference. Although this point could be fine-tuned later on.
         if( ( found1 && ( ! found2 ) ) || ( ( ! found1 ) && found2 ) ) {
            return false;
         }
         // If both values were found, then let's compare them:
         if( std::abs( val1 - val2 ) > 0.001 ) {
            return false;
         }
      }
      */

      // The two objects were found to be equivalent:
      return true;
   }

   bool FileMetaData_v1::operator!=( const FileMetaData_v1& rhs ) const {

      return !( this->operator==( rhs ) );
   }

   bool FileMetaData_v1::value( MetaDataType type, std::string& val ) const {

      // Get the accessor for this type:
      const Accessor< std::string >* acc = metaDataTypeStringAccessorV1( type );
      if( ! acc ) {
         return false;
      }

      // Check if the variable is available:
      if( ! acc->isAvailable( *this ) ) {
         return false;
      }

      // Read the value:
      val = ( *acc )( *this );

      // We were successful:
      return true;
   }

   bool FileMetaData_v1::value( const std::string& type,
                                std::string& val ) const {

      // Create an accessor object:
      const Accessor< std::string > acc( type );

      // Check if this variable is available:
      if( ! acc.isAvailable( *this ) ) {
         return false;
      }

      // Read the value:
      val = acc( *this );

      // We were successful:
      return true;
   }

   bool FileMetaData_v1::setValue( MetaDataType type, const std::string& val ) {

      // Get the accessor for this type:
      const Accessor< std::string >* acc = metaDataTypeStringAccessorV1( type );
      if( ! acc ) {
         return false;
      }

      // Set the value:
      ( *acc )( *this ) = val;

      // We were successful:
      return true;
   }

   bool FileMetaData_v1::setValue( const std::string& type,
                                   const std::string& val ) {

      // Create the accessor object:
      const Accessor< std::string > acc( type );

      // Set the value:
      acc( *this ) = val;

      // We were successful:
      return true;
   }

   bool FileMetaData_v1::value( MetaDataType type, uint32_t& val ) const {

      // Get the accessor for this type:
      const Accessor< uint32_t >* acc = metaDataTypeUIntAccessorV1( type );
      if( ! acc ) {
         return false;
      }

      // Check if the variable is available:
      if( ! acc->isAvailable( *this ) ) {
         return false;
      }

      // Read the value:
      val = ( *acc )( *this );

      // We were successful:
      return true;
   }

   bool FileMetaData_v1::value( const std::string& type,
                                uint32_t& val ) const {

      // Create an accessor object:
      const Accessor< uint32_t > acc( type );

      // Check if this variable is available:
      if( ! acc.isAvailable( *this ) ) {
         return false;
      }

      // Read the value:
      val = acc( *this );

      // We were successful:
      return true;
   }

   bool FileMetaData_v1::setValue( MetaDataType type, uint32_t val ) {

      // Get the accessor for this type:
      const Accessor< uint32_t >* acc = metaDataTypeUIntAccessorV1( type );
      if( ! acc ) {
         return false;
      }

      // Set the value:
      ( *acc )( *this ) = val;

      // We were successful:
      return true;
   }

   bool FileMetaData_v1::setValue( const std::string& type, uint32_t val ) {

      // Create the accessor object:
      const Accessor< uint32_t > acc( type );

      // Set the value:
      acc( *this ) = val;

      // We were successful:
      return true;
   }

   bool FileMetaData_v1::value( MetaDataType type, float& val ) const {

      // Get the accessor for this type:
      const Accessor< float >* acc = metaDataTypeFloatAccessorV1( type );
      if( ! acc ) {
         return false;
      }

      // Check if the variable is available:
      if( ! acc->isAvailable( *this ) ) {
         return false;
      }

      // Read the value:
      val = ( *acc )( *this );

      // We were successful:
      return true;
   }

   bool FileMetaData_v1::value( const std::string& type,
                                float& val ) const {

      // Create an accessor object:
      const Accessor< float > acc( type );

      // Check if this variable is available:
      if( ! acc.isAvailable( *this ) ) {
         return false;
      }

      // Read the value:
      val = acc( *this );

      // We were successful:
      return true;
   }

   bool FileMetaData_v1::setValue( MetaDataType type, float val ) {

      // Get the accessor for this type:
      const Accessor< float >* acc = metaDataTypeFloatAccessorV1( type );
      if( ! acc ) {
         return false;
      }

      // Set the value:
      ( *acc )( *this ) = val;

      // We were successful:
      return true;
   }

   bool FileMetaData_v1::setValue( const std::string& type, float val ) {

      // Create the accessor object:
      const Accessor< float > acc( type );

      // Set the value:
      acc( *this ) = val;

      // We were successful:
      return true;
   }

   bool FileMetaData_v1::value( MetaDataType type, bool& val ) const {

      // Get the accessor for this type:
      const Accessor< char >* acc = metaDataTypeCharAccessorV1( type );
      if( ! acc ) {
         return false;
      }

      // Check if the variable is available:
      if( ! acc->isAvailable( *this ) ) {
         return false;
      }

      // Read the value:
      val = ( *acc )( *this );

      // We were successful:
      return true;
   }

   bool FileMetaData_v1::value( const std::string& type,
                                bool& val ) const {

      // Create an accessor object:
      const Accessor< char > acc( type );

      // Check if this variable is available:
      if( ! acc.isAvailable( *this ) ) {
         return false;
      }

      // Read the value:
      val = acc( *this );

      // We were successful:
      return true;
   }

   bool FileMetaData_v1::setValue( MetaDataType type, bool val ) {

      // Get the accessor for this type:
      const Accessor< char >* acc = metaDataTypeCharAccessorV1( type );
      if( ! acc ) {
         return false;
      }

      // Set the value:
      ( *acc )( *this ) = val;

      // We were successful:
      return true;
   }

   bool FileMetaData_v1::setValue( const std::string& type, bool val ) {

      // Create the accessor object:
      const Accessor< char > acc( type );

      // Set the value:
      acc( *this ) = val;

      // We were successful:
      return true;
   }

   bool FileMetaData_v1::value(const std::string& type,
                               std::vector< uint32_t >& val) const {
     // Create an accessor object:
     const Accessor<std::vector<uint32_t> > acc(type);

     // Check if this variable is available:
     if (!acc.isAvailable(*this)) {
       return false;
     }

     // Read the value:
     val = acc(*this);

     // We were successful:
     return true;
   }

   bool FileMetaData_v1::setValue(const std::string& type,
                                  const std::vector< uint32_t >& val) {
     // Create the accessor object:
     const Accessor<std::vector<uint32_t> > acc(type);

     // Set the value:
     acc(*this) = val;

     // We were successful:
     return true;
   }

/// Helper macro used to print MetaDataType values
#define PRINT_TYPE( TYPE )                      \
   case xAOD::FileMetaData_v1::TYPE:            \
   out << "xAOD::FileMetaData::" << #TYPE;      \
   break

/// This can be used in user code to conveniently print the values of
/// MetaDataType variables in a user friendly manner.
///
/// @param out The output stream to print to
/// @param type The value to print
/// @returns The same stream object that the operator printed to
///
std::ostream& operator<< ( std::ostream& out,
                           xAOD::FileMetaData_v1::MetaDataType type ) {

   switch( type ) {

      PRINT_TYPE( productionRelease );
      PRINT_TYPE( amiTag );
      PRINT_TYPE( AODFixVersion );
      PRINT_TYPE( AODCalibVersion );
      PRINT_TYPE( dataType );
      PRINT_TYPE( geometryVersion );
      PRINT_TYPE( conditionsTag );
      PRINT_TYPE( beamEnergy );
      PRINT_TYPE( beamType );
      PRINT_TYPE( mcProcID );
      PRINT_TYPE( simFlavour );
      PRINT_TYPE( isDataOverlay );
      PRINT_TYPE( mcCampaign );
      PRINT_TYPE( generatorsInfo );
      PRINT_TYPE( dataYear );

   default:
      out << "UNKNOWN (" << static_cast< int >( type ) << ")";
      break;
   }

   // Return the same stream:
   return out;
}


} // namespace xAOD
