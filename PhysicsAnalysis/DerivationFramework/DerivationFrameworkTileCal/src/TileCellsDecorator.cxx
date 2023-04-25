///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// TileCellsDecorator.cxx
// Implementation file for class TileCellsDecorator
///////////////////////////////////////////////////////////////////

// Tile includes
#include "TileCellsDecorator.h"
#include "TileEvent/TileCell.h"
#include "TileIdentifier/TileHWID.h"

// Athena includes
#include "AthenaKernel/errorcheck.h"
#include "StoreGate/WriteDecorHandle.h"
#include "xAODBase/IParticle.h"

// Calo includes
#include "CaloIdentifier/TileID.h"
#include "CaloEvent/CaloCell.h"
#include "CaloIdentifier/TileID.h"


#include <vector>


namespace DerivationFramework {

  static const InterfaceID IID_TileCellsDecorator("TileCellsDecorator", 1, 0);
  const InterfaceID& TileCellsDecorator::interfaceID( ) {  return IID_TileCellsDecorator; }


  TileCellsDecorator::TileCellsDecorator( const std::string& type, const std::string& name, const IInterface* parent )
    : AthAlgTool(type, name, parent)
  {
    declareInterface<TileCellsDecorator>(this);
  }


  StatusCode TileCellsDecorator::initialize() {

    ATH_CHECK( detStore()->retrieve(m_tileID) );
    ATH_CHECK( detStore()->retrieve(m_tileHWID) );

    const std::string baseName = m_muonContainer + ".";

    m_cellsEnergyKey = baseName + m_prefix + m_cellsEnergyKey.key();
    ATH_CHECK( m_cellsEnergyKey.initialize() );

    m_cellsEtKey = baseName + m_prefix + m_cellsEtKey.key();
    ATH_CHECK( m_cellsEtKey.initialize() );

    m_cellsEtaKey = baseName + m_prefix + m_cellsEtaKey.key();
    ATH_CHECK( m_cellsEtaKey.initialize() );

    m_cellsPhiKey = baseName + m_prefix + m_cellsPhiKey.key();
    ATH_CHECK( m_cellsPhiKey.initialize() );

    m_cellsGainKey = baseName + m_prefix + m_cellsGainKey.key();
    ATH_CHECK( m_cellsGainKey.initialize() );

    m_cellsBadKey = baseName + m_prefix + m_cellsBadKey.key();
    ATH_CHECK( m_cellsBadKey.initialize() );

    m_cellsSamplingKey = baseName + m_prefix + m_cellsSamplingKey.key();
    ATH_CHECK( m_cellsSamplingKey.initialize() );

    m_cellsTimeKey = baseName + m_prefix + m_cellsTimeKey.key();
    ATH_CHECK( m_cellsTimeKey.initialize() );

    m_cellsQualityKey = baseName + m_prefix + m_cellsQualityKey.key();
    ATH_CHECK( m_cellsQualityKey.initialize() );

    m_cellsSinThKey = baseName + m_prefix + m_cellsSinThKey.key();
    ATH_CHECK( m_cellsSinThKey.initialize(m_saveTileCellPositionAndDimention) );

    m_cellsCosThKey = baseName + m_prefix + m_cellsCosThKey.key();
    ATH_CHECK( m_cellsCosThKey.initialize(m_saveTileCellPositionAndDimention) );

    m_cellsCotThKey = baseName + m_prefix + m_cellsCotThKey.key();
    ATH_CHECK( m_cellsCotThKey.initialize(m_saveTileCellPositionAndDimention) );

    m_cellsXKey = baseName + m_prefix + m_cellsXKey.key();
    ATH_CHECK( m_cellsXKey.initialize(m_saveTileCellPositionAndDimention) );

    m_cellsYKey = baseName + m_prefix + m_cellsYKey.key();
    ATH_CHECK( m_cellsYKey.initialize(m_saveTileCellPositionAndDimention) );

    m_cellsZKey = baseName + m_prefix + m_cellsZKey.key();
    ATH_CHECK( m_cellsZKey.initialize(m_saveTileCellPositionAndDimention) );

    m_cellsRKey = baseName + m_prefix + m_cellsRKey.key();
    ATH_CHECK( m_cellsRKey.initialize(m_saveTileCellPositionAndDimention) );

    m_cellsDxKey = baseName + m_prefix + m_cellsDxKey.key();
    ATH_CHECK( m_cellsDxKey.initialize(m_saveTileCellPositionAndDimention) );

    m_cellsDyKey = baseName + m_prefix + m_cellsDyKey.key();
    ATH_CHECK( m_cellsDyKey.initialize(m_saveTileCellPositionAndDimention) );

    m_cellsDzKey = baseName + m_prefix + m_cellsDzKey.key();
    ATH_CHECK( m_cellsDzKey.initialize(m_saveTileCellPositionAndDimention) );

    m_cellsDrKey = baseName + m_prefix + m_cellsDrKey.key();
    ATH_CHECK( m_cellsDrKey.initialize(m_saveTileCellPositionAndDimention) );

    m_cellsVolumeKey = baseName + m_prefix + m_cellsVolumeKey.key();
    ATH_CHECK( m_cellsVolumeKey.initialize(m_saveTileCellPositionAndDimention) );

    m_cellsDetaKey = baseName + m_prefix + m_cellsDetaKey.key();
    ATH_CHECK( m_cellsDetaKey.initialize(m_saveTileCellPositionAndDimention) );

    m_cellsDphiKey = baseName + m_prefix + m_cellsDphiKey.key();
    ATH_CHECK( m_cellsDphiKey.initialize(m_saveTileCellPositionAndDimention) );

    m_cellsSideKey = baseName + m_prefix + m_cellsSideKey.key();
    ATH_CHECK( m_cellsSideKey.initialize(m_saveTileCellPositionAndDimention) );

    m_cellsSectionKey = baseName + m_prefix + m_cellsSectionKey.key();
    ATH_CHECK( m_cellsSectionKey.initialize(m_saveTileCellPositionAndDimention) );

    m_cellsModuleKey = baseName + m_prefix + m_cellsModuleKey.key();
    ATH_CHECK( m_cellsModuleKey.initialize(m_saveTileCellPositionAndDimention) );

    m_cellsTowerKey = baseName + m_prefix + m_cellsTowerKey.key();
    ATH_CHECK( m_cellsTowerKey.initialize(m_saveTileCellPositionAndDimention) );

    m_cellsSampleKey = baseName + m_prefix + m_cellsSampleKey.key();
    ATH_CHECK( m_cellsSampleKey.initialize(m_saveTileCellPositionAndDimention) );

    m_cellsPmt1RosKey = baseName + m_prefix + m_cellsPmt1RosKey.key();
    ATH_CHECK( m_cellsPmt1RosKey.initialize(m_saveTileCellPmtInfo) );

    m_cellsPmt2RosKey = baseName + m_prefix + m_cellsPmt2RosKey.key();
    ATH_CHECK( m_cellsPmt2RosKey.initialize(m_saveTileCellPmtInfo) );

    m_cellsPmt1DrawerKey = baseName + m_prefix + m_cellsPmt1DrawerKey.key();
    ATH_CHECK( m_cellsPmt1DrawerKey.initialize(m_saveTileCellPmtInfo) );

    m_cellsPmt2DrawerKey = baseName + m_prefix + m_cellsPmt2DrawerKey.key();
    ATH_CHECK( m_cellsPmt2DrawerKey.initialize(m_saveTileCellPmtInfo) );

    m_cellsPmt1ChannelKey = baseName + m_prefix + m_cellsPmt1ChannelKey.key();
    ATH_CHECK( m_cellsPmt1ChannelKey.initialize(m_saveTileCellPmtInfo) );

    m_cellsPmt2ChannelKey = baseName + m_prefix + m_cellsPmt2ChannelKey.key();
    ATH_CHECK( m_cellsPmt2ChannelKey.initialize(m_saveTileCellPmtInfo) );

    m_cellsPmt1EnergyKey = baseName + m_prefix + m_cellsPmt1EnergyKey.key();
    ATH_CHECK( m_cellsPmt1EnergyKey.initialize(m_saveTileCellPmtInfo) );

    m_cellsPmt2EnergyKey = baseName + m_prefix + m_cellsPmt2EnergyKey.key();
    ATH_CHECK( m_cellsPmt2EnergyKey.initialize(m_saveTileCellPmtInfo) );

    m_cellsPmt1TimeKey = baseName + m_prefix + m_cellsPmt1TimeKey.key();
    ATH_CHECK( m_cellsPmt1TimeKey.initialize(m_saveTileCellPmtInfo) );

    m_cellsPmt2TimeKey = baseName + m_prefix + m_cellsPmt2TimeKey.key();
    ATH_CHECK( m_cellsPmt2TimeKey.initialize(m_saveTileCellPmtInfo) );

    m_cellsPmt1QualityKey = baseName + m_prefix + m_cellsPmt1QualityKey.key();
    ATH_CHECK( m_cellsPmt1QualityKey.initialize(m_saveTileCellPmtInfo) );

    m_cellsPmt2QualityKey = baseName + m_prefix + m_cellsPmt2QualityKey.key();
    ATH_CHECK( m_cellsPmt2QualityKey.initialize(m_saveTileCellPmtInfo) );

    m_cellsPmt1QbitKey = baseName + m_prefix + m_cellsPmt1QbitKey.key();
    ATH_CHECK( m_cellsPmt1QbitKey.initialize(m_saveTileCellPmtInfo) );

    m_cellsPmt2QbitKey = baseName + m_prefix + m_cellsPmt2QbitKey.key();
    ATH_CHECK( m_cellsPmt2QbitKey.initialize(m_saveTileCellPmtInfo) );

    m_cellsPmt1BadKey = baseName + m_prefix + m_cellsPmt1BadKey.key();
    ATH_CHECK( m_cellsPmt1BadKey.initialize(m_saveTileCellPmtInfo) );

    m_cellsPmt2BadKey = baseName + m_prefix + m_cellsPmt2BadKey.key();
    ATH_CHECK( m_cellsPmt2BadKey.initialize(m_saveTileCellPmtInfo) );

    m_cellsPmt1GainKey = baseName + m_prefix + m_cellsPmt1GainKey.key();
    ATH_CHECK( m_cellsPmt1GainKey.initialize(m_saveTileCellPmtInfo) );

    m_cellsPmt2GainKey = baseName + m_prefix + m_cellsPmt2GainKey.key();
    ATH_CHECK( m_cellsPmt2GainKey.initialize(m_saveTileCellPmtInfo) );

    return StatusCode::SUCCESS;
  }

