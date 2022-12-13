/*
 *   Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
 */

#ifndef TRIGL0GEPPERF_ICLUSTERMAKER_H
#define TRIGL0GEPPERF_ICLUSTERMAKER_H

#include <map> 
#include <string>

#include "./Cluster.h"
#include "./CustomCaloCell.h"

#include <memory>

using  GepCellMap = std::map<unsigned int,Gep::CustomCaloCell>;
using  pGepCellMap = std::unique_ptr<GepCellMap>;

namespace Gep{
  class IClusterMaker
  {
  public:
    
    virtual std::vector<Gep::Cluster>
    makeClusters(const pGepCellMap&) const = 0;
    
    virtual std::string getName() const = 0;
    
    virtual ~IClusterMaker() {}
    
  };
}


#endif
