/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "CxxUtils/checker_macros.h"
ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

#include "src/BLMSensorSD.h"

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
#include "G4VUserTrackInformation.hh"

#include "G4AtlasTools/DerivedG4PhysicalVolume.h"
#include "G4AtlasTools/DerivedG4SensitiveDetectorTestSetting.h"
#include "MCTruth/TrackHelper.h"
#include "MCTruth/TrackInformation.h"
#include "MCTruth/VTrackInformation.h"
#include "AtlasHepMC/GenEvent.h"

//set environment
class GaudiEnvironment : public ::testing::Environment {
  protected:
  virtual void SetUp() override {
    Athena_test::initGaudi("BLM_G4_SD/optionForTest.txt", m_svcLoc);
  }
  ISvcLocator* m_svcLoc = nullptr;
};
class BLMSensorSDtest : public ::testing::Test {	
  protected:
    virtual void SetUp() override {
    }

    virtual void TearDown() override {
    }
};
//end of environment setting

TEST_F( BLMSensorSDtest, Initialize )
{
  G4HCofThisEvent hce;
  BLMSensorSD sd1("name1", "name1" );
  sd1.Initialize( &hce );
  ASSERT_TRUE(sd1.m_HitColl.isValid()); //test if the initialization of the smart pointer if successful
}

TEST_F( BLMSensorSDtest, ProcessHits )
{
  G4Step sp;
  G4TouchableHistory th; 
  G4HCofThisEvent hce;

  G4double totalenergydeposit = 3.0;
  std::vector<G4String> physicalname = {"physicalName"};
  G4String logicalname = "Pixel::blmDiamondLog";
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
  G4double charge = 0.0;
  G4int encoding = 0;
  G4int antiencoding = 0;
  G4String astring = "Cerenkov";
  G4ProcessType atype = (G4ProcessType)0;
  G4String nop1 = "opticalphoton";
  G4String nop2 = "opticalphoton";
  G4String nop3 = "photon";
  DerivedG4SensitiveDetectorTestSetting(sp, totalenergydeposit, physicalname, logicalname, copynos, preStepPos, postStepPos, globaltime0, kineticenergy0, velocity0, globaltime, kineticenergy, globaltime1, kineticenergy1, velocity1, steplength, charge, encoding, antiencoding, astring, atype, nop1, nop2, nop3);

  BLMSensorSD sd2("name2", "name2");
  sd2.Initialize( &hce );
  sd2.ProcessHits(&sp, &th );

  HepGeom::Point3D<double> P1,P2;
  P1[SiHit::xEta] = 1;
  P1[SiHit::xPhi] = 0;
  P1[SiHit::xDep] = 0;
  P2[SiHit::xEta] = 2;
  P2[SiHit::xPhi] = 0;
  P2[SiHit::xDep] = 0;

  int barcode = 0;
  HepMcParticleLink plink(barcode);

  SiHitCollection* a = sd2.m_HitColl.ptr();
  ASSERT_EQ(a->begin()->localStartPosition(), P1); //test the localStartPosition value of the Hit generated by the member function ProcessHits and stored in the smart pointer m_HitColl, and it should be P1 based on the setting. The same below
  ASSERT_EQ(a->begin()->localEndPosition(), P2);
  ASSERT_EQ(a->begin()->energyLoss(), 3.0);
  ASSERT_EQ(a->begin()->meanTime(), 0.5);
  ASSERT_EQ(a->begin()->particleLink(), plink);
  ASSERT_EQ(a->begin()->identify(),8497588u);

  ASSERT_EQ(a->size(), 1u); //test the size of the Hit container which should be 1 since only 1 Hit was stored in it

}

TEST_F( BLMSensorSDtest, AddHit )
{
  HepGeom::Point3D<double> lP1,lP2;
  lP1[SiHit::xEta] = 1;
  lP1[SiHit::xPhi] = 0;
  lP1[SiHit::xDep] = 0;
  lP2[SiHit::xEta] = 2;
  lP2[SiHit::xPhi] = 0;
  lP2[SiHit::xDep] = 0;
  
  G4DynamicParticle* dynamicPar = new G4DynamicParticle();
  G4double aValueTime = 1;
  G4ThreeVector ValuePosition(1.0, 1.0, 1.0);
  G4Track track(dynamicPar, aValueTime, ValuePosition);
  TrackHelper trHelp(&track);

  int brlEcap = 0;
  int layerDisk = 0;
  int etaMod = 0;
  int phiMod = 0;
  int side = 0;
  BLMSensorSD sd3("name3", "name3");
  G4HCofThisEvent hce;
  sd3.Initialize( &hce );
  sd3.AddHit(lP1, lP2, 2.0, 1.0, trHelp.GetParticleLink(), 1, brlEcap, layerDisk, etaMod, phiMod, side);

  HepGeom::Point3D<double> P1,P2;
  P1[SiHit::xEta] = 1;
  P1[SiHit::xPhi] = 0;
  P1[SiHit::xDep] = 0;
  P2[SiHit::xEta] = 2;
  P2[SiHit::xPhi] = 0;
  P2[SiHit::xDep] = 0;
  
  int barcode = 0;
  HepMcParticleLink plink(barcode);
  
  SiHitCollection* a = sd3.m_HitColl.ptr();
  ASSERT_EQ(a->begin()->localStartPosition(), P1); //test the localStartPosition value of the Hit added by the member function AddHit, and it should be P1 based on the setting. The same below
  ASSERT_EQ(a->begin()->localEndPosition(), P2);
  ASSERT_EQ(a->begin()->energyLoss(), 2.0);
  ASSERT_EQ(a->begin()->meanTime(), 1.0);
  ASSERT_EQ(a->begin()->particleLink(), plink);
  ASSERT_EQ(a->begin()->identify(), 10245u);

  ASSERT_EQ(a->size(), 1u); //test the size of the Hit container which should be 1 since only 1 Hit was stored in it

}

int main( int argc, char** argv ) {

  auto g=new GaudiEnvironment;
  ::testing::AddGlobalTestEnvironment(g);
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();

}
