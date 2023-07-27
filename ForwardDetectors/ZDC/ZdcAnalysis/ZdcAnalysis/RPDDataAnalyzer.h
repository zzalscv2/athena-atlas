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

class RPDDataAnalyzer
{
private:
  ZDCMsg::MessageFunctionPtr m_msgFunc_p;
  std::string m_tag;
  unsigned int m_nRows;
  unsigned int m_nColumns;
  unsigned int m_nChannels;
  unsigned int m_nSamples;
  std::vector<std::vector<float> > m_chFADCdata;
  std::vector<float> m_chSumADCvec;
  std::vector<float> m_rowSumADCvec;
  std::vector<float> m_columnSumADCvec;

public:
  RPDDataAnalyzer(ZDCMsg::MessageFunctionPtr messageFunc_p, const std::string& tag, unsigned int nRows, unsigned int nColumns, unsigned int nSamples);

  virtual ~RPDDataAnalyzer() = default;

  void loadChannelData(unsigned int channel, const std::vector<uint16_t>& FADCdata);

  void analyzeData();

  unsigned int getChMaxADCSample(unsigned int channel) const;
  float getChSumADC(unsigned int channel) const;
  const std::vector<float>& getChSumADCvec() const;
  const std::vector<float>& getRowSumADCvec() const;
  const std::vector<float>& getColumnSumADCvec() const;

  void reset();
};
#endif
