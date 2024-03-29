/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TILERECUTILS_ITILETIMEBCOFFSETFILTER_H
#define TILERECUTILS_ITILETIMEBCOFFSETFILTER_H

/********************************************************************
 *
 * NAME:     TileTileBCOffsetFilter
 * PACKAGE:  TileCalorimeter/TileRecUtils
 *
 * AUTHOR :  T. Davidek, A. Solodkov
 * CREATED:  Feb 2019
 *
 * PURPOSE:  mask the time offsets by +/-1 or +/-2 bunch crossings in the given
 *           Tile DMU
 *
 *  Input:   TileRawChannelContainer
 *  Output:  TileRawChannelContainer is modified
 *  Parameters:  none
 *
 ********************************************************************/

// Tile includes
#include "TileRecUtils/ITileRawChannelTool.h"
#include "TileConditions/TileEMScale.h"
#include "TileConditions/TileCablingSvc.h"
#include "TileConditions/TileDCSState.h"
#include "TileConditions/TileBadChannels.h"
#include "TileEvent/TileDQstatus.h"
#include "TileEvent/TileDigitsContainer.h"

// Atlas includes
#include "AthenaBaseComps/AthAlgTool.h"
#include "StoreGate/ReadHandleKey.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"

#include <vector>
#include <cmath>

// forward declarations
class TileHWID;
class TileRawChannel;
class TileRawChannelContainer;
class TileRawChannelCollection;
class TileCablingService;

/**
 @class TileTimeBCOffsetFilter
 @brief This tool sets bad status for channels with 25ns or 50ns timing jump
 */
class TileTimeBCOffsetFilter: public extends<AthAlgTool, ITileRawChannelTool> {

  public:

    TileTimeBCOffsetFilter(const std::string& type, const std::string& name,
                           const IInterface* parent);

    virtual ~TileTimeBCOffsetFilter() {};

    virtual StatusCode initialize() override;
    virtual StatusCode finalize() override;

    virtual StatusCode process (TileMutableRawChannelContainer& rchCont, const EventContext& ctx) const override;

  private:

    int find_partner(int ros, int channel) const;
    bool drawer_ok(int ros, int drawer, const TileBadChannels* badChannels,
                   std::vector<int> & channel_time_ok, std::vector<int> & bad_dmu) const;
    bool ch_masked_or_empty(int ros, int drawer, int channel, int gain, const TileBchStatus& chStatus,
                            const TileDQstatus* DQstatus, const TileDCSState* dcsState) const;
    float ref_digits_maxmindiff(int ros, int drawer, int ref_channel) const;

    const TileHWID* m_tileHWID; //!< Pointer to TileHWID

    const TileCablingService* m_cabling; //!< TileCabling instance

    ServiceHandle<TileCablingSvc> m_cablingSvc{ this,
        "TileCablingSvc", "TileCablingSvc", "The Tile cabling service"};

   /**
     * @brief Name of TileDCSState object in condition store
     */
    SG::ReadCondHandleKey<TileDCSState> m_DCSStateKey{this,
        "TileDCS", "TileDCS", "Input Tile DCS status"};

   /**
    * @brief Name of TileEMScale in condition store
    */
    SG::ReadCondHandleKey<TileEMScale> m_emScaleKey{this,
        "TileEMScale", "TileEMScale", "Input Tile EMS calibration constants"};

    /**
     * @brief Name of TileBadChannels in condition store
     */
    SG::ReadCondHandleKey<TileBadChannels> m_badChannelsKey{this,
        "TileBadChannels", "TileBadChannels", "Input Tile bad channel status"};

    // name of TDS container with TileDigits
    SG::ReadHandleKey<TileDigitsContainer> m_digitsContainerKey{this,"TileDigitsContainer","TileDigitsCnt",
                                                                "Input Tile digits container key"};

    // properties
    SG::ReadHandleKey<TileDQstatus> m_DQstatusKey{this, "TileDQstatus", 
                                                  "TileDQstatus", 
                                                  "TileDQstatus key"};

    float m_ene_threshold_3chan;
    float m_ene_threshold_1chan;
    float m_time_threshold_diff;
    float m_time_threshold_ref_ch;
    float m_ene_threshold_aver_time;
    float m_sample_diffmaxmin_threshold_hg;
    float m_sample_diffmaxmin_threshold_lg;
    bool m_checkDCS;

};
#endif

