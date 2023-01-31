/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// IProcessSamplingTool.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef ISF_FATRASINTERFACES_IPROCESSSAMPLINGTOOL_H
#define ISF_FATRASINTERFACES_IPROCESSSAMPLINGTOOL_H

// Gaudi
#include "GaudiKernel/IAlgTool.h"
#include "CxxUtils/checker_macros.h"
#include "TrkExUtils/ExtrapolationCell.h"
#include "TrkExUtils/TargetSurfaces.h"
#include "ISF_Event/ISFParticleContainer.h"

namespace Trk{
  class Track;
  struct PathLimit;
}
  
namespace ISF {
  class ISFParticle;    
}

namespace iFatras {
  
  /** 
   @class IProcessSamplingTool

   sampling the process/free path
       
   @author Sarka.Todorova -at- cern.ch
   
   */
      
  class ATLAS_NOT_THREAD_SAFE IProcessSamplingTool : virtual public IAlgTool { // deprecated: ATLASSIM-6020
     public:
     
       /** Virtual destructor */
       virtual ~IProcessSamplingTool(){}

       /// Creates the InterfaceID and interfaceID() method
       DeclareInterfaceID(IProcessSamplingTool, 1, 0);

       /** Process, path limit */
       virtual Trk::PathLimit sampleProcess(double momentum, double charge, Trk::ParticleHypothesis pHypothesis) const=0;

       /** Process simulation */
       virtual ISF::ISFParticleVector  interact(const ISF::ISFParticle* isp,
						Trk::ExCellCharged& eCell,
						const Trk::Material* mat=0) const=0;
	 
       /** Process simulation */
       virtual ISF::ISFParticleVector  interact(const ISF::ISFParticle* isp,
						Trk::ExCellNeutral& eCell,
						const Trk::Material* mat=0) const=0;
  };

} // end of namespace

#endif 

