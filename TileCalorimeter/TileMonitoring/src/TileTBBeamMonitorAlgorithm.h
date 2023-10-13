/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TILEMONITORING_TILETBBEAMMONITORALGORITHM_H
#define TILEMONITORING_TILETBBEAMMONITORALGORITHM_H

#include "TileEvent/TileBeamElemContainer.h"
#include "TileConditions/TileCablingSvc.h"

#include "AthenaMonitoring/AthMonitorAlgorithm.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "StoreGate/ReadHandleKey.h"

class TileHWID;

/** @class TileTBBeamMonitorAlgorithm
 *  @brief Class for Tile TB Beam elements based monitoring
 */

class TileTBBeamMonitorAlgorithm : public AthMonitorAlgorithm {

  public:

    using AthMonitorAlgorithm::AthMonitorAlgorithm;
    virtual ~TileTBBeamMonitorAlgorithm() = default;
    virtual StatusCode initialize() override;
    virtual StatusCode fillHistograms(const EventContext& ctx) const override;

  private:

    void errorWrongChannel(int frag, int channel) const;

    SG::ReadHandleKey<TileBeamElemContainer> m_beamElemContainerKey{this,
        "TileBeamElemContainer", "TileBeamElemCnt", "Input Tile beam elements container key"};

    /**
     * @brief Name of Tile cabling service
     */
    ServiceHandle<TileCablingSvc> m_cablingSvc{ this,
        "TileCablingSvc", "TileCablingSvc", "The Tile cabling service"};

    Gaudi::Property<std::vector<int>> m_fragIDs{this,
        "TileFragIDs", {0x100, 0x101, 0x200, 0x201, 0x402}, "Tile Frag IDs of modules to process."};

    Gaudi::Property<int> m_TBperiod{this,
        "TBperiod", 2016, "Tile TB period."};

    // Beam Chamber calibration with added offset results from the survey (tigran.mkrtchyan@cern.ch)
    Gaudi::Property<double> m_bc1HorizontalSlope{this,
        "BC1HorizontalSlope", -0.175657, "BC1 horizontal slope."};

    Gaudi::Property<double> m_bc1HorizontalOffset{this,
        "BC1HorizontalOffset", 0.181797 + 0.5, "BC1 horizontal offset."};

    Gaudi::Property<double> m_bc1VerticalSlope{this,
        "BC1VerticalSlope", -0.175965, "BC1 vertical slope."};

    Gaudi::Property<double> m_bc1VerticalOffset{this,
        "BC1VerticalOffset", -0.128910 - 1.9, "BC1 vertical offset."};

    Gaudi::Property<double> m_bc2HorizontalSlope{this,
        "BC2HorizontalSlope", -0.176735, "BC2 horizontal slope."};

    Gaudi::Property<double> m_bc2HorizontalOffset{this,
        "BC2HorizontalOffset", 0.622896039922 - 25., "BC2 horizontal offset."};

    Gaudi::Property<double> m_bc2VerticalSlope{this,
        "BC2VerticalSlope", -0.176182117624, "BC2 vertical slope."};

    Gaudi::Property<double> m_bc2VerticalOffset{this,
        "BC2VerticalOffset", 0.195954125116 + 17.7, "BC2 vertical offset."};

    Gaudi::Property<double> m_beamBC1Z{this,
        "BC1Z", 15600.0, "BC1 z position."};

    Gaudi::Property<double> m_beamBC2Z{this,
        "BC2Z", 2600.0, "BC2 z position."};


    std::vector<int> m_tofGroups;
    std::vector<int> m_sCounterGroups;
    std::vector<int> m_cherenkovGroups;
    std::vector<int> m_muonWallGroups;
    std::map<std::string, int> m_beamChamberGroups;

    enum BEAM_ELEMENTS_NUMBER {N_S_COUNTER = 3, N_CHERENKOV = 3, N_TOF = 3, N_MUON_WALL_PMT = 12};

    const TileHWID* m_tileHWID{nullptr};
};


#endif // TILEMONITORING_TILETBBEAMMONITORALGORITHM_H
