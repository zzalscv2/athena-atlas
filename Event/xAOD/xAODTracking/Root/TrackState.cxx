/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "xAODCore/AuxStoreAccessorMacros.h"
#include "xAODTracking/versions/TrackState_v1.h"

#define DEFINE_API(__TYPE, __GETTER, __SETTER) \
    AUXSTORE_PRIMITIVE_SETTER_AND_GETTER(TrackState_v1, __TYPE, __GETTER, __SETTER)  \
    __TYPE* TrackState_v1::__GETTER##Ptr() { \
        static const SG::AuxElement::Accessor<__TYPE> acc(#__GETTER); \
        return &(acc(*this)); \
    } \
    const __TYPE* TrackState_v1::__GETTER##Ptr() const { \
        static const SG::AuxElement::ConstAccessor<__TYPE> acc(#__GETTER); \
        return &(acc(*this)); \
    }


namespace xAOD {

    DEFINE_API(double, chi2, setChi2)    
    DEFINE_API(double, pathLength, setPathLength)
    DEFINE_API(TrackStateIndexType, previous, setPrevious)
    DEFINE_API(TrackStateIndexType, predicted, setPredicted)
    DEFINE_API(TrackStateIndexType, filtered, setFiltered)
    DEFINE_API(TrackStateIndexType, smoothed, setSmoothed)
    DEFINE_API(TrackStateIndexType, jacobian, setJacobian)
    DEFINE_API(TrackStateIndexType, calibrated, setCalibrated)
    DEFINE_API(TrackStateIndexType, measDim, setMeasDim)
    AUXSTORE_PRIMITIVE_SETTER_AND_GETTER(TrackState_v1, ElementLink<xAOD::UncalibratedMeasurementContainer>, uncalibratedMeasurementLink, setUncalibratedMeasurementLink);
    AUXSTORE_PRIMITIVE_SETTER_AND_GETTER(TrackState_v1,  uint64_t, geometryId, setGeometryId);

}


#undef DEFINE_API