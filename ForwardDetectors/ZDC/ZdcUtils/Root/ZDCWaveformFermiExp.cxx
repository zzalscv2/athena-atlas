/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "ZdcUtils/ZDCWaveformFermiExp.h"

double ZDCWaveformFermiExp::doEvaluate(double time) const 
{
  double tau1 = getTauRise();
  double tau2 = getTauFall();
  double shift = -tau1 * std::log(tau2 / tau1 - 1.0);
  double timeShift = time - shift;
  
  double expTerm = std::exp(-timeShift / tau2);
  double fermiTerm = 1. / (1. + std::exp(-timeShift / tau1));
  
  return expTerm * fermiTerm;
}

