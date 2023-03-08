/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef jetsubstructuremomenttools_softdropobservablestool_header
#define jetsubstructuremomenttools_softdropobservablestool_header

#include "JetSubStructureMomentTools/JetSubStructureMomentToolsBase.h"

class SoftDropObservablesTool :
  public JetSubStructureMomentToolsBase {
    ASG_TOOL_CLASS(SoftDropObservablesTool, IJetModifier)

    public:
      // Constructor and destructor
      SoftDropObservablesTool(const std::string& name);

      int modifyJet(xAOD::Jet &injet) const;

};


#endif
