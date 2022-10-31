/*
 *   Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
 *   */

#ifndef L1TopoSimulation_ControlHistSvc
#define L1TopoSimulation_ControlHistSvc


#include "L1TopoSimulation/IControlHistSvc.h"
#include "L1TopoCoreSim/TopoSteering.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/IIncidentListener.h"
#include "GaudiKernel/ITHistSvc.h"


class ITHistSvc;

namespace LVL1 {


  class ControlHistSvc : public AthAlgTool, virtual public IControlHistSvc{


  public:

    ControlHistSvc(const std::string& type, const std::string& name, 
                         const IInterface* parent);
    virtual ~ControlHistSvc();
    virtual StatusCode initialize() override;
    virtual StatusCode SetHistSvc(const std::unique_ptr<TCS::TopoSteering> &topoSteering, std::string histBaseDir) override;

  private:

     ServiceHandle<ITHistSvc> m_histSvc;
     


  };

}

#endif
