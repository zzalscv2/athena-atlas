/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGACCELEVENT_MODULE_H
#define TRIGACCELEVENT_MODULE_H

#include <iostream>
#include <vector>
#include <memory>

#include "WorkFactory.h"

namespace TrigAccel {

  class Module{
  public:
    Module() = default;
    virtual ~Module() = default;
    virtual const std::vector<int> getFactoryIds() = 0;
    virtual WorkFactory* getFactoryById(int id) = 0;
  };
}

#endif