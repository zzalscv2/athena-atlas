/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#include "src/TRTSensitiveDetector.h"

#include "gtest/gtest.h"

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
#include "G4MaterialTable.hh"
#include "globals.hh"
#include "G4SystemOfUnits.hh"
#include "G4Gamma.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4ProcessManager.hh"

#include "G4AtlasTools/DerivedG4PhysicalVolume.h"
#include "G4AtlasTools/DerivedG4SensitiveDetectorTestSetting.h"
#include "MCTruth/TrackHelper.h"

ATLAS_NO_CHECK_FILE_THREAD_SAFETY; // Thread unsafe TRTSensitiveDetector class is tested in this unit test.

//set environment
class GaudiEnvironment : public ::testing::Environment {
  protected:
  virtual void SetUp() override {
    Athena_test::initGaudi("TRT_G4_SD/optionForTest.txt", m_svcLoc);
  }
  ISvcLocator* m_svcLoc = nullptr;
};
class TRTSensitiveDetectortest : public ::testing::Test {	
  protected:
    virtual void SetUp() override {
    }

    virtual void TearDown() override {
    }
};
//end

TEST_F( TRTSensitiveDetectortest, Initialize)
{
// set materials for constuctor of TRTSensitiveDetector, calling of constuctor can be also considered as the test of InitializeHitProcessing()
  G4String name1, name2, name3;
  G4int Z1, Z2, Z3;
  G4double a1, a2, a3, density1, density2, density3;
  G4Material* Xe = new G4Material(name1="XeCO2O2", Z1=54,a1=131*g/mole, density1=2.942*g/cm3);
  G4Material* Kr = new G4Material(name2="KrCO2O2", Z2=36, a2=83*g/mole, density2=2.413*g/cm3);
  G4Material* Ar = new G4Material(name3="ArCO2O2", Z3=18, a3=40*g/mole, density3=1.395*g/cm3);
  (void)Xe;//just for silence those unused variables
  (void)Kr;
  (void)Ar;
//end

//this block aims to run through "G4Gamma::Definition()->GetProcessManager()->GetProcessList()" in Initialize
  G4ParticleDefinition particle("name",         0.0*MeV,       0.0*MeV,         0.0, 
		    2,              -1,            -1,          
		    0,               0,             0,             
	      "gamma",               0,             0,          22,
	      true,               -1.0,          NULL,
             false,           "photon",          22
	      );

  G4ProcessManager pm(&particle);
  G4Gamma::Definition()->SetProcessManager(&pm);
//end

 G4HCofThisEvent hce;

  TRTSensitiveDetector sd1("sd1", "sd1", 5);
  sd1.Initialize(&hce);
  ASSERT_TRUE(sd1.m_HitColl.isValid()); //test if the initialization of the smart pointer is successful
  sd1.DeleteObjects();
}