  StatusCode TileCellsDecorator::decorate(const xAOD::IParticle* particle, std::vector<const CaloCell*>& cells, const EventContext& ctx) const {

    std::vector< float > cells_energy;
    std::vector< float > cells_et;
    std::vector< float > cells_eta;
    std::vector< float > cells_phi;

    std::vector< int > cells_gain;
    std::vector< bool > cells_bad;
    std::vector< int > cells_sampling;
    std::vector< float > cells_time;
    std::vector< unsigned short > cells_quality;

    std::vector< float > cells_sinTh;
    std::vector< float > cells_cosTh;
    std::vector< float > cells_cotTh;
    std::vector< float > cells_x;
    std::vector< float > cells_y;
    std::vector< float > cells_z;

    std::vector< float > cells_r;
    std::vector< float > cells_dx;
    std::vector< float > cells_dy;
    std::vector< float > cells_dz;
    std::vector< float > cells_dr;
    std::vector< float > cells_volume;
    std::vector< float > cells_dphi;
    std::vector< float > cells_deta;

    std::vector< int > cells_side;
    std::vector< unsigned short > cells_section;
    std::vector< unsigned short > cells_module;
    std::vector< unsigned short > cells_tower;
    std::vector< unsigned short > cells_sample;

    std::vector< unsigned short > cells_pmt1_ros;
    std::vector< unsigned short > cells_pmt2_ros;
    std::vector< unsigned short > cells_pmt1_drawer;
    std::vector< unsigned short > cells_pmt2_drawer;
    std::vector< unsigned short > cells_pmt1_channel;
    std::vector< unsigned short > cells_pmt2_channel;

    std::vector< float > cells_pmt1_energy;
    std::vector< float > cells_pmt2_energy;
    std::vector< float > cells_pmt1_time;
    std::vector< float > cells_pmt2_time;

    std::vector< unsigned short > cells_pmt1_quality;
    std::vector< unsigned short > cells_pmt2_quality;
    std::vector< unsigned short > cells_pmt1_qbit;
    std::vector< unsigned short > cells_pmt2_qbit;

    std::vector< bool > cells_pmt1_bad;
    std::vector< bool > cells_pmt2_bad;
    std::vector< unsigned short > cells_pmt1_gain;
    std::vector< unsigned short > cells_pmt2_gain;


    for (const CaloCell* cell : cells) {

      const CaloDetDescrElement* cell_dde = cell->caloDDE();

      // Cell energy and eta/phi
      cells_energy.push_back( cell->energy() );
      cells_et.push_back( cell->et() );
      cells_eta.push_back( cell->eta() );
      cells_phi.push_back( cell->phi() );


      cells_gain.push_back( cell->gain() );
      cells_bad.push_back( cell->badcell() );
      cells_sampling.push_back( cell_dde->getSampling() );
      cells_time.push_back( cell->time() );
      cells_quality.push_back( cell->quality() );


      if (m_saveTileCellPositionAndDimention) {

        // Cell positioning
        cells_sinTh.push_back( cell->sinTh() );
        cells_cosTh.push_back( cell->cosTh() );
        cells_cotTh.push_back( cell->cotTh() );
        cells_x.push_back( cell->x() );
        cells_y.push_back( cell->y() );
        cells_z.push_back( cell->z() );

        // Cells dimensions
        cells_r.push_back( cell_dde->r() );
        cells_dx.push_back( cell_dde->dx() );
        cells_dy.push_back( cell_dde->dy() );
        cells_dz.push_back( cell_dde->dz() );
        cells_dr.push_back( cell_dde->dr() );
        cells_volume.push_back( cell_dde->volume() );
        cells_dphi.push_back( cell_dde->dphi() );
        cells_deta.push_back( cell_dde->deta() );

        cells_side.push_back( m_tileID->side(cell->ID()) );
        cells_section.push_back( m_tileID->section(cell->ID()) );
        cells_module.push_back( m_tileID->module(cell->ID()) );
        cells_tower.push_back( m_tileID->tower(cell->ID()) );
        cells_sample.push_back( m_tileID->sample(cell->ID()) );

      }

      if (m_saveTileCellPmtInfo) {

        // Tile PMT information
        const TileCell* tile_cell = dynamic_cast<const TileCell*> (cell);

        IdentifierHash hash1 = cell_dde->onl1();
        IdentifierHash hash2 = cell_dde->onl2();

        // Tile cell positioning
        int ros1 = m_tileHWID->ros(m_tileHWID->adc_id(hash1, tile_cell->gain1()));
        int drawer1 = m_tileHWID->drawer(m_tileHWID->adc_id(hash1, tile_cell->gain1()));
        int channel1 = m_tileHWID->channel(m_tileHWID->adc_id(hash1, tile_cell->gain1()));

        int ros2 = -1;
        int drawer2 = -1;
        int channel2 = -1;
        if (hash2 != TileHWID::NOT_VALID_HASH) {
          ros2 = m_tileHWID->ros(m_tileHWID->adc_id(hash2, tile_cell->gain2()));
          drawer2 = m_tileHWID->drawer(m_tileHWID->adc_id(hash2, tile_cell->gain2()));
          channel2 = m_tileHWID->channel(m_tileHWID->adc_id(hash2, tile_cell->gain2()));
        }

        cells_pmt1_ros.push_back( ros1 );
        cells_pmt2_ros.push_back( ros2 );
        cells_pmt1_drawer.push_back( drawer1 );
        cells_pmt2_drawer.push_back( drawer2 );
        cells_pmt1_channel.push_back( channel1 );
        cells_pmt2_channel.push_back( channel2 );

        cells_pmt1_energy.push_back( tile_cell->ene1() );
        cells_pmt2_energy.push_back( tile_cell->ene2() );
        cells_pmt1_time.push_back( tile_cell->time1() );
        cells_pmt2_time.push_back( tile_cell->time2() );
        cells_pmt1_quality.push_back( tile_cell->qual1() );
        cells_pmt2_quality.push_back( tile_cell->qual2() );
        cells_pmt1_qbit.push_back( tile_cell->qbit1() );
        cells_pmt2_qbit.push_back( tile_cell->qbit2() );
        cells_pmt1_bad.push_back( tile_cell->badch1() );
        cells_pmt2_bad.push_back( tile_cell->badch2() );
        cells_pmt1_gain.push_back( tile_cell->gain1() );
        cells_pmt2_gain.push_back( tile_cell->gain2() );
      }

    }

    SG::WriteDecorHandle<xAOD::MuonContainer, std::vector<float>> cellsEnergy(m_cellsEnergyKey, ctx);
    cellsEnergy(*particle) = std::move(cells_energy);

    SG::WriteDecorHandle<xAOD::MuonContainer, std::vector<float>> cellsEt(m_cellsEtKey, ctx);
    cellsEt(*particle) = std::move(cells_et);

    SG::WriteDecorHandle<xAOD::MuonContainer, std::vector<float>> cellsEta(m_cellsEtaKey, ctx);
    cellsEta(*particle) = std::move(cells_eta);

    SG::WriteDecorHandle<xAOD::MuonContainer, std::vector<float>> cellsPhi(m_cellsPhiKey, ctx);
    cellsPhi(*particle) = std::move(cells_phi);

    SG::WriteDecorHandle<xAOD::MuonContainer, std::vector<int>> cellsGain(m_cellsGainKey, ctx);
    cellsGain(*particle) = std::move(cells_gain);

    SG::WriteDecorHandle<xAOD::MuonContainer, std::vector<bool>> cellsBad(m_cellsBadKey, ctx);
    cellsBad(*particle) = std::move(cells_bad);

    SG::WriteDecorHandle<xAOD::MuonContainer, std::vector<int>> cellsSampling(m_cellsSamplingKey, ctx);
    cellsSampling(*particle) = std::move(cells_sampling);

    SG::WriteDecorHandle<xAOD::MuonContainer, std::vector<float>> cellsTime(m_cellsTimeKey, ctx);
    cellsTime(*particle) = std::move(cells_time);

    SG::WriteDecorHandle<xAOD::MuonContainer, std::vector<unsigned short>> cellsQuality(m_cellsQualityKey, ctx);
    cellsQuality(*particle) = std::move(cells_quality);


    if (m_saveTileCellPositionAndDimention) {

      SG::WriteDecorHandle<xAOD::MuonContainer, std::vector<float>> cellsSinTh(m_cellsSinThKey, ctx);
      cellsSinTh(*particle) = std::move(cells_sinTh);

      SG::WriteDecorHandle<xAOD::MuonContainer, std::vector<float>> cellsCosTh(m_cellsCosThKey, ctx);
      cellsCosTh(*particle) = std::move(cells_cosTh);

      SG::WriteDecorHandle<xAOD::MuonContainer, std::vector<float>> cellsCotTh(m_cellsCotThKey, ctx);
      cellsCotTh(*particle) = std::move(cells_cotTh);

      SG::WriteDecorHandle<xAOD::MuonContainer, std::vector<float>> cellsX(m_cellsXKey, ctx);
      cellsX(*particle) = std::move(cells_x);

      SG::WriteDecorHandle<xAOD::MuonContainer, std::vector<float>> cellsY(m_cellsYKey, ctx);
      cellsY(*particle) = std::move(cells_y);

      SG::WriteDecorHandle<xAOD::MuonContainer, std::vector<float>> cellsZ(m_cellsZKey, ctx);
      cellsZ(*particle) = std::move(cells_z);

      SG::WriteDecorHandle<xAOD::MuonContainer, std::vector<float>> cellsR(m_cellsRKey, ctx);
      cellsR(*particle) = std::move(cells_r);

      SG::WriteDecorHandle<xAOD::MuonContainer, std::vector<float>> cellsDx(m_cellsDxKey, ctx);
      cellsDx(*particle) = std::move(cells_dx);

      SG::WriteDecorHandle<xAOD::MuonContainer, std::vector<float>> cellsDy(m_cellsDyKey, ctx);
      cellsDy(*particle) = std::move(cells_dy);

      SG::WriteDecorHandle<xAOD::MuonContainer, std::vector<float>> cellsDz(m_cellsDzKey, ctx);
      cellsDz(*particle) = std::move(cells_dz);

      SG::WriteDecorHandle<xAOD::MuonContainer, std::vector<float>> cellsDr(m_cellsDrKey, ctx);
      cellsDr(*particle) = std::move(cells_dr);

      SG::WriteDecorHandle<xAOD::MuonContainer, std::vector<float>> cellsVolume(m_cellsVolumeKey, ctx);
      cellsVolume(*particle) = std::move(cells_volume);

      SG::WriteDecorHandle<xAOD::MuonContainer, std::vector<float>> cellsDeta(m_cellsDetaKey, ctx);
      cellsDeta(*particle) = std::move(cells_deta);

      SG::WriteDecorHandle<xAOD::MuonContainer, std::vector<float>> cellsDphi(m_cellsDphiKey, ctx);
      cellsDphi(*particle) = std::move(cells_dphi);

      SG::WriteDecorHandle<xAOD::MuonContainer, std::vector<int>> cellsSide(m_cellsSideKey, ctx);
      cellsSide(*particle) = std::move(cells_side);

      SG::WriteDecorHandle<xAOD::MuonContainer, std::vector<unsigned short>> cellsSection(m_cellsSectionKey, ctx);
      cellsSection(*particle) = std::move(cells_section);

      SG::WriteDecorHandle<xAOD::MuonContainer, std::vector<unsigned short>> cellsModule(m_cellsModuleKey, ctx);
      cellsModule(*particle) = std::move(cells_module);

      SG::WriteDecorHandle<xAOD::MuonContainer, std::vector<unsigned short>> cellsTower(m_cellsTowerKey, ctx);
      cellsTower(*particle) = std::move(cells_tower);

      SG::WriteDecorHandle<xAOD::MuonContainer, std::vector<unsigned short>> cellsSample(m_cellsSampleKey, ctx);
      cellsSample(*particle) = std::move(cells_sample);

    }


    if (m_saveTileCellPmtInfo) {

      SG::WriteDecorHandle<xAOD::MuonContainer, std::vector<unsigned short>> cellsPmt1Ros(m_cellsPmt1RosKey, ctx);
      cellsPmt1Ros(*particle) = std::move(cells_pmt1_ros);

      SG::WriteDecorHandle<xAOD::MuonContainer, std::vector<unsigned short>> cellsPmt2Ros(m_cellsPmt2RosKey, ctx);
      cellsPmt2Ros(*particle) = std::move(cells_pmt2_ros);

      SG::WriteDecorHandle<xAOD::MuonContainer, std::vector<unsigned short>> cellsPmt1Drawer(m_cellsPmt1DrawerKey, ctx);
      cellsPmt1Drawer(*particle) = std::move(cells_pmt1_drawer);

      SG::WriteDecorHandle<xAOD::MuonContainer, std::vector<unsigned short>> cellsPmt2Drawer(m_cellsPmt2DrawerKey, ctx);
      cellsPmt2Drawer(*particle) = std::move(cells_pmt2_drawer);

      SG::WriteDecorHandle<xAOD::MuonContainer, std::vector<unsigned short>> cellsPmt1Channel(m_cellsPmt1ChannelKey, ctx);
      cellsPmt1Channel(*particle) = std::move(cells_pmt1_channel);

      SG::WriteDecorHandle<xAOD::MuonContainer, std::vector<unsigned short>> cellsPmt2Channel(m_cellsPmt2ChannelKey, ctx);
      cellsPmt2Channel(*particle) = std::move(cells_pmt2_channel);

      SG::WriteDecorHandle<xAOD::MuonContainer, std::vector<float>> cellsPmt1Energy(m_cellsPmt1EnergyKey, ctx);
      cellsPmt1Energy(*particle) = std::move(cells_pmt1_energy);

      SG::WriteDecorHandle<xAOD::MuonContainer, std::vector<float>> cellsPmt2Energy(m_cellsPmt2EnergyKey, ctx);
      cellsPmt2Energy(*particle) = std::move(cells_pmt2_energy);

      SG::WriteDecorHandle<xAOD::MuonContainer, std::vector<float>> cellsPmt1Time(m_cellsPmt1TimeKey, ctx);
      cellsPmt1Time(*particle) = std::move(cells_pmt1_time);

      SG::WriteDecorHandle<xAOD::MuonContainer, std::vector<float>> cellsPmt2Time(m_cellsPmt2TimeKey, ctx);
      cellsPmt2Time(*particle) = std::move(cells_pmt2_time);

      SG::WriteDecorHandle<xAOD::MuonContainer, std::vector<unsigned short>> cellsPmt1Quality(m_cellsPmt1QualityKey, ctx);
      cellsPmt1Quality(*particle) = std::move(cells_pmt1_quality);

      SG::WriteDecorHandle<xAOD::MuonContainer, std::vector<unsigned short>> cellsPmt2Quality(m_cellsPmt2QualityKey, ctx);
      cellsPmt2Quality(*particle) = std::move(cells_pmt2_quality);

      SG::WriteDecorHandle<xAOD::MuonContainer, std::vector<unsigned short>> cellsPmt1_Qbit(m_cellsPmt1QbitKey, ctx);
      cellsPmt1_Qbit(*particle) = std::move(cells_pmt1_qbit);

      SG::WriteDecorHandle<xAOD::MuonContainer, std::vector<unsigned short>> cellsPmt2_Qbit(m_cellsPmt2QbitKey, ctx);
      cellsPmt2_Qbit(*particle) = std::move(cells_pmt2_qbit);

      SG::WriteDecorHandle<xAOD::MuonContainer, std::vector<bool>> cellsPmt1Bad(m_cellsPmt1BadKey, ctx);
      cellsPmt1Bad(*particle) = std::move(cells_pmt1_bad);

      SG::WriteDecorHandle<xAOD::MuonContainer, std::vector<bool>> cellsPmt2Bad(m_cellsPmt2BadKey, ctx);
      cellsPmt2Bad(*particle) = std::move(cells_pmt2_bad);

      SG::WriteDecorHandle<xAOD::MuonContainer, std::vector<unsigned short>> cellsPmt1Gain(m_cellsPmt1GainKey, ctx);
      cellsPmt1Gain(*particle) = std::move(cells_pmt1_gain);

      SG::WriteDecorHandle<xAOD::MuonContainer, std::vector<unsigned short>> cellsPmt2Gain(m_cellsPmt2GainKey, ctx);
      cellsPmt2Gain(*particle) = std::move(cells_pmt2_gain);

    }


    return StatusCode::SUCCESS;
  }

}
