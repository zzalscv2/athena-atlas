/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file ALFA_EventTPCnv/test/ALFA_ODDigitCnv_p1_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Mar, 2016
 * @brief Tests for ALFA_ODDigitCnv_p1.
 */


#undef NDEBUG
#include "ALFA_EventTPCnv/ALFA_ODDigitCnv_p1.h"
#include "CxxUtils/checker_macros.h"
#include "TestTools/leakcheck.h"
#include <cassert>
#include <iostream>


void compare (const ALFA_ODDigit& p1,
              const ALFA_ODDigit& p2)
{
  assert (p1.getStation() == p2.getStation());
  assert (p1.getSide()    == p2.getSide());
  assert (p1.getPlate()   == p2.getPlate());
  assert (p1.getFiber()   == p2.getFiber());
}


void testit (const ALFA_ODDigit& trans1)
{
  MsgStream log (nullptr, "test");
  ALFA_ODDigitCnv_p1 cnv;
  ALFA_ODDigit_p1 pers;
  cnv.transToPers (&trans1, &pers, log);
  ALFA_ODDigit trans2;
  cnv.persToTrans (&pers, &trans2, log);

  compare (trans1, trans2);
}


void test1 ATLAS_NOT_THREAD_SAFE ()
{
  std::cout << "test1\n";
  Athena_test::Leakcheck check;

  ALFA_ODDigit trans1 (123, 234, 345, 456);
    
  testit (trans1);
}


int main ATLAS_NOT_THREAD_SAFE ()
{
  test1();
  return 0;
}
