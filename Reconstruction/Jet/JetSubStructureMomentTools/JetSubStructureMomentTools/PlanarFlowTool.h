/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef jetsubstructuremomenttools_planarflowtool_header
#define jetsubstructuremomenttools_planarflowtool_header

#include "JetSubStructureMomentTools/JetSubStructureMomentToolsBase.h"

class PlanarFlowTool :
  public JetSubStructureMomentToolsBase {
    ASG_TOOL_CLASS(PlanarFlowTool, IJetModifier)
    
    public:
      // Constructor and destructor
      PlanarFlowTool(const std::string& name);

      int modifyJet(xAOD::Jet &injet) const;
};

#endif
