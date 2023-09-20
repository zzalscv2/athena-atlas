#include "ZdcAnalysis/RPDDataAnalyzer.h"

#include "TMatrixD.h"
#include "TVectorD.h"
#include <limits>

auto zeroVector = [](std::vector<float>& v){ v.assign(v.size(), 0); };
auto zeroVectorVector = [](std::vector<std::vector<float>>& vv){ for (std::vector<float>& v : vv) v.assign(v.size(), 0); };

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
  m_AdcOverflow(config.AdcOverflow),
  m_chCalibFactors(m_nChannels, 1),
  m_chFadcData(m_nChannels, std::vector<float>(m_nSamples, 0)),
  m_chMaxSample(m_nChannels, 0),
  m_chSumAdc(m_nChannels, 0),
  m_chSumAdcCalib(m_nChannels, 0),
  m_chMaxAdc(m_nChannels, 0),
  m_chMaxAdcCalib(m_nChannels, 0),
  m_chPileupFrac(m_nChannels, 0),
  m_chBaseline(m_nChannels, 0),
  m_chPileupFitParams(m_nChannels, std::vector<float>(2, 0)),
  m_rowSumAdc(m_nRows, 0),
  m_rowSumAdcCalib(m_nRows, 0),
  m_columnSumAdc(m_nColumns, 0),
  m_columnSumAdcCalib(m_nColumns, 0),
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
void RPDDataAnalyzer::loadChannelData(unsigned int channel, const std::vector<uint16_t>& FadcData)
{
  if (FadcData.size() != m_nSamples) {
    (*m_msgFunc_p)(ZDCMsg::Fatal, "RPDDataAnalyzer::loadChannelData: received incorrect number of samples in FADC data");
  }
  for (unsigned int sample = 0; sample < m_nSamples; sample++) {
    m_chFadcData.at(channel).at(sample) = FadcData.at(sample);
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
  
  m_chStatus.assign(m_chStatus.size(), 1 << ChValidBit);

  zeroVectorVector(m_chFadcData);
  zeroVectorVector(m_chPileupFitParams);

  m_nChannelsLoaded = 0;
  zeroVector(m_chMaxSample);
  zeroVector(m_chSumAdc);
  zeroVector(m_chSumAdcCalib);
  zeroVector(m_chMaxAdc);
  zeroVector(m_chMaxAdcCalib);
  zeroVector(m_chPileupFrac);
  zeroVector(m_chBaseline);
  zeroVector(m_rowSumAdc);
  zeroVector(m_rowSumAdcCalib);
  zeroVector(m_columnSumAdc);
  zeroVector(m_columnSumAdcCalib);
}

/**
 * Calculate sum and max ADC for each channel, subtracting pedestal.
 * Calculate max sample for each channel.
 * Calculate status bits for each channel and for this side.
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

    // might consider a more sophsticated pileup evalutation later on
    // e.g., theshold on slope of linear fit to baseline samples
    // or integral of baseline samples after nominal subtraction?

    // count the number of positive baseline samples after nominal baseline subtraction
    unsigned int nFitPoints = 0;
    for (unsigned int sample = 0; sample < m_nBaselineSamples; sample++) {
      if (m_chFadcData.at(channel).at(sample) - m_nominalBaseline > 0) nFitPoints++;
    }

    float deriv = m_chFadcData.at(channel).at(1) - m_chFadcData.at(channel).at(0);
    deriv *= m_chCalibFactors.at(channel); // apply calibration before comparing to threshold
    if (-deriv > m_pileup1stDerivThresh) {
      // we don't trust the average of baseline samples for baseline - there is probably pileup
      // we need >= 2 points in the baseline samples to be positive after baseline
      // subtraction in order to perform "exponential fit".
      // if there are < 2 postive baseline samples after baseline subtraction,
      // we say there is NOT pileup (even though derivative threshold is met)
      // and use nominal baseline, instead of averaging, as we typically would.
      if (nFitPoints >= 2) {
        // there is OOT pileup in this channel => expect negative exp in first samples
        // fit log of baseline-subtracted samples to a line (to approximate exponential fit)
        m_chBaseline.at(channel) = m_nominalBaseline; // use nominal baseline
        TMatrixD mat_X(m_nBaselineSamples, 2);
        TVectorD vec_y(m_nBaselineSamples);
        int n_points_skipped = 0;
        float x, y;
        int set_idx;
        for (unsigned int sample = 0; sample < m_nBaselineSamples; sample++) {
          x = sample;
          y = m_chFadcData.at(channel).at(sample) - m_nominalBaseline;
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
        float intercept = vec_theta[0];
        // float intercept_err = sqrt(mat_X_T_X[0][0]);
        float slope = vec_theta[1];
        // float slope_err = sqrt(mat_X_T_X[1][1]);
        m_chPileupFitParams.at(channel) = {intercept, slope};
        
        // calculate fadc data with baseline and pileup contribution subtracted
        std::vector<float> baselinePileupSubtractedData(m_nSamples);
        for (unsigned int sample = 0; sample < m_nSamples; sample++) {
          baselinePileupSubtractedData.at(sample) = m_chFadcData.at(channel).at(sample) - m_nominalBaseline - std::exp(intercept + slope*sample);
        }

        // calculate sumadc, sum range is after baseline until end of signal, subtract exponential contribution from pileup
        // calculate pileup contribution to sumadc (in sum range) -> pileup fraction
        float adcSum = 0;
        float pileupAdcSum = 0;
        for (unsigned int sample = m_nBaselineSamples; sample < m_endSignalSample; sample++) {
          adcSum += baselinePileupSubtractedData.at(sample);
          pileupAdcSum += std::exp(intercept + slope*sample);
        }
        m_chSumAdc.at(channel) = adcSum;
        m_chSumAdcCalib.at(channel) = adcSum*m_chCalibFactors.at(channel);
        if (adcSum > 0) {
          m_chPileupFrac.at(channel) = pileupAdcSum/adcSum;
        } else {
          // avoid dividing by zero, return nonsense value of -1
          m_chPileupFrac.at(channel) = -1;
        }

        // calculate max adc and sample
        float maxAdc = -std::numeric_limits<float>::infinity();
        unsigned int maxSample = 0;
        for (unsigned int sample = 0; sample < m_nSamples; sample++) {
          if (baselinePileupSubtractedData.at(sample) > maxAdc) {
            maxAdc = baselinePileupSubtractedData.at(sample);
            maxSample = sample;
          }
        }
        m_chMaxAdc.at(channel) = maxAdc;
        m_chMaxAdcCalib.at(channel) = maxAdc*m_chCalibFactors.at(channel);
        m_chMaxSample.at(channel) = maxSample;

        m_chStatus.at(channel) |= 1 << ChOutOfTimePileupBit;
        m_sideStatus |= 1 << SideOutOfTimePileupBit;
      } else {
        // not enough points to do "exponential fit" - say there is no pileup and use nominal baseline
        (*m_msgFunc_p)(ZDCMsg::Debug,
          "RPDDataAnalyzer::analyzeData: channel " + std::to_string(channel) + ": pileup 1st derivative threshold"
          " is met but there are not enough positive baseline samples after subtracting nominal baseline to do 'exponential fit'"
          " (there are " + std::to_string(nFitPoints) + " < 2)"
        );
        
        m_chBaseline.at(channel) = m_nominalBaseline;

        // calculate fadc data with baseline subtracted
        std::vector<float> baselineSubtractedData(m_nSamples);
        for (unsigned int sample = 0; sample < m_nSamples; sample++) {
          baselineSubtractedData.at(sample) = m_chFadcData.at(channel).at(sample) - m_nominalBaseline;
        }

        // calculate sumadc, sum range is after baseline until end of signal
        float adcSum = 0;
        for (unsigned int sample = m_nBaselineSamples; sample < m_endSignalSample; sample++) {
          adcSum += baselineSubtractedData.at(sample);
        }
        m_chSumAdc.at(channel) = adcSum;
        m_chSumAdcCalib.at(channel) = adcSum*m_chCalibFactors.at(channel);
        // fractional pileup is zero initialized, and this is what we want for no pileup

        // calculate max sample
        float maxAdc = -std::numeric_limits<float>::infinity();
        unsigned int maxSample = 0;
        for (unsigned int sample = 0; sample < m_nSamples; sample++) {
          if (baselineSubtractedData.at(sample) > maxAdc) {
            maxAdc = baselineSubtractedData.at(sample);
            maxSample = sample;
          }
        }
        m_chMaxAdc.at(channel) = maxAdc;
        m_chMaxAdcCalib.at(channel) = maxAdc*m_chCalibFactors.at(channel);
        m_chMaxSample.at(channel) = maxSample;
      }
    } else {
      // no OOT pileup, baseline is average of baseline samples
      float baseline = 0;
      for (unsigned int sample = 0; sample < m_nBaselineSamples; sample++) {
        baseline += m_chFadcData.at(channel).at(sample);;
      }
      baseline /= m_nBaselineSamples;
      m_chBaseline.at(channel) = baseline;

      // calculate fadc data with baseline subtracted
      std::vector<float> baselineSubtractedData(m_nSamples);
      for (unsigned int sample = 0; sample < m_nSamples; sample++) {
        baselineSubtractedData.at(sample) = m_chFadcData.at(channel).at(sample) - baseline;
      }

      // calculate sumadc, sum range is after baseline until end of signal
      float adcSum = 0;
      for (unsigned int sample = m_nBaselineSamples; sample < m_endSignalSample; sample++) {
        adcSum += baselineSubtractedData.at(sample);
      }
      m_chSumAdc.at(channel) = adcSum;
      m_chSumAdcCalib.at(channel) = adcSum*m_chCalibFactors.at(channel);
      // fractional pileup is zero initialized, and this is what we want for no pileup

      // calculate max sample
      float maxAdc = -std::numeric_limits<float>::infinity();
      unsigned int maxSample = 0;
      for (unsigned int sample = 0; sample < m_nSamples; sample++) {
        if (baselineSubtractedData.at(sample) > maxAdc) {
          maxAdc = baselineSubtractedData.at(sample);
          maxSample = sample;
        }
      }
      m_chMaxAdc.at(channel) = maxAdc;
      m_chMaxAdcCalib.at(channel) = maxAdc*m_chCalibFactors.at(channel);
      m_chMaxSample.at(channel) = maxSample;
    }

    // check for overflow
    for (unsigned int sample = 0; sample < m_nSamples; sample++) {
      if (m_chFadcData.at(channel).at(sample) > m_AdcOverflow) {
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
      m_rowSumAdc.at(row) += m_chSumAdc.at(channel);
      m_rowSumAdcCalib.at(row) += m_chSumAdcCalib.at(channel);
    }
  }
  for (unsigned int column = 0; column < m_nColumns; column++) {
    for (unsigned int row = 0; row < m_nRows; row++) {
      channel = m_nColumns*row + column;
      m_columnSumAdc.at(column) += m_chSumAdc.at(channel);
      m_columnSumAdcCalib.at(column) += m_chSumAdcCalib.at(channel);
    }
  }
}

/**
 * Get the sample with max ADC content in the provided channel.
 * If OOT pileup is detected, exponential contribution is first removed,
 *    then sample with max ADC is found.
*/
unsigned int RPDDataAnalyzer::getChMaxSample(unsigned int channel) const
{
  return m_chMaxSample.at(channel);
}

/**
 * Get the sum ADC of a single channel.
*/
float RPDDataAnalyzer::getChSumAdc(unsigned int channel) const
{
  return m_chSumAdc.at(channel);
}

/**
 * Get the sum ADC of a single channel, applying calibration factors.
*/
float RPDDataAnalyzer::getChSumAdcCalib(unsigned int channel) const
{
  return m_chSumAdcCalib.at(channel);
}

/**
 * Get the max ADC of a single channel.
*/
float RPDDataAnalyzer::getChMaxAdc(unsigned int channel) const
{
  return m_chMaxAdc.at(channel);
}

/**
 * Get the max ADC of a single channel, applying calibration factors.
*/
float RPDDataAnalyzer::getChMaxAdcCalib(unsigned int channel) const
{
  return m_chMaxAdcCalib.at(channel);
}

/**
 * Get OOT pileup sum as a fraction of non-pileup signal sum (zero if no OOT pileup).
*/
float RPDDataAnalyzer::getChPileupFrac(unsigned int channel) const
{
  return m_chPileupFrac.at(channel);
}

/**
 * Get the baseline of a single channel.
 */
float RPDDataAnalyzer::getChBaseline(unsigned int channel) const {
  return m_chBaseline.at(channel);
}

/**
 * Get the exponential fit parameters for pileup if pileup was detected
 * (exp([0] + [1]*sample)).
 */
const std::vector<float>& RPDDataAnalyzer::getChPileupFitParams(unsigned int channel) const {
  return m_chPileupFitParams.at(channel);
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
