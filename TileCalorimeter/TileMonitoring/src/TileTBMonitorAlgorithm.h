/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TILEMONITORING_TILETBMONITORALGORITHM_H
#define TILEMONITORING_TILETBMONITORALGORITHM_H

#include "TileConditions/TileCablingSvc.h"
#include "TileCalibBlobObjs/TileCalibUtils.h"

#include "CaloEvent/CaloCellContainer.h"

#include "AthenaMonitoring/AthMonitorAlgorithm.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "StoreGate/ReadHandleKey.h"

class TileID;
class TileHWID;

/** @class TileTBMonitorAlgorithm
 *  @brief Class for Tile TB based monitoring
 */

class TileTBMonitorAlgorithm : public AthMonitorAlgorithm {

  public:

    using AthMonitorAlgorithm::AthMonitorAlgorithm;
    virtual ~TileTBMonitorAlgorithm() = default;
    virtual StatusCode initialize() override;
    virtual StatusCode fillHistograms(const EventContext& ctx) const override;

  private:

    SG::ReadHandleKey<CaloCellContainer> m_caloCellContainerKey{this,
        "CaloCellContainer", "AllCalo", "Calo cell container name"};

    Gaudi::Property<float> m_cellEnergyThreshold{this,
        "CellEnergyThreshold", 100.0F, "Cell Energy threshold in MeV"};

    Gaudi::Property<float> m_energyThresholdForTime{this,
        "EnergyThresholdForTime", 500.0F, "Channel Energy threshold for time in MeV"};

    Gaudi::Property<float> m_beamEnergy{this,
        "BeamEnergy", 10000.0F, "Beam Energy in MeV"};

    Gaudi::Property<std::vector<std::string>> m_masked{this,
        "Masked", {}, "Masked channels: 'module gain channel,channel' (channels are separated by comma)"};

    Gaudi::Property<std::vector<int>> m_fragIDs{this,
        "TileFragIDs", {0x100, 0x101, 0x200, 0x201, 0x402}, "Tile Frag IDs of modules to process."};

    Gaudi::Property<std::vector<std::vector<double>>> m_xCellLB_A{this,
        "xCellLongBarrelSampleA", {{}}, "Bins X to be filled on Tile LB module 2D map for Cell A per tower"};

    Gaudi::Property<std::vector<std::vector<double>>> m_xCellLB_BC{this,
        "xCellLongBarrelSampleBC", {{}}, "Bins X to be filled on Tile LB module 2D map for Cell BC per tower"};

    Gaudi::Property<std::vector<std::vector<double>>> m_xCellLB_D{this,
        "xCellLongBarrelSampleD", {{}}, "Bins X to be filled on Tile LB module 2D map for Cell D per tower"};

    Gaudi::Property<std::vector<std::vector<double>>> m_yCellLB_A{this,
        "yCellLongBarrelSampleA", {{}}, "Bins Y to be filled on Tile LB module 2D map for Cell A per tower"};

    Gaudi::Property<std::vector<std::vector<double>>> m_yCellLB_BC{this,
        "yCellLongBarrelSampleBC", {{}}, "Bins Y to be filled on Tile LB module 2D map for Cell BC per tower"};

    Gaudi::Property<std::vector<std::vector<double>>> m_yCellLB_D{this,
        "yCellLongBarrelSampleD", {{}}, "Bins Y to be filled on Tile LB module 2D map for Cell D per tower"};

    Gaudi::Property<std::vector<std::vector<double>>> m_xCellEB_A{this,
        "xCellExtendedBarrelSampleA", {{}}, "Bins X to be filled on Tile EB module 2D map for Cell A per tower"};

    Gaudi::Property<std::vector<std::vector<double>>> m_xCellEB_BC{this,
        "xCellExtendedBarrelSampleBC", {{}}, "Bins X to be filled on Tile EB module 2D map for Cell BC per tower"};

    Gaudi::Property<std::vector<std::vector<double>>> m_xCellEB_D{this,
        "xCellExtendedBarrelSampleD", {{}}, "Bins X to be filled on Tile EB module 2D map for Cell D per tower"};

    Gaudi::Property<std::vector<std::vector<double>>> m_yCellEB_A{this,
        "yCellExtendedBarrelSampleA", {{}}, "Bins Y to be filled on Tile EB module 2D map for Cell A per tower"};

    Gaudi::Property<std::vector<std::vector<double>>> m_yCellEB_BC{this,
        "yCellExtendedBarrelSampleBC", {{}}, "Bins Y to be filled on Tile EB module 2D map for Cell BC per tower"};

    Gaudi::Property<std::vector<std::vector<double>>> m_yCellEB_D{this,
        "yCellExtendedBarrelSampleD", {{}}, "Bins Y to be filled on Tile EB module 2D map for Cell D per tower"};

   /**
     * @brief Name of Tile cabling service
     */
    ServiceHandle<TileCablingSvc> m_cablingSvc{ this,
        "TileCablingSvc", "TileCablingSvc", "The Tile cabling service"};


    std::map<std::string, int> m_timeGroups;
    std::map<std::string, int> m_cellMapGroups;

    const TileID* m_tileID{nullptr};
    const TileHWID* m_tileHWID{nullptr};

    std::vector<bool> m_monitoredDrawerIdx;

    std::array<std::array<unsigned char, TileCalibUtils::MAX_CHAN>, TileCalibUtils::MAX_DRAWERIDX> m_maskedChannels = {{}};
    std::vector<std::vector<IdentifierHash>> m_cellsNearTower{9};

    std::vector<std::vector<std::vector<double>>> m_xCellLB;
    std::vector<std::vector<std::vector<double>>> m_yCellLB;

    std::vector<std::vector<std::vector<double>>> m_xCellEB;
    std::vector<std::vector<std::vector<double>>> m_yCellEB;
};


#endif // TILEMONITORING_TILETBMONITORALGORITHM_H
