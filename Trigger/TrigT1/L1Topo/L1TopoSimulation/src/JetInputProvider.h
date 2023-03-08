/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#ifndef L1TopoSimulation_JetInputProvider
#define L1TopoSimulation_JetInputProvider

#include "AthenaBaseComps/AthAlgTool.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "L1TopoSimulation/IInputTOBConverter.h"
#include "GaudiKernel/LockedHandle.h"
#include "TrigT1CaloEvent/JetCMXTopoDataCollection.h"

namespace LVL1 {

   class JetInputProvider : public extends<AthAlgTool, IInputTOBConverter> {
   public:
      JetInputProvider(const std::string& type, const std::string& name, 
                         const IInterface* parent);
      
      virtual ~JetInputProvider();

      virtual StatusCode initialize();

      virtual StatusCode fillTopoInputEvent(TCS::TopoInputEvent& ) const; 

   private:

      ToolHandle<GenericMonitoringTool> m_monTool {this, "MonTool", "", "Monitoring tool to create online histograms"};

      SG::ReadHandleKey< DataVector<JetCMXTopoData> >  m_jetLocation;    //!<  Jet ROIs SG key

   };
}

#endif
