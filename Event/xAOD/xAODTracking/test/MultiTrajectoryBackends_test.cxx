/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/


#define BOOST_TEST_MODULE MultiTrajectoryBasic_test
#include <boost/test/data/test_case.hpp>
#include <boost/test/included/unit_test.hpp>

// Local include(s):
#include "xAODTracking/TrackStateContainer.h"
#include "xAODTracking/TrackStateAuxContainer.h"

#include "xAODTracking/TrackMeasurement.h"
#include "xAODTracking/TrackMeasurementContainer.h"
#include "xAODTracking/TrackMeasurementAuxContainer.h"

#include "xAODTracking/TrackParameters.h"
#include "xAODTracking/TrackParametersContainer.h"
#include "xAODTracking/TrackParametersAuxContainer.h"


#include "xAODTracking/TrackJacobian.h"
#include "xAODTracking/TrackJacobianContainer.h"
#include "xAODTracking/TrackJacobianAuxContainer.h"

#include "Identifier/IdentifierHash.h"
#include "xAODMeasurementBase/UncalibratedMeasurementContainer.h"
#include "xAODInDetMeasurement/StripClusterContainer.h"
#include "xAODInDetMeasurement/StripClusterAuxContainer.h"
#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "xAODInDetMeasurement/PixelClusterAuxContainer.h"

#include <any>


