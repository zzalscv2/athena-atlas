/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// EDM include(s):
#include "xAODCore/AuxStoreAccessorMacros.h"
// Local include(s):
#include "MuonReadoutGeometryR4/MdtReadoutElement.h"
#include "TrkEventPrimitives/ParamDefs.h"
#include "xAODMuonPrepData/versions/MdtDriftCircle_v1.h"

namespace {
    static const std::string preFixStr{"MDT_"};
    static const SG::AuxElement::Accessor<uint8_t> accDriftStatus{preFixStr + "status"};
}
#define IMPLEMENT_SETTER_GETTER( DTYPE, GETTER, SETTER)                          \
      DTYPE MdtDriftCircle_v1::GETTER() const {                                  \
         static const SG::AuxElement::Accessor<DTYPE> acc{preFixStr + #GETTER};  \
         return acc(*this);                                                      \
      }                                                                          \
                                                                                 \
      void MdtDriftCircle_v1::SETTER(DTYPE value) {                        \
         static const SG::AuxElement::Accessor<DTYPE> acc{preFixStr + #GETTER};  \
         acc(*this) = value;                                                     \
      }
                                                                          
namespace xAOD {
using MdtDriftCircleStatus = MdtDriftCircle_v1::MdtDriftCircleStatus;

IMPLEMENT_SETTER_GETTER(int16_t, tdc, setTdc)
IMPLEMENT_SETTER_GETTER(int16_t, adc, setAdc)
IMPLEMENT_SETTER_GETTER(uint16_t, driftTube, setTube)
IMPLEMENT_SETTER_GETTER(uint8_t, tubeLayer, setLayer)

void MdtDriftCircle_v1::setStatus(MdtDriftCircleStatus st) {
    accDriftStatus(*this) = st;
}

IdentifierHash MdtDriftCircle_v1::measurementHash() const {
    return MuonGMR4::MdtReadoutElement::measurementHash(tubeLayer(),
                                                        driftTube());
}
MdtDriftCircleStatus MdtDriftCircle_v1::status() const {
    return static_cast<MdtDriftCircleStatus>(accDriftStatus(*this));
}
float MdtDriftCircle_v1::driftRadius() const {
    return localPosition<1>()[Trk::locR];
}
/** @brief Returns the covariance of the drift radius*/
float MdtDriftCircle_v1::driftRadiusCov() const {
    return localCovariance<1>()(Trk::locR, Trk::locR);
}
/** @brief Returns the uncertainty on the drift radius*/
float MdtDriftCircle_v1::driftRadiusUncert() const {
    return std::sqrt(driftRadiusCov());
}
void MdtDriftCircle_v1::setDriftRadius(float r) {
    localPosition<1>()[Trk::locR] = r;
}
void MdtDriftCircle_v1::setDriftRadCov(float cov) {
    localCovariance<1>()(Trk::locR, Trk::locR) = cov;
}

}  // namespace xAOD
#undef IMPLEMENT_SETTER_GETTER
