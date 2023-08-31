#include "ZdcAnalysis/RPDDataAnalyzer.h"

#include "TMatrixD.h"
#include "TVectorD.h"
#include <limits>

RPDDataAnalyzer::RPDDataAnalyzer(
  ZDCMsg::MessageFunctionPtr messageFunc_p, const std::string& tag, const RPDConfig& config
) :
  m_msgFunc_p(messageFunc_p),
  m_tag(tag),
  m_nRows(config.nRows),
  m_nColumns(config.nColumns),
  m_nChannels(m_nRows*m_nColumns),
  m_nSamples(config.nSamples),
  m_nBaselineSamples(config.nBaselineSamples),
  m_endSignalSample(config.endSignalSample),
  m_nominalBaseline(config.nominalBaseline),
  m_pileup1stDerivThresh(config.pileup1stDerivThresh),
  m_ADCoverflow(config.ADCoverflow),
  m_chCalibFactors(m_nChannels, 1),
  m_chFADCdata(m_nChannels, std::vector<float>(m_nSamples, 0)),
  m_chSumADCvec(m_nChannels, 0),
  m_chSumADCvecCalib(m_nChannels, 0),
  m_chPileupFracVec(m_nChannels, 0),
  m_rowSumADCvec(m_nRows, 0),
  m_rowSumADCvecCalib(m_nRows, 0),
  m_columnSumADCvec(m_nColumns, 0),
  m_columnSumADCvecCalib(m_nColumns, 0),
  m_chStatus(m_nChannels, 1 << ChValidBit)
{
  if (m_endSignalSample == 0) m_endSignalSample = m_nSamples; // default value 0 -> end of waveform
}

/**
 * Set (multiplicative) calibration factors for RPD channels.
*/
void RPDDataAnalyzer::setCalibFactors(const std::vector<float>& chCalibFactors)
{
  if (chCalibFactors.size() != m_nChannels) {
    (*m_msgFunc_p)(ZDCMsg::Fatal,
      "RPDDataAnalyzer::setCalibFactors: received incorrect number of channels in calibration factors ("
      + std::to_string(chCalibFactors.size()) + " != " + std::to_string(m_nChannels) + ")"
    );
  }
  m_chCalibFactors = chCalibFactors;
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
  m_nChannelsLoaded++;
}

/**
 * Reset all member variables to zero.
*/
void RPDDataAnalyzer::reset()
{
  // reset status bits; valid bit is true by default
  m_sideStatus = 1 << SideValidBit;
  for (unsigned int channel = 0; channel < m_nChannels; channel++) {
    m_chStatus.at(channel) = 1 << ChValidBit;
  }

  for (unsigned int channel = 0; channel < m_nChannels; channel++) {
    m_chFADCdata.at(channel).assign(m_nSamples, 0);
  }
  m_nChannelsLoaded = 0;
  m_chSumADCvec.assign(m_nChannels, 0);
  m_chSumADCvecCalib.assign(m_nChannels, 0);
  m_chPileupFracVec.assign(m_nChannels, 0);
  m_rowSumADCvec.assign(m_nRows, 0);
  m_rowSumADCvecCalib.assign(m_nRows, 0);
  m_columnSumADCvec.assign(m_nChannels, 0);
  m_columnSumADCvecCalib.assign(m_nChannels, 0);
}

