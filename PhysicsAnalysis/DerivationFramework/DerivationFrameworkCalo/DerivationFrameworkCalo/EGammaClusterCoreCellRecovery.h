/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// EGammaClusterCoreCellRecovery.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
// Decorate egamma objects with the energies in L2 and L3 that are in cells
// not included in the original supercluser due to the timing cut in topocluster
// building. This is an AOD fix for data (and mc, but effect is small for mc)
// produced with rel23 (and 24 ?)

#ifndef DERIVATIONFRAMEWORK_EGammaClusterCoreCellRecovery_H
#define DERIVATIONFRAMEWORK_EGammaClusterCoreCellRecovery_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKeyArray.h"
#include "CaloEvent/CaloCellContainer.h"
#include "xAODEgamma/EgammaContainer.h"
#include "xAODCaloEvent/CaloCluster.h"

namespace DerivationFramework {

  class EGammaClusterCoreCellRecovery
    : public AthAlgTool
    , public IAugmentationTool
  {
  public:
    EGammaClusterCoreCellRecovery(const std::string& t,
				  const std::string& n,
				  const IInterface* p);
    ~EGammaClusterCoreCellRecovery() = default;
    StatusCode initialize();
    StatusCode finalize() { return StatusCode::SUCCESS; }
    virtual StatusCode addBranches() const;
    
  private:
    SG::ReadHandleKey<xAOD::EgammaContainer> m_SGKey_photons
      { this, "SGKey_photons", "Photons", "SG key of photon container" };
    
    SG::ReadHandleKey<xAOD::EgammaContainer> m_SGKey_electrons
      { this, "SGKey_electrons", "Electrons", "SG key of electron container" };

    SG::ReadHandleKey<CaloCellContainer> m_SGKey_caloCells
      { this, "SGKey_caloCells", "AllCalo", "SG key of calo cell container" };
    
    SG::WriteDecorHandleKeyArray<xAOD::EgammaContainer>
      m_SGKey_photons_decorations{
      this,
	"SGKey_photons_decorations_noConf",
	  {},
	"SG keys for photon decorations not really configurable"
	  };
    
    SG::WriteDecorHandleKeyArray<xAOD::EgammaContainer>
      m_SGKey_electrons_decorations{
      this,
	"SGKey_electrons_decorations_noConf",
	  {},
	"SG keys for electrons decorations not really configurable"
	  };

    // phi size of layer 2 and 3 cells
    static constexpr double m_phiSize = 2*M_PI/256;

    // Sizes of the window to search for missing cells
    static const int m_nL2 = 5*7;
    static const int m_nL3 = 10;

    struct existingCells {
      bool existL2[m_nL2];
      bool existL3[m_nL3];
    };
    struct missCoreInfo {
      unsigned short nCells[2];
      float eCells[2]; 
    };
    missCoreInfo decorateObject(const CaloCellContainer *caloCells,
				const xAOD::Egamma*& egamma) const;
    StatusCode findMaxECell(const xAOD::CaloCluster *clus,
			    double &etamax, double &phimax) const;
    existingCells buildCellArrays(const xAOD::CaloCluster *clus,
				  double etamax, double phimax) const;
  };
  
}

#endif // DERIVATIONFRAMEWORK_EGammaClusterCoreCellRecovery_H
