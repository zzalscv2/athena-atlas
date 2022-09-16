/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// MultipleScatteringSamplerGaussianMixture.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef ISF_FATRASTOOLS_MULTIPLESCATTERINGSAMPLERGAUSSIANMIXTURE_H
#define ISF_FATRASTOOLS_MULTIPLESCATTERINGSAMPLERGAUSSIANMIXTURE_H
 
// Gaudi
#include "AthenaBaseComps/AthAlgTool.h"
// Trk
#include "ISF_FatrasInterfaces/IMultipleScatteringSampler.h"
#include "TrkEventPrimitives/PropDirection.h"
#include "GaudiKernel/ServiceHandle.h"

#include "AthenaKernel/IAtRndmGenSvc.h"

#include "CLHEP/Random/RandFlat.h"
#include "GeoPrimitives/GeoPrimitives.h"
#include "EventPrimitives/EventPrimitives.h"

namespace Trk {
  class MaterialProperties;
}

namespace iFatras {
  
  /**@class MultipleScatteringSamplerGaussianMixture
     
     @author Noemi.Calace@cern.ch , Andreas.Salzburger@cern.ch
  */
  
  class MultipleScatteringSamplerGaussianMixture : public extends<AthAlgTool, IMultipleScatteringSampler> {
      
  public:
      /** AlgTool like constructor */
      MultipleScatteringSamplerGaussianMixture(const std::string&,const std::string&,const IInterface*);
      
      /**Virtual destructor*/
      virtual ~MultipleScatteringSamplerGaussianMixture();
      
      /** AlgTool initailize method.*/
      StatusCode initialize();
      
      /** AlgTool finalize method */
      StatusCode finalize();
      
      /** Calculate the sigma on theta introduced by multiple scattering,
	  according to the RutherFord-Scott Formula           
      */
      double simTheta(const Trk::MaterialProperties& mat,
		      double p,
		      double pathcorrection,
		      Trk::ParticleHypothesis particle=Trk::pion,
		      double deltaE=0.) const;
      
  private:
      bool                  m_log_include;           //!< boolean switch to include log term  
      
      bool                  m_optGaussianMixtureG4;  //!< modifies the Fruehwirth/Regler model to fit with G4
      
      //========== used for Gaussian mixture model =================================================
      
      /** Random Generator service  */
      ServiceHandle<IAtRndmGenSvc>                 m_rndGenSvc;
      /** Random engine  */
      CLHEP::HepRandomEngine*                      m_randomEngine;
      std::string                                  m_randomEngineName;                   //!< Name of the random number stream
    };
  
  
} // end of namespace


#endif // ISF_FATRASTOOLS_MULTIPLESCATTERINGSAMPLERGAUSSIANMIXTURE_H
