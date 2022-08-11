/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "L1TopoRDO/BlockTypes.h"

namespace L1Topo{

  L1Topo::BlockTypes blockType(const uint32_t word, uint32_t offset, uint32_t size){
    return static_cast<BlockTypes>(word>>offset & size);
  }

  
  std::string blockTypeString(L1Topo::BlockTypes type) {
    if (type==L1Topo::BlockTypes::HEADER) {return "header";}
    else if (type==L1Topo::BlockTypes::FIBRE) {return "fibre";}
    else if (type==L1Topo::BlockTypes::STATUS) {return "status";}
    else if (type==L1Topo::BlockTypes::EM_TOB) {return "EM TOB";}
    else if (type==L1Topo::BlockTypes::TAU_TOB) {return "Tau TOB";}
    else if (type==L1Topo::BlockTypes::MUON_TOB) {return "Muon TOB";}
    else if (type==L1Topo::BlockTypes::JET1_TOB) {return "Jet1 TOB";}
    else if (type==L1Topo::BlockTypes::JET2_TOB) {return "Jet2 TOB";}
    else if (type==L1Topo::BlockTypes::ENERGY_TOB) {return "Energy TOB";}
    else if (type==L1Topo::BlockTypes::L1TOPO_TOB) {return "L1Topo TOB";}
    else {return "unknown";}
  }
    
} // namespace L1Topo
