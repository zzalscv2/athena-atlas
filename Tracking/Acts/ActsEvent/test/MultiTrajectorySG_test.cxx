/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#undef NDEBUG
#define BOOST_TEST_MODULE MultiTrajectorySG_test
#include <boost/test/data/test_case.hpp>
#include <boost/test/included/unit_test.hpp>

#include <memory>
#include "TestTools/initGaudi.h"
#include "AthenaKernel/getMessageSvc.h"
#include "GaudiKernel/MsgStream.h"
#include "StoreGate/StoreGateSvc.h"
#include "CxxUtils/ubsan_suppress.h"
#include <TInterpreter.h>
#include "xAODTracking/TrackParameters.h"

#include "ActsEvent/MultiTrajectory.h"
#include "AthLinks/DataLink.h"

BOOST_AUTO_TEST_SUITE(EventDataMultiTrajectorySG)


struct PopulateSG {
  StoreGateSvc* sg = nullptr;

  PopulateSG() {
    CxxUtils::ubsan_suppress ( []() { TInterpreter::Instance(); } );
    using namespace std;
    ISvcLocator* pSvcLoc{nullptr};
    BOOST_CHECK(Athena_test::initGaudi(pSvcLoc) );

    BOOST_CHECK(pSvcLoc->service("StoreGateSvc", sg, true).isSuccess());

    auto trackStateBackend = std::make_unique<xAOD::TrackStateContainer>();
    auto trackStateBackendAux = std::make_unique<xAOD::TrackStateAuxContainer>();
    trackStateBackend->setStore(trackStateBackendAux.get());

    auto parametersBackend = std::make_unique<xAOD::TrackParametersContainer>();
    auto parametersBackendAux = std::make_unique<xAOD::TrackParametersAuxContainer>();
    parametersBackend->setStore(parametersBackendAux.get());  


    auto jacobianBackend = std::make_unique<xAOD::TrackJacobianContainer>();
    auto jacobianBackendAux = std::make_unique<xAOD::TrackJacobianAuxContainer>();
    jacobianBackend->setStore(jacobianBackendAux.get());

    auto measurementsBackend = std::make_unique<xAOD::TrackMeasurementContainer>();
    auto measurementsBackendAux = std::make_unique<xAOD::TrackMeasurementAuxContainer>();
    measurementsBackend->setStore(measurementsBackendAux.get());

    ActsTrk::MutableMultiTrajectory mmtj(trackStateBackend.get(), parametersBackend.get(), jacobianBackend.get(), measurementsBackend.get() );
    // TODO add some content to fill backends here

    BOOST_CHECK(sg->record(trackStateBackend.release(), "trackState").isSuccess());
    BOOST_CHECK(sg->record(trackStateBackendAux.release(), "trackStateAux.").isSuccess());
    BOOST_CHECK(sg->record(parametersBackend.release(), "parameters").isSuccess());
    BOOST_CHECK(sg->record(parametersBackendAux.release(), "parametersAux.").isSuccess());
    BOOST_CHECK(sg->record(jacobianBackend.release(), "jacobian").isSuccess());
    BOOST_CHECK(sg->record(jacobianBackendAux.release(), "jacobianAux.").isSuccess());
    BOOST_CHECK(sg->record(measurementsBackend.release(), "measurements").isSuccess());
    BOOST_CHECK(sg->record(measurementsBackendAux.release(), "measurementsAux.").isSuccess());


  }
};

BOOST_FIXTURE_TEST_CASE(BuildFromDL, PopulateSG) {
  DataLink<xAOD::TrackStateContainer> trackStateDL("trackState", sg);
  BOOST_CHECK(trackStateDL.isValid());
  DataLink<xAOD::TrackParametersContainer> trackParametersDL("parameters", sg);
  BOOST_CHECK(trackParametersDL.isValid());
  DataLink<xAOD::TrackJacobianContainer> trackJacobianDL("jacobian", sg);
  BOOST_CHECK(trackJacobianDL.isValid());
  DataLink<xAOD::TrackMeasurementContainer> trackMeasurementDL("measurements", sg);
  BOOST_CHECK(trackMeasurementDL.isValid());
  {
    auto cmtj = std::make_unique<ActsTrk::ConstMultiTrajectory>(trackStateDL, trackParametersDL, trackJacobianDL, trackMeasurementDL);
    BOOST_CHECK(sg->record(cmtj.release(), "MTJ").isSuccess());
  }
  {
    const ActsTrk::ConstMultiTrajectory* back;
    BOOST_CHECK( sg->retrieve(back, "MTJ").isSuccess());
  }
}
  

BOOST_AUTO_TEST_SUITE_END()