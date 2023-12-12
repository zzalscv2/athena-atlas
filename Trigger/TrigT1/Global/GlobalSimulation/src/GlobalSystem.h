//  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#ifndef GLOBALSIM_SYSTEM_H
#define GLOBALSIM_SYSTEM_H

/*
 * A GlobalSystem contains GEP board objects. It creates a DatarRepository
 * for the board Algorithms to read from and write to.
 *
 * The run method accepts a visitor which allows communication with
 * the external containing system.
 */

#include "GEPBoard.h"
#include "IDataRepositoryVisitor.h"
#include <vector>



namespace GlobalSim {

  class DataRepository;
  
  class GlobalSystem {
  public:
    GlobalSystem() = default;

    GlobalSystem(std::vector<std::unique_ptr<GEPBoard>> GEPBoards);
    bool run(const TCS::TopoInputEvent& event,
	     const std::shared_ptr<IDataRepositoryVisitor>& v);
  private:
    std::vector<std::unique_ptr<GEPBoard>> m_GEPBoards;
  };
}
#endif
