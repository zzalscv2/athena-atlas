/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "CxxUtils/checker_macros.h"
ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

#include "src/ALFA_SensitiveDetector.h"

#include "gtest/gtest.h"

#include <vector>

#include "TestTools/initGaudi.h"

#include "G4HCofThisEvent.hh"
#include "G4Step.hh"
#include "G4TouchableHistory.hh"

#include "G4Track.hh"
#include "G4StepPoint.hh"
#include "G4DynamicParticle.hh"
#include "G4ThreeVector.hh"
#include "G4Box.hh"
#include "G4NistManager.hh"
#include "G4Material.hh"
#include "G4VPhysicalVolume.hh"
#include "G4SystemOfUnits.hh"

#include "G4AtlasTools/DerivedG4PhysicalVolume.h"
#include "G4AtlasTools/DerivedG4SensitiveDetectorTestSetting.h"

//set environment
class GaudiEnvironment : public ::testing::Environment {
  protected:
  virtual void SetUp() override {
    Athena_test::initGaudi("ALFA_G4_SD/optionForTest.txt", m_svcLoc);
  }
  ISvcLocator* m_svcLoc = nullptr;
};
class ALFA_SensitiveDetectortest : public ::testing::Test {	
  protected:
    virtual void SetUp() override {
    }

    virtual void TearDown() override {
    }
};
//end of environment setting

TEST_F( ALFA_SensitiveDetectortest, Initialize )
{
  G4HCofThisEvent hce;
  ALFA_SensitiveDetector sd1( "name1", "name1", "name1" );
  sd1.Initialize(&hce);
  ASSERT_TRUE(sd1.m_HitCollection.isValid()); //test if the initialization of m_HitCollection is successful
  ASSERT_TRUE(sd1.m_ODHitCollection.isValid()); //test if the initialization of m_ODHitCollection is successful
}

TEST_F( ALFA_SensitiveDetectortest, ProcessHits1 )//test first block of ProcessHits, which is "if(vol_name.find("GVS") != std::string::npos)". when you need to test it, just uncomment the TEST_F
{
  G4HCofThisEvent hce;
  G4Step sp;
  G4TouchableHistory th;
  
  G4double totalenergydeposit = 0.8;
  std::vector<G4String> physicalname = {"GVSB7R1"};
  G4String logicalname = "BBBBBBBBBTubeGas";
  std::vector<G4int> copynos = {2009};
  G4ThreeVector preStepPos = G4ThreeVector(0,0,1);
  G4ThreeVector postStepPos = G4ThreeVector(0,0,2);
  G4double globaltime0 = 0.5;
  G4double kineticenergy0 = 1.5;
  G4double velocity0 = 99.93100;
  G4double globaltime = 5.0;
  G4double kineticenergy = 0.5;
  G4double globaltime1 = 0.5;
  G4double kineticenergy1 = 0.5;
  G4double velocity1 = 99.93100;
  G4double steplength = 1.0;
  G4double charge = 1.0;
  G4int encoding = 22;
  G4int antiencoding = 22;
  G4String astring = "Cerenkov";
  G4ProcessType atype = (G4ProcessType)0;
  G4String nop1 = "opticalphoton";
  G4String nop2 = "opticalphoton";
  G4String nop3 = "photon";
  DerivedG4SensitiveDetectorTestSetting(sp, totalenergydeposit, physicalname, logicalname, copynos, preStepPos, postStepPos, globaltime0, kineticenergy0, velocity0, globaltime, kineticenergy, globaltime1, kineticenergy1, velocity1, steplength, charge, encoding, antiencoding, astring, atype, nop1, nop2, nop3);

  ALFA_SensitiveDetector sd21( "name21", "name21", "name21" );
  sd21.Initialize(&hce);
  sd21.ProcessHits(&sp, &th);

  ALFA_HitCollection * a = sd21.m_HitCollection.ptr();
  ASSERT_TRUE( a->begin()->GetHitID() == 0 ); //test the HitID value of the Hit generated by the member function ProcessHits, the same below
  ASSERT_TRUE( a->begin()->GetParticleEncoding() == 22 );
  ASSERT_TRUE( a->begin()->GetKineticEnergy() == 1.5 );
  ASSERT_FLOAT_EQ( a->begin()->GetEnergyDeposit(), 0.8 );
  ASSERT_TRUE( a->begin()->GetPreStepX() == 0 );
  ASSERT_TRUE( a->begin()->GetPreStepY() == 0 );
  ASSERT_TRUE( a->begin()->GetPreStepZ() == 1 );
  ASSERT_TRUE( a->begin()->GetPostStepX() == 0 );
  ASSERT_TRUE( a->begin()->GetPostStepY() == 0 );
  ASSERT_TRUE( a->begin()->GetPostStepZ() == 2 );
  ASSERT_FLOAT_EQ( a->begin()->GetGlobalTime(), 5 );
  ASSERT_TRUE( a->begin()->GetSignFiber() == -1 );
  ASSERT_TRUE( a->begin()->GetPlateNumber() == 100 );
  ASSERT_TRUE( a->begin()->GetFiberNumber() == -1 );
  ASSERT_TRUE( a->begin()->GetStationNumber() == 3 );
  ASSERT_TRUE( sd21.m_numberOfHits == 1 );
}

