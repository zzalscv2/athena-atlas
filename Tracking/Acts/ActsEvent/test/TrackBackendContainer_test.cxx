/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#undef NDEBUG
#define BOOST_TEST_MODULE MultiTrajectoryBasic_test
#include <boost/test/data/test_case.hpp>
#include <boost/test/included/unit_test.hpp>
#include <Acts/EventData/TrackContainer.hpp>

#include "ActsEvent/MultiTrajectory.h"
#include "ActsEvent/TrackBackendContainer.h"
#include "xAODTracking/TrackBackendContainer.h"
#include "xAODTracking/TrackBackendAuxContainer.h"

BOOST_AUTO_TEST_SUITE(EventDataMultiTrajectory)

BOOST_AUTO_TEST_CASE(ConstCompilesWithInterface) {
  ACTS_STATIC_CHECK_CONCEPT(Acts::ConstTrackContainerBackend,
                            ActsTrk::ConstTrackBackendContainer);

  // Doesn't compile --- ConstTrackContainerBackend is not sufficient
  // to meet the requirements of TrackContainer.
  //using ConstTrackContainer = Acts::TrackContainer<ActsTrk::ConstTrackBackendContainer, ActsTrk::ConstMultiTrajectory>;

}

BOOST_AUTO_TEST_CASE(MutableCompilesWithInterface) {
  ACTS_STATIC_CHECK_CONCEPT(Acts::TrackContainerBackend,
                            ActsTrk::MutableTrackBackendContainer);

  using MutableTrackContainer = Acts::TrackContainer<ActsTrk::MutableTrackBackendContainer, ActsTrk::MutableMultiTrajectory>;
}

struct EmptyBackend {
  EmptyBackend() {
    backend = std::make_unique<xAOD::TrackBackendContainer>();
    backendAux = std::make_unique<xAOD::TrackBackendAuxContainer>();
    backend->setStore(backendAux.get());

    m = std::make_unique<ActsTrk::MutableTrackBackendContainer>(backend.get());
    c = m.get();
  }

  std::unique_ptr<xAOD::TrackBackendContainer> backend;
  std::unique_ptr<xAOD::TrackBackendAuxContainer> backendAux;

  std::unique_ptr<ActsTrk::MutableTrackBackendContainer> m;
  const ActsTrk::ConstTrackBackendContainer* c = nullptr;
};

BOOST_FIXTURE_TEST_CASE(BareContainerFill, EmptyBackend) {
  using namespace Acts::HashedStringLiteral;

  BOOST_CHECK_EQUAL(m->size_impl(), 0);
  m->addColumn_impl<int>("author");
  BOOST_CHECK(m->hasColumn_impl("author"_hash));
  BOOST_CHECK(m->hasColumn_impl("z0"_hash) == false);

  m->addTrack_impl();

  *std::any_cast<float*>(m->component_impl("chi2"_hash, 0)) = 2.5f;
  *std::any_cast<unsigned int*>(m->component_impl("nHoles"_hash, 0)) = 2;
  *std::any_cast<int*>(m->component_impl("author"_hash, 0)) = 77;

  BOOST_CHECK_EQUAL(*std::any_cast<float*>(m->component_impl("chi2"_hash, 0)), 2.5f);
  BOOST_CHECK_EQUAL(*std::any_cast<unsigned int*>(m->component_impl("nHoles"_hash, 0)), 2);
  BOOST_CHECK_EQUAL(*std::any_cast<int*>(m->component_impl("author"_hash, 0)), 77);

  BOOST_CHECK_EQUAL(m->size_impl(), 1);  
  m->addTrack_impl();
  BOOST_CHECK_EQUAL(*std::any_cast<float*>(m->component_impl("chi2"_hash, 1)), 0.0f);
  BOOST_CHECK_EQUAL(*std::any_cast<unsigned int*>(m->component_impl("nHoles"_hash, 1)), 0);
  BOOST_CHECK_EQUAL(*std::any_cast<int*>(m->component_impl("author"_hash, 1)), 0);



  // test if can access the data back from const interface
  BOOST_CHECK(c->hasColumn_impl("chi2"_hash));
  BOOST_CHECK(c->hasColumn_impl("author"_hash));
  BOOST_CHECK(c->hasColumn_impl("z0"_hash) == false);

  BOOST_CHECK_EQUAL(*std::any_cast<const float*>(c->component_impl("chi2"_hash, 0)), 2.5f);
  BOOST_CHECK_EQUAL(*std::any_cast<const unsigned int*>(c->component_impl("nHoles"_hash, 0)), 2);
  BOOST_CHECK_EQUAL(*std::any_cast<const int*>(c->component_impl("author"_hash, 0)), 77);
}


BOOST_FIXTURE_TEST_CASE(ContainerFill, EmptyBackend) {
  // TODO test complete container
}

BOOST_AUTO_TEST_SUITE_END()
