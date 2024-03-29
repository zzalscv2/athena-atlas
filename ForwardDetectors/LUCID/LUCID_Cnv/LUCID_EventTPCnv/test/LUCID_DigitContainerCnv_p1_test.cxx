/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file LUCID_EventTPCnv/test/LUCID_DigitContainerCnv_p1_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Feb, 2016
 * @brief Tests for LUCID_DigitContainerCnv_p1.
 */


#undef NDEBUG
#include "LUCID_EventTPCnv/LUCID_DigitContainerCnv_p1.h"
#include "TestTools/leakcheck.h"
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
  assert (false          == p2.isHit());
}



void compare (const LUCID_DigitContainer& p1,
              const LUCID_DigitContainer& p2)
{
  assert (p1.size() == p2.size());
  for (size_t i = 0; i < p1.size(); i++)
    compare (*p1[i], *p2[i]);
}


void testit (const LUCID_DigitContainer& trans1)
{
  MsgStream log (nullptr, "test");
  LUCID_DigitContainerCnv_p1 cnv;
  LUCID_DigitContainer_p1 pers;
  cnv.transToPers (&trans1, &pers, log);
  LUCID_DigitContainer trans2;
  cnv.persToTrans (&pers, &trans2, log);

  compare (trans1, trans2);
}


void test1 ATLAS_NOT_THREAD_SAFE ()
{
  std::cout << "test1\n";
  Athena_test::Leakcheck check;

  LUCID_DigitContainer trans1;
  for (int i=0; i < 10; i++) {
    int o = i*100;
    trans1.push_back (new LUCID_Digit (1+o, 2+o, 3+o, 4+o, 5+o, 6+o, i&1));
  }
    
  testit (trans1);
}


int main ATLAS_NOT_THREAD_SAFE ()
{
  test1();
  return 0;
}
