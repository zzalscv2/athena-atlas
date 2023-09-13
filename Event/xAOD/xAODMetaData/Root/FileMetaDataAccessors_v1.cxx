/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// System include(s):
#include <iostream>

// Local include(s):
#include "FileMetaDataAccessors_v1.h"

/// Helper macro for implementing the accessor function
#define DECLARE_STRING_ACCESSOR( TYPE )                              \
   case FileMetaData_v1::TYPE:                                       \
   do {                                                              \
      static const SG::AuxElement::Accessor< std::string > acc( #TYPE ); \
      return &acc;                                                   \
   } while( 0 )

/// Helper macro for implementing the accessor function
#define DECLARE_UINT_ACCESSOR( TYPE )                                \
   case FileMetaData_v1::TYPE:                                       \
   do {                                                              \
      static const SG::AuxElement::Accessor< uint32_t > acc( #TYPE ); \
      return &acc;                                                   \
   } while( 0 )

/// Helper macro for implementing the accessor function
#define DECLARE_FLOAT_ACCESSOR( TYPE )                               \
   case FileMetaData_v1::TYPE:                                       \
   do {                                                              \
      static const SG::AuxElement::Accessor< float > acc( #TYPE );   \
      return &acc;                                                   \
   } while( 0 )

/// Helper macro for implementing the accessor function
#define DECLARE_CHAR_ACCESSOR( TYPE )                                \
   case FileMetaData_v1::TYPE:                                       \
   do {                                                              \
      const static SG::AuxElement::Accessor< char > acc( #TYPE );    \
      return &acc;                                                   \
   } while( 0 )

namespace xAOD {

   const SG::AuxElement::Accessor< std::string >*
   metaDataTypeStringAccessorV1( FileMetaData_v1::MetaDataType type ) {

      switch( type ) {

         DECLARE_STRING_ACCESSOR( productionRelease );
         DECLARE_STRING_ACCESSOR( AODFixVersion );
         DECLARE_STRING_ACCESSOR( AODCalibVersion );
         DECLARE_STRING_ACCESSOR( dataType );
         DECLARE_STRING_ACCESSOR( amiTag );
         DECLARE_STRING_ACCESSOR( geometryVersion );
         DECLARE_STRING_ACCESSOR( conditionsTag );
         DECLARE_STRING_ACCESSOR( beamType );
         DECLARE_STRING_ACCESSOR( simFlavour );
         DECLARE_STRING_ACCESSOR( mcCampaign );
         DECLARE_STRING_ACCESSOR( generatorsInfo );

      default:
         std::cerr << "xAOD::FileMetaData_v1    ERROR No string accessor for "
                   << "type: " << type << std::endl;
         return nullptr;
      }

      // Just to make sure the compiler doesn't complain:
      return nullptr;
   }

   const SG::AuxElement::Accessor< uint32_t >*
   metaDataTypeUIntAccessorV1( FileMetaData_v1::MetaDataType type ) {

      switch( type ) {

         DECLARE_UINT_ACCESSOR( dataYear );

      default:
         std::cerr << "xAOD::FileMetaData_v1    ERROR No uint32_t accessor for "
                   << "type: " << type << std::endl;
         return nullptr;
      }

      // Just to make sure the compiler doesn't complain:
      return nullptr;
   }

   const SG::AuxElement::Accessor< float >*
   metaDataTypeFloatAccessorV1( FileMetaData_v1::MetaDataType type ) {

      switch( type ) {

         DECLARE_FLOAT_ACCESSOR( beamEnergy );
         DECLARE_FLOAT_ACCESSOR( mcProcID );

      default:
         std::cerr << "xAOD::FileMetaData_v1    ERROR No float accessor for "
                   << "type: " << type << std::endl;
         return nullptr;
      }

      // Just to make sure the compiler doesn't complain:
      return nullptr;
   }

   const SG::AuxElement::Accessor< char >*
   metaDataTypeCharAccessorV1( FileMetaData_v1::MetaDataType type ) {

      switch( type ) {

         DECLARE_CHAR_ACCESSOR( isDataOverlay );

      default:
         std::cerr << "xAOD::FileMetaData_v1    ERROR No char accessor for "
                   << "type: " << type << std::endl;
         return nullptr;
      }

      // Just to make sure the compiler doesn't complain:
      return nullptr;
   }

} // namespace xAOD
