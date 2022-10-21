/*
   Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
 */

#ifndef HIMONITORINGZDCTOOL_H
#define HIMONITORINGZDCTOOL_H


#include "AthenaMonitoring/ManagedMonitorToolBase.h"


class TH1D;
class TH2D_LW;


class HIMonitoringZdcTool: public ManagedMonitorToolBase
{
public:
  HIMonitoringZdcTool(const std::string& type, const std::string& name,
                      const IInterface* parent);

  virtual ~HIMonitoringZdcTool();

  virtual StatusCode bookHistogramsRecurrent();
  virtual StatusCode bookHistograms();
  virtual StatusCode fillHistograms();
  virtual StatusCode procHistograms();


  void book_hist();
private:
  static constexpr int s_Nsamp {
    7
  };
  static constexpr int s_Nmod {
    4
  };
  static constexpr int s_Nside {
    2
  };


  /// histograms
  TH1D* m_hamp[s_Nmod][s_Nside] {};
  //TH1D* m_hamp_NEW[s_Nmod][s_Nside] {};
  TH1D* m_hampG0[s_Nmod][s_Nside] {};
  TH1D* m_hampG1[s_Nmod][s_Nside] {};
  TH1D* m_hSumSideAmp[s_Nside] {};
  //TH1D* m_hSumSideAmp_NEW[s_Nside] {};
  TH1D* m_hSumSideAmpG0[s_Nside] {};
  TH1D* m_hSumSideAmpG1[s_Nside] {};
  TH2D_LW* m_hEM_HAD1[s_Nside] {};
  TH2D_LW* m_hHAD1_HAD2[s_Nside] {};
  TH2D_LW* m_hHAD2_HAD3[s_Nside] {};
  TH2D_LW* m_hSideAC {};

  //TH2D_LW* m_hEM_HAD1_NEW[s_Nside] {};
  //TH2D_LW* m_hHAD1_HAD2_NEW[s_Nside] {};
  //TH2D_LW* m_hHAD2_HAD3_NEW[s_Nside] {};
  //TH2D_LW* m_hSideAC_NEW {};
};

#endif
