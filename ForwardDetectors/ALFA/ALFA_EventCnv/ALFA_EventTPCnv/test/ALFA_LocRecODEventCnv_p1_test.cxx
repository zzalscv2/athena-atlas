/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file ALFA_EventTPCnv/test/ALFA_LocRecODEventCnv_p1_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Mar, 2016
 * @brief Tests for ALFA_LocRecODEventCnv_p1.
 */


#undef NDEBUG
#include "ALFA_EventTPCnv/ALFA_LocRecODEventCnv_p1.h"
#include "CxxUtils/checker_macros.h"
#include "TestTools/leakcheck.h"
#include <cassert>
#include <iostream>


void compare (const ALFA_LocRecODEvent& p1,
              const ALFA_LocRecODEvent& p2)
{
  assert (p1.getAlgoNum() == p2.getAlgoNum());
  assert (p1.getPotNum() == p2.getPotNum());
  assert (p1.getSide() == p2.getSide());
  assert (p1.getYposition() == p2.getYposition());
  assert (p1.getOverY() == p2.getOverY());
  assert (p1.getNumY() == p2.getNumY());
  assert (p1.getFibSel() == p2.getFibSel());
}


void testit (const ALFA_LocRecODEvent& trans1)
{
  MsgStream log (nullptr, "test");
  ALFA_LocRecODEventCnv_p1 cnv;
  ALFA_LocRecODEvent_p1 pers;
  cnv.transToPers (&trans1, &pers, log);
  ALFA_LocRecODEvent trans2;
  cnv.persToTrans (&pers, &trans2, log);

  compare (trans1, trans2);
}


void test1 ATLAS_NOT_THREAD_SAFE ()
{
  std::cout << "test1\n";
  Athena_test::Leakcheck check;

  ALFA_LocRecODEvent trans1 (123, 234, 345,
                             10.5, 11.5,
                             543,
                             std::vector<int> {5, 6, 7});
  testit (trans1);
}


int main ATLAS_NOT_THREAD_SAFE ()
{
  test1();
  return 0;
}
