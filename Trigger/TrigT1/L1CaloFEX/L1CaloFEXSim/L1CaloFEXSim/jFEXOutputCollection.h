/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

//***************************************************************************
//                           jFEXOutputCollection.h  -  
//                              -------------------
//     begin                : 09 12 2020
//     email                : varsiha.sothilingam@cern.ch
//  **************************************************************************


#ifndef JFEX_OUTPUT_COLLECTION_H
#define JFEX_OUTPUT_COLLECTION_H

#include "AthenaKernel/CLASS_DEF.h"
#include "StoreGate/StoreGateSvc.h"
#include <map>
#include <iostream>
#include <vector>
#include <string>

namespace LVL1 {
  class jFEXOutputCollection
  {
  public:
    jFEXOutputCollection() {};
    ~jFEXOutputCollection();
    void clear();
    void addValue_smallRJet(std::string, int);
    void fill_smallRJet();
    void addValue_largeRJet(std::string, int);
    void fill_largeRJet();
    void addValue_tau(std::string, int);
    void fill_tau();
    int SRsize();
    int LRsize();
    int tausize();
    std::map<std::string, int>* get_smallRJet(int);
    std::map<std::string, int>* get_largeRJet(int);
    std::map<std::string, int>* get_tau(int);

  private:
    std::map<std::string, int> m_values_tem_smallRJet;
    std::vector<std::map<std::string, int>*> m_allvalues_smallRJet;
    std::map<std::string, int> m_values_tem_largeRJet;
    std::vector<std::map<std::string, int>*> m_allvalues_largeRJet;
    std::map<std::string, int> m_values_tem_tau;
    std::vector<std::map<std::string, int>*> m_allvalues_tau;
  };
}

CLASS_DEF(LVL1::jFEXOutputCollection, 1317184196 , 1 )

#endif
