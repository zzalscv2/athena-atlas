/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file McParticleEventTPCnv/test/TruthParticleContainerCnv_p5_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Dec, 2019
 * @brief Tests for TruthParticleContainerCnv_p5.
 */


#undef NDEBUG
#include "McParticleEventTPCnv/TruthParticleContainerCnv_p5.h"
#include "../src/RootTruthParticleCnvTool.h"
#include "GeneratorObjects/McEventCollection.h"
#include "SGTools/TestStore.h"
#include "TestTools/leakcheck.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/ThreadLocalContext.h"
// Athena
#include "AthenaKernel/ExtendedEventContext.h"
#include <cassert>
#include <iostream>


void compare (const TruthParticle& p1,
              const TruthParticle& p2)
{
  assert ( p1.px() == p2.px() );
  assert ( p1.py() == p2.py() );
  assert ( p1.pz() == p2.pz() );
  assert ( p1.e()  == p2.e() );
  assert ( p1.charge()  == p2.charge() );
  assert ( p1.genParticle() == p2.genParticle() );
  assert ( p1.genEventIndex() == p2.genEventIndex() );

  assert (p1.nParents() == p2.nParents());
  for (unsigned int i = 0; i < p1.nParents(); i++) {
    assert (p1.mother(i)->getAthenaBarCode() == p2.mother(i)->getAthenaBarCode());
  }

  assert (p1.nDecay() == p2.nDecay());
  for (unsigned int i = 0; i < p1.nDecay(); i++) {
    assert (p1.child(i)->getAthenaBarCode() == p2.child(i)->getAthenaBarCode());
  }
}


void compare (const TruthParticleContainer& p1,
              const TruthParticleContainer& p2)
{
  assert (p1.size() == p2.size());
  for (size_t i = 0; i < p1.size(); i++) {
    compare (*p1[i], *p2[i]);
  }

  assert (p1.genEventLink() == p2.genEventLink());
  assert (p1.etIsolationsLink() == p2.etIsolationsLink());
}


void transToPers (const TruthParticleContainer& trans1,
                  TruthParticleContainer_p5& pers)
{
  pers.m_genEvent.m_contName = trans1.genEventLink().dataID();
  pers.m_genEvent.m_elementIndex = trans1.genEventLink().index();
  pers.m_genEvent.m_SGKeyHash = trans1.genEventLink().key();

  pers.m_etIsolations.m_contName = trans1.etIsolationsLink().dataID();
  pers.m_etIsolations.m_elementIndex = trans1.etIsolationsLink().index();
  pers.m_etIsolations.m_SGKeyHash = trans1.etIsolationsLink().key();
}


void testit (const TruthParticleContainer& trans1,
             ITruthParticleCnvTool& cnvTool)
{
  MsgStream log (0, "test");
  TruthParticleContainerCnv_p5 cnv (&cnvTool);
  TruthParticleContainer_p5 pers;
  transToPers (trans1, pers);
  TruthParticleContainer trans2;
  cnv.persToTrans (&pers, &trans2, log);

  compare (trans1, trans2);
}


void test1 (SGTest::TestStore& store)
{
  std::cout << "test1\n";

  auto evcoll = std::make_unique<McEventCollection>();
#ifdef HEPMC3
  //Signal process id is obsolete in HepMC3
  evcoll->push_back (std::make_unique<HepMC::GenEvent>());
  evcoll->back()->set_event_number(4);
  evcoll->push_back (std::make_unique<HepMC::GenEvent>());
  evcoll->back()->set_event_number(5);
  auto ge = std::make_unique<HepMC::GenEvent>();
  ge->set_event_number(7);
  auto gv = HepMC::newGenVertexPtr();
  std::vector<HepMC::GenParticlePtr> parts;
  for (size_t i = 0; i < 5; i++) {
    auto gp = HepMC::newGenParticlePtr(HepMC::FourVector (i*10 + 1.5,i*10 + 2.5,i*10 + 3.5,i*10 + 4.5),i+20);
    parts.push_back (gp);
    gv->add_particle_out (gp);
  }
  ge->add_vertex (gv);
#else
  evcoll->push_back (std::make_unique<HepMC::GenEvent>(1000082, 4));
  evcoll->push_back (std::make_unique<HepMC::GenEvent>(1000087, 5));

  auto ge = std::make_unique<HepMC::GenEvent>(1000083, 7);
  auto gv = std::make_unique<HepMC::GenVertex>();
  std::vector<HepMC::GenParticle*> parts;
  for (size_t i = 0; i < 5; i++) {
    auto gp = std::make_unique<HepMC::GenParticle>
      (HepMC::FourVector (i*10 + 1.5,
                          i*10 + 2.5,
                          i*10 + 3.5,
                          i*10 + 4.5),
       i+20);
    parts.push_back (gp.get());
    gv->add_particle_out (gp.release());
  }
  ge->add_vertex (gv.release());
#endif
  evcoll->push_back (std::move(ge));
  store.record (std::move(evcoll), "GEN_AOD");
  // create a dummy EventContext
  EventContext ctx;
  ctx.setExtension( Atlas::ExtendedEventContext( &store ) );
  Gaudi::Hive::setCurrentContext( ctx );


  RootTruthParticleCnvTool cnvTool;
  ElementLink<McEventCollection> evlink ("GEN_AOD", 2);
  ElementLink<TruthEtIsolationsContainer> isoLink ("isol", 1);

  TruthParticleContainer trans1;
  trans1.setGenEvent (evlink);
  trans1.setEtIsolations (isoLink);

  for (size_t i = 0; i < 5; i++) {
    auto tp = std::make_unique<TruthParticle> (parts[i], &trans1);
    tp->setCharge (cnvTool.chargeFromPdgId (tp->pdgId()));
    trans1.push_back (std::move (tp));
  }

  //Athena_test::Leakcheck check;
  testit (trans1, cnvTool);
}


int main()
{
  std::cout << "McParticleEventTPCnv/TruthParticleContainerCnv_p5_test\n";
  std::unique_ptr<SGTest::TestStore> store = SGTest::getTestStore();
  test1 (*store);
  return 0;
}
