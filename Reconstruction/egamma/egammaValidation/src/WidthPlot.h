/*
Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EGAMMAVALIDATION_WIDTHPLOTS_H
#define EGAMMAVALIDATION_WIDTHPLOTS_H

#include "IHistograms.h"

class ITHistSvc;

namespace egammaMonitoring {
  
  class WidthPlot {
  public:

    WidthPlot(std::string name, std::string folder, ITHistSvc * &rootHistSvc);
    ~WidthPlot(){ };
    StatusCode fill(IHistograms *input);

  private:
    std::string m_name;
    std::string m_folder;
    ITHistSvc*  m_rootHistSvc = nullptr;

  };
  
}

#endif


