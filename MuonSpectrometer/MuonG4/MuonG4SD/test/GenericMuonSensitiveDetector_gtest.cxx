/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#include "src/GenericMuonSensitiveDetector.h"

#include "gtest/gtest.h"

#include "TestTools/initGaudi.h"

#include "G4HCofThisEvent.hh"
#include "G4Step.hh"
#include "G4TouchableHistory.hh"

#include "G4AtlasTools/DerivedG4PhysicalVolume.h"
#include "G4AtlasTools/DerivedG4SensitiveDetectorTestSetting.h"

//set environment
class GaudiEnvironment : public ::testing::Environment {
  protected:
  virtual void SetUp() override {
    Athena_test::initGaudi("MuonG4SD/optionForTest.txt", m_svcLoc);
  }
  ISvcLocator* m_svcLoc = nullptr;
};
class GenericMuonSensitiveDetectortest : public ::testing::Test {	
  protected:
    virtual void SetUp() override {
    }

    virtual void TearDown() override {
    }
};
//end of environment setting

TEST_F ( GenericMuonSensitiveDetectortest, Initialize )
{
  G4HCofThisEvent hce;
  GenericMuonSensitiveDetector sd1("name1", "name1" );
  sd1.Initialize( &hce );
  ASSERT_TRUE(sd1.m_GenericMuonHitCollection.isValid()); //check if initialization of m_GenericMuonHitCollection is successful
}

TEST_F ( GenericMuonSensitiveDetectortest, ProcessHits )
{
  G4Step sp;
  G4TouchableHistory th; 
  G4HCofThisEvent hce;  

  G4double totalenergydeposit = 3.0;
  G4String physicalname = "physicalName";
  G4String logicalname = "logicalName";
  G4int copyno = 1000;
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
  DerivedG4SensitiveDetectorTestSetting(sp, totalenergydeposit, physicalname, logicalname, copyno, preStepPos, postStepPos, globaltime0, kineticenergy0, velocity0, globaltime, kineticenergy, globaltime1, kineticenergy1, velocity1, steplength, charge, encoding, antiencoding, astring, atype, nop1, nop2, nop3);

  GenericMuonSensitiveDetector sd2("name2", "name2");
  sd2.Initialize( &hce );
  sd2.ProcessHits(&sp, &th );

  int barcode = 0;
  HepMcParticleLink plink(barcode);

  GenericMuonSimHitCollection* a = sd2.m_GenericMuonHitCollection.ptr();
  ASSERT_EQ(a->begin()->GenericId(),0);//test if the HitID value of the Hit generated by this member function is right, the same below
  ASSERT_EQ(a->begin()->globalTime(),0.5);
  ASSERT_EQ(a->begin()->globalpreTime(),0.5);
  ASSERT_EQ(a->begin()->globalPosition(), Amg::Vector3D(0,0,2));
  ASSERT_EQ(a->begin()->localPosition(), Amg::Vector3D(0,0,2));
  ASSERT_EQ(a->begin()->globalPrePosition(), Amg::Vector3D(0,0,1));
  ASSERT_EQ(a->begin()->localPrePosition(), Amg::Vector3D(0,0,1));
  ASSERT_EQ(a->begin()->particleEncoding(), 22);
  ASSERT_EQ(a->begin()->kineticEnergy(), 0.5);
  ASSERT_EQ(a->begin()->globalDirection(), Amg::Vector3D(0,0,0));
  ASSERT_EQ(a->begin()->depositEnergy(), 0);
  ASSERT_EQ(a->begin()->StepLength(), 0);
  ASSERT_EQ(a->begin()->particleLink(), plink);

  ASSERT_EQ(a->size(), 1u); //test the current size of the Hit container
}

int main( int argc, char** argv ) {

  auto g=new GaudiEnvironment;
  ::testing::AddGlobalTestEnvironment(g);
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();

}

