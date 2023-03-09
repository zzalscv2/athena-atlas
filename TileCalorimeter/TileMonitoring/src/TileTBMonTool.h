/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// ********************************************************************
//
// NAME:     TileTBMonTool.h
// ********************************************************************
#ifndef TILEMONITORING_TILETBMONTOOL_H
#define TILEMONITORING_TILETBMONTOOL_H

#include "TileFatherMonTool.h"

#include "Identifier/IdentifierHash.h"

class TileDQstatus;

#include <vector>
#include <string>

/** @class TileTBMonTool
 *  @brief Class for TileCal TB monitoring
 */

class ATLAS_NOT_THREAD_SAFE TileTBMonTool : public TileFatherMonTool {  // deprecated: ATLASRECTS-7259

  public:

    TileTBMonTool(const std::string & type, const std::string & name, const IInterface* parent);

    ~TileTBMonTool();

    virtual StatusCode initialize();

    //pure virtual methods
    virtual StatusCode bookHistograms();
    virtual StatusCode fillHistograms();
    virtual StatusCode procHistograms();

  private:

    void fillHitMap(int side, int section, int module, int tower, int sample, double energy);
    void initFirstEvent(void);
    void bookTimeHistograms(int ros, int drawer);

    std::string m_cellContainer; 


    TH1F* m_tileTotalEventEnergy{};

    TProfile2D* m_tileTBHitMapLBC01{};
    TProfile2D* m_tileTBHitMapEBC02{};
    TProfile2D* m_tileTBHitMap{};
    TH2F* m_tileEventEnergyVsCellsNumber{};
    TH2F* m_CtotVsClong{};
    TH1F* m_Ctot{};
    TH1F* m_Clong{};
    TH1F* m_hotCell_LBC02_A{nullptr};

    TProfile* m_tileTBChannelTime[4][64] = {};

    bool m_isFirstEvent{};
    unsigned char m_maskedChannels[276][48] = {}; // max drawerIdx = 276 and max channel = 48
    std::vector<std::string> m_masked;

    double m_maxTotalEnergy{};
    double m_cellEnergyThreshold{};
    float m_energyThresholdForTime{};
    double m_beamEnergy{};
    std::vector<std::vector<IdentifierHash>> m_cellsNearTower;
};

#endif