/*TEST_F( ALFA_SensitiveDetectortest, ProcessHits2 )//test second block of ProcessHits, which is "if (vol_test_str.compare("ALFA_Fi") == 0)". if you want to test this block, what you just need to do is uncomment the TEST_F meanwhile keep other TEST_Fs for this method commented
{
  G4HCofThisEvent hce;
  G4Step sp;
  G4TouchableHistory th;

  G4double totalenergydeposit = 0.8;
  std::vector<G4String> physicalname = {"ALFA_FiBBBUB[3] [2] [1]";}
  G4String logicalname = "BBBBBBBBBTubeGas";
  std::vector<G4int> copynos = {2009};
  G4ThreeVector preStepPos = G4ThreeVector(0,0,1);
  G4ThreeVector postStepPos = G4ThreeVector(0,0,2);
  G4double globaltime0 = 0.5;
  G4double kineticenergy0 = 1.5;
  G4double velocity0 = 99.93100;
  G4double globaltime = 5.0;
  G4double kineticenergy = 0.5;
  G4double globaltime1 = 0.5;
  G4double kineticenergy1 = 0.5;
  G4double velocity1 = 99.93100;
  G4double steplength = 1.0;
  G4double charge = 1.0;
  G4int encoding = 22;
  G4int antiencoding = 22;
  G4String astring = "Cerenkov";
  G4ProcessType atype = (G4ProcessType)0;
  G4String nop1 = "opticalphoton";
  G4String nop2 = "opticalphoton";
  G4String nop3 = "photon";
  DerivedG4SensitiveDetectorTestSetting(sp, totalenergydeposit, physicalname, logicalname, copynos, preStepPos, postStepPos, globaltime0, kineticenergy0, velocity0, globaltime, kineticenergy, globaltime1, kineticenergy1, velocity1, steplength, charge, encoding, antiencoding, astring, atype, nop1, nop2, nop3);

  ALFA_SensitiveDetector sd22( "name22", "name22", "name22" );
  sd22.Initialize(&hce);
  sd22.ProcessHits(&sp, &th);

  ALFA_HitCollection * a = sd22.m_HitCollection.ptr();
  ASSERT_TRUE( a->begin()->GetHitID() == 0 ); //test the HitID value of the Hit generated by the member function ProcessHits, the same below
  ASSERT_TRUE( a->begin()->GetParticleEncoding() == 22 );
  ASSERT_TRUE( a->begin()->GetKineticEnergy() == 1.5 );
  ASSERT_FLOAT_EQ( a->begin()->GetEnergyDeposit(), 0.8 );
  ASSERT_TRUE( a->begin()->GetPreStepX() == 0 );
  ASSERT_TRUE( a->begin()->GetPreStepY() == 0 );
  ASSERT_TRUE( a->begin()->GetPreStepZ() == 1 );
  ASSERT_TRUE( a->begin()->GetPostStepX() == 0 );
  ASSERT_TRUE( a->begin()->GetPostStepY() == 0 );
  ASSERT_TRUE( a->begin()->GetPostStepZ() == 2 );
  ASSERT_FLOAT_EQ( a->begin()->GetGlobalTime(), 5 );
  ASSERT_TRUE( a->begin()->GetSignFiber() == 1 );
  ASSERT_TRUE( a->begin()->GetPlateNumber() == 2 );
  ASSERT_TRUE( a->begin()->GetFiberNumber() == 1 );
  ASSERT_TRUE( a->begin()->GetStationNumber() == 3 );
  ASSERT_TRUE( sd22.m_numberOfHits == 1 );

}*/

