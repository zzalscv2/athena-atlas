#ifndef eTowerMakerFromSuperCells_H
#define eTowerMakerFromSuperCells_H

// STL
#include <string>

// Athena/Gaudi
#include "AthenaBaseComps/AthAlgorithm.h"
#include "L1CaloFEXSim/eTower.h"
#include "L1CaloFEXSim/eTowerContainer.h"
#include "L1CaloFEXSim/eTowerBuilder.h"
#include "L1CaloFEXSim/eSuperCellTowerMapper.h"
#include "xAODTrigL1Calo/TriggerTowerContainer.h"

namespace LVL1 {

class eTowerMakerFromSuperCells : public AthAlgorithm
{
 public:
  //using AthReentrantAlgorithm::AthReentrantAlgorithm;

  eTowerMakerFromSuperCells(const std::string& name, ISvcLocator* pSvcLocator);
  ~eTowerMakerFromSuperCells() = default;

  virtual StatusCode initialize();
  virtual StatusCode execute(/*const EventContext& ctx*/);// const;

 private:

  SG::WriteHandleKey<LVL1::eTowerContainer> m_eTowerContainerSGKey {this, "MyETowers", "eTowerContainer", "MyETowers"};

  ToolHandle<IeTowerBuilder> m_eTowerBuilderTool {this, "eTowerBuilderTool", "LVL1::eTowerBuilder", "Tool that builds eTowers for simulation"};
  ToolHandle<IeSuperCellTowerMapper> m_eSuperCellTowerMapperTool {this, "eSuperCellTowerMapperTool", "LVL1::eSuperCellTowerMapper", "Tool that maps supercells to eTowers"};

};

} // end of LVL1 namespace
#endif
