/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "JetSubStructureMomentTools/VolatilityTool.h"
#include "JetSubStructureUtils/Volatility.h"

VolatilityTool::VolatilityTool(const std::string& name) : 
  JetSubStructureMomentToolsBase(name)
{
  declareProperty("NumberOfIterations", m_num_iterations = 100);
  declareProperty("ZCut", m_zcut = 0.01);
  declareProperty("DCut", m_dcut_fctr = 0.5);
  declareProperty("ExpMin", m_exp_min = 0.0);
  declareProperty("ExpMax", m_exp_max = 0.0);
  declareProperty("Rigidity", m_rigidity = 0.1);
  declareProperty("TruncationFactor", m_truncation_fctr = 0.0);
}

int VolatilityTool::modifyJet(xAOD::Jet &jet) const {
  if(!checkForConstituents(jet)) return 1;

  JetSubStructureUtils::Volatility volatility(m_num_iterations, m_zcut, m_dcut_fctr,
      m_exp_min, m_exp_max, m_rigidity, m_truncation_fctr);
  double val = volatility.result(jet);
  ATH_MSG_VERBOSE("Adding jet volatility: " << val);
  jet.setAttribute("Volatility", val);
  return 0;
}
