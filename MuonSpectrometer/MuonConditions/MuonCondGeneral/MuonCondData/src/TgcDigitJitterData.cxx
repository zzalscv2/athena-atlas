/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonCondData/TgcDigitJitterData.h"
///
#include "GeoPrimitives/GeoPrimitivesToStringConverter.h"
#include "CLHEP/Random/RandFlat.h"
#include "CLHEP/Random/RandomEngine.h"

TgcDigitJitterData::TgcDigitJitterData():
   AthMessaging{"TgcDigitJitterData"} {}

double TgcDigitJitterData::drawJitter(const Amg::Vector3D& localDir,
                                      CLHEP::HepRandomEngine* rndmEngine) const {
    const double injectionAngle = std::atan2(std::abs(localDir.z()), std::abs(localDir.x())) / Gaudi::Units::deg;

    const size_t ithAngle = std::min(static_cast<size_t>(injectionAngle / m_angleInterval), m_bins.size() - 1);
    const size_t jthAngle = std::min(ithAngle + 1, m_bins.size() -1);
    ATH_MSG_DEBUG("Direction "<<Amg::toString(localDir)<<", injection angle: "<<injectionAngle
                <<", bin i: "<<ithAngle<<", bin j: "<<jthAngle);
    const double wAngle = std::fmod(injectionAngle, m_angleInterval);

    double jitter{0.}, prob{1.}, probRef{0.};
    const std::vector<double>& ith_probs{m_bins[ithAngle].timeProbs};
    const std::vector<double>& jth_probs{m_bins[jthAngle].timeProbs};
    unsigned int trials{0};
    while (prob > probRef) {
        prob = CLHEP::RandFlat::shoot(rndmEngine, 0.0, 1.0);
        jitter = CLHEP::RandFlat::shoot(rndmEngine, 0.0, 1.0) * m_timeInterval;
        size_t ithJitter = static_cast<size_t>(jitter);
        // probability distribution calculated from weighted sum between
        // neighboring bins of angles
        probRef = (1. - wAngle) * (ithJitter < ith_probs.size() ? ith_probs[ithJitter] : 0.) +
                  wAngle * (ithJitter < jth_probs.size() ? jth_probs[ithJitter] : 0.);
        ATH_MSG_VERBOSE("Trial: "<<(++trials)<<" jitter: "<<jitter<<", prob: "<<prob
                      <<", probRef: "<<probRef);
    }
    return jitter;
}
void TgcDigitJitterData::cacheAngleInterval(const double minAngle, std::vector<double>&& timeProbs) {
    m_bins.emplace_back(minAngle, std::move(timeProbs));
}
StatusCode TgcDigitJitterData::initialize() {
    if (m_bins.empty()) {
        ATH_MSG_FATAL("No jitter bins were defined.");
        return StatusCode::FAILURE;
    }
    std::sort(m_bins.begin(), m_bins.end());
    for (size_t b = 0 ; b < m_bins.size() - 1; ++b) {
        m_bins[b].maxAngle = m_bins[b+1].minAngle;
        if (b == 0) {
          m_angleInterval = m_bins[b].maxAngle - m_bins[b].minAngle;
        } else if (std::abs(m_bins[b].maxAngle - m_bins[b].minAngle - m_angleInterval) >
                   std::numeric_limits<double>::epsilon()) {
            ATH_MSG_FATAL("The angular bin "<<b<<" ranging from "<<m_bins[b].minAngle
                      <<" to "<<m_bins[b].maxAngle<<" is outside of the fixed interval "
                      <<m_angleInterval<<". Only fixed bin size is supported");
            return StatusCode::FAILURE;
        }
        if (m_bins[b].timeProbs.empty()) {
          ATH_MSG_FATAL("The angular bin "<<b<<" has no probabilities associated.");
          return StatusCode::FAILURE;
        }
        m_timeInterval = std::max(m_timeInterval, -1. + m_bins[b].timeProbs.size());
    }
    return StatusCode::SUCCESS;
}
