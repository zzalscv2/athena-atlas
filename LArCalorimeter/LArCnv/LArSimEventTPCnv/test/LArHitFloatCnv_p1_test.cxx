/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file LArSimEventTPCnv/test/LArHitFloatCnv_p1_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Mar, 2016
 * @brief Tests for LArHitFloatCnv_p1.
 */


#undef NDEBUG
#include "LArSimEventTPCnv/LArHitFloatCnv_p1.h"
#include "CxxUtils/checker_macros.h"
#include "TestTools/leakcheck.h"
#include <cassert>
#include <iostream>


void compare (const LArHitFloat& p1,
              const LArHitFloat& p2)
{
  assert (p1.cellID() == p2.cellID());
  assert (p1.energy() == p2.energy());
  assert (p1.time() == p2.time());
}


void testit (const LArHitFloat& trans1)
{
  MsgStream log (0, "test");
  LArHitFloatCnv_p1 cnv;
  LArHit_p1 pers;
  cnv.transToPers (&trans1, &pers, log);
  LArHitFloat trans2;
  cnv.persToTrans (&pers, &trans2, log);

  compare (trans1, trans2);
}


void test1 ATLAS_NOT_THREAD_SAFE ()
{
  std::cout << "test1\n";
  Athena_test::Leakcheck check;

  LArHitFloat trans1 (Identifier (0x1234), 10.5, 12.5);
    
  testit (trans1);
}


int main ATLAS_NOT_THREAD_SAFE ()
{
  test1();
  return 0;
}
