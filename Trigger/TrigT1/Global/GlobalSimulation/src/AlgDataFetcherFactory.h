//  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#ifndef GLOBALSIM_ALGDATAFETCHERFACTORY_H
#define GLOBALSIM_ALGDATAFETCHERFACTORY_H


#include <memory>
#include <optional>

#include "IAlgDataFetcher.h"

namespace TrigConf{
  class L1Menu;
}

namespace GlobalSim {
  std::pair<std::optional<std::shared_ptr<IAlgDataFetcher>>,
    std::vector<std::string>>
  makeAlgDataFetcher(bool, bool, const TrigConf::L1Menu*);
}

#endif
