/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/


#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/MsgStream.h"

#include "StoreGate/ReadHandle.h"

#include "CLHEP/Units/SystemOfUnits.h"

#include "CaloGeoHelpers/CaloPhiRange.h"

#include "CaloGeoHelpers/CaloSampling.h"
#include "CaloEvent/CaloCell.h"
#include "CaloEvent/CaloTower.h"
#include "CaloEvent/CaloTowerContainer.h"

#include "CaloTowerMonitor.h"

#include <string>
#include <cmath>

using CLHEP::GeV;
using CLHEP::deg;


StatusCode CaloTowerMonitor::initialize()
{
  ATH_CHECK( m_histSvc.retrieve() );
  ////////////////////////
  // Book Distributions //
  ////////////////////////

  auto book1 = [&] (const std::string& name,
                    const std::string& title,
                    int nx,
                    float xlo,
                    float xhi,
                    TH1*& hptr) -> StatusCode
  {
    return m_histSvc->regHist ("/" + m_streamName + "/CaloTowerMonitor/" + name,
                               std::make_unique<TH1F> (name.c_str(), title.c_str(),
                                                       nx, xlo, xhi),
                               hptr);
  };
  auto book2 = [&] (const std::string& name,
                    const std::string& title,
                    int nx,
                    float xlo,
                    float xhi,
                    int ny,
                    float ylo,
                    float yhi,
                    TH2*& hptr) -> StatusCode
  {
    return m_histSvc->regHist ("/" + m_streamName + "/CaloTowerMonitor/" + name,
                               std::make_unique<TH2F> (name.c_str(), title.c_str(),
                                                       nx, xlo, xhi,
                                                       ny, ylo, yhi),
                               hptr);
  };

  // number of towers
  ATH_CHECK( book1 ("nTowersVsEta", "Number of CaloTowers vs Eta",
                    100, -5., 5.,
                    m_nTowersVsEta) );
  ATH_CHECK( book1 ("nTowersVsPhi", "Number of CaloTowers vs Phi [deg]",
                    100, -200., 200.,
                    m_nTowersVsPhi) );

  // tower energies
  ATH_CHECK( book1 ("eTowers", "CaloTower E [GeV]",
                    100, 0., 100.,
                    m_eTowers) );
  ATH_CHECK( book2 ("eTowersVsEta", "CaloTower E [GeV] vs Eta",
                    100, -5., 5.,
                    100, 0., 100.,
                    m_eTowersVsEta) );
  ATH_CHECK( book2 ("eTowersVsPhi", "CaloTower E [Gev] vs Phi [deg]",
                    100, -200., 200.,
                    100, 0., 100.,
                    m_eTowersVsPhi) );
  ATH_CHECK( book1 ("eLogTowers", "CaloTower log(E/GeV)",
                    100, -7., 3.,
                    m_eLogTowers) );

  // tower et's
  ATH_CHECK( book1 ("etTowers", "CaloTower Et [GeV]",
                    100, 0., 100.,
                    m_etTowers) );
  ATH_CHECK( book2 ("etTowersVsEta", "CaloTower Et [GeV] vs Eta",
                    100, -5., 5.,
                    100, 0., 100.,
                    m_etTowersVsEta) );
  ATH_CHECK( book2 ("etTowersVsPhi", "CaloTower Et [Gev] vs Phi [deg]",
                    100, -200., 200.,
                    100, 0., 100.,
                    m_etTowersVsPhi) );
  ATH_CHECK( book1 ("etLogTowers", "CaloTower log(Et/GeV)",
                    100, -7., 3.,
                    m_etLogTowers) );

  // tower shapes
  ATH_CHECK( book2 ("cellsInEtaVsPhi", "CaloTower Shape",
                    100, -0.5, 0.5,
                    100, -0.5, 0.5,
                    m_cellsInEtaVsPhi) );
  ATH_CHECK( book1 ("nCellsInTower", "Number of Cells in CaloTower",
                    100, 0., 100.,
                    m_nCellsInTower) );
  ATH_CHECK( book2 ("nCellsInTowerVsEta", "Number of Cells vs Eta",
                    100, -5., 5.,
                    100, 0., 100.,
                    m_nCellsInTowerVsEta) );
  ATH_CHECK( book2 ("nCellsInTowerVsPhi", "Number of Cells vs Phi [deg]",
                    100, -200., 200.,
                    100, 0., 100.,
                    m_nCellsInTowerVsPhi) );

  // eta direction matches
  ATH_CHECK( book2 ("etaTowerVsCell", "Cell Eta vs TowerEta",
                    100, -5., 5.,
                    100, -5., 5.,
                    m_etaTowerVsCell) );
  ATH_CHECK( book2 ("etaTowerVsCellCalosPreSamplerB",
                    "Cell Eta vs Tower Eta PreSampB",
                    100, -5., 5.,
                    100, -5., 5.,
                    m_etaTowerVsCellCalos[CaloSampling::PreSamplerB]) );
  ATH_CHECK( book2 ("etaTowerVsCellCalosEMB1",
                    "Cell Eta vs Tower Eta EMB1",
                    100, -5., 5.,
                    100, -5., 5.,
                    m_etaTowerVsCellCalos[CaloSampling::EMB1]) );
  ATH_CHECK( book2 ("etaTowerVsCellCalosPreSamplerE",
                    "Cell Eta vs Tower Eta PreSampE",
                    100, -5., 5.,
                    100, -5., 5.,
                    m_etaTowerVsCellCalos[CaloSampling::PreSamplerE]) );
  ATH_CHECK( book2 ("etaTowerVsCellCalosEME1",
                    "Cell Eta vs Tower Eta EME1",
                    100, -5., 5.,
                    100, -5., 5.,
                    m_etaTowerVsCellCalos[CaloSampling::EME1]) );
  ATH_CHECK( book2 ("etaTowerVsCellCalosHEC0",
                    "Cell Eta vs Tower Eta HEC0",
                    100, -5., 5.,
                    100, -5., 5.,
                    m_etaTowerVsCellCalos[CaloSampling::HEC0]) );
  ATH_CHECK( book2 ("etaTowerVsCellCalosTileBar0",
                    "Cell Eta vs Tower Eta TileBar0",
                    100, -5., 5.,
                    100, -5., 5.,
                    m_etaTowerVsCellCalos[CaloSampling::TileBar0]) );
  ATH_CHECK( book2 ("etaTowerVsCellCalosTileExt0",
                    "Cell Eta vs Tower Eta TileExt0",
                    100, -5., 5.,
                    100, -5., 5.,
                    m_etaTowerVsCellCalos[CaloSampling::TileExt0]) );
  ATH_CHECK( book2 ("etaTowerVsCellCalosFCAL0",
                    "Cell Eta vs Tower Eta FCAL0",
                    100, -5., 5.,
                    100, -5., 5.,
                    m_etaTowerVsCellCalos[CaloSampling::FCAL0]) );
  ATH_CHECK( book2 ("etaTowerVsCellCalosFCAL1",
                    "Cell Eta vs Tower Eta FCAL1",
                    100, -5., 5.,
                    100, -5., 5.,
                    m_etaTowerVsCellCalos[CaloSampling::FCAL1]) );
  ATH_CHECK( book2 ("etaTowerVsCellCalosFCAL2",
                    "Cell Eta vs Tower Eta FCAL2",
                    100, -5., 5.,
                    100, -5., 5.,
                    m_etaTowerVsCellCalos[CaloSampling::FCAL2]) );

  // phi direction matches
  ATH_CHECK( book2 ("phiTowerVsCell", "Cell Phi vs TowerPhi",
                    100, -200., 200.,
                    100, -200., 200.,
                    m_phiTowerVsCell) );
  ATH_CHECK( book2 ("phiTowerVsCellCalosPreSamplerB",
                    "Cell Phi vs Tower Phi PreSampB",
                    100, -200., 200.,
                    100, -200., 200.,
                    m_phiTowerVsCellCalos[CaloSampling::PreSamplerB]) );
  ATH_CHECK( book2 ("phiTowerVsCellCalosEMB1",
                    "Cell Phi vs Tower Phi EMB1",
                    100, -200., 200.,
                    100, -200., 200.,
                    m_phiTowerVsCellCalos[CaloSampling::EMB1]) );
  ATH_CHECK( book2 ("phiTowerVsCellCalosPreSamplerE",
                    "Cell Phi vs Tower Phi PreSampE",
                    100, -200., 200.,
                    100, -200., 200.,
                    m_phiTowerVsCellCalos[CaloSampling::PreSamplerE]) );
  ATH_CHECK( book2 ("phiTowerVsCellCalosEME1",
                    "Cell Phi vs Tower Phi EME1",
                    100, -200., 200.,
                    100, -200., 200.,
                    m_phiTowerVsCellCalos[CaloSampling::EME1]) );
  ATH_CHECK( book2 ("phiTowerVsCellCalosHEC0",
                    "Cell Phi vs Tower Phi HEC0",
                    100, -200., 200.,
                    100, -200., 200.,
                    m_phiTowerVsCellCalos[CaloSampling::HEC0]) );
  ATH_CHECK( book2 ("phiTowerVsCellCalosTileBar0",
                    "Cell Phi vs Tower Phi TileBar0",
                    100, -200., 200.,
                    100, -200., 200.,
                    m_phiTowerVsCellCalos[CaloSampling::TileBar0]) );
  ATH_CHECK( book2 ("phiTowerVsCellCalosTileExt0",
                    "Cell Phi vs Tower Phi TileExt0",
                    100, -200., 200.,
                    100, -200., 200.,
                    m_phiTowerVsCellCalos[CaloSampling::TileExt0]) );
  ATH_CHECK( book2 ("phiTowerVsCellCalosFCAL0",
                    "Cell Phi vs Tower Phi FCAL0",
                    100, -200., 200.,
                    100, -200., 200.,
                    m_phiTowerVsCellCalos[CaloSampling::FCAL0]) );
  ATH_CHECK( book2 ("phiTowerVsCellCalosFCAL1",
                    "Cell Phi vs Tower Phi FCAL1",
                    100, -200., 200.,
                    100, -200., 200.,
                    m_phiTowerVsCellCalos[CaloSampling::FCAL1]) );
  ATH_CHECK( book2 ("phiTowerVsCellCalosFCAL2",
                    "Cell Phi vs Tower Phi FCAL2",
                    100, -200., 200.,
                    100, -200., 200.,
                    m_phiTowerVsCellCalos[CaloSampling::FCAL2]) );

  ATH_CHECK( m_collectionNames.initialize() );

  return StatusCode::SUCCESS;
}


