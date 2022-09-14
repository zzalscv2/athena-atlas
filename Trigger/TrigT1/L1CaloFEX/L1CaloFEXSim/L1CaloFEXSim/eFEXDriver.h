#ifndef EFEXDRIVER_H
#define EFEXDRIVER_H

// STL
#include <string>

// Athena/Gaudi
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AthenaBaseComps/AthAlgorithm.h"
#include "L1CaloFEXSim/eTower.h"
#include "L1CaloFEXSim/eTowerContainer.h"
#include "L1CaloFEXToolInterfaces/IeFEXSysSim.h"
#include "L1CaloFEXSim/eFEXSim.h"
#include "L1CaloFEXSim/eFEXOutputCollection.h"
#include "xAODTrigger/eFexEMRoIContainer.h"
#include "xAODTrigger/eFexTauRoIContainer.h"
#include "L1CaloFEXSim/eFakeTower.h"

class CaloIdManager;

namespace LVL1 {

class eFEXDriver : public AthAlgorithm
{
 public:
  //using AthReentrantAlgorithm::AthReentrantAlgorithm;

  eFEXDriver(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~eFEXDriver();

  virtual StatusCode initialize();
  virtual StatusCode execute(/*const EventContext& ctx*/);// const;
  StatusCode finalize();

 private:

  int m_numberOfEvents = 0;

  SG::WriteHandleKey<eFEXOutputCollection> m_eFEXOutputCollectionSGKey {this, "MyOutputs", "eFEXOutputCollection", "MyOutputs"};

  ToolHandle<IeFEXSysSim> m_eFEXSysSimTool {this, "eFEXSysSimTool", "LVL1::eFEXSysSim", "Tool that creates the eFEX System Simulation"};

};

} // end of LVL1 namespace
#endif
