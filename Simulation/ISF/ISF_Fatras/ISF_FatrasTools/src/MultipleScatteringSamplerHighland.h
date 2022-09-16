/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// MultipleScatteringSamplerHighland.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef ISF_FATRASTOOLS_MULTIPLESCATTERINGSAMPLERHIGHLAND_H
#define ISF_FATRASTOOLS_MULTIPLESCATTERINGSAMPLERHIGHLAND_H
 
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
  
  /**@class MultipleScatteringSamplerHighland
     
     The Formula used is the highland formula for the projected scattering angle :
     
     @f$ \theta_{ms} = \frac{13.6MeV}{p}\cdot\sqrt{t/X_{0}}[1 + 0.038\ln(t/X_{0})] @f$
     
     What is returned is the square of the expectation value of the deflection 
     @f$ < (\theta_ms)^2 > = \sigma_ms^2 @f$
     
     @author Noemi.Calace@cern.ch , Andreas.Salzburger@cern.ch
  */
  
  class MultipleScatteringSamplerHighland : public extends<AthAlgTool, IMultipleScatteringSampler> {
      
  public:
      /** AlgTool like constructor */
      MultipleScatteringSamplerHighland(const std::string&,const std::string&,const IInterface*);
      
      /**Virtual destructor*/
      virtual ~MultipleScatteringSamplerHighland();
      
      /** AlgTool initailize method.*/
      StatusCode initialize();
      
      /** AlgTool finalize method */
      StatusCode finalize();
      
      /** Calculate the theta introduced by multiple scattering,
	  according to the RutherFord-Scott Formula           
      */
      double simTheta(const Trk::MaterialProperties& mat,
		      double p,
		      double pathcorrection,
		      Trk::ParticleHypothesis particle=Trk::pion,
		      double deltaE=0.) const;
      
  private:
      bool                  m_log_include;           //!< boolean switch to include log term  
           
      /** Random Generator service  */
      ServiceHandle<IAtRndmGenSvc>                 m_rndGenSvc;
      /** Random engine  */
      CLHEP::HepRandomEngine*                      m_randomEngine;
      std::string                                  m_randomEngineName;                   //!< Name of the random number stream

    };
  
  
} // end of namespace


#endif // ISF_FATRASTOOLS_MULTIPLESCATTERINGSAMPLERHIGHLAND_H
