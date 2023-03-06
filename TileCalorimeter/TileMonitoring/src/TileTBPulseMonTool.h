/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TILEMONITORING_TILETBPULSEMONTOOL_H
#define TILEMONITORING_TILETBPULSEMONTOOL_H

#include "TileFatherMonTool.h"

class ITileBadChanTool;

#include <vector>
#include <string>

class TileInfo;

/** @class
 *  @brief Class for TileCal pulse shape monitoring
 */

class ATLAS_NOT_THREAD_SAFE TileTBPulseMonTool : public TileFatherMonTool {  // deprecated: ATLASRECTS-7259

  public:

    TileTBPulseMonTool(const std::string & type, const std::string & name, const IInterface* parent);

    ~TileTBPulseMonTool();

    virtual StatusCode initialize() override;

    //pure virtual methods
    virtual StatusCode bookHistograms() override;
    virtual StatusCode fillHistograms() override;
    virtual StatusCode procHistograms() override;

  private:

    virtual StatusCode bookHistogramsPerModule(int ros, int drawer);

    std::string m_digitsContainerName;
    std::string m_rawChannelContainerName;

    ToolHandle<ITileBadChanTool> m_tileBadChanTool;

    TH2F* m_pulseHist2[5][64][48][2];
    TProfile* m_pulseProfile[5][64][48][2];

    void initFirstEvent(void);

    bool m_isFirstEvent;

    int m_useDemoCabling;
    std::vector<float> m_timeRange;
    // TileInfo
    std::string m_infoName;
    const TileInfo* m_tileInfo;
    int m_t0SamplePosition;
};

#endif
