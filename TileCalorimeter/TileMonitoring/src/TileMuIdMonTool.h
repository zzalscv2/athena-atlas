/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// ********************************************************************
//
// NAME:     TileMuIdMonTool.h
// PACKAGE:  TileMonitoring
//
// AUTHOR:   Luca Fiorini (Luca.Fiorini@cern.ch)
//	     
//
// ********************************************************************
#ifndef TILEMUIDMONTOOL_H
#define TILEMUIDMONTOOL_H

#include "TileFatherMonTool.h"
#include <vector>
#include <cstdint>
class TH1F;
class TH2F;
class TProfile;

/** @class TileMuIdMonTool
 *  @brief Class for TileMuid based monitoring
 */

class ATLAS_NOT_THREAD_SAFE TileMuIdMonTool: public TileFatherMonTool {  // deprecated: ATLASRECTS-7259

  public:

    TileMuIdMonTool(const std::string & type, const std::string & name, const IInterface* parent);

    virtual ~TileMuIdMonTool();

    virtual StatusCode initialize() final;

    //pure virtual methods
    StatusCode bookHistograms();
    StatusCode fillHistograms();
    StatusCode procHistograms();
    StatusCode checkHists(bool fromFinalize);

    StatusCode bookHistTrig(int trig);

    void cleanHistVec(); //necessary to avoid problems at the eb, lumi blocks boundaries

  private:

    int32_t m_TileMuidTrig;

    std::string m_muidsContName;

    std::vector<TH1F*> m_TileMuidTags;
    std::vector<TH1F*> m_TileMuidEnergy;
    std::vector<TH2F*> m_TileMuidEtaPhi;
    std::vector<TH1F*> m_TileMuidEta;
    std::vector<TH1F*> m_TileMuidPhi;
    std::vector<TH1F*> m_TileMuidQuality;
    std::vector<TH2F*> m_TileMuidEneQuality;

    std::vector<TProfile*> m_TileMuidEneEta;
    std::vector<TProfile*> m_TileMuidEnePhi;

    std::vector<TH1F*> m_TileMuid1TagEnergy;
    std::vector<TH2F*> m_TileMuid1TagEtaPhi;
    std::vector<TH1F*> m_TileMuid1TagEta;
    std::vector<TH1F*> m_TileMuid1TagPhi;
    std::vector<TH1F*> m_TileMuid1TagQuality;
    std::vector<TH2F*> m_TileMuid1TagEneQuality;

    std::vector<TH1F*> m_TileMuid2TagEnergy;
    std::vector<TH2F*> m_TileMuid2TagEtaPhi;
    std::vector<TH1F*> m_TileMuid2TagEta;
    std::vector<TH1F*> m_TileMuid2TagPhi;
    std::vector<TH1F*> m_TileMuid2TagQuality;
    std::vector<TH2F*> m_TileMuid2TagEneQuality;

};

#endif
