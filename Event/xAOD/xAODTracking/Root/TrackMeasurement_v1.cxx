/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "xAODCore/AuxStoreAccessorMacros.h"
#include "xAODTracking/versions/TrackMeasurement_v1.h"


namespace xAOD {
    const SG::AuxElement::Accessor<std::vector<double>> TrackMeasurement_v1::measAcc("meas");
    const SG::AuxElement::Accessor<std::vector<double>> TrackMeasurement_v1::covMatrixAcc("covMatrix");

    AUXSTORE_OBJECT_SETTER_AND_GETTER(TrackMeasurement_v1,
                                        std::vector<double>,
                                        meas,
                                        setMeas)

    AUXSTORE_OBJECT_SETTER_AND_GETTER(TrackMeasurement_v1,
                                        std::vector<double>,
                                        covMatrix,
                                        setCovMatrix)

    AUXSTORE_OBJECT_SETTER_AND_GETTER(TrackMeasurement_v1,
                                        ElementLink<xAOD::UncalibratedMeasurementContainer>,
                                        uncalibratedMeasurementLink,
                                        setUncalibratedMeasurementLink)

    AUXSTORE_OBJECT_SETTER_AND_GETTER(TrackMeasurement_v1,
                                        std::uint64_t,
                                        projector,
                                        setProjector)

    const std::uint64_t* TrackMeasurement_v1::projectorPtr() const {
        static const ConstAccessor<std::uint64_t> acc("projector");
        return &(acc(*this));
    }
    std::uint64_t* TrackMeasurement_v1::projectorPtr() {
        static const Accessor<std::uint64_t> acc("projector");
         return &(acc(*this));
    }


    const UncalibratedMeasurement* TrackMeasurement_v1::uncalibratedMeasurement() const {
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



    void TrackMeasurement_v1::resize(size_t sz) {
        measAcc(*this).resize(sz);
        covMatrixAcc(*this).resize(sz * sz);
    }

    size_t TrackMeasurement_v1::size() const {
      return measAcc(*this).size();
    }
}
