/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "gtest/gtest.h"
#include "TrkExUtils/MaterialInteraction.h"
#include "TrkGeometry/Material.h"

//Test the MaterialInteraction functions with a few randomly chose values
//Material taken from Calorimeter scintillator values


TEST(MaterialInteraction, dEdl_ionization) {
  double p = 42.0;
  auto mat = new Trk::Material {424.35, 707.43, 11.16, 5.61, 0.001};// Scintillator/Glue (G4 def.)
  auto particle = Trk::electron;
  double sigma = 0;
  double kazL = 0;

  double energy_loss = Trk::MaterialInteraction::dEdl_ionization(p, mat, particle, sigma, kazL);
  EXPECT_FLOAT_EQ(energy_loss, -0.2338797);
  EXPECT_FLOAT_EQ(sigma, 0.030872595);
  EXPECT_FLOAT_EQ(kazL, 0);
}

TEST(MaterialInteraction, PDG_energyLoss_ionization) {
  double p = 42.0;
  auto mat = new Trk::Material {424.35, 707.43, 11.16, 5.61, 0.001};// Scintillator/Glue (G4 def.)
  auto particle = Trk::electron;
  double sigma = 0;
  double kazL = 0;
  double path = 42.0;

  double energy_loss = Trk::MaterialInteraction::PDG_energyLoss_ionization(p, mat, particle, sigma, kazL, path);
  EXPECT_FLOAT_EQ(energy_loss, -6.7087164);
  EXPECT_FLOAT_EQ(sigma, 0.54986054);
  EXPECT_FLOAT_EQ(kazL, 0.32421023);
}

TEST(MaterialInteraction, dEdl_radiation) {
  double p = 420000.0;
  auto mat = new Trk::Material {424.35, 707.43, 11.16, 5.61, 0.001};// Scintillator/Glue (G4 def.)
  auto particle = Trk::muon;
  double sigma = 0;

  double radiation = Trk::MaterialInteraction::dEdl_radiation(p, mat, particle, sigma);
  EXPECT_FLOAT_EQ(radiation, -0.096964628);
  EXPECT_FLOAT_EQ(sigma, 0.038813673);
}


TEST(MaterialInteraction, sigmaMS) {
  double dInX0 = 42.0;
  double p = 42.0;
  double beta = 42.0;
  EXPECT_FLOAT_EQ(Trk::MaterialInteraction::sigmaMS(dInX0, p, beta), 0.042868309);
}
