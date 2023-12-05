/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file ZdcEventTPCnv/test/ZDC_SimFiberHit_CollectionCnv_p1_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Mar, 2016
 * @brief Tests for ZDC_SimFiberHit_CollectionCnv_p1.
 */


#undef NDEBUG
#include "ZdcEventTPCnv/ZDC_SimFiberHit_CollectionCnv_p1.h"
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


void compare (const ZDC_SimFiberHit_Collection& p1,
              const ZDC_SimFiberHit_Collection& p2)
{
  //assert (p1.Name() == p2.Name());
  assert (p1.size() == p2.size());
  for (size_t i = 0; i < p1.size(); i++)
    compare (p1[i], p2[i]);
}


void testit (const ZDC_SimFiberHit_Collection& trans1)
{
  MsgStream log (nullptr, "test");
  ZDC_SimFiberHit_CollectionCnv_p1 cnv;
  ZDC_SimFiberHit_Collection_p1 pers;
  cnv.transToPers (&trans1, &pers, log);
  ZDC_SimFiberHit_Collection trans2;
  cnv.persToTrans (&pers, &trans2, log);

  compare (trans1, trans2);
}


void test1 ATLAS_NOT_THREAD_SAFE ()
{
  std::cout << "test1\n";
  ZDC_SimFiberHit_Collection dum1 ("coll");
  Athena_test::Leakcheck check;

  ZDC_SimFiberHit_Collection trans1 ("coll");
  ZdcID zdcID;
  for (int i = 0; i < 10; i++) {
    int o = i*100;
    Identifier ID = zdcID.channel_id( i/5, i/2, i/2, o);
    trans1.Emplace (ID, 21+o, 12345.5+o);
  }
    
  testit (trans1);
}


int main ATLAS_NOT_THREAD_SAFE ()
{
  test1();
  return 0;
}
