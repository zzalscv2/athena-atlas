/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file MuonEventTPCnv/test/sTgcPrepDataCnv_p1_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Jan, 2016
 * @brief Regression tests.
 */

#undef NDEBUG
#include "MuonEventTPCnv/MuonPrepRawData/sTgcPrepDataCnv_p1.h"
#include "CxxUtils/checker_macros.h"
#include "TestTools/leakcheck.h"
#include "GaudiKernel/MsgStream.h"
#include <cassert>
#include <iostream>


void compare (const Trk::PrepRawData& p1,
              const Trk::PrepRawData& p2)
{
  assert (p1.localPosition()[0] == p2.localPosition()[0]);
  assert (p1.localCovariance() == p2.localCovariance());
  assert (p1.rdoList().size() == p2.rdoList().size() );
}


void compare (const Muon::MuonCluster& p1,
              const Muon::MuonCluster& p2)
{
  compare (static_cast<const Trk::PrepRawData&>(p1),
           static_cast<const Trk::PrepRawData&>(p2));
  //assert (p1.globalPosition() == p2.globalPosition());
}


void compare (const Muon::sTgcPrepData& p1,
              const Muon::sTgcPrepData& p2)
{
  compare (static_cast<const Muon::MuonCluster&>(p1),
           static_cast<const Muon::MuonCluster&>(p2));
  assert (p1.detectorElement() == p2.detectorElement());
  //assert (p1.getBcBitMap() == p2.getBcBitMap());
  //assert (p1.globalPosition() == p2.globalPosition());
}

void testit (const Muon::sTgcPrepData& trans1)
{
  MsgStream log (nullptr, "test");
  sTgcPrepDataCnv_p1 cnv;
  Muon::sTgcPrepData_p1 pers;
  cnv.transToPers (&trans1, &pers, log);
  Muon::sTgcPrepData trans2;
  cnv.persToTrans (&pers, &trans2, log);

  compare (trans1, trans2);
}


void test1 ATLAS_NOT_THREAD_SAFE ()
{
  std::cout << "test1\n";
  Athena_test::Leakcheck check;

  Amg::Vector2D locpos (2.5, 3.5);

  Amg::MatrixX cov(1,1);
  cov(0,0) = 101;

  std::vector<Identifier> rdoList { Identifier(1274),
                                    Identifier(1234),
                                    Identifier(1178) };

  Muon::sTgcPrepData trans1 (Identifier (1234),
                             IdentifierHash (1234),
                             std::move(locpos),
                             std::move(rdoList),
                             std::move(cov),
                             nullptr,
                             123);
                            
  testit (trans1);
}


int main ATLAS_NOT_THREAD_SAFE ()
{
  test1();
  return 0;
}
