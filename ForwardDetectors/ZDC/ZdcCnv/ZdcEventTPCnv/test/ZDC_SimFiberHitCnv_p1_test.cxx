/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file ZdcEventTPCnv/test/ZDC_SimFiberHitCnv_p1_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Mar, 2016
 * @brief Tests for ZDC_SimFiberHitCnv_p1.
 */


#undef NDEBUG
#include "ZdcEventTPCnv/ZDC_SimFiberHitCnv_p1.h"
#include "ZdcIdentifier/ZdcID.h"
#include "CxxUtils/checker_macros.h"
#include "TestTools/leakcheck.h"
#include <cassert>
#include <iostream>


void compare (const ZDC_SimFiberHit& p1,
              const ZDC_SimFiberHit& p2)
{
  assert (p1.getID() == p2.getID());
  assert (p1.getEdep() == p2.getEdep());
  assert (p1.getNPhotons() == p2.getNPhotons());
}


void testit (const ZDC_SimFiberHit& trans1)
{
  MsgStream log (nullptr, "test");
  ZDC_SimFiberHitCnv_p1 cnv;
  ZDC_SimFiberHit_p1 pers;
  cnv.transToPers (&trans1, &pers, log);
  ZDC_SimFiberHit trans2;
  cnv.persToTrans (&pers, &trans2, log);

  compare (trans1, trans2);
}


void test1 ATLAS_NOT_THREAD_SAFE ()
{
  std::cout << "test1\n";
  Athena_test::Leakcheck check;

  ZdcID zdcID;
  Identifier ID = zdcID.channel_id(1,2,3,4);

  ZDC_SimFiberHit trans1 (ID, 123, 12345.5);
    
  testit (trans1);
}


int main ATLAS_NOT_THREAD_SAFE ()
{
  test1();
  return 0;
}
