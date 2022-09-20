/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigMuonEFIdtpCommon.h"
#include "FourMomUtils/xAODP4Helpers.h"

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

float TrigMuonEFIdtpCommon::qOverPMatching(const xAOD::TrackParticle* metrack, const xAOD::TrackParticle* idtrack)
{
   float qOverPsignif = -10;

   if( idtrack && metrack ) {
      float mePt = metrack->pt();
      float idPt = idtrack->pt();
      float meSinTheta = sin(metrack->theta());
      float idSinTheta = sin(idtrack->theta());
      if( std::abs(meSinTheta) > 1e-5 && std::abs(idSinTheta) > 1e-5 ) {
	 float meP   = mePt / meSinTheta;
	 float idP   = idPt / idSinTheta;
	 float sigma = std::sqrt( idtrack->definingParametersCovMatrix()(4,4) + metrack->definingParametersCovMatrix()(4,4) );
	 if( std::abs(sigma) > 1e-5 ) {
	    qOverPsignif  = std::abs( (metrack->charge() / meP) - (idtrack->charge() / idP) ) /  sigma; 
	 }
      }
   }

   return qOverPsignif;
}

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

float TrigMuonEFIdtpCommon::matchingMetric(const xAOD::TrackParticle* metrack, const xAOD::TrackParticle* idtrack)
{
   float qoverp = TrigMuonEFIdtpCommon::qOverPMatching(metrack,idtrack);
   float dr = xAOD::P4Helpers::deltaR(idtrack,metrack);

   float metric = dr + std::abs(qoverp);

   return metric;
}

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

