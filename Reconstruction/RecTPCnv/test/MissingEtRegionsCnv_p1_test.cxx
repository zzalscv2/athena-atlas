/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file RecTPCnv/test/MissingEtRegionsCnv_p1_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Nov, 2019
 * @brief Regression tests.
 */

#undef NDEBUG
#include "RecTPCnv/MissingEtRegionsCnv_p1.h"
#include "MissingETEvent/MissingEtRegions.h"
#include "TestTools/leakcheck.h"
#include "GaudiKernel/MsgStream.h"
#include <cassert>
#include <iostream>


void compare (const MissingEtRegions& p1,
              const MissingEtRegions& p2)
{
  assert (p1.exRegVec() == p2.exRegVec());
  assert (p1.eyRegVec() == p2.eyRegVec());
  assert (p1.etSumRegVec() == p2.etSumRegVec());
}


void testit (const MissingEtRegions& trans1)
{
  MsgStream log (0, "test");
  MissingEtRegionsCnv_p1 cnv;
  MissingEtRegions_p1 pers;
  cnv.transToPers (&trans1, &pers, log);
  MissingEtRegions trans2;
  cnv.persToTrans (&pers, &trans2, log);
  compare (trans1, trans2);
}


void test1()
{
  std::cout << "test1\n";
  Athena_test::Leakcheck check;

  MissingEtRegions trans1;
  trans1.setExRegVec (std::vector<double> {20.5, 21.5, 22.5});
  trans1.setEyRegVec (std::vector<double> {23.5, 24.5, 25.5});
  trans1.setEtSumRegVec (std::vector<double> {26.5, 27.5, 28.5});

  testit (trans1);
}


int main()
{
  std::cout << "RecTPCnv/MissingEtRegionsCnv_p1\n";
  test1();
  return 0;
}
