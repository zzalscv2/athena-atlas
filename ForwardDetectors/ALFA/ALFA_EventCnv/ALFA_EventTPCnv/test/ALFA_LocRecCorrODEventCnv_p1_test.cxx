/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file ALFA_EventTPCnv/test/ALFA_LocRecCorrODEventCnv_p1_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Mar, 2016
 * @brief Tests for ALFA_LocRecCorrODEventCnv_p1.
 */


#undef NDEBUG
#include "ALFA_EventTPCnv/ALFA_LocRecCorrODEventCnv_p1.h"
#include "CxxUtils/checker_macros.h"
#include "TestTools/leakcheck.h"
#include <cassert>
#include <iostream>


void compare (const ALFA_LocRecCorrODEvent& p1,
              const ALFA_LocRecCorrODEvent& p2)
{
  assert (p1.getAlgoNum() == p2.getAlgoNum());
  assert (p1.getPotNum() == p2.getPotNum());
  assert (p1.getSide() == p2.getSide());
  assert (p1.getYpositionLHC() == p2.getYpositionLHC());
  assert (p1.getZpositionLHC() == p2.getZpositionLHC());
  assert (p1.getYpositionPot() == p2.getYpositionPot());
  assert (p1.getYpositionStat() == p2.getYpositionStat());
  assert (p1.getYpositionBeam() == p2.getYpositionBeam());
}


void testit (const ALFA_LocRecCorrODEvent& trans1)
{
  MsgStream log (nullptr, "test");
  ALFA_LocRecCorrODEventCnv_p1 cnv;
  ALFA_LocRecCorrODEvent_p1 pers;
  cnv.transToPers (&trans1, &pers, log);
  ALFA_LocRecCorrODEvent trans2;
  cnv.persToTrans (&pers, &trans2, log);

  compare (trans1, trans2);
}


void test1 ATLAS_NOT_THREAD_SAFE ()
{
  std::cout << "test1\n";
  Athena_test::Leakcheck check;

  ALFA_LocRecCorrODEvent trans1 (123, 234, 345,
                                 10.5, 11.5,
                                 13.5, 14.5, 15.5);
    
  testit (trans1);
}


int main ATLAS_NOT_THREAD_SAFE ()
{
  test1();
  return 0;
}
