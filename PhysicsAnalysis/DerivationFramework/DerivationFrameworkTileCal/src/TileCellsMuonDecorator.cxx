///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// TileCellsMuonDecorator.cxx
// Implementation file for class TileCellsMuonDecorator
///////////////////////////////////////////////////////////////////

// Tile includes
#include "TileCellsMuonDecorator.h"

// Athena
#include "AthenaKernel/errorcheck.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteDecorHandle.h"

#include <vector>
#include <algorithm>

namespace DerivationFramework {

  StatusCode TileCellsMuonDecorator::initialize() {

    ATH_CHECK(m_trackInCalo.retrieve());
    ATH_CHECK(m_tracksInCone.retrieve());
    ATH_CHECK(m_cellsDecorator.retrieve());

    ATH_CHECK( m_muonContainerKey.initialize() );

    const std::string baseName = m_muonContainerKey.key() + ".";

    m_selectedMuKey = baseName + m_prefix + m_selectedMuKey.key();
    ATH_CHECK( m_selectedMuKey.initialize() );

    m_econeMuKey = baseName + m_prefix + m_econeMuKey.key() + std::to_string(int(m_isoCone * 100));
    ATH_CHECK( m_econeMuKey.initialize() );

    m_cellsMuonXKey = baseName + m_prefix + m_cellsMuonXKey.key();
    ATH_CHECK( m_cellsMuonXKey.initialize(m_saveTileCellMuonInfo) );

    m_cellsMuonYKey = baseName + m_prefix + m_cellsMuonYKey.key();
    ATH_CHECK( m_cellsMuonYKey.initialize(m_saveTileCellMuonInfo) );

    m_cellsMuonZKey = baseName + m_prefix + m_cellsMuonZKey.key();
    ATH_CHECK( m_cellsMuonZKey.initialize(m_saveTileCellMuonInfo) );

    m_cellsMuonEtaKey = baseName + m_prefix + m_cellsMuonEtaKey.key();
    ATH_CHECK( m_cellsMuonEtaKey.initialize(m_saveTileCellMuonInfo) );

    m_cellsMuonPhiKey = baseName + m_prefix + m_cellsMuonPhiKey.key();
    ATH_CHECK( m_cellsMuonPhiKey.initialize(m_saveTileCellMuonInfo) );

    m_cellsToMuonDxKey = baseName + m_prefix + m_cellsToMuonDxKey.key();
    ATH_CHECK( m_cellsToMuonDxKey.initialize(m_saveTileCellMuonInfo) );

    m_cellsToMuonDyKey = baseName + m_prefix + m_cellsToMuonDyKey.key();
    ATH_CHECK( m_cellsToMuonDyKey.initialize(m_saveTileCellMuonInfo) );

    m_cellsToMuonDzKey = baseName + m_prefix + m_cellsToMuonDzKey.key();
    ATH_CHECK( m_cellsToMuonDzKey.initialize(m_saveTileCellMuonInfo) );

    m_cellsToMuonDetaKey = baseName + m_prefix + m_cellsToMuonDetaKey.key();
    ATH_CHECK( m_cellsToMuonDetaKey.initialize(m_saveTileCellMuonInfo) );

    m_cellsToMuonDphiKey = baseName + m_prefix + m_cellsToMuonDphiKey.key();
    ATH_CHECK( m_cellsToMuonDphiKey.initialize(m_saveTileCellMuonInfo) );

    m_cellsMuonDxKey = baseName + m_prefix + m_cellsMuonDxKey.key();
    ATH_CHECK( m_cellsMuonDxKey.initialize(m_saveTileCellMuonInfo) );

    m_cellsMuonDeDxKey = baseName + m_prefix + m_cellsMuonDeDxKey.key();
    ATH_CHECK( m_cellsMuonDeDxKey.initialize(m_saveTileCellMuonInfo) );

    return StatusCode::SUCCESS;
  }

