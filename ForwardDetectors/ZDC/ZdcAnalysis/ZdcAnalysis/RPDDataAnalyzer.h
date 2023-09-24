/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ZDCANALYSIS_RPDDataAnalyzer_h
#define ZDCANALYSIS_RPDDataAnalyzer_h

#include <string>
#include <vector>
#include <limits>

#include "ZdcAnalysis/ZDCMsg.h"

#include "CxxUtils/checker_macros.h"
ATLAS_NO_CHECK_FILE_THREAD_SAFETY; 

struct RPDConfig {
  unsigned int nRows;
  unsigned int nColumns;
  unsigned int nSamples;
  unsigned int nBaselineSamples;
  unsigned int endSignalSample;
  unsigned int nominalBaseline;
  float pileup1stDerivThresh;
  unsigned int AdcOverflow;
};

class RPDDataAnalyzer
{
private:
  ZDCMsg::MessageFunctionPtr m_msgFunc_p;
  std::string m_tag;
  unsigned int m_nRows;
  unsigned int m_nColumns;
  unsigned int m_nChannels;
  unsigned int m_nChannelsLoaded = 0;
  unsigned int m_nSamples;
  unsigned int m_nBaselineSamples; /** number of samples before signal expected to be baseline */
  unsigned int m_endSignalSample; /** sample expected to be the end of signal; sum until (not including) this sample */
  unsigned int m_nominalBaseline; /** baseline value to use when OOT pileup is detected */
  float m_pileup1stDerivThresh; /** OOT pileup if -derivative in first two samples is greater than this number */
  unsigned int m_AdcOverflow;
  std::vector<float> m_chCalibFactors;

  std::vector<std::vector<float> > m_chFadcData;
  std::vector<float> m_chMaxSample;
  std::vector<float> m_chSumAdc;
  std::vector<float> m_chSumAdcCalib;
  std::vector<float> m_chMaxAdc;
  std::vector<float> m_chMaxAdcCalib;
  std::vector<float> m_chPileupFrac; /** OOT pileup sum as a fraction of non-pileup signal sum (0 if no OOT pileup, -1 if sumadc <= 0) */
  std::vector<float> m_chBaseline;
  std::vector<std::vector<float>> m_chPileupFitParams;
  std::vector<float> m_rowSumAdc;
  std::vector<float> m_rowSumAdcCalib;
  std::vector<float> m_columnSumAdc;
  std::vector<float> m_columnSumAdcCalib;
  std::vector<unsigned int> m_chStatus;
  unsigned int m_sideStatus = 1 << SideValidBit;

public:
  enum {
    // channel-level status bits
    ChValidBit = 0, /** analysis is good, e.g. no channel overflow */
    ChOutOfTimePileupBit = 1,
    ChOverflowBit = 2,

    // side/rpd-level status bits
    SideValidBit = 0, /** analysis is good, e.g. no channel overflow */
    SideOutOfTimePileupBit = 1, /** at least one channel has OOT pileup */
    SideOverflowBit = 2, /** at least one channel has overflow */
  };
  
  RPDDataAnalyzer(ZDCMsg::MessageFunctionPtr messageFunc_p, const std::string& tag, const RPDConfig& config);

  virtual ~RPDDataAnalyzer() = default;

  void setCalibFactors(const std::vector<float>& chFactors);
  void loadChannelData(unsigned int channel, const std::vector<uint16_t>& FadcData);
  void analyzeData();

  unsigned int getChMaxSample(unsigned int channel) const;
  float getChSumAdc(unsigned int channel) const;
  float getChSumAdcCalib(unsigned int channel) const;
  float getChMaxAdc(unsigned int channel) const;
  float getChMaxAdcCalib(unsigned int channel) const;
  float getChPileupFrac(unsigned int channel) const;
  float getChBaseline(unsigned int channel) const;
  const std::vector<float>& getChPileupFitParams(unsigned int channel) const;

  unsigned int getChStatus(unsigned int channel) const;
  unsigned int getSideStatus() const;

  void reset();
};
#endif
