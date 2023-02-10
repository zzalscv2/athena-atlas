/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#ifndef L1TopoSimulation_EMTauInputProvider
#define L1TopoSimulation_EMTauInputProvider

#include "AthenaBaseComps/AthAlgTool.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "L1TopoSimulation/IInputTOBConverter.h"
#include "GaudiKernel/LockedHandle.h"
#include "TrigT1CaloEvent/CPCMXTopoData.h"

namespace LVL1 {

   class EMTauInputProvider : public extends<AthAlgTool, IInputTOBConverter> {
   public:
      EMTauInputProvider(const std::string& type, const std::string& name, 
                         const IInterface* parent);
      
      virtual ~EMTauInputProvider();

      virtual StatusCode initialize();

      virtual StatusCode fillTopoInputEvent(TCS::TopoInputEvent& ) const; 

   private:

      /** \brief calculates eta and phi from roiWord*/
      void CalculateCoordinates(int32_t roiWord, double & eta, double & phi) const;

      ToolHandle<GenericMonitoringTool> m_monTool {this, "MonTool", "", "Monitoring tool to create online histograms"};

      SG::ReadHandleKey<DataVector<LVL1::CPCMXTopoData>> m_emTauLocation;    //!<  EMTAU ROI SG key

   };
}

#endif
