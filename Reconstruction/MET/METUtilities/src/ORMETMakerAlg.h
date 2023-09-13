///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// ORMETMakerAlg.h

#ifndef ORMETMakerAlg_H
#define ORMETMakerAlg_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include "METMakerAlg.h"
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/DataHandle.h"

#include "xAODEgamma/Electron.h"
#include "xAODEgamma/Photon.h"
#include "xAODMuon/Muon.h"
#include "xAODTau/TauJet.h"

#include "xAODJet/JetContainer.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODEgamma/PhotonContainer.h"
#include "xAODTau/TauJetContainer.h"

#include "xAODMissingET/MissingETContainer.h"
#include "xAODMissingET/MissingETAssociationMap.h"


class IMETMaker;
class IAsgElectronLikelihoodTool;
class IAsgPhotonIsEMSelector;
namespace CP {
  class IMuonSelectionTool;
}
namespace TauAnalysisTools {
  class ITauSelectionTool;
}

namespace met {
  class ORMETMakerAlg : public METMakerAlg {

  public: 

    /// Constructor with parameters:
    ORMETMakerAlg(const std::string& name, ISvcLocator* pSvcLocator);

    /// Destructor:
    virtual ~ORMETMakerAlg(); 

    /// Athena algorithm's Hooks
    virtual StatusCode  initialize() override;
    virtual StatusCode  execute() override;
    virtual StatusCode  finalize() override;

  private: 

    /// Default constructor:
    ORMETMakerAlg();

    virtual bool accept(const xAOD::Electron* el) final;
    virtual bool accept(const xAOD::Photon* ph) final;
    virtual bool accept(const xAOD::TauJet* tau) final;
    virtual bool accept(const xAOD::Muon* muon) final;

    
    std::string m_soft;

    SG::ReadHandleKey<xAOD::MissingETAssociationMap> m_ORMetMapKey;

    SG::WriteHandleKey<xAOD::PFOContainer> m_chargedPFOContainerWriteHandleKey{this,"PFOChargedOutputName","OverlapRemovedCHSChargedParticleFlowObjects","WriteHandleKey for charged PFO"}; //jetOR
    SG::WriteHandleKey<xAOD::PFOContainer> m_neutralPFOContainerWriteHandleKey{this,"PFONeutralOutputName","OverlapRemovedCHSNeutralParticleFlowObjects","WriteHandleKey for charged PFO"}; //jetOR
    SG::WriteHandleKey<xAOD::PFOContainer> m_PFOContainerWriteHandleKey{this,"PFOOutputName","OverlapRemovedCHSParticleFlowObjects","WriteHandleKey for PFO"}; //jetOR


SG::ReadHandleKey<xAOD::PFOContainer> m_inPFOKey{this, "InPFOKey", "", "ReadHandleKey for modified  PFlow Objects"};
SG::WriteHandleKey<xAOD::PFOContainer> m_outPFOKey{this, "OutPFOKey", "", "WriteHandleKey for modified PFlow Objects"};

    bool m_doRetrieveORconstit;
    bool m_retainMuonConstit;
    bool m_doORMet;


    double m_electronPT;
    double m_muonPT;
    double m_photonPT;
    double m_tauPT;

    double m_electronETA;
    double m_muonETA;
    double m_photonETA;
    double m_tauETA;

  
    bool m_useUnmatched;
    bool m_doJVT;
   
  }; 

}

#endif

