/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
/*
 */

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

#ifndef TILERAWCHANNELNOISEFILTER_H
#define TILERAWCHANNELNOISEFILTER_H

// Tile includes
#include "TileIdentifier/TileRawChannelUnit.h"
#include "TileRecUtils/ITileRawChannelTool.h"
#include "TileConditions/TileEMScale.h"
#include "TileConditions/TileBadChannels.h"
#include "TileConditions/TileSampleNoise.h"
#include "TileEvent/TileDQstatus.h"

// Atlas includes
#include "AthenaBaseComps/AthAlgTool.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"

// forward declarations
class TileHWID;
class TileRawChannel;
class TileRawChannelContainer;
class TileRawChannelCollection;
class TileInfo;

/**
 @class TileRawChannelNoiseFilter
 @brief This tool subtracts common-mode noise from all TileRawChannels in one container
 */
class TileRawChannelNoiseFilter: public extends<AthAlgTool, ITileRawChannelTool> {

  public:

    /** AlgTool like constructor */
    TileRawChannelNoiseFilter(const std::string& type, const std::string& name,
        const IInterface* parent);

    /** Virtual destructor */
    virtual ~TileRawChannelNoiseFilter() {};

    /** AlgTool initialize method.*/
    virtual StatusCode initialize() override;
    /** AlgTool finalize method */
    virtual StatusCode finalize() override;

    /** process the coherent noise subtruction algorithm and correct TileRawChannel amplitudes */
    virtual StatusCode process (TileMutableRawChannelContainer& rchCont, const EventContext& ctx) const override;


  private:

    const TileHWID* m_tileHWID; //!< Pointer to TileHWID

   /**
    * @brief Name of TileEMScale in condition store
    */
     SG::ReadCondHandleKey<TileEMScale> m_emScaleKey{this,
         "TileEMScale", "TileEMScale", "Input Tile EMS calibration constants"};

    /**
     * @brief Name of TileSampleNoise in condition store
     */
    SG::ReadCondHandleKey<TileSampleNoise> m_sampleNoiseKey{this,
        "TileSampleNoise", "TileSampleNoise", "Input Tile sample noise"};

    /**
     * @brief Name of TileBadChannels in condition store
     */
    SG::ReadCondHandleKey<TileBadChannels> m_badChannelsKey{this,
        "TileBadChannels", "TileBadChannels", "Input Tile bad channel status"};

    // properties
    SG::ReadHandleKey<TileDQstatus> m_DQstatusKey{this, "TileDQstatus", 
                                                  "TileDQstatus", 
                                                  "TileDQstatus key"};

    float m_truncationThresholdOnAbsEinSigma;
    float m_minimumNumberOfTruncatedChannels;
    bool m_useTwoGaussNoise;
    bool m_useGapCells;
    float m_maxNoiseSigma;
    // TileInfo
    std::string m_infoName;
    const TileInfo* m_tileInfo;
    float m_ADCmaskValueMinusEps; //!< indicates channels which were masked in background dataset
};

#endif // TILERAWCHANNELNOISEFILTER_H
