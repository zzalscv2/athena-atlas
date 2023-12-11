/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// JetPullTool.cxx

#include "JetSubStructureMomentTools/JetPullTool.h"
#include "JetSubStructureUtils/Pull.h" 

JetPullTool::JetPullTool(const std::string& name) : 
JetSubStructureMomentToolsBase(name) {
  declareProperty("UseEtaInsteadOfY", m_useEtaInsteadOfY = false);
  declareProperty("IncludeTensorMoments", m_includeTensorMoments = false);
}

int JetPullTool::modifyJet(xAOD::Jet &injet) const {
    
  fastjet::PseudoJet jet;
  bool decorate = SetupDecoration(jet,injet);
  
  std::map<std::string,double> values;
  values["PullMag"]  = -999;
  values["PullPhi"]  = -999;
  values["Pull_C00"] = -999;
  values["Pull_C01"] = -999;
  values["Pull_C10"] = -999;
  values["Pull_C11"] = -999;

  if (decorate) {
    JetSubStructureUtils::Pull pull(m_useEtaInsteadOfY);
    values = pull.result(jet);
  }

  injet.setAttribute(m_prefix+"PullMag", values["PullMag"]);
  injet.setAttribute(m_prefix+"PullPhi", values["PullPhi"]);

  if(m_includeTensorMoments) {
    injet.setAttribute(m_prefix+"Pull_C00", values["Pull_C00"]);
    injet.setAttribute(m_prefix+"Pull_C01", values["Pull_C01"]);
    injet.setAttribute(m_prefix+"Pull_C10", values["Pull_C10"]);
    injet.setAttribute(m_prefix+"Pull_C11", values["Pull_C11"]);
  }

  return 0;
}
