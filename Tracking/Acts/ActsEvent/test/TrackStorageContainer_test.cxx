/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include <boost/test/tools/old/interface.hpp>
#undef NDEBUG
#define BOOST_TEST_MODULE MultiTrajectoryBasic_test
#include <boost/test/data/test_case.hpp>
#include <boost/test/included/unit_test.hpp>
#include <Acts/EventData/TrackContainer.hpp>

#include "ActsEvent/MultiTrajectory.h"
#include "ActsEvent/TrackStorageContainer.h"
#include "xAODTracking/TrackStorageContainer.h"
#include "xAODTracking/TrackStorageAuxContainer.h"

BOOST_AUTO_TEST_SUITE(EventDataTrackStorage)

BOOST_AUTO_TEST_CASE(ConstCompilesWithInterface) {
  ACTS_STATIC_CHECK_CONCEPT(Acts::ConstTrackContainerBackend,
                            ActsTrk::TrackStorageContainer);
}

BOOST_AUTO_TEST_CASE(MutableCompilesWithInterface) {
  ACTS_STATIC_CHECK_CONCEPT(Acts::TrackContainerBackend,
                            ActsTrk::MutableTrackStorageContainer);

  using MutableTrackContainer = Acts::TrackContainer<ActsTrk::MutableTrackStorageContainer, ActsTrk::MutableMultiTrajectory>;
}

struct EmptyBackend {
  EmptyBackend() {
    m = std::make_unique<ActsTrk::MutableTrackStorageContainer>();
  }
  std::unique_ptr<ActsTrk::MutableTrackStorageContainer> m;
};

BOOST_FIXTURE_TEST_CASE(AllStaticxAODVaraiblesAreKnown, EmptyBackend) {
  for (auto id : m->backend()->getConstStore()->getAuxIDs()) {
    const std::string name = SG::AuxTypeRegistry::instance().getName(id);
    BOOST_CHECK( ActsTrk::TrackStorageContainer::staticVariables.count(name) == 1);
  }
  BOOST_CHECK( m->backend()->getConstStore()->getAuxIDs().size() == ActsTrk::TrackStorageContainer::staticVariables.size());
}


struct FilledBackend : public EmptyBackend {
  FilledBackend() 
  {
    m->addColumn_impl<short>("author");
    m->addTrack_impl();

    using namespace Acts::HashedStringLiteral;
    *std::any_cast<float*>(m->component_impl("chi2"_hash, 0)) = 2.5f;
    *std::any_cast<ActsTrk::IndexType*>(m->component_impl("tipIndex"_hash, 0)) = 8;
    *std::any_cast<unsigned int*>(m->component_impl("nHoles"_hash, 0)) = 2;
    *std::any_cast<short*>(m->component_impl("author"_hash, 0)) = 77;
    m->addTrack_impl();
  }
};



BOOST_FIXTURE_TEST_CASE(BareContainerFill, FilledBackend) {
  using namespace Acts::HashedStringLiteral;

  BOOST_CHECK_EQUAL(m->size_impl(), 2);
  BOOST_CHECK(m->hasColumn_impl("author"_hash));
  BOOST_CHECK(m->hasColumn_impl("tipIndex"_hash));

  BOOST_CHECK(m->hasColumn_impl("z0"_hash) == false);

  BOOST_CHECK_EQUAL(*std::any_cast<float*>(m->component_impl("chi2"_hash, 0)), 2.5f);
  BOOST_CHECK_EQUAL(*std::any_cast<unsigned int*>(m->component_impl("nHoles"_hash, 0)), 2);
  BOOST_CHECK_EQUAL(*std::any_cast<short*>(m->component_impl("author"_hash, 0)), 77);
  BOOST_CHECK_EQUAL(*std::any_cast<ActsTrk::IndexType*>(m->component_impl("tipIndex"_hash, 0)), 8);
    
  BOOST_CHECK_EQUAL(*std::any_cast<float*>(m->component_impl("chi2"_hash, 1)), 0.0f);
  BOOST_CHECK_EQUAL(*std::any_cast<unsigned int*>(m->component_impl("nHoles"_hash, 1)), 0);
  BOOST_CHECK_EQUAL(*std::any_cast<short*>(m->component_impl("author"_hash, 1)), 0);
  BOOST_CHECK_EQUAL(*std::any_cast<ActsTrk::IndexType*>(m->component_impl("tipIndex"_hash, 1)), 0);

}


BOOST_FIXTURE_TEST_CASE(ImmutableAccess, FilledBackend) {
  using namespace Acts::HashedStringLiteral;

  auto c = std::make_unique<ActsTrk::TrackStorageContainer>(m->backend());
  c->restoreDecorations();
  BOOST_CHECK_EQUAL(c->size_impl(), 2);
  BOOST_CHECK(c->hasColumn_impl("author"_hash));
  BOOST_CHECK(c->hasColumn_impl("z0"_hash) == false);

  BOOST_CHECK_EQUAL(*std::any_cast<const float*>(c->component_impl("chi2"_hash, 0)), 2.5f);
  BOOST_CHECK_EQUAL(*std::any_cast<const unsigned int*>(c->component_impl("nHoles"_hash, 0)), 2);

  BOOST_CHECK_EQUAL(*std::any_cast<const short*>(c->component_impl("author"_hash, 0)), 77);

  BOOST_CHECK_EQUAL(*std::any_cast<const float*>(c->component_impl("chi2"_hash, 1)), 0.0f);
  BOOST_CHECK_EQUAL(*std::any_cast<const unsigned int*>(c->component_impl("nHoles"_hash, 1)), 0);
  BOOST_CHECK_EQUAL(*std::any_cast<const short*>(c->component_impl("author"_hash, 1)), 0);


}

BOOST_AUTO_TEST_SUITE_END()
