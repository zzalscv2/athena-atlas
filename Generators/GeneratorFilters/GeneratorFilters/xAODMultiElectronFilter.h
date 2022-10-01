/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GENERATORFILTERS_xAODMULTIELECTRONFILTER_H
#define GENERATORFILTERS_xAODMULTIELECTRONFILTER_H

#include "GeneratorModules/GenFilter.h"

class xAODMultiElectronFilter : public GenFilter {
public:

  xAODMultiElectronFilter(const std::string& name, ISvcLocator* pSvcLocator);
  virtual StatusCode filterEvent();

private:

  double m_ptmin;
  double m_etaRange;
  int m_nElectrons;

};

#endif
