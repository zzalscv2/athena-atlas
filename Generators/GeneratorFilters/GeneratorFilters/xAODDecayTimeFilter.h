/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/


#ifndef GENERATORFILTERS_XAODDECAYTIMEFILTER_H
#define GENERATORFILTERS_XAODDECAYTIMEFILTER_H

#include "GeneratorModules/GenFilter.h"
#include "GaudiKernel/ServiceHandle.h"
#include "xAODTruth/TruthParticle.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODTruth/TruthParticleAuxContainer.h"
#include "AthenaKernel/IAthRNGSvc.h"

#include "xAODTruth/TruthEvent.h"
#include "xAODTruth/TruthEventContainer.h"

namespace CLHEP {
  class HepRandomEngine;
}

class xAODDecayTimeFilter : public GenFilter {
public:

  xAODDecayTimeFilter(const std::string& name, ISvcLocator* pSvcLocator);
  virtual StatusCode filterInitialize();
  virtual StatusCode filterEvent();

private:
   CLHEP::HepRandomEngine* getRandomEngine(const std::string& streamName,
                                          const EventContext& ctx) const;

   double tau(const xAOD::TruthParticle* ptr) const;
   float m_lifetimeLow;
   float m_lifetimeHigh;
   float m_seedlifetime;
   ServiceHandle<IAthRNGSvc> m_rndmSvc{this, "RndmSvc", "AthRNGSvc"};
   bool m_flatlifetime; 
   std::vector<int> m_particleID;
};



#endif
