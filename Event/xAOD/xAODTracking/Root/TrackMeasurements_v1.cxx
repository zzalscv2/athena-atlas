/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#include "xAODCore/AuxStoreAccessorMacros.h"
#include "xAODTracking/versions/TrackMeasurements_v1.h"


namespace xAOD {

    TrackMeasurements_v1::VectorMap TrackMeasurements_v1::measurements() {
        static const SG::AuxElement::Accessor<std::vector<double>> acc("measurements");
        if (!acc.isAvailable(*this)) throw std::runtime_error("Missing 'measurements' in TrackMeasurements_v1");
        return VectorMap{acc(*this).data()};
    }

    TrackMeasurements_v1::ConstVectorMap TrackMeasurements_v1::measurements() const {
        static const SG::AuxElement::ConstAccessor<std::vector<double>> acc("measurements");
        if (!acc.isAvailable(*this)) throw std::runtime_error("Missing 'measurements' in TrackMeasurements_v1");
        return ConstVectorMap{acc(*this).data()};
    }


    TrackMeasurements_v1::MatrixMap TrackMeasurements_v1::covariance() {
        static const SG::AuxElement::Accessor<std::vector<double>> acc("covariance");
        if (!acc.isAvailable(*this)) throw std::runtime_error("Missing 'covariance' in TrackMeasurements_v1");;
        return MatrixMap{acc(*this).data()};
    }

    TrackMeasurements_v1::ConstMatrixMap TrackMeasurements_v1::covariance() const {
        static const SG::AuxElement::ConstAccessor<std::vector<double>> acc("covariance");
        if (!acc.isAvailable(*this)) throw std::runtime_error("Missing 'covariance' in TrackMeasurements_v1");;
        return ConstMatrixMap{acc(*this).data()};
    }


    void TrackMeasurements_v1::resize(size_t sz) {
        {
            static const SG::AuxElement::Accessor<std::vector<double>> acc("measurements");
            if (!acc.isAvailable(*this)) throw std::runtime_error("Missing 'measurements' in TrackMeasurements_v1");
            acc(*this).resize(sz);

        }
        {
            static const SG::AuxElement::Accessor<std::vector<double>> acc("covariance");
            if (!acc.isAvailable(*this)) throw std::runtime_error("Missing 'covariance' in TrackMeasurements_v1");;
            acc(*this).resize(sz * sz);
        }
    }
}