/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PHYSVALMONITORING_EVENTINFO_PLOTS_H
#define PHYSVALMONITORING_EVENTINFO_PLOTS_H

#include "TrkValHistUtils/PlotBase.h"
#include "xAODEventInfo/EventInfo.h"

namespace PhysVal {
class EventInfoPlots : public PlotBase {
 public:
  EventInfoPlots(PlotBase* pParent, const std::string& sDir);
  void fill(const xAOD::EventInfo* evt);

 private:
  TH1* m_averageInteractionsPerCrossing = nullptr;
  TH1* m_actualInteractionsPerCrossing = nullptr;
  TH1* m_beamSpotWeight = nullptr;
  TH1* m_beamPosSigmaX = nullptr;
  TH1* m_beamPosSigmaY = nullptr;
  TH1* m_beamPosSigmaXY = nullptr;
  TH1* m_beamPosSigmaZ = nullptr;

  virtual void initializePlots();
};
}  // namespace PhysVal

#endif