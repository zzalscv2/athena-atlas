/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

/**
   @class LArSamples::FitMonitor
   @brief storage of the time histories of all the cells
 */

#ifndef LArSamples_FitMonitor_H
#define LArSamples_FitMonitor_H

#include "LArSamplesMon/MonitorBase.h"
#include "CxxUtils/checker_macros.h"
  
namespace LArSamples {
    
  class ATLAS_NOT_THREAD_SAFE FitMonitor : public MonitorBase  
  {
  
    public:
   
      /** @brief Constructor  */
      FitMonitor(const Interface& interface) : MonitorBase(interface) { } 
      
      bool makeSummary(const char* fileName) const;
  };
}
  
#endif
