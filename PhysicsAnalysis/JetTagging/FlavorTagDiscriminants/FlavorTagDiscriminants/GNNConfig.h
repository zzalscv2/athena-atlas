/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GNN_CONFIG_H
#define GNN_CONFIG_H

#include "nlohmann/json.hpp"
#include <string>

namespace GNNConfig {

  enum class OutputNodeType {
    UNKNOWN,
    FLOAT,
    VECCHAR,
    VECFLOAT
  };

  struct OutputNodeConfig {
    std::string label;
    OutputNodeType type;
  };

  struct Config {
    std::vector<OutputNodeConfig> outputs;
  };
}

#endif
