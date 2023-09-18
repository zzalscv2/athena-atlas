/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EGAMMAVALIDATION_RECOELECTRONHISTOGRAMS_H
#define EGAMMAVALIDATION_RECOELECTRONHISTOGRAMS_H

#include "xAODEgamma/Electron.h"
#include "ParticleHistograms.h"

class TH2D;
class TH3D;

namespace egammaMonitoring {

  class RecoElectronHistograms : public ParticleHistograms {

  public:
    using ParticleHistograms::ParticleHistograms;

    std::map<std::string, TH2D*> histo2DMap;
    std::map<std::string, TH3D*> histo3DMap;
    
    StatusCode initializePlots();

    using ParticleHistograms::fill;
    void fill(const xAOD::Electron& elrec);

    void isData(bool b = true) { m_isData = b; }

  private:
    bool m_isData = false;

  };

}

#endif
