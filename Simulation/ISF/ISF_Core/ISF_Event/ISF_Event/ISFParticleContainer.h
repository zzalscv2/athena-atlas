/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// ISFParticleContainer.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
//
#ifndef ISF_EVENT_ISFPARTICLECONTAINER_H
#define ISF_EVENT_ISFPARTICLECONTAINER_H

// STL includes
#include <list>
#include <vector>

// forward declarations
namespace ISF {
  class ISFParticle;
}

namespace ISF {
  /** generic ISFParticle container (not necessarily a std::list!) */
  typedef std::list<ISF::ISFParticle*>                ISFParticleContainer;
  typedef std::list<const ISF::ISFParticle*>          ConstISFParticleContainer;
  /** ISFParticle vector */
  typedef std::vector<ISF::ISFParticle *>             ISFParticleVector;
  typedef std::vector<const ISF::ISFParticle *>       ConstISFParticleVector;
}

#endif // ISF_EVENT_ISFPARTICLECONTAINER_H

