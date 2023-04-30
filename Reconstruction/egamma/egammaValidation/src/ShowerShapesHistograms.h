/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EGAMMAVALIDATION_SHOWERSHAPESHISTOGRAMS_H
#define EGAMMAVALIDATION_SHOWERSHAPESHISTOGRAMS_H

#include <map>
#include "xAODEgamma/Egamma.h"

class TH1D;
class TH2D;
class ITHistSvc;

namespace egammaMonitoring {

  class ShowerShapesHistograms {
  public:

    // Histos
    ShowerShapesHistograms(std::string name,
			   std::string title,
			   std::string folder,
			   ITHistSvc * &rootHistSvc) :
      m_name(std::move(std::move(name))),
      m_title(std::move(std::move(title))),
      m_folder(std::move(std::move(folder))),
      m_rootHistSvc(rootHistSvc) {}

    std::map<std::string, TH1D* > histoMap;
    std::map<std::string, TH2D* > histo2DMap;

    StatusCode initializePlots();
    void fill(const xAOD::Egamma& egamma);

  protected:
    std::string m_name;
    std::string m_title;
    std::string m_folder;
    ITHistSvc*  m_rootHistSvc = nullptr;

  };

}

#endif