TEST_F( TRTSensitiveDetectortest, ProcessHits )
{
//this block aims to run through "G4Gamma::Definition()->GetProcessManager()->GetProcessList()" in Initialize
  G4ParticleDefinition particle("name1",         0.0*MeV,       0.0*MeV,         0.0, 
		    2,              -1,            -1,          
		    0,               0,             0,             
	      "gamma1",               0,             0,          22,
	      true,               -1.0,          NULL,
             false,           "photon1",          22
	      );

  G4ProcessManager pm(&particle);
  G4Gamma::Definition()->SetProcessManager(&pm);
//end

  G4HCofThisEvent hce;  
  G4Step sp;
  G4TouchableHistory th;

  G4double totalenergydeposit = 3.0;
  G4String physicalname = "physicalName";
  G4String logicalname = "whatever";
  G4int copyno = 1000;
  G4ThreeVector preStepPos = G4ThreeVector(0,0,1);
  G4ThreeVector postStepPos = G4ThreeVector(0,0,2);
  G4double globaltime0 = 0.5;
  G4double kineticenergy0 = 1.0;
  G4double velocity0 = 99.93100;
  G4double globaltime = 5.0;
  G4double kineticenergy = 0.5;
  G4double globaltime1 = 0;
  G4double kineticenergy1 = 0.5;
  G4double velocity1 = 99.93100;
  G4double steplength = 1.0;
  G4double charge = 0.0;
  G4int encoding = 22;
  G4int antiencoding = 22;
  G4String astring = "radiation";
  G4ProcessType atype = (G4ProcessType)6;
  G4String nop1 = "anyon";
  G4String nop2 = "anyon";
  G4String nop3 = "anyon";
  DerivedG4SensitiveDetectorTestSetting(sp, totalenergydeposit, physicalname, logicalname, copyno, preStepPos, postStepPos, globaltime0, kineticenergy0, velocity0, globaltime, kineticenergy, globaltime1, kineticenergy1, velocity1, steplength, charge, encoding, antiencoding, astring, atype, nop1, nop2, nop3);

  TRTSensitiveDetector sd2("sd2", "sd2", 5);
  sd2.Initialize(&hce);
  sd2.ProcessHits(&sp, &th);

  TRTUncompressedHitCollection* a = sd2.m_HitColl.ptr();
  ASSERT_EQ(a->begin()->GetHitID(), 67586); //test the HitID value of the Hit generated by the member function ProcessHits and stored in the smart pointer m_HitColl, and it should be 67586 based on the setting.The same below
  HepMcParticleLink plink;
  ASSERT_EQ(a->begin()->particleLink(), plink);
  ASSERT_EQ(a->begin()->GetParticleEncoding(), 22);
  ASSERT_EQ(a->begin()->GetKineticEnergy(), 1);
  ASSERT_EQ(a->begin()->GetEnergyDeposit(), 3000);
  ASSERT_EQ(a->begin()->GetPreStepX(), 0);
  ASSERT_EQ(a->begin()->GetPreStepY(), 0);
  ASSERT_EQ(a->begin()->GetPreStepZ(), 1);
  ASSERT_EQ(a->begin()->GetPostStepX(), 0);
  ASSERT_EQ(a->begin()->GetPostStepY(), 0);
  ASSERT_EQ(a->begin()->GetPostStepZ(), 2);
  ASSERT_EQ(a->begin()->GetGlobalTime(), 0.25);

  sd2.DeleteObjects();

}

TEST_F( TRTSensitiveDetectortest, AddHit )
{
//this block aims to run through "G4Gamma::Definition()->GetProcessManager()->GetProcessList()" in Initialize
  G4ParticleDefinition particle("name2",         0.0*MeV,       0.0*MeV,         0.0,
                    2,              -1,            -1,
                    0,               0,             0,
              "gamma2",               0,             0,          22,
              true,               -1.0,          NULL,
             false,           "photon2",          22
              );

  G4ProcessManager pm(&particle);
  G4Gamma::Definition()->SetProcessManager(&pm);
//end

  int hitID = 1;
  HepMcParticleLink partLink;
  int particleEncoding = 1;
  float kineticEnergy = 1;
  float energyDepositInKeV = 1;
  float preStepX = 1;
  float preStepY = 1;
  float preStepZ = 1;
  float postStepX = 1;
  float postStepY = 1;
  float postStepZ = 1;
  float globalTime = 1;

  TRTSensitiveDetector sd3("name3", "name3");
  G4HCofThisEvent hce;
  sd3.Initialize( &hce );
  sd3.AddHit(hitID, partLink, particleEncoding, kineticEnergy, energyDepositInKeV, preStepX, preStepY, preStepZ, postStepX, postStepY, postStepZ, globalTime);

  TRTUncompressedHitCollection* a = sd3.m_HitColl.ptr();
  ASSERT_EQ(a->begin()->GetHitID(), 1); //test the HitID value of the Hit added by the member function AddHit, and it should be the same with my input value 1. The same below
  HepMcParticleLink plink;
  ASSERT_EQ(a->begin()->particleLink(), plink);
  ASSERT_EQ(a->begin()->GetParticleEncoding(), 1);
  ASSERT_EQ(a->begin()->GetKineticEnergy(), 1);
  ASSERT_EQ(a->begin()->GetEnergyDeposit(), 1);
  ASSERT_EQ(a->begin()->GetPreStepX(), 1);
  ASSERT_EQ(a->begin()->GetPreStepY(), 1);
  ASSERT_EQ(a->begin()->GetPreStepZ(), 1);
  ASSERT_EQ(a->begin()->GetPostStepX(), 1);
  ASSERT_EQ(a->begin()->GetPostStepY(), 1);
  ASSERT_EQ(a->begin()->GetPostStepZ(), 1);
  ASSERT_EQ(a->begin()->GetGlobalTime(), 1);

  sd3.DeleteObjects();
}

int main( int argc, char** argv ) {

  auto g=new GaudiEnvironment;
  ::testing::AddGlobalTestEnvironment(g);
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();

}
