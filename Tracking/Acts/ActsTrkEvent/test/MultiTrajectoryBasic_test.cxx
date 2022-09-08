/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#define BOOST_TEST_MODULE MultiTrajectoryBasic_test
#include <boost/test/data/test_case.hpp>
#include <boost/test/included/unit_test.hpp>

#include "ActsTrkEvent/MultiTrajectory.h"
#include "xAODTracking/TrackStateAuxContainer.h"
#include "xAODTracking/TrackParametersAuxContainer.h"


namespace {


struct EmptyMTJ { // setup empty MTJ
  EmptyMTJ() {
    trackStateBackend = std::make_unique<xAOD::TrackStateContainer>();
    trackStateBackendAux = std::make_unique<xAOD::TrackStateAuxContainer>();
    trackStateBackend->setStore(trackStateBackendAux.get());

    parametersBackend = std::make_unique<xAOD::TrackParametersContainer>();
    parametersBackendAux = std::make_unique<xAOD::TrackParametersAuxContainer>();
    parametersBackend->setStore(parametersBackendAux.get());

    // TODO other backends

    t = std::make_unique<ActsTrk::MultiTrajectory<ActsTrk::IsReadWrite>>(trackStateBackend.get(), parametersBackend.get());
  }
  std::unique_ptr<xAOD::TrackStateContainer> trackStateBackend;
  std::unique_ptr<xAOD::TrackStateAuxContainer> trackStateBackendAux;
  std::unique_ptr<xAOD::TrackParametersContainer> parametersBackend;
  std::unique_ptr<xAOD::TrackParametersAuxContainer> parametersBackendAux;
  std::unique_ptr<ActsTrk::MultiTrajectory<ActsTrk::IsReadWrite>> t;
};


BOOST_FIXTURE_TEST_CASE(Move, EmptyMTJ) {
    EmptyMTJ w;
    BOOST_CHECK( w.t->has_backends());
    ActsTrk::MultiTrajectory<ActsTrk::IsReadOnly> ro(std::move(*(w.t.get())));
    BOOST_CHECK( w.t->has_backends() == false);
    BOOST_CHECK( ro.has_backends());
}


// TODO remaining tests


}