/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "ZdcUtils/ZDCWaveformLTLinStep.h"

double ZDCWaveformLTLinStep::doEvaluate(double time) const 
{
  double tauRise = getTauRise();
  double tauFall = getTauFall();
  double tauLin = getAddtlShapeValue(0);
  
  // The maximum occurs after the turnon of the pulse, that should be t = 0
  //
  double timeMax = std::log((1.0 - std::exp(tauLin/tauRise))/(1.0 - std::exp(tauLin/tauFall)))/(1./tauRise-1./tauFall);
  double timeAdj = time + timeMax;
  
  if (timeAdj < 0) return 0;

  double term1 = (tauFall*(1.0 - std::exp(-timeAdj/tauFall)) - tauRise*(1.0 - std::exp(-timeAdj/tauRise)));
  double term2 = 0.;

  if (timeAdj > tauLin) term2 = (-tauFall*(1.0 - std::exp(-(timeAdj-tauLin)/tauFall)) +
				 tauRise*(1.0 - std::exp(-(timeAdj-tauLin)/tauRise)));

  return tauRise/tauLin*(term1 + term2)/ (tauFall-tauRise);
}

