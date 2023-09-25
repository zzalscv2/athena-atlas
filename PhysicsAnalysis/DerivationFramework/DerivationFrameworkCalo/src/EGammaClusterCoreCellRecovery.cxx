/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////
// EGammaClusterCoreCellRecovery.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
// Author: G. Unal (Guillaume.Unal@cern.ch)
// Decorate egamma objects with the energies in L2 and L3 that are in cells
// not included in the original supercluser due to the timing cut in topocluster
// building. This is an AOD fix for data (and mc, but effect is small for mc)
// produced with rel23 (and 24 ?)

#include "DerivationFrameworkCalo/EGammaClusterCoreCellRecovery.h"
#include "CaloEvent/CaloCell.h"

#include <TString.h>
#include <string>
namespace {}

// Constructor
DerivationFramework::EGammaClusterCoreCellRecovery::EGammaClusterCoreCellRecovery(const std::string& t,
										  const std::string& n,
										  const IInterface* p)
  : AthAlgTool(t, n, p)
{
  declareInterface<DerivationFramework::IAugmentationTool>(this);
}


// Athena initialize and finalize
StatusCode
DerivationFramework::EGammaClusterCoreCellRecovery::initialize()
{
  ATH_MSG_VERBOSE("initialize() ...");

  ATH_CHECK(m_SGKey_caloCells.initialize());
  
  if (m_SGKey_photons.key().empty() && m_SGKey_electrons.key().empty()) {
    ATH_MSG_FATAL("No e-gamma collection provided for thinning. At least one "
                  "egamma collection (photons/electrons) must be provided!");
    return StatusCode::FAILURE;
  }

  if (!m_SGKey_electrons.key().empty()) {
    ATH_MSG_DEBUG("Using " << m_SGKey_electrons << " for electrons");
    ATH_CHECK(m_SGKey_electrons.initialize());

    const std::string containerKey = m_SGKey_electrons.key();
    for (int i = 2; i <= 3; i++) {
      for (int t = 0; t <=1 ; t++) {
	m_SGKey_electrons_decorations.emplace_back(
	  Form("%s.%sadded_Lr%d", containerKey.c_str(), (t == 0 ? "n" : "E"), i));
      }
    }
    ATH_CHECK(m_SGKey_electrons_decorations.initialize());
    if (msgLvl(MSG::DEBUG)) {
      ATH_MSG_DEBUG("Decorations for " << containerKey);
      for (const auto& s : m_SGKey_electrons_decorations)
	{ ATH_MSG_DEBUG(s.key()); }
    }
  }

  if (!m_SGKey_photons.key().empty()) {
    ATH_MSG_DEBUG("Using " << m_SGKey_photons << " for photons");
    ATH_CHECK(m_SGKey_photons.initialize());

    const std::string containerKey = m_SGKey_photons.key();
    for (int i = 2; i <= 3; i++) {
      for (int t = 0; t <= 1 ; t++) {
	m_SGKey_photons_decorations.emplace_back(
	  Form("%s.%sadded_Lr%d", containerKey.c_str(), (t == 0 ? "n" : "E"), i));
      }
    }    
    ATH_CHECK(m_SGKey_photons_decorations.initialize());
    if (msgLvl(MSG::DEBUG)) {
      ATH_MSG_DEBUG("Decorations for " << containerKey);
      for (const auto& s : m_SGKey_photons_decorations)
	{ ATH_MSG_DEBUG(s.key()); }
    }
  }

  return StatusCode::SUCCESS;
}


// The decoration itself
StatusCode
DerivationFramework::EGammaClusterCoreCellRecovery::addBranches() const
{
  const EventContext& ctx = Gaudi::Hive::currentContext();
  
  SG::ReadHandle<CaloCellContainer> caloCellContainer(m_SGKey_caloCells,ctx);
  
  std::vector<SG::WriteDecorHandle<xAOD::EgammaContainer, char>> decon;
  std::vector<SG::WriteDecorHandle<xAOD::EgammaContainer, float>> decoE;
  decon.reserve(2);
  decoE.reserve(2);
  
  // Photon decorations
  if (!m_SGKey_photons.key().empty()) {
    
    for (int i = 0; i < 2; i++) {
      decon.emplace_back(
			     m_SGKey_photons_decorations[i * 2], ctx);
      decoE.emplace_back(
			     m_SGKey_photons_decorations[i * 2 + 1], ctx);
    }
    
    // Retrieve photon container
    SG::ReadHandle<xAOD::EgammaContainer> photonContainer(m_SGKey_photons, ctx);

    // Decorate photons
    for (const auto* photon : *photonContainer.ptr()) {
      DerivationFramework::EGammaClusterCoreCellRecovery::missCoreInfo res =
        decorateObject(caloCellContainer.ptr(), photon);
      for (int i = 0; i < 2; i++) {
	decon[i](*photon) = res.nCells[i];
	decoE[i](*photon) = res.eCells[i];
      }
    }
  }

  // Electron decorations
  if (!m_SGKey_electrons.key().empty()) {

    for (int i = 0; i < 2; i++) {
      decon[i] = SG::WriteDecorHandle<xAOD::EgammaContainer, char>(
            m_SGKey_electrons_decorations[i * 2], ctx);
      decoE[i] = SG::WriteDecorHandle<xAOD::EgammaContainer, float>(
            m_SGKey_electrons_decorations[i * 2 + 1], ctx);
    }

    // Retrieve electron container
    SG::ReadHandle<xAOD::EgammaContainer> electronContainer(m_SGKey_electrons,
                                                            ctx);
    
    // Decorate electrons
    for (const auto* electron : *electronContainer.ptr()) {
      DerivationFramework::EGammaClusterCoreCellRecovery::missCoreInfo res =
        decorateObject(caloCellContainer.ptr(), electron);
      for (int i = 0; i < 2; i++) {
	decon[i](*electron) = res.nCells[i];
	decoE[i](*electron) = res.eCells[i];
      }
    }
  }

  return StatusCode::SUCCESS;
}

