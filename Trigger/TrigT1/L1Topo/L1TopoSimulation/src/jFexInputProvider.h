/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/


#ifndef L1TOPOSIMULATION_JFEXINPUTPROVIDER_H
#define L1TOPOSIMULATION_JFEXINPUTPROVIDER_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "L1TopoSimulation/IInputTOBConverter.h"
#include "GaudiKernel/LockedHandle.h"

// jFEX EDMs
#include "xAODTrigger/jFexSRJetRoIContainer.h"
#include "xAODTrigger/jFexLRJetRoIContainer.h"
#include "xAODTrigger/jFexTauRoIContainer.h"
#include "xAODTrigger/jFexMETRoIContainer.h"
#include "xAODTrigger/jFexSumETRoIContainer.h"
#include "xAODTrigger/jFexFwdElRoIContainer.h"

namespace LVL1 {

   class jFexInputProvider : public extends<AthAlgTool, IInputTOBConverter> {
   public:
      jFexInputProvider(const std::string& type, const std::string& name, 
                         const IInterface* parent);
      
      virtual ~jFexInputProvider();

      virtual StatusCode initialize() override final;
      virtual StatusCode fillTopoInputEvent(TCS::TopoInputEvent& ) const override final;

   private:
      StatusCode fillSRJet(TCS::TopoInputEvent& inputEvent) const;
      StatusCode fillLRJet(TCS::TopoInputEvent& inputEvent) const;

      StatusCode fillEM(TCS::TopoInputEvent& inputEvent) const;
      StatusCode fillTau(TCS::TopoInputEvent& inputEvent) const;

      StatusCode fillXE(TCS::TopoInputEvent& inputEvent) const;
      StatusCode fillTE(TCS::TopoInputEvent& inputEvent) const;

      ToolHandle<GenericMonitoringTool> m_monTool {this, "MonTool", "", "Monitoring tool to create online histograms"};

      StringProperty m_gFEXJetLoc {""};

      SG::ReadHandleKey<xAOD::jFexSRJetRoIContainer> m_jJet_EDMKey {this, "jFexSRJetRoIKey", "L1_jFexSRJetRoI", "jFEX Jet EDM"};
      SG::ReadHandleKey<xAOD::jFexLRJetRoIContainer> m_jLJet_EDMKey {this, "jFexLRJetRoIKey", "L1_jFexLRJetRoI", "jFEX LJet EDM"};
      SG::ReadHandleKey<xAOD::jFexFwdElRoIContainer> m_jEM_EDMKey {this, "jFexFwdElRoIKey", "L1_jFexFwdElRoI", "jFEX EM EDM"};     
      SG::ReadHandleKey<xAOD::jFexTauRoIContainer> m_jTau_EDMKey {this, "jFexTauRoIKey", "L1_jFexTauRoI", "jFEX Tau EDM"};
      SG::ReadHandleKey<xAOD::jFexMETRoIContainer> m_jXE_EDMKey {this, "jFexMETRoIKey", "L1_jFexMETRoI", "jFEX XE EDM"};
      SG::ReadHandleKey<xAOD::jFexSumETRoIContainer> m_jTE_EDMKey {this, "jFexSumETRoIKey", "L1_jFexSumETRoI", "jFEX TE EDM"};
     
      // jFex to L1Topo conversion factors
      static const int m_Et_conversion;
      static const double m_sumEt_conversion;
      static const int m_phi_conversion;
      static const int m_eta_conversion;

      static const double m_EtDouble_conversion;
      static const double m_sumEtDouble_conversion;
      static const double m_phiDouble_conversion;
      static const double m_etaDouble_conversion;

  };
}

#endif
