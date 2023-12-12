/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBAL_GLOBALL1TOPOSIMULATION_H
#define GLOBAL_GLOBALL1TOPOSIMULATION_H

/*
 *  GlobalL1TopoSimulation runs L1Topo Algorithms.
 * It is heavily based on the L1TopoSimmulation package.
 *
 * GlobalL1TopoSimulation builds a call digraph of L1TopoSimulation
 * The Algorithms are placed in "topological order" (graph algorithm term,
 * NOT L1Topo related term). Algorithms read from and write to a DataRepository.
 * The DataRepository contents are accessed in a type-safe and type-complete
 * (ie all types held in DataRpository are examined). Type-completeness
 * is enforced by double dispatch: any changes in the types held by
 * the DataRepository will require updates to the classes of objects
 * visiting the repository.
 *
 */
 
#include "L1TopoSimulation/IInputTOBConverter.h"

#include "TrigConfData/L1TopoAlgorithm.h"  // config algs

#include "L1TopoEvent/TopoInputEvent.h"
#include "L1TopoInterfaces/IL1TopoHistSvc.h"

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "Digraph.h"
#include  "IGlobalAlg.h"
#include  "AlgData.h"
#include "GlobalSystem.h"

#include <memory>

#include "CxxUtils/checker_macros.h"

namespace TrigConf{
  class L1TopoAlgorithm;
  class L1Menu;
}

namespace TCS {
  class  SortingConnector;
  class Connector;
}


namespace GlobalSim {
  class GSAlgFactory;

  using L1AlgVec =  std::vector<const TrigConf::L1TopoAlgorithm*>;
  using ADP = std::shared_ptr<AlgData>;
  
  class GlobalL1TopoSimulation : public AthReentrantAlgorithm {
  public:
     
    GlobalL1TopoSimulation(const std::string& name, ISvcLocator *pSvcLocator);
    
    virtual StatusCode initialize  ATLAS_NOT_THREAD_SAFE () override;
    virtual StatusCode execute (const EventContext& ctx) const override;
    virtual StatusCode finalize() override;

    enum class ConnectorType{INPUT, COUNTING, SORTING, DECISION};

  private:
    std::vector<std::shared_ptr<IGlobalAlg>> m_gsAlgs;

    std::shared_ptr<IL1TopoHistSvc> m_topoHistSvc;
    ToolHandle<LVL1::IInputTOBConverter> m_jetInputProvider {
      this,
      "JetInputProvider",
      "LVL1::JetInputProvider/JetInputProvider",
      "Tool to fill the Jet TOBs of the topo input event"};

    ToolHandle<LVL1::IInputTOBConverter> m_emtauInputProvider {
      this,
	"EMTAUInputProvider",
	"LVL1::EMTauInputProvider/EMTauInputProvider",
	"Tool to fill the EMTAU TOBs of the topo input event"};
    
    ToolHandle<LVL1::IInputTOBConverter> m_energyInputProvider {
      this,
      "EnergyInputProvider",
      "LVL1::EnergyInputProvider/EnergyInputProvider",
      "Tool to fill the energy and MET TOBs of the topo input event"};

    Gaudi::Property<bool>
    m_doEConns {this, "doEconns", true,
		"work with eletrical (decision) Algorithms"};

    Gaudi::Property<int>
    m_nGEPBoards {this, "nGEPBoards", 1,
		  "number of GEP Boards in the Global system"};

    
 
    
    Gaudi::Property<bool>
    m_doOConns {this, "doOconns", true,
		"work with optical (counting) Algorithms"};

    /*
     * 26/09/23 Muon code gives run time error (with
     * asetup Athena, main, lates) 

    ToolHandle<LVL1::IInputTOBConverter> m_muonInputProvider {
      this,
      "MuonInputProvider",
      "LVL1::MuonInputProvider/MuonInputProvider",
      "Tool to fill the muon TOBs of the topo input event"};
    */


    StatusCode addGSAlgorithms(const std::vector<ADP>&);

  };


}
#endif