DerivationFramework::EGammaClusterCoreCellRecovery::missCoreInfo
DerivationFramework::EGammaClusterCoreCellRecovery::decorateObject(
  const CaloCellContainer* caloCells, const xAOD::Egamma*& egamma) const
{
  DerivationFramework::EGammaClusterCoreCellRecovery::missCoreInfo result{};

  const xAOD::CaloCluster *clus = egamma->caloCluster();
  if (!clus) {
    ATH_MSG_WARNING("No associated egamma cluster. Do nothing");
    return result;
  }
 
  // Find max energy cell in layer 2
  double etamax = -999., phimax = -999.;
  if (findMaxECell(clus,etamax,phimax).isFailure()) {
    ATH_MSG_WARNING("Problem in finding maximum energy cell in layer 2");
    return result;
  }

  // Build rectangular cluster arount it (and also in L3)
  DerivationFramework::EGammaClusterCoreCellRecovery::existingCells arrayCells =
    buildCellArrays(clus, etamax, phimax);

  // Now compare with the cells stored in the CaloCell container
  // and compute the energy from missed cells

  // define required size = 3x7 in barrel and 5x7 in endcap for layer 2
  int iphi1 = 0, iphi2 = 6;
  int ieta1 = 0, ieta2 = 4;
  if (std::abs(etamax) < 1.475) { 
    ieta1 = 1;
    ieta2 = 3;
  }
  
  for (const auto * cell : *caloCells) {
    //const CaloCell* cell = (*first_cell);
    if (!cell->caloDDE()) {
      ATH_MSG_WARNING("Calo cell without detector element ?? eta = "
		      << cell->eta() << " phi = " << cell->phi());
      continue;
    }

    double deta = cell->caloDDE()->eta_raw()-etamax;
    double dphi = cell->caloDDE()->phi_raw()-phimax;
    if (dphi < -M_PI) dphi += 2*M_PI;
    if (dphi > M_PI)  dphi -= 2*M_PI;
    
    if (std::abs(deta) > 0.1 || std::abs(dphi) > 0.1) continue;

    int layer = cell->caloDDE()->getSampling();
    if (layer != CaloSampling::EMB2 && layer != CaloSampling::EME2 &&
	layer != CaloSampling::EMB3 && layer != CaloSampling::EME3)
      continue;
    
    if ((layer == CaloSampling::EMB2 || layer == CaloSampling::EME2) &&
	std::abs(deta) < 0.07 && std::abs(dphi) < 0.085) {
      int ieta = (int)((deta+0.052)/0.025);
      int iphi = (int)((dphi+0.075)/m_phiSize);
      if (ieta >= ieta1 && ieta <= ieta2 && iphi >= iphi1 && iphi <= iphi2) {
	int index = 7*ieta+iphi;
	if (!arrayCells.existL2[index]) {
	  if (std::abs(cell->time())>12.) {      // TBC: is this safe enough given the rounding of time ?
	    result.nCells[0]++;
	    result.eCells[0] += cell->energy();
	  } // would have been rejected by time cut
	}   // cell not in cluster
      }     // eta-phi window cut
    } else if ((layer == CaloSampling::EMB3 || layer == CaloSampling::EME3) &&
	       std::abs(deta) < 0.05 && std::abs(dphi) < 0.06) {
      int ieta = (int)(std::abs(deta)/0.025);
      int iphi = (int)((dphi+0.05)/m_phiSize);
      if (ieta <= 1 && iphi >= 0 && iphi <= 4) {
	int index = 5*ieta+iphi;
	if (!arrayCells.existL3[index]) {
	  if (std::abs(cell->time())>12.) {
	    result.nCells[1]++;
	    result.eCells[1] += cell->energy();
	  }
	}
      }
    } 
  }  // loop over cell container
  
  ATH_MSG_DEBUG("Added cells in L2, n = " << result.nCells[0] << " E = " << result.eCells[0]
		<< " and in L3, n = " << result.nCells[1] << " E = " << result.eCells[1]);
  return result;
}

