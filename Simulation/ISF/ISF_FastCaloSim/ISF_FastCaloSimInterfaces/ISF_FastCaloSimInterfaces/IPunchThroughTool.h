/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ISF_FASTCALOSIMINTERFACES_IPUNCHTHROUGHTOOL_H
#define ISF_FASTCALOSIMINTERFACES_IPUNCHTHROUGHTOOL_H

// Gaudi
#include "GaudiKernel/IAlgTool.h"

// ISF includes
#include "ISF_Event/ISFParticle.h"
#include "ISF_Event/ISFParticleVector.h"

#include "ISF_FastCaloSimEvent/TFCSSimulationState.h"


namespace Trk{
  class Track;
}

namespace ISF {

  /**
     @class IPunchThroughTool

      Interface for a tool which takes a ISF::ISFParticle as input
      and returns a vector of ISFParticles

     @author Elmar.Ritsch@cern.ch , Anna.Usanova@cern.ch
   */

 class IPunchThroughTool : virtual public IAlgTool {
     public:

        /** Virtual destructor */
        virtual ~IPunchThroughTool(){}

        /// Creates the InterfaceID and interfaceID() method
        DeclareInterfaceID(IPunchThroughTool, 1, 0);

        /** Creates new vector of ISFParticle out of a given ISFParticle */
        virtual const ISF::ISFParticleVector* computePunchThroughParticles(const ISFParticle& isfp, const TFCSSimulationState& simulstate ) const = 0;
 };

} // end of namespace

#endif