/*TEST_F( ALFA_SensitiveDetectortest, ProcessHits3 )//test the 3rd block of ProcessHits, similarly, if you want to test the block, just uncomment this TEST_F while keep others commented
{
  G4HCofThisEvent hce;
  G4Step sp;
  G4TouchableHistory th;

  G4double totalenergydeposit = 0.8;
  std::vector<G4String> physicalname = {"ODFiberU1[3][2][1]"};
  G4String logicalname = "BBBBBBBBBTubeGas";
  std::vector<G4int> copynos = {2009};
  G4ThreeVector preStepPos = G4ThreeVector(0,0,1);
  G4ThreeVector postStepPos = G4ThreeVector(0,0,2);
  G4double globaltime0 = 0.5;
  G4double kineticenergy0 = 1.5;
  G4double velocity0 = 99.93100;
  G4double globaltime = 5.0;
  G4double kineticenergy = 0.5;
  G4double globaltime1 = 0.5;
  G4double kineticenergy1 = 0.5;
  G4double velocity1 = 99.93100;
  G4double steplength = 1.0;
  G4double charge = 1.0;
  G4int encoding = 22;
  G4int antiencoding = 22;
  G4String astring = "Cerenkov";
  G4ProcessType atype = (G4ProcessType)0;
  G4String nop1 = "opticalphoton";
  G4String nop2 = "opticalphoton";
  G4String nop3 = "photon";
  DerivedG4SensitiveDetectorTestSetting(sp, totalenergydeposit, physicalname, logicalname, copynos, preStepPos, postStepPos, globaltime0, kineticenergy0, velocity0, globaltime, kineticenergy, globaltime1, kineticenergy1, velocity1, steplength, charge, encoding, antiencoding, astring, atype, nop1, nop2, nop3);

  ALFA_SensitiveDetector sd23( "name23", "name23", "name23" );
  sd23.Initialize(&hce);
  sd23.ProcessHits(&sp, &th);

  ALFA_ODHitCollection * a = sd23.m_ODHitCollection.ptr();
  ASSERT_TRUE( a->begin()->GetHitID() == 0 ); //test the HitID value of the Hit generated by the member function ProcessHits, the same below
  ASSERT_TRUE( a->begin()->GetParticleEncoding() == 22 );
  ASSERT_TRUE( a->begin()->GetKineticEnergy() == 1.5 );
  ASSERT_FLOAT_EQ( a->begin()->GetEnergyDeposit(), 0.8 );
  ASSERT_TRUE( a->begin()->GetPreStepX() == 0 );
  ASSERT_TRUE( a->begin()->GetPreStepY() == 0 );
  ASSERT_TRUE( a->begin()->GetPreStepZ() == 1 );
  ASSERT_TRUE( a->begin()->GetPostStepX() == 0 );
  ASSERT_TRUE( a->begin()->GetPostStepY() == 0 );
  ASSERT_TRUE( a->begin()->GetPostStepZ() == 2 );
  ASSERT_FLOAT_EQ( a->begin()->GetGlobalTime(), 5 );
  ASSERT_TRUE( a->begin()->GetSignFiber() == 1 );
  ASSERT_TRUE( a->begin()->GetODSide() == 1 );
  ASSERT_TRUE( a->begin()->GetPlateNumber() == 2 );
  ASSERT_TRUE( a->begin()->GetFiberNumber() == 1 );
  ASSERT_TRUE( a->begin()->GetStationNumber() == 3 );
  ASSERT_TRUE( sd23.m_numberOfODHits == 1 );

}*/

TEST_F( ALFA_SensitiveDetectortest, StartOfAthenaEvent )
{
  G4HCofThisEvent hce;
  ALFA_SensitiveDetector sd3("name3", "name3", "name3");
  sd3.Initialize(&hce);
  sd3.StartOfAthenaEvent();

// the following lines aim to test the values of member variables(m_numberOfHits and m_numberOfODHits) initialized by the member function StartOfAthenaEvent()
  ASSERT_TRUE( sd3.m_numberOfHits == 0 );
  ASSERT_TRUE( sd3.m_numberOfODHits == 0 );
}

TEST_F( ALFA_SensitiveDetectortest, EndOfAthenaEvent )
{
  G4HCofThisEvent hce;
  ALFA_SensitiveDetector sd4("name4", "name4", "name4");
  sd4.Initialize(&hce);
  sd4.EndOfAthenaEvent();

//the following lines aims to test the values of the member variables(m_eventNumber, m_numberOfHits and m_numberOfODHits) assigned by the member function EndOfAthenaEvent()
  ASSERT_TRUE( sd4.m_eventNumber == 1 );
  ASSERT_TRUE( sd4.m_numberOfHits == 0 );
  ASSERT_TRUE( sd4.m_numberOfODHits == 0 );
}

int main( int argc, char** argv ) {

  auto *g=new GaudiEnvironment;
  ::testing::AddGlobalTestEnvironment(g);
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();

}
