/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "ZdcUtils/ZDCWaveformSampler.h"
#include <algorithm>

std::vector<unsigned int> ZDCWaveformSampler::Generate(unsigned int channel, float amplitude, float T0)
{
  std::vector<unsigned int> samples;
  const ZDCWaveformBase* waveformPtr = m_waveformChanPtrs.at(channel).get();

  //
  // We loop over the requested number of samples
  //
  float time = m_timeMin;

  for (unsigned int isample = 0; isample < m_numSamples; isample++) {
    //
    // The waveform generates a shape with unit amplitude with maximum at t = 0
    //   we shift the evaluation to account for the actual T0
    //
    float valueUnitNorm = waveformPtr->evaluate(time - T0);

    // Now we scale up by the amplitue
    //
    float value = std::floor(valueUnitNorm*amplitude);

    // We apply the pedestal shift and force value > 0
    //
    value = std::max(value + m_pedestal, float(0.));
    
    // Now we truncate at the maximum ADC value
    //
    unsigned int valueIntTrunc = std::min(value, m_maxADC);
    samples.push_back(valueIntTrunc);

    time += m_deltaT;
  }

  return samples;
}