namespace {


BOOST_AUTO_TEST_CASE(TrackMeasurement_build) {
    constexpr static size_t sz = 6;

    xAOD::TrackMeasurementContainer measurements;
    xAOD::TrackMeasurementAuxContainer aux;
    measurements.setStore(&aux);

    std::vector<double> semirandoms = {0.12, 0.92};
    for ( size_t p=0; p < semirandoms.size(); ++p) {
        auto par = new xAOD::TrackMeasurement();    
        measurements.push_back(par);
        par->resize();
        for ( size_t i = 0; i < sz; ++i) {
            par->measEigen()(i) = i * semirandoms[p];
            for ( size_t j = 0; j < sz; ++j) {
                par->covMatrixEigen()(i, j) = (i+j) * semirandoms[p];
            }
        }
    }
    for ( size_t p=0; p < semirandoms.size(); ++p) {
        const xAOD::TrackMeasurement* par = measurements.at(p);
        for ( size_t i = 0; i < sz; ++i) {
            const double stored = par->measEigen()(i);
            const double expected = i * semirandoms[p];
            BOOST_CHECK_EQUAL(stored, expected);
            for ( size_t j = 0; j < sz; ++j) {
                const double stored = par->covMatrixEigen()(i, j);
                const double expected = (i+j) * semirandoms[p];
                BOOST_CHECK_EQUAL(stored, expected);
            }
        }
    }
}

// cppcheck-suppress syntaxError
BOOST_AUTO_TEST_CASE(TrackMeasurementLinksToUncalibratedMeasurement){
    xAOD::TrackMeasurementContainer measurements;
    xAOD::TrackMeasurementAuxContainer aux;
    measurements.setStore(&aux);

    xAOD::StripClusterContainer stripClusters;
    xAOD::StripClusterAuxContainer stripsAux;
    stripClusters.setStore( &stripsAux );

    xAOD::PixelClusterContainer pixelClusters;
    xAOD::PixelClusterAuxContainer pixelAux;
    pixelClusters.setStore( &pixelAux );

    // add some clusters
    auto stripCluster1 = new xAOD::StripCluster();
    stripClusters.push_back(stripCluster1);
    stripCluster1->setIdentifierHash(90077);
    stripCluster1->localPosition<3>()(0) = 0.11;
    stripCluster1->localPosition<3>()(1) = 0.12;
    stripCluster1->localPosition<3>()(2) = 0.13;

    auto stripCluster2 = new xAOD::StripCluster();
    stripClusters.push_back(stripCluster2);
    stripCluster2->setIdentifierHash(90099);
    stripCluster2->localPosition<3>()(0) = 7.11;
    stripCluster2->localPosition<3>()(1) = 9.12;
    stripCluster2->localPosition<3>()(2) = 20.13;


    auto pixelCluster1 = new xAOD::PixelCluster();
    pixelClusters.push_back(pixelCluster1);
    pixelCluster1->setIdentifierHash(10077);
    pixelCluster1->localPosition<3>()(0) = 0.11;
    pixelCluster1->localPosition<3>()(1) = 0.12;
    pixelCluster1->localPosition<3>()(2) = 0.13;

    auto pixelCluster2 = new xAOD::PixelCluster();
    pixelClusters.push_back(pixelCluster2);
    pixelCluster2->setIdentifierHash(10099);
    pixelCluster2->localPosition<3>()(0) = 7.11;
    pixelCluster2->localPosition<3>()(1) = 9.12;
    pixelCluster2->localPosition<3>()(2) = 20.13;

    // link to measurements
    {
        auto m = new xAOD::TrackMeasurement();    
        measurements.push_back(m);
        m->setUncalibratedMeasurementLink({stripClusters, 1}); // skipping intentionally 1st element
    }
    {
        auto m = new xAOD::TrackMeasurement();    
        measurements.push_back(m);
        m->setUncalibratedMeasurementLink({pixelClusters, 1}); // skipping intentionally 1st element
    }
    {
        auto m = new xAOD::TrackMeasurement();    
        measurements.push_back(m);
        m->setUncalibratedMeasurementLink({pixelClusters, 0}); // reordering
    }

    const std::vector<IdentifierHash> expectedIDs = {90099, 10099, 10077};
    std::vector<IdentifierHash> pointedIDs;

    for ( auto m: measurements) {
        const xAOD::UncalibratedMeasurement * u = m->uncalibratedMeasurement();
        pointedIDs.push_back(u->identifierHash());
    }
    BOOST_CHECK_EQUAL_COLLECTIONS(expectedIDs.begin(), expectedIDs.end(), pointedIDs.begin(), pointedIDs.end());


}

BOOST_AUTO_TEST_CASE(TrackParameters_build) {
    constexpr static size_t sz = 6;

    xAOD::TrackParametersContainer pars;
    xAOD::TrackParametersAuxContainer aux;
    pars.setStore(&aux);

    std::vector<double> semirandoms = {0.7, 0.32};
    for ( size_t p=0; p < semirandoms.size(); ++p) {
        auto par = new xAOD::TrackParameters();    
        pars.push_back(par);
        par->resize();
        for ( size_t i = 0; i < sz; ++i) {
            par->paramsEigen()(i) = i * semirandoms[p];
            for ( size_t j = 0; j < sz; ++j) {
                par->covMatrixEigen()(i, j) = (i+j) * semirandoms[p];
            }
        }
    }
    for ( size_t p=0; p < semirandoms.size(); ++p) {
        const xAOD::TrackParameters* par = pars.at(p);
        for ( size_t i = 0; i < sz; ++i) {
            const double stored = par->paramsEigen()(i);
            const double expected = i * semirandoms[p];
            BOOST_CHECK_EQUAL(stored, expected);
            for ( size_t j = 0; j < sz; ++j) {
                const double stored = par->covMatrixEigen()(i, j);
                const double expected = (i+j) * semirandoms[p];
                BOOST_CHECK_EQUAL(stored, expected);
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(TrackJacobian_build) {
    xAOD::TrackJacobianContainer jacs;
    xAOD::TrackJacobianAuxContainer aux;
    jacs.setStore( &aux );
    constexpr static size_t sz = 6;
    std::vector<double> semirandoms = {0.1, 0.02, 1.6};

    // build
    for ( size_t j=0; j < semirandoms.size(); ++j) {
        auto jac = new xAOD::TrackJacobian();
        jacs.push_back(jac);
        jac->resize();
    }
    // fill
    for ( size_t j=0; j < semirandoms.size(); ++j) {
        for ( size_t i = 0; i < sz; ++i )
           for ( size_t k = 0; k < sz; ++k )
                jacs[j]->jacEigen()(i, k) = semirandoms[j]*i*k;
    }
    // check
    for ( size_t j=0; j < semirandoms.size(); ++j) {
        for ( size_t i = 0; i < sz; ++i )
            for ( size_t k = 0; k < sz; ++k )
                BOOST_CHECK_EQUAL(jacs[j]->jacEigen()(i, k), semirandoms[j]*i*k);
    }
}
}
