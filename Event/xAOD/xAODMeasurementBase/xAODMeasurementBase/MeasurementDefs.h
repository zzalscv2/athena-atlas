
/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMEASUREMENTBASE_MEASUREMENTDEFS_H
#define XAODMEASUREMENTBASE_MEASUREMENTDEFS_H
// EDM include(s):
#include <array>

#include "AthContainers/AuxElement.h"
#include "EventPrimitives/EventPrimitives.h"

#define AUX_MEASUREMENTVAR(VAR, DIM)                         \
    do {                                                     \
        static const std::string varName =                   \
            std::string{#VAR} + "Dim" + std::to_string(DIM); \
        static const auxid_t auxId = getAuxID(varName, VAR); \
        regAuxVar(auxId, varName, VAR);                      \
    } while (false);

namespace xAOD {

/// Define the type of the uncalibrated measurement
/// Wondering whether we should just
enum class UncalibMeasType {
    Other = 0,
    // InDet
    PixelClusterType = 1,
    StripClusterType = 2,
    // Muon
    MdtDriftCircleType = 3,
};

/// @ detector ID element hash
using DetectorIDHashType = unsigned int;
using DetectorIdentType = long unsigned int;
/// xAOD Accessor to the position
template <size_t N>
using PosAccessor = SG::AuxElement::Accessor<std::array<float, N>>;
/// xAOD Accessor to the covariance
template <size_t N>
using CovAccessor = SG::AuxElement::Accessor<std::array<float, N * N>>;
/// Abrivation of the Matrix & Covariance definitions
template <size_t N>
using MeasVector = Eigen::Matrix<float, N, 1>;
template <size_t N>
using MeasMatrix = Eigen::Matrix<float, N, N>;

template <size_t N>
using VectorMap = Eigen::Map<MeasVector<N>>;
template <size_t N>
using ConstVectorMap = Eigen::Map<const MeasVector<N>>;

template <size_t N>
using MatrixMap = Eigen::Map<MeasMatrix<N>>;
template <size_t N>
using ConstMatrixMap = Eigen::Map<const MeasMatrix<N>>;

}  // namespace xAOD
#endif