/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef _ZDCWaveformSampler_h_
#define _ZDCWaveformSampler_h_

// Class that provides the ability to "sample" waveforms -- i.e. generate the analog
//   of Flash ADC samples -- at a specified frequency. The time at which the pulse is generated
//   can either be fixed or provided "event"-by-event. The amplitude must be provided for each
//   event as the Waveform has, by construction (pun intended), unity amplitude.
//
//   A valid waveform pointer must be provided at construction, but it can also be changed.
//
//   The sampler can operate with multiple channels, in which case multiple waveform objects
//     must be provided
//
//

#include <vector>
#include <memory>
#include "ZdcUtils/ZDCWaveform.h"

class ZDCWaveformSampler
{
  float m_freqMHz;
  float m_timeMin;
  unsigned int m_numSamples;
  unsigned int m_numBits;
  unsigned int m_pedestal;
  unsigned int m_numChannels;
  
  float m_deltaT;
  float m_maxADC;

  bool m_haveDefaultT0;
  float m_defaultT0;
  
  std::vector<std::shared_ptr<ZDCWaveformBase> > m_waveformChanPtrs;
  
public:
  
  ZDCWaveformSampler(float freqMHz, float timeMin, unsigned int numSamples, unsigned int nBits, unsigned int pedestal,
		     std::shared_ptr<ZDCWaveformBase> waveformPtr) :
    m_freqMHz(freqMHz),
    m_timeMin(timeMin),
    m_numSamples(numSamples),
    m_numBits(nBits),
    m_pedestal(pedestal),
    m_numChannels(1),
    m_deltaT(1000./freqMHz),
    m_maxADC((1<<nBits) - 1),
    m_haveDefaultT0(false),
    m_defaultT0(0.),
    m_waveformChanPtrs(1, waveformPtr)
  {}

  ZDCWaveformSampler(float freqMHz, float timeMin, unsigned int numSamples, unsigned int nBits, unsigned int pedestal,
		     std::vector<std::shared_ptr<ZDCWaveformBase> > waveformPtrVec) :
    m_freqMHz(freqMHz),
    m_timeMin(timeMin),
    m_numSamples(numSamples),
    m_numBits(nBits),
    m_pedestal(pedestal),
    m_numChannels(waveformPtrVec.size()),
    m_deltaT(1000./freqMHz),
    m_maxADC((1<<nBits) - 1),
    m_haveDefaultT0(false),
    m_defaultT0(0.),
    m_waveformChanPtrs(waveformPtrVec)
  {}

  void SetDefaultT0(float T0) {
    m_haveDefaultT0 = true;
    m_defaultT0 = T0;
  }
  
  std::vector<unsigned int> Generate(float amplitude)
  {
    if (m_numChannels != 1) throw std::runtime_error("ZDCWaveformSampler::Generate called with one parameter on an object with multiple channels.");
    if (!m_haveDefaultT0) throw std::runtime_error("ZDCWaveformSampler::Generate called with no default t0.");
    return Generate(amplitude, m_defaultT0);
  }
  
  std::vector<unsigned int> Generate(float amplitude, float T0)
  {
    if (m_numChannels != 1) throw std::runtime_error("ZDCWaveformSampler::Generate called with one parameter on an object with multiple channels.");
    return Generate(0, amplitude, T0);
  }

  std::vector<unsigned int> Generate(unsigned int channel, float amplitude, float T0);
};

#endif
