//  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#ifndef GLOBALSIM_IALGDATAFETCHER_H
#define GLOBALSIM_IALGDATAFETCHER_H

#include "AlgData.h"

#include <memory>
#include <vector>
#include <string>

// AlgData objects contain dataed need to instantiate
// GlobalSim Algorithms. These typically obtain data
// from a configuration object (eg l1menu).
//
// IGlobalFetcher is a PABC for classes which produce AlgData instances.
//
namespace GlobalSim {
  using ADP = std::shared_ptr<AlgData>;

  class IAlgDataFetcher {
  public:
    virtual ~IAlgDataFetcher() {}

    virtual const std::vector<ADP>& algData() const = 0;

    // AlgData in execution order
    virtual std::vector<ADP> execOrderedAlgData() const = 0;

    virtual bool isValid() const = 0;
    virtual std::vector<std::string>  errMsgs() const = 0;
    virtual std::string toString() const = 0;
  };
}

#endif


