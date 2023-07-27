#include "ZdcAnalysis/RPDDataAnalyzer.h"

RPDDataAnalyzer::RPDDataAnalyzer(ZDCMsg::MessageFunctionPtr messageFunc_p, const std::string& tag, unsigned int nRows, unsigned int nColumns, unsigned int nSamples) :
  m_msgFunc_p(messageFunc_p),
  m_tag(tag),
  m_nRows(nRows),
  m_nColumns(nColumns),
  m_nChannels(nRows*nColumns),
  m_nSamples(nSamples),
  m_chFADCdata(m_nChannels, std::vector<float>(nSamples, 0)),
  m_chSumADCvec(m_nChannels, 0),
  m_rowSumADCvec(m_nRows, 0),
  m_columnSumADCvec(m_nColumns, 0)
{
}

/**
 * Load a single channel's FADC data into member variable vector.
*/
void RPDDataAnalyzer::loadChannelData(unsigned int channel, const std::vector<uint16_t>& FADCdata)
{
  if (FADCdata.size() != m_nSamples) {
    (*m_msgFunc_p)(ZDCMsg::Fatal, "RPDDataAnalyzer::loadChannelData: received incorrect number of samples in FADCdata");
  }
  
  for (unsigned int sample = 0; sample < m_nSamples; sample++) {
    m_chFADCdata.at(channel).at(sample) = FADCdata.at(sample);
  }
}

/**
 * Reset all member variables to zero.
*/
void RPDDataAnalyzer::reset()
{
  for (unsigned int channel = 0; channel < m_nChannels; channel++) {
    m_chFADCdata.at(channel).assign(m_nSamples, 0);
  }
  m_chSumADCvec.assign(m_nChannels, 0);
  m_rowSumADCvec.assign(m_nRows, 0);
  m_columnSumADCvec.assign(m_nChannels, 0);
}

/**
 * Calculate sum ADC for each channel, subtracting pedestal.
*/
void RPDDataAnalyzer::analyzeData()
{
  // First check that we have valid data from all channels

  for (unsigned int channel = 0; channel < m_nChannels; channel++) {
    // pedestal subtraction
    // we naively assume that pedestal is the first sample; we could also consider using an average within some range
    float pedestal = m_chFADCdata.at(channel).at(0);
    float adc_sum = 0;
    for (unsigned int sample = 0; sample < m_nSamples; sample++) {
      float adc = m_chFADCdata.at(channel).at(sample);
      adc -= pedestal;
      adc_sum += adc;
    }
    m_chSumADCvec.at(channel) = adc_sum;
  }
  // calculate row and column sums
  unsigned int channel;
  for (unsigned int row = 0; row < m_nRows; row++) {
    for (unsigned int column = 0; column < m_nColumns; column++) {
      channel = m_nColumns*row + column;
      m_rowSumADCvec.at(row) += m_chSumADCvec.at(channel);
    }
  }
  for (unsigned int column = 0; column < m_nColumns; column++) {
    for (unsigned int row = 0; row < m_nRows; row++) {
      channel = m_nColumns*row + column;
      m_columnSumADCvec.at(column) += m_chSumADCvec.at(channel);
    }
  }
}

/**
 * Get the sample with maximum ADC content in the provided channel.
*/
unsigned int RPDDataAnalyzer::getChMaxADCSample(unsigned int channel) const
{
  float maxADC = -std::numeric_limits<float>::infinity();
  unsigned int maxSample = 0;
  for (unsigned int sample = 0; sample < m_nSamples; sample++) {
    float adc = m_chFADCdata.at(channel).at(sample);
    if (adc > maxADC) {
      maxADC = adc;
      maxSample = sample;
    }
  }
  return maxSample;
}

/**
 * Get the sum ADC of a single channel.
*/
float RPDDataAnalyzer::getChSumADC(unsigned int channel) const
{
  return m_chSumADCvec.at(channel);
};

/**
 * Get the sum ADC of all channels as a vector.
*/
const std::vector<float>& RPDDataAnalyzer::getChSumADCvec() const
{
  return m_chSumADCvec;
}

/**
 * Sum the sum ADC of channels in each row and return as a vector.
*/
const std::vector<float>& RPDDataAnalyzer::getRowSumADCvec() const
{
  return m_rowSumADCvec;
}

/**
 * Sum the sum ADC of channels in each column and return as a vector.
*/
const std::vector<float>& RPDDataAnalyzer::getColumnSumADCvec() const
{
  return m_columnSumADCvec;
}
