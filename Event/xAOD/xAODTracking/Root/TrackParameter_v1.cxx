/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#include "xAODCore/AuxStoreAccessorMacros.h"
#include "xAODTracking/versions/TrackParameter_v1.h"


namespace xAOD {
    static const SG::AuxElement::Accessor<std::vector<double>> paramsAcc("params");

    TrackParameter_v1::VectorMap TrackParameter_v1::paramsEigen() {
        return VectorMap{paramsAcc(*this).data()};
    }

    TrackParameter_v1::ConstVectorMap TrackParameter_v1::paramsEigen() const {
        return ConstVectorMap{paramsAcc(*this).data()};
    }

    static const SG::AuxElement::Accessor<std::vector<double>> covMatrixAcc("covMatrix");

    TrackParameter_v1::MatrixMap TrackParameter_v1::covMatrixEigen() {
        return MatrixMap{covMatrixAcc(*this).data()};
    }
    TrackParameter_v1::ConstMatrixMap TrackParameter_v1::covMatrixEigen() const {
        return ConstMatrixMap{covMatrixAcc(*this).data()};
    }

    void TrackParameter_v1::resize(size_t sz) {
        paramsAcc(*this).resize(sz);
        covMatrixAcc(*this).resize(sz * sz);
    }
}