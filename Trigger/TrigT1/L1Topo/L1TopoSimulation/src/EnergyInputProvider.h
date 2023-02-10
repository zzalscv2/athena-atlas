/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#ifndef L1TopoSimulation_EnergyInputProvider
#define L1TopoSimulation_EnergyInputProvider

#include "AthenaBaseComps/AthAlgTool.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "L1TopoSimulation/IInputTOBConverter.h"
#include "GaudiKernel/LockedHandle.h"
#include "TrigT1CaloEvent/EnergyTopoData.h"

namespace LVL1 {

   class EnergyInputProvider : public extends<AthAlgTool, IInputTOBConverter> {
   public:
      EnergyInputProvider(const std::string& type, const std::string& name, 
                          const IInterface* parent);
      
      virtual ~EnergyInputProvider();

      virtual StatusCode initialize();

      virtual StatusCode fillTopoInputEvent(TCS::TopoInputEvent& ) const; 

   private:

      ToolHandle<GenericMonitoringTool> m_monTool {this, "MonTool", "", "Monitoring tool to create online histograms"};

      SG::ReadHandleKey< LVL1::EnergyTopoData > m_energyLocation;    //!<  EnergyROI SG key

   };
}

#endif