StatusCode CaloTowerMonitor::execute()
{
  const EventContext& ctx = getContext();
  // constant
  //  double mathPi = 2. * asin(1.);
  // retrieve data container
  for (const SG::ReadHandleKey<CaloTowerContainer>& k : m_collectionNames) {
    // pointer to tower container
    SG::ReadHandle<CaloTowerContainer> theTowers (k, ctx);
    for (const CaloTower* aTower : *theTowers) {
      // check tower properties
      //	      if ( aTower->eta() == 0 || aTower->getNumberOfCells() == 0 )
      //		{
      //		  log << MSG::WARNING
      //		      << "CaloTower @ "
      //		      << aTower
      //		      << " with (eta/phi) = ("
      //		      << aTower->eta()
      //		      << "/"
      //		      << aTower->phi()
      //		      << ") has e = "
      //		      << aTower->e() / GeV
      //		      << " GeV and #cells = "
      //		      << aTower->getNumberOfCells()
      //		      << endmsg;
      //		}
      //	      else
      if ( aTower->getNumberOfCells() != 0 )
      {
        // variables
        double e     = aTower->e()  * (1./GeV);
        double et    = aTower->et() * (1./GeV);
        double eta   = aTower->eta();
        double phi   = aTower->phi();
        //		  if ( phi > mathPi ) phi -= 2. * mathPi;
        double cells = (double) aTower->getNumberOfCells();
        ATH_MSG_DEBUG( "Tower @"
                       << aTower
                       << " E = "
                       << e << " GeV (eta,phi) = ("
                       << eta << "," << phi << ")"  );
        // fill distributions
        m_nTowersVsEta->Fill(eta,1.);
        m_nTowersVsPhi->Fill(phi*(1/deg),1.);
        // ATH_MSG_INFO( "fill tower e   " << e  );
        m_eTowers->Fill(e,1.);
        m_eTowersVsEta->Fill(eta,e,1.);
        m_eTowersVsPhi->Fill(phi*(1./deg),e,1.);
        if ( e > 0. )
        {
          m_eLogTowers->Fill(log10(e),1.);
        }
        // ATH_MSG_INFO( "fill tower et  " << et  );
        m_etTowers->Fill(et,1.);
        m_etTowersVsEta->Fill(eta,et,1.);
        m_etTowersVsPhi->Fill(phi*(1./deg),et,1.);
        if ( et > 0. )
        {
          m_etLogTowers->Fill(log10(et),1.);
        }
        // tower shape
        // ATH_MSG_INFO( "fill tower cls " << cells  );
        m_nCellsInTower->Fill(cells,1.);
        m_nCellsInTowerVsEta->Fill(eta,cells,1.);
        m_nCellsInTowerVsPhi->Fill(phi*(1./deg),cells,1.);
        for (const CaloCell* aCell : *aTower)
        {
          // calculate distance
          double cellEta  = aCell->eta();
          double cellPhi  = aCell->phi();
          //  if ( cellPhi > mathPi ) cellPhi -= 2. * mathPi;
          double deltaEta = eta - aCell->eta();
          double deltaPhi = CaloPhiRange::diff(phi, aCell->phi());
          // log << MSG::INFO << "fill cell deta,dphi " 
          //  << deltaEta << "," << deltaPhi << endmsg;
          m_cellsInEtaVsPhi->Fill(deltaEta,deltaPhi,1.);
          // direction matches
          //log << MSG::INFO << "fill cell eta " 
          //	  << cellEta << endmsg;
          m_etaTowerVsCell->Fill(cellEta,eta,1.);
          //log << MSG::INFO << "fill tower phi " 
          //		  << cellPhi/deg << endmsg;
          m_phiTowerVsCell->Fill(cellPhi*(1./deg),phi*(1./deg),1.);
          CaloSampling::CaloSample theSample = aCell->caloDDE()->getSampling();
          CaloSampling::CaloSample takeSample = theSample;
          switch ( theSample )
          {
          case CaloSampling::EMB2:
          case CaloSampling::EMB3:
            takeSample = CaloSampling::EMB1;
            break;
          case CaloSampling::EME2:
          case CaloSampling::EME3:
            takeSample = CaloSampling::EME1;
            break;
          case CaloSampling::HEC1:
          case CaloSampling::HEC2:
          case CaloSampling::HEC3:
            takeSample = CaloSampling::HEC0;
            break;
          case CaloSampling::TileBar1:
          case CaloSampling::TileBar2:
          case CaloSampling::TileGap1:
          case CaloSampling::TileGap2:
          case CaloSampling::TileGap3:
            takeSample = CaloSampling::TileBar0;
            break;
          case CaloSampling::TileExt1:
          case CaloSampling::TileExt2:
            takeSample = CaloSampling::TileExt0;
            break;
          default:
            break;
          }
          m_etaTowerVsCellCalos[takeSample]->Fill(cellEta,eta,1.);
          m_phiTowerVsCellCalos[takeSample]->Fill(cellPhi*(1./deg),
                                                  phi*(1./deg),1.);
        } // cell loop
      } // tower kinematics ok
    } // tower loop
  } // collection loop
  return StatusCode::SUCCESS;
}		    