/**
 * Calculate sum ADC for each channel, subtracting pedestal.
 * Check for overflow, out of time pileup.
*/
void RPDDataAnalyzer::analyzeData()
{
  if (m_nChannelsLoaded != m_nChannels) {
    (*m_msgFunc_p)(ZDCMsg::Warn,
      "RPDDataAnalyzer::analyzeData: analyzing data with " + std::to_string(m_nChannelsLoaded)+ " of "
      + std::to_string(m_nChannels) + " channels loaded"
    );
  }
  for (unsigned int channel = 0; channel < m_nChannels; channel++) {
    // check for OOT pileup
    float deriv = m_chFADCdata.at(channel).at(1) - m_chFADCdata.at(channel).at(0);
    if (-deriv > m_pileup1stDerivThresh) {
      // there is OOT pileup in this channel => expect negative exp in first samples
      // fit log of baseline-subtracted samples to a line (to approximate exponential fit)
      
      TMatrixD mat_X(m_nBaselineSamples, 2);
      TVectorD vec_y(m_nBaselineSamples);
      int n_points_skipped = 0;
      float x, y;
      int set_idx;
      for (unsigned int sample = 0; sample < m_nBaselineSamples; sample++) {
        x = sample;
        y = m_chFADCdata.at(channel).at(sample) - m_nominalBaseline;
        if (y <= 0) {
          // (*m_msgFunc_p)(ZDCMsg::Debug,
          //   "RPDDataAnalyzer::analyzeData: excluding negative baseline-subtracted FADC value from fit (RPD channel "
          //   + std::to_string(channel) + ", sample " + std::to_string(sample) + ")"
          // );
          n_points_skipped++;
          continue;
        }
        set_idx = sample - n_points_skipped;
        mat_X[set_idx][0] = 1.0;
        mat_X[set_idx][1] = x;
        vec_y[set_idx] = std::log(y); // y
      }
      if (n_points_skipped != 0) {
        mat_X.ResizeTo(m_nBaselineSamples - n_points_skipped, 2);
        vec_y.ResizeTo(m_nBaselineSamples - n_points_skipped);
      }
      TMatrixD mat_X_T(mat_X);
      mat_X_T.T();
      TMatrixD mat_X_T_X = mat_X_T*mat_X;
      mat_X_T_X.Invert(); // now this is the covariance matrix
      TVectorD vec_theta = mat_X_T_X*mat_X_T*vec_y;
      TVectorD vec_y_pred = mat_X*vec_theta;
      float analytic_chisqr = 0;
      for (uint i = 0; i < m_nBaselineSamples - n_points_skipped; i++) {
        analytic_chisqr += (vec_y[i] - vec_y_pred[i])*(vec_y[i] - vec_y_pred[i]);
      }
      float intercept = vec_theta[0];
      // float intercept_err = sqrt(mat_X_T_X[0][0]);
      float slope = vec_theta[1];
      // float slope_err = sqrt(mat_X_T_X[1][1]);
      
      // sum range is after baseline until end of signal
      // subtract contribution from exponential
      float adcSum = 0;
      float adc;
      for (unsigned int sample = m_nBaselineSamples; sample < m_endSignalSample; sample++) {
        adc = m_chFADCdata.at(channel).at(sample);
        adc -= m_nominalBaseline;
        adc -= exp(intercept + slope*sample);
        adcSum += adc;
      }
      // calculate sum ADC for pileup
      float pileupAdcSum = 0;
      for (unsigned int sample = 0; sample < m_endSignalSample; sample++) {
        pileupAdcSum += exp(intercept + slope*sample);
      }
      m_chSumADCvec.at(channel) = adcSum;
      m_chSumADCvecCalib.at(channel) = adcSum*m_chCalibFactors.at(channel);
      if (adcSum > 0) {
        m_chPileupFracVec.at(channel) = pileupAdcSum/adcSum;
      } else {
        // avoid dividing by zero or returning a negative number
        // this should be large - why not infinity?
        m_chPileupFracVec.at(channel) = std::numeric_limits<float>::infinity();
      }

      m_chStatus.at(channel) |= 1 << ChOutOfTimePileupBit;
      m_sideStatus |= 1 << SideOutOfTimePileupBit;
    } else {
      // no OOT pileup, assume that baseline is the first sample
      float baseline = m_chFADCdata.at(channel).at(0);
      float adcSum = 0;
      float adc;
      // sum range is after baseline until end of signal
      for (unsigned int sample = m_nBaselineSamples; sample < m_endSignalSample; sample++) {
        adc = m_chFADCdata.at(channel).at(sample);
        adc -= baseline;
        adcSum += adc;
      }
      m_chSumADCvec.at(channel) = adcSum;
      m_chSumADCvecCalib.at(channel) = adcSum*m_chCalibFactors.at(channel);
      // fractional pileup is zero initialized, and this is what we want for no pileup
    }

    // check for overflow
    for (unsigned int sample = m_nBaselineSamples; sample < m_endSignalSample; sample++) {
      if (m_chFADCdata.at(channel).at(sample) > m_ADCoverflow) {
        m_chStatus.at(channel) |= 1 << ChOverflowBit;
        m_sideStatus |= 1 << SideOverflowBit;
        // overflow -> analysis is not valid
        m_chStatus.at(channel) &= ~(1 << ChValidBit);
        m_sideStatus &= ~(1 << SideValidBit);
      }
    }
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
 * Get the sum ADC of a single channel, applying calibration factors.
*/
float RPDDataAnalyzer::getChSumADCcalib(unsigned int channel) const
{
  return m_chSumADCvecCalib.at(channel);
};

/**
 * Get OOT pileup sum as a fraction of non-pileup signal sum (zero if no OOT pileup).
*/
float RPDDataAnalyzer::getChPileupFrac(unsigned int channel) const
{
  return m_chPileupFracVec.at(channel);
};

/**
 * Get the sum ADC of all channels as a vector.
*/
const std::vector<float>& RPDDataAnalyzer::getChSumADCvec() const
{
  return m_chSumADCvec;
}

/**
 * Get the sum ADC of all channels as a vector, applying calibration factors.
*/
const std::vector<float>& RPDDataAnalyzer::getChSumADCvecCalib() const
{
  return m_chSumADCvecCalib;
}

/**
 * Sum the sum ADC of channels in each row and return as a vector.
*/
const std::vector<float>& RPDDataAnalyzer::getRowSumADCvec() const
{
  return m_rowSumADCvec;
}

/**
 * Sum the sum ADC of channels in each row and return as a vector, applying calibration factors.
*/
const std::vector<float>& RPDDataAnalyzer::getRowSumADCvecCalib() const
{
  return m_rowSumADCvecCalib;
}

/**
 * Sum the sum ADC of channels in each column and return as a vector.
*/
const std::vector<float>& RPDDataAnalyzer::getColumnSumADCvec() const
{
  return m_columnSumADCvec;
}

/**
 * Sum the sum ADC of channels in each column and return as a vector, applying calibration factors.
*/
const std::vector<float>& RPDDataAnalyzer::getColumnSumADCvecCalib() const
{
  return m_columnSumADCvecCalib;
}

/**
 * Get status word for a single channel.
 */
unsigned int RPDDataAnalyzer::getChStatus(unsigned int channel) const
{
  return m_chStatus.at(channel);
}

/**
 * Get status word for this side/RPD.
 */
unsigned int RPDDataAnalyzer::getSideStatus() const
{
  return m_sideStatus;
}
