//  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#ifndef GLOBSIM_IDATAREPOSITORYVISITOR_H
#define GLOBSIM_IDATAREPOSITORYVISITOR_H

#include "GlobalSimDefs.h"
#include <vector>
#include <ostream>

/*
 * PABC for DataRepository Visitors
 * Changes  in the type content oF Depository will force
 * an update of this interface and its implementations.
 * This class is an interface for L1TopoSimulation IO obkectys
 * eg InputTOBS, TOBS, Deceision and Count objects.
 */

namespace GlobalSim {

  class IDataRepositoryVisitor {
    public:
    virtual void process(const std::vector<GSInputTOBArray>&) = 0;
    virtual void process(const std::vector<GSTOBArray>&) = 0;
    virtual void process(const std::vector<GSTOBArrayPtrVec>&) = 0;
    virtual void process(const std::vector<GSCount>&)  = 0;
    virtual void process(const std::vector<GSDecision>&)  = 0;
    
  };


}

#endif
