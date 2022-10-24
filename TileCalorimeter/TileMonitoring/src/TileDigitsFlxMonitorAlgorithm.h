/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TILEMONITORING_TILEDIGITSFLXALGORITHM_H
#define TILEMONITORING_TILEDIGITSFLXALGORITHM_H

// Tile includes
#include "TileEvent/TileDigitsContainer.h"


// Atlas includes
#include "AthenaMonitoring/AthMonitorAlgorithm.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "StoreGate/ReadHandleKey.h"

class TileHWID;

class TileDigitsFlxMonitorAlgorithm : public AthMonitorAlgorithm {
  public:
    using AthMonitorAlgorithm::AthMonitorAlgorithm;
    virtual ~TileDigitsFlxMonitorAlgorithm() = default;
    virtual StatusCode initialize() override;
    virtual StatusCode fillHistograms( const EventContext& ctx ) const override;
  private:

    SG::ReadHandleKey<TileDigitsContainer> m_digitsContainerKey{this,
        "TileDigitsContainer", "TileDigitsCnt", "Tile digits container"};

    const TileHWID* m_tileHWID{nullptr};

    Gaudi::Property<int> m_firstSample{this, "FirstSample", 0, "First sample to put into histogram"};
    Gaudi::Property<int> m_lastSample{this, "LastSample", 16, "Last sample to put into histogram"}; 
    Gaudi::Property<std::vector<int>> m_fragIDs{this, "FragIDs", {}, "Tile Frag IDs of modules to be monitored. Empty=ALL"};

};
#endif // TILEMONITORING_TILEDIGITSFLXALGORITHM_H
