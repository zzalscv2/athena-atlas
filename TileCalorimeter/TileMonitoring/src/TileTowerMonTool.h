/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// ********************************************************************
//
// NAME:     TileTowerMonTool.h
// PACKAGE:  TileMonitoring
//
// AUTHOR:   Luca Fiorini (Luca.Fiorini@cern.ch)
//	     
//
// ********************************************************************
#ifndef TILETOWERMONTOOL_H
#define TILETOWERMONTOOL_H

#include "TileFatherMonTool.h"
#include <cstdint>
#include <vector>

class TH2F;
class TH1F;

/** @class TileTowerMonTool
 *  @brief Class for TileTower based monitoring
 */

class ATLAS_NOT_THREAD_SAFE TileTowerMonTool: public TileFatherMonTool {  // deprecated: ATLASRECTS-7259

  public:

    TileTowerMonTool(const std::string & type, const std::string & name, const IInterface* parent);

    virtual ~TileTowerMonTool();

    virtual StatusCode initialize() final;

    //pure virtual methods
    StatusCode bookHistograms();
    StatusCode fillHistograms();
    StatusCode procHistograms();
    StatusCode checkHists(bool fromFinalize);

    StatusCode bookHistTrigPart(int trig, int part);
    StatusCode bookHistTrig(int trig);

    void cleanHistVec();

  private:

    int32_t m_TileTowerTrig;
    double m_Threshold;
    std::string m_towersContName;
    std::vector<TH2F*> m_TileTowerEtaPhi;
    std::vector<TH2F*> m_TileTowerEtaPhiDiff;
    std::vector<TH1F*> m_TileTowerEneDiff;
    std::vector<TH1F*> m_TileAllTowerEnergy[NPartHisto];
    std::vector<TH1F*> m_TileTowerEt[NPartHisto];
};

#endif
