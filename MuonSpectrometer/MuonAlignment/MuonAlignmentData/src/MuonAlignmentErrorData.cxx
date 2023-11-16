/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonAlignmentData/MuonAlignmentErrorData.h"

void MuonAlignmentErrorData::setDeviations(std::vector<Deviation> vec) {
    m_deviations = std::move(vec);
}

const std::vector<MuonAlignmentErrorData::Deviation>& MuonAlignmentErrorData::getDeviations() const {
    return m_deviations;
}

void MuonAlignmentErrorData::setClobVersion(std::string clobVersion) {
    m_clobVersion = std::move(clobVersion);
}
const std::string& MuonAlignmentErrorData::getClobVersion() const {
    return m_clobVersion;
}

void MuonAlignmentErrorData::setHasNswHits(bool val) {
    m_hasNswHits = val;
}
bool MuonAlignmentErrorData::hasNswHits() const {
    return m_hasNswHits;
}

