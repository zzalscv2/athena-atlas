/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ZDCANALYSIS_RPDDataAnalyzer_h
#define ZDCANALYSIS_RPDDataAnalyzer_h

#include <string>
#include <vector>
#include <functional>
#include <bitset>

#include "ZdcAnalysis/ZDCMsg.h"

#include "CxxUtils/checker_macros.h"
ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

struct RPDConfig {
  unsigned int nRows;
  unsigned int nColumns;
  unsigned int nSamples;
  unsigned int nBaselineSamples;
  unsigned int endSignalSample;
  float pulse2ndDerivThresh;
  float postPulseFracThresh;
  unsigned int goodPulseSampleStart;
  unsigned int goodPulseSampleStop;
  unsigned int nominalBaseline;
  float pileupBaselineSumThresh;
  float pileupBaselineStdDevThresh;
  unsigned int nNegativesAllowed;
  unsigned int AdcOverflow;
};

class RPDDataAnalyzer
{
 public:
  enum {
    ValidBit                        =  0, // analysis and output are valid
    OutOfTimePileupBit              =  1, // OOT detected, pileup subtraction attempted
    OverflowBit                     =  2, // overflow detected => invalid
    PrePulseBit                     =  3, // pulse detected before expected range => invalid
    PostPulseBit                    =  4, // pulse detected after expected range => invalid
    NoPulseBit                      =  5, // no pulse detected => invalid
    BadAvgBaselineSubtrBit          =  6, // subtraction of avg. of baseline samples yielded too many negatives => invalid
    InsufficientPileupFitPointsBit  =  7, // baseline samples indicate pileup, but there are not enough points to perform fit -> nominal baseline used without pileup subtraction
    PileupStretchedExpFitFailBit    =  8, // fit to stretched exponential failed -> fallback to exponential fit
    PileupStretchedExpGrowthBit     =  9, // fit to stretched exponential does not decay -> fallback to exponential fit
    PileupBadStretchedExpSubtrBit   = 10, // subtraction of stretched exponential fit yielded too many negatives -> fallback to exponential fit
    PileupExpFitFailBit             = 11, // fit to exponential failed => invalid IF stretched exponential fit is also bad
    PileupExpGrowthBit              = 12, // fit to exponential does not decay => invalid IF stretched exponential fit is also bad
    PileupBadExpSubtrBit            = 13, // subtraction of stretched exponential yielded too many negatives => invalid IF stretched exponential fit is also bad
    PileupStretchedExpPulseLikeBit  = 14, // fit to stretched exponential probably looks more like a pulse than pileup
  };

  RPDDataAnalyzer(ZDCMsg::MessageFunctionPtr messageFunc_p, const std::string& tag, const RPDConfig& config, std::vector<float> const& calibFactors);

  virtual ~RPDDataAnalyzer() = default;

  void loadChannelData(unsigned int channel, const std::vector<uint16_t>& FadcData);
  void analyzeData();

  unsigned int getChMaxSample(unsigned int channel) const;
  float getChSumAdc(unsigned int channel) const;
  float getChSumAdcCalib(unsigned int channel) const;
  float getChMaxAdc(unsigned int channel) const;
  float getChMaxAdcCalib(unsigned int channel) const;
  float getChPileupFrac(unsigned int channel) const;
  float getChBaseline(unsigned int channel) const;
  const std::vector<float>& getChPileupExpFitParams(unsigned int channel) const;
  const std::vector<float>& getChPileupStretchedExpFitParams(unsigned int channel) const;
  const std::vector<float>& getChPileupExpFitParamErrs(unsigned int channel) const;
  const std::vector<float>& getChPileupStretchedExpFitParamErrs(unsigned int channel) const;
  float getChPileupExpFitMSE(unsigned int channel) const;
  float getChPileupStretchedExpFitMSE(unsigned int channel) const;

  unsigned int getChStatus(unsigned int channel) const;
  unsigned int getSideStatus() const;

  void reset();

 private:
  bool checkOverflow(unsigned int channel);
  bool checkPulses(unsigned int channel);
  unsigned int countSignalRangeNegatives(std::vector<float> const& values) const;
  bool doBaselinePileupSubtraction(unsigned int channel);
  void calculateMaxSampleMaxAdc(unsigned int channel);
  void calculateSumAdc(unsigned int channel);

  void setSideStatusBits();

  bool doPileupExpFit(unsigned int channel);
  bool doPileupStretchedExpFit(unsigned int channel);
  float calculateBaselineSamplesMSE(unsigned int channel, std::function<float(unsigned int)> const& fit) const;

