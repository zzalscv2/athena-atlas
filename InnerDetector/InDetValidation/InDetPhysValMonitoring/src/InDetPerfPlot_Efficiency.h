/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETPHYSVALMONITORING_INDETPERFPLOT_EFFICIENCY
#define INDETPHYSVALMONITORING_INDETPERFPLOT_EFFICIENCY
/**
 * @file InDetPerfPlot_Efficiency.cxx
 * @author Gabrel Facini <Gabriel.Facini@cern.ch>
 * Wed Oct 29 09:58:58 CET 2014
 *
 * a lot of this is copied from EfficiencyPlots in the TrkValHistUtils which is dumb
 * the point is that many instances of this will be created so more control of the names
 * is needed.  I don't have permission for that package and time is short...as usual
 **/



// local includes
#include "InDetPlotBase.h"
#include "xAODTruth/TruthParticle.h"
// std includes
#include <string>
class TEfficiency;

///class holding Pt plots for Inner Detector RTT Validation and implementing fill methods
class InDetPerfPlot_Efficiency: public InDetPlotBase {
public:
  InDetPerfPlot_Efficiency(InDetPlotBase* pParent, const std::string& dirName);

  void fill(const xAOD::TruthParticle& truth, const bool isGood, float weight, float mu);
private:
  TEfficiency* m_efficiency_vs_pteta{};
  TEfficiency* m_efficiency_vs_ptmu{};

  TEfficiency* m_efficiency_vs_eta{};
  TEfficiency* m_efficiency_vs_pt{};
  TEfficiency* m_efficiency_vs_pt_low{};
  TEfficiency* m_efficiency_vs_pt_high{};
  TEfficiency* m_efficiency_vs_pt_log{};
  TEfficiency* m_efficiency_vs_lowpt{};
  TEfficiency* m_efficiency_vs_phi{};
  TEfficiency* m_efficiency_vs_d0{};
  TEfficiency* m_efficiency_vs_d0_abs{};
  TEfficiency* m_efficiency_vs_z0{};
  TEfficiency* m_efficiency_vs_z0_abs{};
  TEfficiency* m_efficiency_vs_R{};
  TEfficiency* m_efficiency_vs_Z{};
  TEfficiency* m_efficiency_vs_mu{};

  TEfficiency* m_extended_efficiency_vs_d0{};
  TEfficiency* m_extended_efficiency_vs_d0_abs{};
  TEfficiency* m_extended_efficiency_vs_z0{};
  TEfficiency* m_extended_efficiency_vs_z0_abs{};
  TEfficiency* m_efficiency_vs_prodR{};
  TEfficiency* m_efficiency_vs_prodR_extended{};
  TEfficiency* m_efficiency_vs_prodZ{};
  TEfficiency* m_efficiency_vs_prodZ_extended{};

  TEfficiency* m_TrkRec_eta{};
  TEfficiency* m_TrkRec_d0{};
  TEfficiency* m_TrkRec_prodR{};
  TEfficiency* m_TrkRec_pT{};
  TEfficiency* m_TrkRec_mu{};
  TEfficiency* m_TrkRec_eta_d0{};
  TEfficiency* m_TrkRec_eta_prodR{};
  TEfficiency* m_TrkRec_eta_pT{};

  // plot base has nop default implementation of this; we use it to book the histos
  void initializePlots();
  void finalizePlots();
};

#endif
