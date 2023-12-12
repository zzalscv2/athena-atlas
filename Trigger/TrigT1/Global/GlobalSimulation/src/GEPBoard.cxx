//  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#include "GEPBoard.h"

namespace GlobalSim {
  GEPBoard::GEPBoard(const std::vector<std::shared_ptr<IGlobalAlg>>& gsAlgs) :
    m_gsAlgs{gsAlgs}{
  }
  
  bool  GEPBoard::run(DataRepository& repository,
		      const TCS::TopoInputEvent& inputEvent) {

    for (const auto& alg : m_gsAlgs) {
      alg->run(repository, inputEvent);
    }

    return true;
  }
}
