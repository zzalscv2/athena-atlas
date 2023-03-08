/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef jetsubstructuremomenttools_angularitytool_header
#define jetsubstructuremomenttools_angularitytool_header

#include "JetSubStructureMomentTools/JetSubStructureMomentToolsBase.h"

class AngularityTool :
  public JetSubStructureMomentToolsBase {
    ASG_TOOL_CLASS(AngularityTool, IJetModifier)
    
    public:
      // Constructor and destructor
      AngularityTool(const std::string& name);

      int modifyJet(xAOD::Jet &injet) const;
};

#endif
