/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/


#define BOOST_TEST_MODULE MultiTrajectoryBasic_test
#include <boost/test/data/test_case.hpp>
#include <boost/test/included/unit_test.hpp>

// Local include(s):
#include "xAODTracking/TrackStateContainer.h"
#include "xAODTracking/TrackStateAuxContainer.h"

#include "xAODTracking/TrackMeasurements.h"
#include "xAODTracking/TrackMeasurementsContainer.h"
#include "xAODTracking/TrackMeasurementsAuxContainer.h"

#include "xAODTracking/TrackParameters.h"
#include "xAODTracking/TrackParametersContainer.h"
#include "xAODTracking/TrackParametersAuxContainer.h"


#include "xAODTracking/TrackJacobian.h"
#include "xAODTracking/TrackJacobianContainer.h"
#include "xAODTracking/TrackJacobianAuxContainer.h"



namespace {


BOOST_AUTO_TEST_CASE(TrackMeasurements) {
    constexpr static size_t sz = 6;

    xAOD::TrackMeasurementsContainer pars;
    xAOD::TrackMeasurementsAuxContainer aux;
    pars.setStore(&aux);

    std::vector<double> semirandoms = {0.12, 0.92};
    for ( size_t p=0; p < semirandoms.size(); ++p) {
        auto par = new xAOD::TrackMeasurements();    
        pars.push_back(par);
        par->resize();
        for ( size_t i = 0; i < sz; ++i) {
            par->measurements()(i) = i * semirandoms[p];
            for ( size_t j = 0; j < sz; ++j) {
                par->covariance()(i, j) = (i+j) * semirandoms[p];
            }
        }
    }
    for ( size_t p=0; p < semirandoms.size(); ++p) {
        const xAOD::TrackMeasurements* par = pars.at(p);
        for ( size_t i = 0; i < sz; ++i) {
            const double stored = par->measurements()(i);
            const double expected = i * semirandoms[p];
            BOOST_CHECK_EQUAL(stored, expected);
            for ( size_t j = 0; j < sz; ++j) {
                const double stored = par->covariance()(i, j);
                const double expected = (i+j) * semirandoms[p];
                BOOST_CHECK_EQUAL(stored, expected);
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(TrackParameters) {
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
            par->parameters()(i) = i * semirandoms[p];
            for ( size_t j = 0; j < sz; ++j) {
                par->covariance()(i, j) = (i+j) * semirandoms[p];
            }
        }
    }
    for ( size_t p=0; p < semirandoms.size(); ++p) {
        const xAOD::TrackParameters* par = pars.at(p);
        for ( size_t i = 0; i < sz; ++i) {
            const double stored = par->parameters()(i);
            const double expected = i * semirandoms[p];
            BOOST_CHECK_EQUAL(stored, expected);
            for ( size_t j = 0; j < sz; ++j) {
                const double stored = par->covariance()(i, j);
                const double expected = (i+j) * semirandoms[p];
                BOOST_CHECK_EQUAL(stored, expected);
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(TrackJacobian) {
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
                jacs[j]->values()(i, k) = semirandoms[j]*i*k;
    }
    // check
    for ( size_t j=0; j < semirandoms.size(); ++j) {
        for ( size_t i = 0; i < sz; ++i )
            for ( size_t k = 0; k < sz; ++k )
                BOOST_CHECK_EQUAL(jacs[j]->values()(i, k), semirandoms[j]*i*k);
    }
}
}
