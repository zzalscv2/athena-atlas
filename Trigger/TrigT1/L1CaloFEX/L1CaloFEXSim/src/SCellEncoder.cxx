#include "L1CaloFEXSim/SCellEncoder.h"
#include "L1CaloFEXSim/SCellIndexing.h"
#include "StoreGate/WriteDecorHandle.h"
#include "xAODTrigger/eFexTauRoIContainer.h"

namespace {
static const SG::AuxElement::Decorator<LVL1::supercells_t>
    decSuperCells("SuperCells");
}

LVL1::SCellEncoder::SCellEncoder() { m_scell_values.reserve(99); }

void LVL1::SCellEncoder::setSuperCells(const unsigned int layer0[][3],
                                       const unsigned int layer1[][3],
                                       const unsigned int layer2[][3],
                                       const unsigned int layer3[][3],
                                       const unsigned int layer4[][3]) {

  for (size_t i = 0; i < LVL1::locMap.size(); i++) {
    int eta = LVL1::locMap[i][0];
    int phi = LVL1::locMap[i][1];
    int layer = LVL1::locMap[i][2];
    switch (layer) {
    case 0:
      m_scell_values.push_back(layer0[eta][phi]);
      break;
    case 1:
      m_scell_values.push_back(layer1[eta][phi]);
      break;
    case 2:
      m_scell_values.push_back(layer2[eta][phi]);
      break;
    case 3:
      m_scell_values.push_back(layer3[eta][phi]);
      break;
    case 4:
      m_scell_values.push_back(layer4[eta][phi]);
      break;
    }
  }
}

void LVL1::SCellEncoder::decorateEFexTauRoI(xAOD::eFexTauRoI *myTauEDM) {
  decSuperCells(*myTauEDM) = m_scell_values;
}
