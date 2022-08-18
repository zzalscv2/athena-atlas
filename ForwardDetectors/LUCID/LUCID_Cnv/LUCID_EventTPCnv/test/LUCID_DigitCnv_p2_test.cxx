/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file LUCID_EventTPCnv/test/LUCID_DigitCnv_p2_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Feb, 2016
 * @brief Tests for LUCID_DigitCnv_p2.
 */


#undef NDEBUG
#include "LUCID_EventTPCnv/LUCID_DigitCnv_p2.h"
#include "CxxUtils/checker_macros.h"
#include "TestTools/leakcheck.h"
#include <cassert>
#include <iostream>


void compare (const LUCID_Digit& p1,
              const LUCID_Digit& p2)
{
  assert (p1.getTubeID() == p2.getTubeID());
  assert (p1.getNpe()    == p2.getNpe());
  assert (p1.getNpeGas() == p2.getNpeGas());
  assert (p1.getNpePmt() == p2.getNpePmt());
  assert (p1.getQDC()    == p2.getQDC());
  assert (p1.getTDC()    == p2.getTDC());
  assert (p1.isHit()     == p2.isHit());
}


void testit (const LUCID_Digit& trans1)
{
  MsgStream log (nullptr, "test");
  LUCID_DigitCnv_p2 cnv;
  LUCID_Digit_p2 pers;
  cnv.transToPers (&trans1, &pers, log);
  LUCID_Digit trans2;
  cnv.persToTrans (&pers, &trans2, log);

  compare (trans1, trans2);
}


void test1 ATLAS_NOT_THREAD_SAFE ()
{
  std::cout << "test1\n";
  Athena_test::Leakcheck check;

  LUCID_Digit trans1 (1, 2.5, 3, 4, 5, 6, true);
    
  testit (trans1);
}


int main ATLAS_NOT_THREAD_SAFE ()
{
  test1();
  return 0;
}
