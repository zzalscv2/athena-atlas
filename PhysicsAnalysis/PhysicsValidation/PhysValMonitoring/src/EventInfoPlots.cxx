/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "EventInfoPlots.h"

namespace PhysVal {
EventInfoPlots::EventInfoPlots(PlotBase* pParent, const std::string& sDir)
    : PlotBase(pParent, sDir) {}

void EventInfoPlots::initializePlots() {
  m_beamSpotWeight = Book1D("beamSpotWeight", "beamSpotWeight; weight ;Events", 100, 0., 10.);

  m_averageInteractionsPerCrossing = Book1D(
      "averageInteractionsPerCrossing",
      "averageInteractionsPerCrossing; average-#mu ;Events", 350, 0., 350);
  m_actualInteractionsPerCrossing =
      Book1D("actualInteractionsPerCrossing",
             "actualInteractionsPerCrossing; actual-#mu ;Events", 350, 0., 350);
  m_beamPosSigmaX = Book1D("beamPosSigmaX", "beamPosSigmaX; #sigma_{x} [mm] ;Events", 100, 0., 0.1);
  m_beamPosSigmaY = Book1D("beamPosSigmaY", "beamPosSigmaY; #sigma_{y} [mm] ;Events", 100, 0., 0.1);
  m_beamPosSigmaXY =
      Book1D("beamPosSigmaXY", "beamPosSigmaXY; #sigma_{xy} [mm] ;Events", 100, 0., 0.1);
  m_beamPosSigmaZ = Book1D("beamPosSigmaZ", "beamPosSigmaZ; #sigma_{z} [mm] ;Events", 100, 0., 100.);
}

void EventInfoPlots::fill(const xAOD::EventInfo* evt) {
  const auto beam_spot_weight = evt->beamSpotWeight();
  m_beamSpotWeight->Fill(beam_spot_weight);
  m_averageInteractionsPerCrossing->Fill(evt->averageInteractionsPerCrossing(),
                                         beam_spot_weight);
  m_actualInteractionsPerCrossing->Fill(evt->actualInteractionsPerCrossing(),
                                        beam_spot_weight);
  m_beamPosSigmaX->Fill(evt->beamPosSigmaX(), beam_spot_weight);
  m_beamPosSigmaY->Fill(evt->beamPosSigmaY(), beam_spot_weight);
  m_beamPosSigmaXY->Fill(evt->beamPosSigmaXY(), beam_spot_weight);
  m_beamPosSigmaZ->Fill(evt->beamPosSigmaZ(), beam_spot_weight);
}
}  // namespace PhysVal
