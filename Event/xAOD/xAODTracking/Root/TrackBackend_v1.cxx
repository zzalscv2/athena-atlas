/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "xAODCore/AuxStoreAccessorMacros.h"
#include "xAODTracking/versions/TrackBackend_v1.h"


namespace xAOD {

    AUXSTORE_OBJECT_SETTER_AND_GETTER(TrackBackend_v1,
                                        std::vector<double>,
                                        params,
                                        setParams)

    AUXSTORE_OBJECT_SETTER_AND_GETTER(TrackBackend_v1,
                                        std::vector<double>,
                                        covParams,
                                        setCovParams)

    AUXSTORE_OBJECT_SETTER_AND_GETTER(TrackBackend_v1,
                                        std::vector<unsigned int>,
                                        nMeasurements,
                                        setnMeasurements)

    AUXSTORE_OBJECT_SETTER_AND_GETTER(TrackBackend_v1,
                                        std::vector<unsigned int>,
                                        nHoles,
                                        setnHoles)

    AUXSTORE_OBJECT_SETTER_AND_GETTER(TrackBackend_v1,
                                        std::vector<float>,
                                        chi2,
                                        setChi2)

    AUXSTORE_OBJECT_SETTER_AND_GETTER(TrackBackend_v1,
                                        std::vector<unsigned int>,
                                        ndf,
                                        setNdf)

    AUXSTORE_OBJECT_SETTER_AND_GETTER(TrackBackend_v1,
                                        std::vector<unsigned int>,
                                        nOutliers,
                                        setnOutliers)

    AUXSTORE_OBJECT_SETTER_AND_GETTER(TrackBackend_v1,
                                        std::vector<unsigned int>,
                                        nSharedHits,
                                        setnSharedHits)

    const SG::AuxElement::Accessor<std::vector<double> > xAOD::TrackBackend_v1::s_paramsAcc{"params"};
    const SG::AuxElement::Accessor<std::vector<double> > xAOD::TrackBackend_v1::s_covParamsAcc{"covParams"};


    void TrackBackend_v1::resize(size_t sz) {
        s_paramsAcc(*this).resize(sz);
        s_covParamsAcc(*this).resize(sz * sz);
    }

    size_t TrackBackend_v1::size() const {
        return s_paramsAcc(*this).size();
    }
}
