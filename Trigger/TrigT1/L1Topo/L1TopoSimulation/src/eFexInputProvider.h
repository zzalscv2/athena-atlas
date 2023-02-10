// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#ifndef L1TOPOSIMULATION_EFEXINPUTPROVIDER_H
#define L1TOPOSIMULATION_EFEXINPUTPROVIDER_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "L1TopoSimulation/IInputTOBConverter.h"
#include "GaudiKernel/LockedHandle.h"

//EM/Tau EDMs
#include "xAODTrigger/eFexEMRoIContainer.h"
#include "xAODTrigger/eFexTauRoIContainer.h"

namespace LVL1 {

   class eFexInputProvider : public extends<AthAlgTool, IInputTOBConverter> {
   public:
      eFexInputProvider(const std::string& type, const std::string& name, 
                         const IInterface* parent);
      
      virtual ~eFexInputProvider();

      virtual StatusCode initialize() override final;

      virtual StatusCode fillTopoInputEvent(TCS::TopoInputEvent& ) const override final;

   private:

      /** \brief calculates eta and phi from roiWord*/
      void CalculateCoordinates(int32_t roiWord, double & eta, double & phi) const;
     
      StatusCode fillEM(TCS::TopoInputEvent& inputEvent) const;
      StatusCode fillTau(TCS::TopoInputEvent& inputEvent) const;

      ToolHandle<GenericMonitoringTool> m_monTool {this, "MonTool", "", "Monitoring tool to create online histograms"};

     SG::ReadHandleKey<xAOD::eFexEMRoIContainer> m_eEM_EDMKey {this, "eFexEMRoIKey", "L1_eEMRoI", "eFEXEM EDM"};
     SG::ReadHandleKey<xAOD::eFexTauRoIContainer> m_eTau_EDMKey {this, "eFexTauRoIKey", "L1_eTauRoI", "eFEXTau EDM"};

     // eFex to L1Topo conversion factors
     static const double m_EtDouble_conversion;
     static const double m_phiDouble_conversion;
     static const double m_etaDouble_conversion;

   };
}

#endif
