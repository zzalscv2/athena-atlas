/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "xAODTracking/versions/TrackStorage_v1.h"

#include "xAODCore/AuxStoreAccessorMacros.h"
#include "xAODTracking/AuxAccessorMacro.h"


namespace xAOD {

AUXSTORE_OBJECT_SETTER_AND_GETTER(TrackStorage_v1, std::vector<double>, 
                                  params, setParams)

AUXSTORE_OBJECT_SETTER_AND_GETTER(TrackStorage_v1, std::vector<double>,
                                  covParams, setCovParams)

DEFINE_API(TrackStorage_v1, unsigned int, nMeasurements, setnMeasurements)

DEFINE_API(TrackStorage_v1, unsigned int, nHoles, setnHoles)

DEFINE_API(TrackStorage_v1, double, chi2, setChi2)

DEFINE_API(TrackStorage_v1, unsigned int, ndf, setNdf)

DEFINE_API(TrackStorage_v1, unsigned int, nOutliers, setnOutliers)

DEFINE_API(TrackStorage_v1, unsigned int, nSharedHits, setnSharedHits)

const SG::AuxElement::Accessor<std::vector<double> >
    xAOD::TrackStorage_v1::s_paramsAcc{"params"};
const SG::AuxElement::Accessor<std::vector<double> >
    xAOD::TrackStorage_v1::s_covParamsAcc{"covParams"};

void TrackStorage_v1::resize(size_t sz) {
  s_paramsAcc(*this).resize(sz);
  s_covParamsAcc(*this).resize(sz * sz);
}

size_t TrackStorage_v1::size() const {
  return s_paramsAcc(*this).size();
}
}  // namespace xAOD
