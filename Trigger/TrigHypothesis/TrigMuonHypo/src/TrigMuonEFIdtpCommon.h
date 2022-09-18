/*
# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGMUONHYPO_TRIGMUONEFIDTPCOMMON_H
#define TRIGMUONHYPO_TRIGMUONEFIDTPCOMMON_H

#include "xAODTracking/TrackParticleContainer.h"

namespace TrigMuonEFIdtpCommon  {

float qOverPMatching(const xAOD::TrackParticle* metrack, const xAOD::TrackParticle* idtrack);
float matchingMetric(const xAOD::TrackParticle* metrack, const xAOD::TrackParticle* idtrack);

}

#endif

