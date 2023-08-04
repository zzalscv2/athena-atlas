
//
//  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
//

#include "xAODJet/Jet.h"

#include "gmock/gmock.h"

using ::testing::Return;
using ::testing::_;
using ::testing::SetArgReferee;

// Suppress a false positive from gtest.
#if __GNUC__ >= 12
# pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif


namespace Athena_test {

  // Mock for xAOD::Jet with method handle Jet::getAttribute<T>
  class MockxAODJet : public xAOD::Jet {
  public:
    MockxAODJet() {}
    MOCK_CONST_METHOD0( pt, double() );
    MOCK_CONST_METHOD0( eta, double() );
    MOCK_CONST_METHOD0( phi, double() );
    MOCK_CONST_METHOD0( m, double () );
    MOCK_CONST_METHOD0( e, double() );
    MOCK_CONST_METHOD0( et, double() );
    MOCK_CONST_METHOD0( rapidity, double() );
    MOCK_CONST_METHOD0( p4, TLorentzVector() );
    MOCK_CONST_METHOD0( position, unsigned int() );
    MOCK_CONST_METHOD0( toString, std::string() );
    MOCK_CONST_METHOD2( getAttributeFloat, bool( const std::string&, 
						 float& ) );
  };

}  

// Template method specialisation
namespace xAOD {

  template<> inline
  bool Jet::getAttribute<float>( const std::string& name, 
				 float& value ) const {
    if (auto mockJet = dynamic_cast<const Athena_test::MockxAODJet*>( this )) {
      return mockJet->getAttributeFloat( name, value );
    }
    return false;
  }
  
}

namespace Athena_test {

  // Tests with mock object, these show that the mock
  // object works correctly incl. the specialised template
  // method

  class MockxAODJetTest : public ::testing::Test {
  public:
    MockxAODJetTest() : jet( &mockjet ) {}
    MockxAODJet mockjet;
    xAOD::Jet* jet;
  };

  // cppcheck-suppress syntaxError
  TEST_F( MockxAODJetTest, jetpt ) {
    ON_CALL( mockjet, pt() ).WillByDefault( Return( 1.0 ) );
    EXPECT_CALL( mockjet, pt() ).Times( 1 );
    EXPECT_EQ( jet->pt(), 1.0 );
  }

  TEST_F( MockxAODJetTest, jetgetAttribute ) {
    float f = 0;
    EXPECT_CALL( mockjet, getAttributeFloat( "test", f ) )
      .WillOnce( testing::DoAll( SetArgReferee<1>( 42 ), Return(true) ) );
    float value = 0;
    EXPECT_TRUE( jet->getAttribute<float>( "test", value ) );
    EXPECT_EQ( 42, value );
  }

}

int main( int argc, char **argv ) {
  ::testing::InitGoogleMock( &argc, argv );
  return RUN_ALL_TESTS();
}