  StatusCode TileCellsMuonDecorator::addBranches() const {

    const EventContext& ctx = Gaudi::Hive::currentContext();

    SG::ReadHandle<xAOD::MuonContainer> muons(m_muonContainerKey, ctx);
    ATH_CHECK( muons.isValid() );

    std::vector<const CaloCell*> cells;

    SG::WriteDecorHandle<xAOD::MuonContainer, int> selected_mu(m_selectedMuKey, ctx);

    for ( const xAOD::Muon* mu : *muons ) {

      std::vector< float > cells_mu_x;
      std::vector< float > cells_mu_y;
      std::vector< float > cells_mu_z;
      std::vector< float > cells_mu_eta;
      std::vector< float > cells_mu_phi;

      std::vector< float > cells_to_mu_dx;
      std::vector< float > cells_to_mu_dy;
      std::vector< float > cells_to_mu_dz;
      std::vector< float > cells_to_mu_deta;
      std::vector< float > cells_to_mu_dphi;

      std::vector< float > cells_mu_dx;
      std::vector< float > cells_mu_dedx;


      if (m_selectMuons &&
          (mu->muonType() != xAOD::Muon::Combined
           || mu->pt() < m_minPt
           || std::abs(mu->eta()) > m_maxAbsEta)) {

        selected_mu(*mu) = 0;
        continue;
      }

      const xAOD::TrackParticle* mu_track = mu->trackParticle(xAOD::Muon::InnerDetectorTrackParticle);
      const xAOD::CaloCluster* mu_cluster = mu->cluster();
      if (mu_track && mu_cluster && mu_cluster->getCellLinks()) {

        float e_trk_in_isocone(0.0);
        std::vector<const xAOD::TrackParticle*> tracks_in_cone;
        m_tracksInCone->particlesInCone(mu_track->eta(), mu_track->phi(), m_isoCone, tracks_in_cone);
        for (const xAOD::TrackParticle* track : tracks_in_cone) {
          if (track != mu_track) e_trk_in_isocone += track->e();
        }

        SG::WriteDecorHandle<xAOD::MuonContainer, float> econe_mu(m_econeMuKey, ctx);
        econe_mu(*mu) = e_trk_in_isocone;

        if (m_selectMuons && (e_trk_in_isocone > m_maxRelEtrkInIsoCone * mu_track->e())) {
          selected_mu(*mu) = 0;
          continue;
        }

        selected_mu(*mu) = 1;

        cells.clear();
        for (const CaloCell* cell : *mu_cluster) {

          const CaloDetDescrElement* cell_dde = cell->caloDDE();
          if (!(cell_dde->is_tile())) continue;

          cells.push_back(cell);

          if (m_saveTileCellMuonInfo) {
            std::vector<double> coordinates = m_trackInCalo->getXYZEtaPhiInCellSampling(mu_track, cell);

            if (coordinates.size() == 5 ) {

              float path_length = m_trackInCalo->getPathInsideCell(mu_track, cell);
              cells_mu_dx.push_back( path_length );
              cells_mu_dedx.push_back( (path_length > 0 ? (cell->energy() / path_length) : -1.0) );

              cells_mu_x.push_back(coordinates[0]);
              cells_mu_y.push_back(coordinates[1]);
              cells_mu_z.push_back(coordinates[2]);
              cells_mu_eta.push_back(coordinates[3]);
              cells_mu_phi.push_back(coordinates[4]);

              cells_to_mu_dx.push_back(cell->x() - coordinates[0]);
              cells_to_mu_dy.push_back(cell->y() - coordinates[1]);
              cells_to_mu_dz.push_back(cell->z() - coordinates[2]);
              cells_to_mu_deta.push_back(cell->eta() - coordinates[3]);
              cells_to_mu_dphi.push_back( KinematicUtils::deltaPhi(coordinates[4], cell->phi()) );

            } else {

              cells_mu_dx.push_back( 0.0 );
              cells_mu_dedx.push_back( -2.0 );

              cells_mu_x.push_back(0.0);
              cells_mu_y.push_back(0.0);
              cells_mu_z.push_back(0.0);
              cells_mu_eta.push_back(0.0);
              cells_mu_phi.push_back(0.0);

              cells_to_mu_dx.push_back(0.0);
              cells_to_mu_dy.push_back(0.0);
              cells_to_mu_dz.push_back(0.0);
              cells_to_mu_deta.push_back(0.0);
              cells_to_mu_dphi.push_back(0.0);

            }

          }
        }

        ATH_CHECK( m_cellsDecorator->decorate(mu, cells, ctx) );

      } else {
        selected_mu(*mu) = 0;
      }


      if (m_saveTileCellMuonInfo) {

        SG::WriteDecorHandle<xAOD::MuonContainer, std::vector<float>> cellsMuonX(m_cellsMuonXKey, ctx);
        cellsMuonX(*mu) = std::move(cells_mu_x);

        SG::WriteDecorHandle<xAOD::MuonContainer, std::vector<float>> cellsMuonY(m_cellsMuonYKey, ctx);
        cellsMuonY(*mu) = std::move(cells_mu_y);

        SG::WriteDecorHandle<xAOD::MuonContainer, std::vector<float>> cellsMuonZ(m_cellsMuonZKey, ctx);
        cellsMuonZ(*mu) = std::move(cells_mu_z);

        SG::WriteDecorHandle<xAOD::MuonContainer, std::vector<float>> cellsMuonEta(m_cellsMuonEtaKey, ctx);
        cellsMuonEta(*mu) = std::move(cells_mu_eta);

        SG::WriteDecorHandle<xAOD::MuonContainer, std::vector<float>> cellsMuonPhi(m_cellsMuonPhiKey, ctx);
        cellsMuonPhi(*mu) = std::move(cells_mu_phi);

        SG::WriteDecorHandle<xAOD::MuonContainer, std::vector<float>> cellsToMuonDx(m_cellsToMuonDxKey, ctx);
        cellsToMuonDx(*mu) = std::move(cells_to_mu_dx);

        SG::WriteDecorHandle<xAOD::MuonContainer, std::vector<float>> cellsToMuonDy(m_cellsToMuonDyKey, ctx);
        cellsToMuonDy(*mu) = std::move(cells_to_mu_dy);

        SG::WriteDecorHandle<xAOD::MuonContainer, std::vector<float>> cellsToMuonDz(m_cellsToMuonDzKey, ctx);
        cellsToMuonDz(*mu) = std::move(cells_to_mu_dz);

        SG::WriteDecorHandle<xAOD::MuonContainer, std::vector<float>> cellsToMuonDeta(m_cellsToMuonDetaKey, ctx);
        cellsToMuonDeta(*mu) = std::move(cells_to_mu_deta);

        SG::WriteDecorHandle<xAOD::MuonContainer, std::vector<float>> cellsToMuonDphi(m_cellsToMuonDphiKey, ctx);
        cellsToMuonDphi(*mu) = std::move(cells_to_mu_dphi);

        SG::WriteDecorHandle<xAOD::MuonContainer, std::vector<float>> cellsMuonDx(m_cellsMuonDxKey, ctx);
        cellsMuonDx(*mu) = std::move(cells_mu_dx);

        SG::WriteDecorHandle<xAOD::MuonContainer, std::vector<float>> cellsMuonDeDx(m_cellsMuonDeDxKey, ctx);
        cellsMuonDeDx(*mu) = std::move(cells_mu_dedx);

      }

    }

    return StatusCode::SUCCESS;
  }

}
