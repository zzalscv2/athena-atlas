/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef eTowerMakerFromEfexTowers_H
#define eTowerMakerFromEfexTowers_H

// STL
#include <string>

// Athena/Gaudi
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AthenaBaseComps/AthAlgorithm.h"
#include "L1CaloFEXSim/eTowerContainer.h"
#include "xAODTrigL1Calo/eFexTowerContainer.h"

class CaloIdManager;

namespace LVL1 {

class eTowerMakerFromEfexTowers : public AthAlgorithm
{
 public:
  //using AthReentrantAlgorithm::AthReentrantAlgorithm;

  eTowerMakerFromEfexTowers(const std::string& name, ISvcLocator* pSvcLocator);
  ~eTowerMakerFromEfexTowers() = default;

  virtual StatusCode initialize();
  virtual StatusCode execute(/*const EventContext& ctx*/);// const;

 private:

  int m_numberOfEvents = 0;

    SG::ReadHandleKey<xAOD::eFexTowerContainer> m_eFexTowerContainerSGKey {this, "InputTowers", "L1_eFexDataTowers", "efexTowers (use L1_eFexEmulatedTowers for built from SC, or L1_eFexDataTowers for efex readout"};
    SG::WriteHandleKey<LVL1::eTowerContainer> m_eTowerContainerSGKey {this, "MyETowers", "eTowerContainer", "MyETowers"};
    ToolHandle<IeTowerBuilder> m_eTowerBuilderTool {this, "eTowerBuilderTool", "LVL1::eTowerBuilder", "Tool that builds eTowers for simulation"};
};

} // end of LVL1 namespace
#endif
