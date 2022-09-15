/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#include "xAODCore/AuxStoreAccessorMacros.h"
#include "xAODTracking/versions/TrackParameters_v1.h"


namespace xAOD {

    TrackParameters_v1::VectorMap TrackParameters_v1::parameters() {
        static const SG::AuxElement::Accessor<std::vector<double>> acc("parameters");
        if (!acc.isAvailable(*this)) throw std::runtime_error("Missing 'parameters' in TrackParameters_v1");
        return VectorMap{acc(*this).data()};
    }

    TrackParameters_v1::ConstVectorMap TrackParameters_v1::parameters() const {
        static const SG::AuxElement::ConstAccessor<std::vector<double>> acc("parameters");
        if (!acc.isAvailable(*this)) throw std::runtime_error("Missing 'parameters' in TrackParameters_v1");
        return ConstVectorMap{acc(*this).data()};
    }


    TrackParameters_v1::MatrixMap TrackParameters_v1::covariance() {
        static const SG::AuxElement::Accessor<std::vector<double>> acc("covariance");
        if (!acc.isAvailable(*this)) throw std::runtime_error("Missing 'covariance' in TrackParameters_v1");;
        return MatrixMap{acc(*this).data()};
    }
    TrackParameters_v1::ConstMatrixMap TrackParameters_v1::covariance() const {
        static const SG::AuxElement::ConstAccessor<std::vector<double>> acc("covariance");
        if (!acc.isAvailable(*this)) throw std::runtime_error("Missing 'covariance' in TrackParameters_v1");;
        return ConstMatrixMap{acc(*this).data()};
    }

    void TrackParameters_v1::resize(size_t sz) {
        {
            static const SG::AuxElement::Accessor<std::vector<double>> acc("parameters");
            if (!acc.isAvailable(*this)) throw std::runtime_error("Missing 'parameters' in TrackParameters_v1");
            acc(*this).resize(sz);
        }
        {
            static const SG::AuxElement::Accessor<std::vector<double>> acc("covariance");
            if (!acc.isAvailable(*this)) throw std::runtime_error("Missing 'covariance' in TrackParameters_v1");;
            acc(*this).resize(sz * sz);
        }
    }
}