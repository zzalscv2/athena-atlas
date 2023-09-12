/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///
/// @file Unit test for the equality operator of xAOD::FileMetaData
///
/// The equality operator of xAOD::FileMetaData is used extensively while
/// merging metadata between files, so it needs to be checked for consistency
/// quite carefully.
///

// System include(s):
#include <iostream>
#include <string>

// Local include(s):
#include "xAODMetaData/FileMetaData.h"

/// Helper macro for evaluating logical tests
#define SIMPLE_ASSERT( EXP )                                        \
   do {                                                             \
      const bool result = EXP;                                      \
      if( ! result ) {                                              \
         std::cerr << __FILE__ << ":" << __LINE__                   \
                   << " Expression \"" << #EXP                      \
                   << "\" failed the evaluation" << std::endl;      \
         return 1;                                                  \
      }                                                             \
   } while( 0 )

int main() {
   // For the string literals operator s
   using namespace std::string_literals;

   // Set up two objects with their private auxiliary stores, which will be
   // used in the tests:
   xAOD::FileMetaData meta1, meta2;
   meta1.makePrivateStore();
   meta2.makePrivateStore();

   // First, set them to equivalent values:
   SIMPLE_ASSERT( meta1.setValue( xAOD::FileMetaData::productionRelease,
                                  "1.2.3"s ) == true );
   SIMPLE_ASSERT( meta1.setValue( xAOD::FileMetaData::conditionsTag,
                                  "Cond-1"s ) == true );
   SIMPLE_ASSERT( meta1.setValue( xAOD::FileMetaData::beamEnergy,
                                  123.456f ) == true );
   SIMPLE_ASSERT( meta1.setValue( xAOD::FileMetaData::dataYear,
                                  2023u ) == true );     

   SIMPLE_ASSERT( meta2.setValue( xAOD::FileMetaData::productionRelease,
                                  "1.2.3"s ) == true );
   SIMPLE_ASSERT( meta2.setValue( xAOD::FileMetaData::conditionsTag,
                                  "Cond-1"s ) == true );
   SIMPLE_ASSERT( meta2.setValue( xAOD::FileMetaData::beamEnergy,
                                  123.456f ) == true );
   SIMPLE_ASSERT( meta2.setValue( xAOD::FileMetaData::dataYear,
                                  2023u ) == true );  

   // And then check their equivalency:
   SIMPLE_ASSERT( meta1 == meta2 );

   // Make sure that minor floating point differences are overlooked:
   SIMPLE_ASSERT( meta2.setValue( xAOD::FileMetaData::beamEnergy,
                                  123.455999f ) == true );
   SIMPLE_ASSERT( meta1 == meta2 );

   // Now check that a difference is also found:
   SIMPLE_ASSERT( meta1.setValue( xAOD::FileMetaData::productionRelease,
                                  "1.2.4"s ) == true );
   SIMPLE_ASSERT( meta1 != meta2 );

   // Now check that a difference is also found with integers:
   SIMPLE_ASSERT( meta1.setValue( xAOD::FileMetaData::productionRelease,
                                  "1.2.3"s ) == true );
   SIMPLE_ASSERT( meta1.setValue( xAOD::FileMetaData::dataYear,
                                  2022u ) == true );  
   SIMPLE_ASSERT( meta1 != meta2 );

   // Clear out the two objects:
   meta1.releasePrivateStore();
   meta2.releasePrivateStore();
   meta1.makePrivateStore();
   meta2.makePrivateStore();

   // Set them up with different types of properties:
   SIMPLE_ASSERT( meta1.setValue( xAOD::FileMetaData::productionRelease,
                                  "1.2.3"s ) == true );
   SIMPLE_ASSERT( meta1.setValue( xAOD::FileMetaData::conditionsTag,
                                  "Cond-1"s ) == true );
   SIMPLE_ASSERT( meta1.setValue( xAOD::FileMetaData::beamEnergy,
                                  123.456f ) == true );
   SIMPLE_ASSERT( meta1.setValue( xAOD::FileMetaData::dataYear,
                                  2023u ) == true );  

   SIMPLE_ASSERT( meta2.setValue( xAOD::FileMetaData::productionRelease,
                                  "1.2.3"s ) == true );
   SIMPLE_ASSERT( meta2.setValue( xAOD::FileMetaData::conditionsTag,
                                  "Cond-1"s ) == true );
   SIMPLE_ASSERT( meta2.setValue( xAOD::FileMetaData::beamType,
                                  "Collision"s ) == true );
   SIMPLE_ASSERT( meta1.setValue( xAOD::FileMetaData::dataYear,
                                  2022u ) == true );  

   // Check that they are not equivalent:
   SIMPLE_ASSERT( meta1 != meta2 );

   // Clear out the two objects:
   meta1.releasePrivateStore();
   meta2.releasePrivateStore();
   meta1.makePrivateStore();
   meta2.makePrivateStore();

   // Now try using some custom variables as well:
   SIMPLE_ASSERT( meta1.setValue( xAOD::FileMetaData::productionRelease,
                                  "1.2.3"s ) == true );
   SIMPLE_ASSERT( meta1.setValue( xAOD::FileMetaData::conditionsTag,
                                  "Cond-1"s ) == true );
   SIMPLE_ASSERT( meta1.setValue( "AnalysisType", "SUSY"s ) == true );

   SIMPLE_ASSERT( meta2.setValue( xAOD::FileMetaData::productionRelease,
                                 "1.2.3"s ) == true );
   SIMPLE_ASSERT( meta2.setValue( xAOD::FileMetaData::conditionsTag,
                                 "Cond-1"s ) == true );
   SIMPLE_ASSERT( meta2.setValue( "AnalysisType", "SUSY"s ) == true );

   SIMPLE_ASSERT( meta1 == meta2 );

   // And check whether a difference in a user type is detected:
   SIMPLE_ASSERT( meta2.setValue( "AnalysisType", "Higgs"s ) == true );
   SIMPLE_ASSERT( meta1 != meta2 );

   // Try it for a floating point difference as well:
   SIMPLE_ASSERT( meta2.setValue( "AnalysisType", "SUSY"s ) == true );
   SIMPLE_ASSERT( meta1 == meta2 );
   SIMPLE_ASSERT( meta1.setValue( "AnalysisValue", 1.2f ) == true );
   SIMPLE_ASSERT( meta2.setValue( "AnalysisValue", 1.3f ) == true );
   SIMPLE_ASSERT( meta1 != meta2 );

   // Finally, set an unsupported variable type on the objects, and check that
   // the code complains about them. But still lets the equivalence stand.
   SIMPLE_ASSERT( meta2.setValue( "AnalysisValue", 1.2f ) == true );
   SIMPLE_ASSERT( meta1 == meta2 );
   meta1.auxdata< int >( "IntValue" ) = 1;
   meta2.auxdata< int >( "IntValue" ) = 2;
   SIMPLE_ASSERT( meta1 == meta2 );

   // Tell the user that everything went successfully:
   std::cout << "All tests succeeded" << std::endl;

   // Return gracefully:
   return 0;
}
