/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file ParticleEventTPCnv/test/CompositeParticleCnv_p1_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Nov, 2015
 * @brief Regression tests.
 */

#undef NDEBUG
#include "ParticleEventTPCnv/CompositeParticleContainerCnv_p1.h"
#include "TestTools/leakcheck.h"
#include "ParticleEvent/CompositeParticleContainer.h"
#include "SGTools/TestStore.h"
#include "AthAllocators/DataPool.h"
#include "CxxUtils/checker_macros.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/ThreadLocalContext.h"
#include "TruthUtils/HepMCHelpers.h"
#include <cassert>
#include <iostream>

void compare (const ParticleEvent::Base& p1,
              const ParticleEvent::Base& p2)
{
  assert (p1.dataType() == p2.dataType());
  assert (p1.originLink() == p2.originLink());
  assert (p1.hasCharge() == p2.hasCharge());
  if (p1.hasCharge())
    assert (p1.charge() == p2.charge());
  assert (p1.hasPdgId() == p2.hasPdgId());
  if (p1.hasPdgId())
    assert (p1.pdgId() == p2.pdgId());
}


void compare (const P4ImplPxPyPzE& p1,
              const P4ImplPxPyPzE& p2)
{
  assert (p1.px() == p2.px());
  assert (p1.py() == p2.py());
  assert (p1.pz() == p2.pz());
  assert (p1.e() == p2.e());
}


void compare (const CompositeParticleNavigation& p1,
              const CompositeParticleNavigation& p2)
{
  assert (p1.size() == p2.size());
  CompositeParticleNavigation::object_iter i1 = p1.begin();
  CompositeParticleNavigation::object_iter i2 = p2.begin();
  for (size_t i = 0; i < p1.size(); ++i, ++i1, ++i2) {
    assert (*i1 == *i2);
    assert (p1.getParameter(i1) == p2.getParameter(i2));
  }
}


void compare (const CompositeParticle& p1,
              const CompositeParticle& p2)
{
  compare (p1.particleBase(), p2.particleBase());
  compare (p1.momentumBase(), p2.momentumBase());
  compare (p1.navigableBase(), p2.navigableBase());
}


void compare (const CompositeParticleContainer& c1,
              const CompositeParticleContainer& c2)
{
  assert (c1.size() == c2.size());
  for (size_t i = 0; i < c1.size(); i++)
    compare (*c1[i], *c2[i]);
}


void test1 ATLAS_NOT_THREAD_SAFE ()
{
  std::cout << "test1\n";
  (void)Gaudi::Hive::currentContext();
  AthenaBarCodeImpl dumbc; // Get services created.
  dumbc.getVersion();
  ElementLink<VxContainer> origlink ("orig", 10);
  {
    CompositeParticle tdum;
    tdum.navigableBase().insertElement (INav4MomLink ("part", 12));
    *tdum.navigableBase().begin();
  }
  MsgStream log (0, "test");
  {
    // Need to instantiate an instance of this before Leakcheck,
    // to get the allocator created.  But the instance will also
    // hold a lock on the allocator, so can't leave this live
    // or we'll deadlock.
    DataPool<CompositeParticle> pooldum;
  }
  Athena_test::Leakcheck check;

  CompositeParticleContainer trans1;
  auto p1 = std::make_unique<CompositeParticle>();
  p1->set4Mom (CLHEP::HepLorentzVector(100,200,300,400));
  p1->set_dataType (ParticleDataType::FastShower);
  p1->set_origin (origlink);
  p1->navigableBase().insertElement (INav4MomLink ("part", 12));
  p1->navigableBase().insertElement (INav4MomLink ("part", 19));

  auto p2 = std::make_unique<CompositeParticle>(*p1);
  p2->set_charge (1.5);

  auto p3 = std::make_unique<CompositeParticle>(*p1);
  p3->set_pdgId (MC::TQUARK);

  trans1.push_back (std::move(p1));
  trans1.push_back (std::move(p2));
  trans1.push_back (std::move(p3));

  CompositeParticleContainerCnv_p1 cnv;
  CompositeParticleContainer_p1 pers;
  cnv.transToPers (&trans1, &pers, log);

  CompositeParticleContainer trans2;
  cnv.persToTrans (&pers, &trans2, log);

  compare (trans1, trans2);
  {
    DataPool<CompositeParticle> pooldum;
    pooldum.erase();
  }
}


int main ATLAS_NOT_THREAD_SAFE ()
{
  SGTest::initTestStore();
  test1();
  return 0;
}
