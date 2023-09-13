/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// ********************************************************************
//
// NAME:     TileClusterMonTool.h
// PACKAGE:  TileMonitoring
//
// AUTHOR:   Luca Fiorini (Luca.Fiorini@cern.ch)
//	     
//
// ********************************************************************
#ifndef TILECLUSTERMONTOOL_H
#define TILECLUSTERMONTOOL_H

#include "TileFatherMonTool.h"

#include <array>
#include <vector>
#include <cstdint>

class TH1F;
class TProfile;
class TH2F;

/** @class TileClusterMonTool
 *  @brief Class for TileCluster based monitoring
 */

class ATLAS_NOT_THREAD_SAFE TileClusterMonTool: public TileFatherMonTool {  // deprecated: ATLASRECTS-7259

  public:

    TileClusterMonTool(const std::string & type, const std::string & name, const IInterface* parent);

    virtual ~TileClusterMonTool();

    virtual StatusCode initialize() final;

    //pure virtual methods
    StatusCode bookHistograms();
    StatusCode fillHistograms();
    StatusCode procHistograms();
    StatusCode checkHists(bool fromFinalize);

    StatusCode bookHistTrig(int trig);

    void cleanHistVec(); //necessary to avoid problems at the eb, lumi blocks boundaries

  private:

    int32_t m_TileClusterTrig;
    double m_Threshold;
    std::string m_clustersContName;

    std::vector<TH1F*> m_TilenClusters;
    std::vector<TH1F*> m_TileClusternCells;
    std::vector<TH1F*> m_TileClusterEnergy[NPartHisto];
    std::vector<TProfile*> m_TileClusterEneLumi;
    std::vector<TH1F*> m_TileClusterEt;
    std::vector<TH2F*> m_TileClusterEtaPhi;
    std::vector<TProfile*> m_TileClusterEneEta;
    std::vector<TProfile*> m_TileClusterEnePhi;
    std::vector<TProfile2D*> m_TileClusterEneEtaPhi;
    std::vector<TH2F*> m_TileClusterAllEtaPhi;
    std::vector<TH2F*> m_TileClusterEtaPhiDiff;
    std::vector<TH1F*> m_TileClusterEneDiff;
    std::vector<TH1F*> m_TileClusterTimeDiff;
    std::vector<TH1F*> m_TileAllClusterEnergy;
    std::vector<TH1F*> m_TileClusterSumEt;
    std::vector<TH1F*> m_TileClusterSumPx;
    std::vector<TH1F*> m_TileClusterSumPy;
    std::array<TProfile*, 5> m_partitionTimeLB;
    std::array<TProfile*, 5> m_paritionTimeOnlineLB;

    bool m_doOnline;
    int32_t m_oldLumiblock;

    bool m_fillTimingHistograms;
    float m_cellEnergyThresholdForTiming;
    int m_nLumiblocks;
};

#endif