DerivationFramework::EGammaClusterCoreCellRecovery::existingCells
DerivationFramework::EGammaClusterCoreCellRecovery::buildCellArrays(
  const xAOD::CaloCluster *clus, double etamax, double phimax) const {

  DerivationFramework::EGammaClusterCoreCellRecovery::existingCells result{};
  // Just to be sure. But should not be needed
  for (int i = 0; i < m_nL2; i++)
    { result.existL2[i] = 0; }
  for (int i = 0; i < m_nL3; i++)
    { result.existL3[i] = 0; }
  
  const CaloClusterCellLink* cellLinks = clus->getCellLinks();
  if (!cellLinks) {
    ATH_MSG_WARNING("No cell link for cluster. Do nothing");
    return result;
  }

  CaloClusterCellLink::const_iterator it_cell = cellLinks->begin(),
    it_cell_e = cellLinks->end();

  for (; it_cell != it_cell_e; ++it_cell) {
    const CaloCell* cell = (*it_cell);
    if (cell) {
      if (!cell->caloDDE()) {
	ATH_MSG_WARNING("Calo cell without detector element ?? eta = "
			<< cell->eta() << " phi = " << cell->phi());
	continue;
      }
      int layer = cell->caloDDE()->getSampling();
      if (layer != CaloSampling::EMB2 && layer != CaloSampling::EME2 &&
	  layer != CaloSampling::EMB3 && layer != CaloSampling::EME3)
	continue;
      
      double deta = cell->caloDDE()->eta_raw() - etamax;
      double dphi = cell->caloDDE()->phi_raw() - phimax;
      if (dphi < -M_PI) dphi += 2*M_PI;
      if (dphi > M_PI)  dphi -= 2*M_PI;
      if ((layer == CaloSampling::EMB2 || layer == CaloSampling::EME2) && 
	  std::abs(deta) < 0.07 && std::abs(dphi) < 0.085) {
	int ieta = (int)((deta+0.052)/0.025);
	int iphi = (int)((dphi+0.075)/m_phiSize);
	if (ieta < 0 || ieta > 4 || iphi < 0 || iphi > 6) {
	  ATH_MSG_WARNING("Should never happen ieta = " << ieta
			  << " iphi = " << iphi);
	} else {
	  int index = 7*ieta+iphi;
	  result.existL2[index] = true;
	}
      } else if ((layer == CaloSampling::EMB3 || layer == CaloSampling::EME3) &&
		 std::abs(deta) < 0.05 && std::abs(dphi) < 0.06) {
	int ieta = (int)(std::abs(deta)/0.025);
	int iphi = (int)((dphi+0.05)/m_phiSize);
	if (ieta > 1 || iphi < 0 || iphi > 4) {
	  ATH_MSG_WARNING("Should never happen ieta = " << ieta
			  << " iphi = " << iphi);
	} else {
	  int index = 5*ieta+iphi;
	  result.existL3[index] = true; 
	}
      } // L2 or L3, in window vs max
    }   // cell exists
  }     // loop over cells

  return result;
}


StatusCode
DerivationFramework::EGammaClusterCoreCellRecovery::findMaxECell(
  const xAOD::CaloCluster *clus, double &etamax, double &phimax) const
{
  const CaloClusterCellLink* cellLinks = clus->getCellLinks();
  if (!cellLinks) {
    ATH_MSG_WARNING("No cell link for cluster. Do nothing");
    return StatusCode::SUCCESS;
  }

  CaloClusterCellLink::const_iterator it_cell = cellLinks->begin(),
    it_cell_e = cellLinks->end();

  // find maximum cell energy in layer 2
  double emax   = 0.;
  for(; it_cell != it_cell_e; ++it_cell) {
    const CaloCell* cell = (*it_cell);
    if (cell) {
      if (!cell->caloDDE()) {
	ATH_MSG_WARNING("Calo cell without detector element ?? eta = "
			<< cell->eta() << " phi = " << cell->phi());
	continue;
      }
      int layer = cell->caloDDE()->getSampling();
      if (layer == CaloSampling::EMB2 || layer == CaloSampling::EME2) {
	if (cell->energy() > emax) {
	  emax   = cell->energy();
	  etamax = cell->caloDDE()->eta_raw();
	  phimax = cell->caloDDE()->phi_raw();
	}
      }
    }
  }
  if (msgLvl(MSG::DEBUG)) {
    if (emax > 0) {
      ATH_MSG_DEBUG("Maximum layer 2 energy cell, E = " << emax
		    << " eta = " << etamax << " phi = " << phimax);
    } else {
      ATH_MSG_DEBUG("No layer 2 cell ! Should never happen");
    }
  }
  return StatusCode::SUCCESS;
}
