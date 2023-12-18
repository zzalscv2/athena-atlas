/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file InDetEventTPCnv/test/TRT_DriftCircleContainerCnv_p2_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Dec, 2015
 * @brief Regression tests.
 */

#undef NDEBUG
#include "InDetEventTPCnv/TRT_DriftCircleContainerCnv_p2.h"
#include "TestTools/leakcheck.h"
//#include "TRT_ReadoutGeometry/TRT_DetectorManager.h"
#include "StoreGate/StoreGateSvc.h"
#include "TestTools/initGaudi.h"
#include "InDetIdentifier/TRT_ID.h"
#include "IdDictParser/IdDictParser.h"
#include "SGTools/TestStore.h"
#include "TRT_ReadoutGeometry/TRT_DetElementContainer.h"
#include "CxxUtils/checker_macros.h"

#include "GaudiKernel/MsgStream.h"

#include <cassert>
#include <iostream>
#include <memory>

void compare (const Trk::PrepRawData& p1,
              const Trk::PrepRawData& p2)
{
  assert (p1.identify() == p2.identify());
  assert (p1.localPosition()[0] == p2.localPosition()[0]);
  assert (p1.rdoList() == p2.rdoList());
  assert (p1.localCovariance()(0,0) == p2.localCovariance()(0,0));
}


void compare (const InDet::TRT_DriftCircle& p1,
              const InDet::TRT_DriftCircle& p2)
{
  compare (static_cast<const Trk::PrepRawData&> (p1),
           static_cast<const Trk::PrepRawData&> (p2));
  assert (p1.getWord() == p2.getWord());
}


void compare (const InDet::TRT_DriftCircleContainer& p1,
              const InDet::TRT_DriftCircleContainer& p2)
{
  InDet::TRT_DriftCircleContainer::const_iterator it1 = p1.begin();
  InDet::TRT_DriftCircleContainer::const_iterator it1e = p1.end();
  InDet::TRT_DriftCircleContainer::const_iterator it2 = p2.begin();
  InDet::TRT_DriftCircleContainer::const_iterator it2e = p2.end();
  while (it1 != it1e && it2 != it2e) {
    assert (it1.hashId() == it2.hashId());
    const InDet::TRT_DriftCircleCollection& coll1 = **it1;
    const InDet::TRT_DriftCircleCollection& coll2 = **it2;
    assert (coll1.size() == coll2.size());
    for (size_t j = 0; j < coll1.size(); j++) {
      compare (*coll1[j], *coll2[j]);
    }
    ++it1;
    ++it2;
  }
  assert (it1 == it1e && it2 == it2e);
}


void testit (const InDet::TRT_DriftCircleContainer& trans1, const TRT_ID& trt_id)
{
  MsgStream log (nullptr, "test");
  TRT_DriftCircleContainerCnv_p2 cnv;
  cnv.setIdHelper(&trt_id);
  cnv.setUseDetectorElement(false);
  InDet::TRT_DriftCircleContainer_p2 pers;
  cnv.transToPers (&trans1, &pers, log);
  std::unique_ptr<InDet::TRT_DriftCircleContainer> trans2
    (cnv.createTransient (&pers, log));
  #if 0

  compare (trans1, *trans2);
  #endif
}


std::unique_ptr<const InDet::TRT_DriftCircleContainer>
makeclusts (const TRT_ID& trt_id)
{
  auto cont = std::make_unique<InDet::TRT_DriftCircleContainer>(5);

  for (int hash=2; hash <= 3; hash++) {
    auto coll = std::make_unique<InDet::TRT_DriftCircleCollection>(IdentifierHash(hash));
    coll->setIdentifier (trt_id.layer_id (hash));

    for (int i=0; i < 10; i++) {
      int offs = i*10 + hash*100;
      Identifier clusId (coll->identify().get_identifier32().get_compact() + offs+1234);

      Amg::Vector2D locpos (offs+1.5, offs+2.5);
      std::vector<Identifier> rdoList { clusId };

      Amg::MatrixX cov(2,2);
      for (int i=0; i < 2; i++)
        for (int j=0; j < 2; j++)
          cov(i,j) = 100*(i+1)*(j+1) + offs;

      auto cl = std::make_unique<InDet::TRT_DriftCircle>
        (clusId,
         locpos,
         std::move(rdoList),
         std::move(cov),
         nullptr,
         54321);
      coll->push_back (std::move (cl));
    }
    assert( cont->addCollection (coll.release(), hash).isSuccess() );
  }

  // gcc4.9 doesn't allow returning cont directly here; fixed in 5.2.
  return std::unique_ptr<const InDet::TRT_DriftCircleContainer> (cont.release());
}


void test1 ATLAS_NOT_THREAD_SAFE (const TRT_ID& trt_id)
{
  std::cout << "test1\n";
  MsgStream log (nullptr, "test");

  {
    // Do it once without leak checking to get services initialized.
    std::unique_ptr<const InDet::TRT_DriftCircleContainer> cont = makeclusts(trt_id);
    testit (*cont,trt_id);
  }

  // And again with leak checking.
  Athena_test::Leakcheck check;
  std::unique_ptr<const InDet::TRT_DriftCircleContainer> cont = makeclusts(trt_id);
  testit (*cont,trt_id);
}


const TRT_ID& make_dd()
{
  auto trt_id = std::make_unique<TRT_ID>();
  const TRT_ID& ret = *trt_id;
  IdDictParser parser;
  parser.register_external_entity ("InnerDetector",
                                   "IdDictInnerDetector.xml");
  IdDictMgr& idd = parser.parse ("IdDictParser/ATLAS_IDS.xml");
  trt_id->initialize_from_dictionary (idd);

  ISvcLocator* svcLoc = Gaudi::svcLocator();
  StoreGateSvc* detStore = nullptr;
  assert ( svcLoc->service("DetectorStore", detStore).isSuccess() );
  assert ( detStore->record (std::move (trt_id), "TRT_ID") );

  //auto sct_dd = std::make_unique<InDetDD::TRT_DetectorManager>(detStore);
  //assert ( detStore->record (std::move (sct_dd), "TRT_DetectorDescription") );
  return ret;
}


int main ATLAS_NOT_THREAD_SAFE ()
{
  ISvcLocator* pSvcLoc;
  if (!Athena_test::initGaudi("InDetEventTPCnv_test.txt", pSvcLoc)) {
    std::cerr << "This test can not be run" << std::endl;
    return 0;
  }

  const TRT_ID& trt_id = make_dd();

  test1 (trt_id);
  return 0;
}
