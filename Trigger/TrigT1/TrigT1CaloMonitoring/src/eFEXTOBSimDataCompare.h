/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

//***************************************************************************
//                           eFEXTOBSimDataCompare -  description
//                           ------------------------------
//     Small utility function to compare TOBs from data and simulation
//     begin                : 19 08 2022
//     email                : paul.daniel.thompson@CERN.CH
//  ***************************************************************************/

#include <set>
#include <iostream>

namespace LVL1 {


  template <typename T> void compareTOBs(T& dataTOBs, T& simTOBs, std::set<uint32_t> &simEqDataWord0s) {

    // loop over input TOBs and simulated TOBs and fill a std set for those where the first TOB word matches
    // 

    std::cout << "compareTOBs ndata " << dataTOBs->size() << std::endl;
    std::cout << "compareTOBs nsim " << simTOBs->size() << std::endl;

    // Use a std::set of TOB word0s to match TOBs and simTOBs
    std::set<uint32_t> tobWord0sData;

    // Fill set with word0 of TOBs 
    for (auto t : *dataTOBs) {
      uint32_t word0Data = t->word0();
      std::cout << "compareTOBs data " << word0Data << std::endl;
      tobWord0sData.insert(word0Data);
    }
    
    // Set simEqData if the TOB word0s match
    for (auto t : *simTOBs) {
      uint32_t word0Sim = t->word0();
      std::cout << "compareTOBs sim " << word0Sim << std::endl;
      if (tobWord0sData.find(word0Sim) != tobWord0sData.end()) simEqDataWord0s.insert(word0Sim);
    }  

  }
} // end of namespace bracket

