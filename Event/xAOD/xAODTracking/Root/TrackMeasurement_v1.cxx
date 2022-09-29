/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#include "xAODCore/AuxStoreAccessorMacros.h"
#include "xAODTracking/versions/TrackMeasurements_v1.h"


namespace xAOD {

    AUXSTORE_OBJECT_SETTER_AND_GETTER(TrackMeasurements_v1,
                                        std::vector<double>,
                                        meas,
                                        setMeas)

    static const SG::AuxElement::Accessor<std::vector<double>> measAcc("meas");
    TrackMeasurements_v1::VectorMap TrackMeasurements_v1::measEigen() {
        return VectorMap{measAcc(*this).data()};
    }

    TrackMeasurements_v1::ConstVectorMap TrackMeasurements_v1::measEigen() const {
        return ConstVectorMap{measAcc(*this).data()};
    }

    AUXSTORE_OBJECT_SETTER_AND_GETTER(TrackMeasurements_v1,
                                        std::vector<double>,
                                        covMatrix,
                                        setCovMatrix)
    static const SG::AuxElement::Accessor<std::vector<double>> covMatrixAcc("covMatrix");
    TrackMeasurements_v1::MatrixMap TrackMeasurements_v1::covMatrixEigen() {
        return MatrixMap{covMatrixAcc(*this).data()};
    }

    TrackMeasurements_v1::ConstMatrixMap TrackMeasurements_v1::covMatrixEigen() const {
        return ConstMatrixMap{covMatrixAcc(*this).data()};
    }

    AUXSTORE_OBJECT_SETTER_AND_GETTER(TrackMeasurements_v1,
                                        ElementLink<xAOD::UncalibratedMeasurementContainer>,
                                        uncalibratedMeasurementLink,
                                        setUncalibratedMeasurementLink)

    const UncalibratedMeasurement* TrackMeasurements_v1::uncalibratedMeasurement() const {
        static const ConstAccessor<ElementLink<UncalibratedMeasurementContainer> >
            acc("uncalibratedMeasurementLink");
        if( ! acc.isAvailable( *this ) ) {
            return nullptr;
        }
        const ElementLink<UncalibratedMeasurementContainer>& link = acc(*this);
        if( ! link.isValid() ) {
            return nullptr;
        }
        return *link;
    }



    void TrackMeasurements_v1::resize(size_t sz) {
        measAcc(*this).resize(sz);
        covMatrixAcc(*this).resize(sz * sz);
    }
}