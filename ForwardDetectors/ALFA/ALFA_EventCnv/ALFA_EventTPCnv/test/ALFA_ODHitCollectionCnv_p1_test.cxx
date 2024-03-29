/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file ALFA_EventTPCnv/test/ALFA_ODHitCollectionCnv_p1_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Mar, 2016
 * @brief Tests for ALFA_ODHitCollectionCnv_p1.
 */


#undef NDEBUG
#include "ALFA_EventTPCnv/ALFA_ODHitCollectionCnv_p1.h"
#include "CxxUtils/checker_macros.h"
#include "TestTools/leakcheck.h"
#include <cassert>
#include <iostream>


void compare (const ALFA_ODHit& p1,
              const ALFA_ODHit& p2)
{
  assert (p1.GetHitID() == p2.GetHitID());
  assert (p1.GetParticleEncoding() == p2.GetParticleEncoding());
  assert (p1.GetKineticEnergy() == p2.GetKineticEnergy());
  assert (p1.GetEnergyDeposit() == p2.GetEnergyDeposit());
  assert (p1.GetPreStepX() == p2.GetPreStepX());
  assert (p1.GetPreStepY() == p2.GetPreStepY());
  assert (p1.GetPreStepZ() == p2.GetPreStepZ());
  assert (p1.GetPostStepX() == p2.GetPostStepX());
  assert (p1.GetPostStepY() == p2.GetPostStepY());
  assert (p1.GetPostStepZ() == p2.GetPostStepZ());
  assert (p1.GetGlobalTime() == p2.GetGlobalTime());
  assert (p1.GetSignFiber() == p2.GetSignFiber());
  assert (p1.GetPlateNumber() == p2.GetPlateNumber());
  assert (p1.GetFiberNumber() == p2.GetFiberNumber());
  assert (p1.GetStationNumber() == p2.GetStationNumber());
  assert (p1.GetODSide() == p2.GetODSide());
}


void compare (const ALFA_ODHitCollection& p1,
              const ALFA_ODHitCollection& p2)
{
  assert (p1.Name() == p2.Name());
  assert (p1.size() == p2.size());
  for (size_t i=0; i < p1.size(); i++)
    compare (p1[i], p2[i]);
}


void testit (const ALFA_ODHitCollection& trans1)
{
  MsgStream log (nullptr, "test");
  ALFA_ODHitCollectionCnv_p1 cnv;
  ALFA_ODHitCollection_p1 pers;
  cnv.transToPers (&trans1, &pers, log);
  ALFA_ODHitCollection trans2;
  cnv.persToTrans (&pers, &trans2, log);

  compare (trans1, trans2);
}


void test1 ATLAS_NOT_THREAD_SAFE ()
{
  std::cout << "test1\n";
  // Get dvl info created outside of leak checking.
  ALFA_ODHitCollection dum1 ("coll");
  Athena_test::Leakcheck check;

  ALFA_ODHitCollection trans1 ("coll");
  for (int i=0; i < 10; i++) {
    int o = i*100;
    trans1.Emplace (123+o, 234+o, 345+o,
                    10.5+o, 11.5+o,
                    12.5+o, 13.5+o, 14.5+o,
                    15.5+o, 16.5+o, 17.7+o,
                    18.5+o,
                    456+o, i%2, 567+o, 678+o, 789+o);
  }
    
  testit (trans1);
}


int main ATLAS_NOT_THREAD_SAFE ()
{
  test1();
  return 0;
}
