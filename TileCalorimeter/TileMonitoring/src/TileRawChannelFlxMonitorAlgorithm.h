/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TILEMONITORING_TILERAWCHANNELFLXALGORITHM_H
#define TILEMONITORING_TILERAWCHANNELFLXALGORITHM_H

// Tile includes
#include "TileEvent/TileRawChannelContainer.h"


// Atlas includes
#include "AthenaMonitoring/AthMonitorAlgorithm.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "StoreGate/ReadHandleKey.h"

class TileHWID;

class TileRawChannelFlxMonitorAlgorithm : public AthMonitorAlgorithm {

  public:

    using AthMonitorAlgorithm::AthMonitorAlgorithm;
    virtual ~TileRawChannelFlxMonitorAlgorithm() = default;
    virtual StatusCode initialize() override;
    virtual StatusCode fillHistograms( const EventContext& ctx ) const override;

  private:

    SG::ReadHandleKey<TileRawChannelContainer> m_rawChannelContainerKeyLegacy{this,
        "TileRawChannelContainerLegacy", "TileRawChannelFit", "Tile raw channel container"};

    SG::ReadHandleKey<TileRawChannelContainer> m_rawChannelContainerKeyFlx{this,
        "TileRawChannelContainerFlx", "TileRawChannelFlxFit", "Tile raw channel container for Felix"};

    const TileHWID* m_tileHWID{nullptr};

    Gaudi::Property<int> m_firstSample{this, "FirstSample", 0, "First sample to put into histogram"};
    Gaudi::Property<int> m_lastSample{this, "LastSample", 16, "Last sample to put into histogram"};

    Gaudi::Property<std::vector<int>> m_fragIDsToCompare{this, "TileFragIDsToCompare", {0x201, 0x402}, "Tile Frag IDs of modules to compare."};

    Gaudi::Property<unsigned int> m_felixScale{this, "FelixScale", 1, "Scale factor between Felix and Legacy ADC counts"}; // 1 for pedestal run or 4 for physics run

};
#endif // TILEMONITORING_TILERAWCHANNELFLXALGORITHM_H