  ZDCMsg::MessageFunctionPtr m_msgFunc_p;
  std::string m_tag;
  unsigned int m_nRows;
  unsigned int m_nColumns;
  unsigned int m_nChannels;
  unsigned int m_nChannelsLoaded = 0;
  unsigned int m_nSamples;
  unsigned int m_nBaselineSamples; /** Number of baseline samples; the sample equal to this number is the start of signal region */
  unsigned int m_endSignalSample; /** Samples before (not including) this sample are the signal region; nSamples goes to end of window */
  float m_pulse2ndDerivThresh; /** Second differences less than or equal to this number indicate a pulse */
  float m_postPulseFracThresh; /** If there is a good pulse and post-pulse and size of post-pulse as a fraction of good pulse is less than or equal to this number, ignore post-pulse */
  unsigned int m_goodPulseSampleStart; /** Pulses before this sample are considered pre-pulses */
  unsigned int m_goodPulseSampleStop; /** Pulses after this sample are considered post-pulses */
  unsigned int m_nominalBaseline; /** The global nominal baseline; used when pileup is detected */
  float m_pileupBaselineSumThresh; /** Baseline sums less than this number indicate there is not pileup */
  float m_pileupBaselineStdDevThresh; /** Baseline standard deviations less than this number indicate there is not pileup */
  unsigned int m_nNegativesAllowed; /** Maximum number of negative ADC values after baseline and pileup subtraction allowed in signal range */
  unsigned int m_AdcOverflow; /** ADC values greater than or equal to this number are considered overflow */
  std::vector<float> m_calibFactors; /** multiplicative calibration factors to apply to raw data; per channel */

  std::vector<std::vector<float>> m_chFadcData; /** raw RPD data; index channel then sample */
  std::vector<std::vector<float>> m_chCorrectedFadcData; /** RPD data with baseline and pileup subtracted; index channel then sample */
  std::vector<float> m_chMaxSample; /** sample of max of RPD data in signal range after pileup subtraction; per channel */
  std::vector<float> m_chSumAdc; /** sum of RPD data in signal range after baseline and pileup subtraction; per channel */
  std::vector<float> m_chSumAdcCalib; /** sum of RPD data in signal range after baseline and pileup subtraction, with calibration factors applied; per channel */
  std::vector<float> m_chMaxAdc; /** max of RPD data in signal range after baseline and pileup subtraction; per channel */
  std::vector<float> m_chMaxAdcCalib; /** max of RPD data in signal range after baseline and pileup subtraction, with calibration factors applied; per channel */
  std::vector<float> m_chPileupFrac; /** OOT pileup sum as a fraction of non-pileup sum in entire window (0 if no OOT pileup, -1 if sum ADC <= 0); per channel */
  std::vector<float> m_chBaseline; /** baseline used in baseline subtraction; per channel */
  std::vector<std::vector<float>> m_chPileupExpFitParams; /** parameters for pileup exponential fit (if pileup was detected and fit did not fail): exp( [0] + [1]*sample ); per channel */
  std::vector<std::vector<float>> m_chPileupStretchedExpFitParams; /** parameters for pileup stretched exponential fit (if pileup was detected and fit did not fail): exp( [0] + [1]*(sample + 4)**(0.5) + [2]*(sample + 4)**(-0.5) ); per channel */
  std::vector<std::vector<float>> m_chPileupExpFitParamErrs; /** parameter errors for pileup exponential fit (if pileup was detected and fit did not fail); per channel */
  std::vector<std::vector<float>> m_chPileupStretchedExpFitParamErrs; /** parameter errors for pileup stretched exponential fit (if pileup was detected and fit did not fail); per channel */
  std::vector<bool> m_chPileupFuncType; /** true when stretched exponential is used, false if exponential is used for pileup subtraction (if pileup was detected); per channel */
  std::vector<std::function<float(unsigned int)>> m_chExpPileupFuncs; /** pileup exponential fit function (if pileup was detected and fit did not fail); per channel */
  std::vector<std::function<float(unsigned int)>> m_ch2ndOrderStretchedExpPileupFuncs; /** pileup stretched exponential fit function (if pileup was detected and fit did not fail); per channel */
  std::vector<float> m_chExpPileupMSE; /** mean squared error of pileup exponential fit in baseline samples (if pileup was detected and fit did not fail); per channel */
  std::vector<float> m_ch2ndOrderStretchedExpPileupMSE; /** mean squared error of pileup stretched exponential fit in baseline samples (if pileup was detected and fit did not fail); per channel */
  std::vector<std::bitset<32>> m_chStatus; /** status bits per channel */
  std::bitset<32> m_sideStatus; /** status bits for side */
};
#endif
