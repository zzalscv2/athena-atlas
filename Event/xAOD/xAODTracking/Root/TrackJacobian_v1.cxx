/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#include "xAODCore/AuxStoreAccessorMacros.h"
#include "xAODTracking/versions/TrackJacobian_v1.h"

namespace xAOD {

    TrackJacobian_v1::MatrixMap TrackJacobian_v1::values() {
        static const SG::AuxElement::Accessor<std::vector<double>> acc("values");
        if (!acc.isAvailable(*this)) throw std::runtime_error("Missing 'values' in TrackJacobian_v1");;
        return MatrixMap{acc(*this).data()};
    }

    TrackJacobian_v1::ConstMatrixMap TrackJacobian_v1::values() const {
        static const SG::AuxElement::ConstAccessor<std::vector<double>> acc("values");
        if (!acc.isAvailable(*this)) throw std::runtime_error("Missing 'values' in TrackJacobian_v1");;
        return ConstMatrixMap{acc(*this).data()};
    }

    void TrackJacobian_v1::resize(size_t sz) {
        static const SG::AuxElement::Accessor<std::vector<double>> acc("values");
        if (!acc.isAvailable(*this)) throw std::runtime_error("Missing 'values' in TrackJacobian_v1");;
        acc(*this).resize(sz);
    }
}