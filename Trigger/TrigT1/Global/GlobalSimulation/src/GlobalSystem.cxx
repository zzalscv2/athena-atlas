//  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#include "GlobalSystem.h"
#include "DataRepository.h"

namespace GlobalSim {
  GlobalSystem::GlobalSystem(std::vector<std::unique_ptr<GEPBoard>> GEPBoards) :
    m_GEPBoards{std::move(GEPBoards)}{}

  bool GlobalSystem::run(const TCS::TopoInputEvent& event,
			 const std::shared_ptr<IDataRepositoryVisitor>& v) {
    
    // Input MUX simulation to be placed here
    // For now, simply loop over GEP boards
    for (auto& board : m_GEPBoards) {
      auto repository = DataRepository();
      board->run(repository, event);
      if(v) {repository.accept(*v);}
    }

    return true;
  }
}
