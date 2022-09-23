/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

//! This class implements a tool to calculate ID input variables and add them to the tau aux store
/*!
 * Tau ID input variable calculator tool
 *
 * Author: Lorenz Hauswald
 */

#ifndef DITAUREC_MUHADIDVARCALCULATOR_H
#define DITAUREC_MUHADIDVARCALCULATOR_H

#include "tauRecTools/TauRecToolBase.h"
#include "xAODTau/TauJet.h"
#include <string>

class MuHadIDVarCalculator: public TauRecToolBase
{
  ASG_TOOL_CLASS2(MuHadIDVarCalculator, TauRecToolBase, ITauToolBase)

    public:
  
  MuHadIDVarCalculator(const std::string& name );
  
  ~MuHadIDVarCalculator() override = default ;

  StatusCode eventInitialize() override ;
  
  StatusCode execute(xAOD::TauJet&) override ;

  static const float DEFAULT ;
  
 private:

  std::string m_vertexContainerKey;
  int m_nVtx;
  float m_mu;
};

#endif
