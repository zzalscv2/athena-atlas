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
  unsigned int ADCoverflow;
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
  unsigned int m_ADCoverflow;
  std::vector<float> m_chCalibFactors;

  std::vector<std::vector<float> > m_chFADCdata;
  std::vector<float> m_chSumADCvec;
  std::vector<float> m_chSumADCvecCalib;
  std::vector<float> m_rowSumADCvec;
  std::vector<float> m_rowSumADCvecCalib;
  std::vector<float> m_columnSumADCvec;
  std::vector<float> m_columnSumADCvecCalib;
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
    SideOverflowBit = 2, /** at least one channel has OOT pileup */
  };
  
  RPDDataAnalyzer(ZDCMsg::MessageFunctionPtr messageFunc_p, const std::string& tag, const RPDConfig& config);

  virtual ~RPDDataAnalyzer() = default;

  void setCalibFactors(const std::vector<float>& chFactors);
  void loadChannelData(unsigned int channel, const std::vector<uint16_t>& FADCdata);
  void analyzeData();

  unsigned int getChMaxADCSample(unsigned int channel) const;
  float getChSumADC(unsigned int channel) const;
  float getChSumADCcalib(unsigned int channel) const;
  const std::vector<float>& getChSumADCvec() const;
  const std::vector<float>& getChSumADCvecCalib() const;
  const std::vector<float>& getRowSumADCvec() const;
  const std::vector<float>& getRowSumADCvecCalib() const;
  const std::vector<float>& getColumnSumADCvec() const;
  const std::vector<float>& getColumnSumADCvecCalib() const;

  unsigned int getChStatus(unsigned int channel) const;
  unsigned int getSideStatus() const;

  void reset();
};
#endif
