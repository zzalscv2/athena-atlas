/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "CxxUtils/checker_macros.h"
ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

#include "src/CSCSensitiveDetector.h"

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
class CSCSensitiveDetectortest : public ::testing::Test {	
  protected:
    virtual void SetUp() override {
    }

    virtual void TearDown() override {
    }
};
//end of environment setting

TEST_F ( CSCSensitiveDetectortest, Initialize )
{
  G4HCofThisEvent hce;
  CSCSensitiveDetector sd1("name1", "name1" );
  sd1.Initialize( &hce );
  ASSERT_TRUE(sd1.m_myCSCHitColl.isValid()); //check if initialization of m_myCSCHitColl is successful
}

TEST_F ( CSCSensitiveDetectortest, ProcessHits )
{
  G4Step sp;
  G4TouchableHistory th; 
  G4HCofThisEvent hce;  

  G4double totalenergydeposit = 3.0;
  std::vector<G4String> physicalname = {"World::World","Atlas::Atlas","MUONQ02::MUONQ02","Muon::MuonSys","av_102_impr_1_Muon::CSL2CSC02_pv_1_cl[2]CSC02component","Muon::CscMultilayer","CscArCO2"};//set a proper name for the physical volume, which will decide which block of the tested class would be executed(i.e. block "else if ((npos = volName.find("CSC")) != std::string::npos && isAssembly )")
  G4String logicalname = "BBBBBBBBBTubeGas";
  std::vector<G4int> copynos = {0,0,0,0,1110600003,16969,0};
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
  G4String nop1 = "gamma";//set particle name as gamma
  G4String nop2 = "gamma";
  G4String nop3 = "gamma";
  DerivedG4SensitiveDetectorTestSetting(sp, totalenergydeposit, physicalname, logicalname, copynos, preStepPos, postStepPos, globaltime0, kineticenergy0, velocity0, globaltime, kineticenergy, globaltime1, kineticenergy1, velocity1, steplength, charge, encoding, antiencoding, astring, atype, nop1, nop2, nop3);//invoking of this function aims to setting testing environment

  CSCSensitiveDetector sd2("name2", "name2" );
  sd2.Initialize( &hce );
  sd2.ProcessHits(&sp, &th );//invoke the tested member function

  int barcode = 0;
  HepMcParticleLink plink(barcode);

  CSCSimHitCollection* a = sd2.m_myCSCHitColl.ptr();
  ASSERT_EQ(a->begin()->globalTime(), 0.5); //test if the globalTime value of the Hit generated by this member function is right, the same below
  ASSERT_EQ(a->begin()->energyDeposit(), 3);
  ASSERT_EQ(a->begin()->getHitStart(), Amg::Vector3D(0,0,1));
  ASSERT_EQ(a->begin()->getHitEnd(), Amg::Vector3D(0,0,2));
  ASSERT_EQ(a->begin()->particleID(), 1);
  ASSERT_EQ(a->begin()->CSCid(), 234691);
  ASSERT_EQ(a->begin()->kineticEnergy(), 1.5);
  ASSERT_EQ(a->begin()->particleLink(), plink);

  ASSERT_EQ(a->size(), 1u); //test the current size of the Hit container

}

int main( int argc, char** argv ) {

  auto g=new GaudiEnvironment;
  ::testing::AddGlobalTestEnvironment(g);
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();

}

