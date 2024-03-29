/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file ALFA_EventTPCnv/test/ALFA_HitCnv_p1_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Mar, 2016
 * @brief Tests for ALFA_HitCnv_p1.
 */


#undef NDEBUG
#include "ALFA_EventTPCnv/ALFA_HitCnv_p1.h"
#include "CxxUtils/checker_macros.h"
#include "TestTools/leakcheck.h"
#include <cassert>
#include <iostream>


void compare (const ALFA_Hit& p1,
              const ALFA_Hit& p2)
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
}


void testit (const ALFA_Hit& trans1)
{
  MsgStream log (nullptr, "test");
  ALFA_HitCnv_p1 cnv;
  ALFA_Hit_p1 pers;
  cnv.transToPers (&trans1, &pers, log);
  ALFA_Hit trans2;
  cnv.persToTrans (&pers, &trans2, log);

  compare (trans1, trans2);
}


void test1 ATLAS_NOT_THREAD_SAFE ()
{
  std::cout << "test1\n";
  Athena_test::Leakcheck check;

  ALFA_Hit trans1 (123, 234, 345,
                   10.5, 11.5,
                   12.5, 13.5, 14.5,
                   15.5, 16.5, 17.7,
                   18.5,
                   456, 567, 678, 789);
    
  testit (trans1);
}


int main ATLAS_NOT_THREAD_SAFE ()
{
  test1();
  return 0;
}
