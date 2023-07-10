/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// MultipleScatteringSamplerGeneralMixture.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef ISF_FATRASTOOLS_MULTIPLESCATTERINGSAMPLERGENERALMIXTURE_H
#define ISF_FATRASTOOLS_MULTIPLESCATTERINGSAMPLERGENERALMIXTURE_H
 
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
#include "CxxUtils/checker_macros.h"

namespace Trk {
  class MaterialProperties;
}

namespace iFatras {
  
  /**@class MultipleScatteringSamplerGeneralMixture
     
     @author Noemi.Calace@cern.ch , Andreas.Salzburger@cern.ch, Artem.Basalaev@cern.ch
  */
  
  // deprecated: ATLASSIM-6020
  class ATLAS_NOT_THREAD_SAFE MultipleScatteringSamplerGeneralMixture : public extends<AthAlgTool, IMultipleScatteringSampler> {
      
  public:
      /** AlgTool like constructor */
      MultipleScatteringSamplerGeneralMixture(const std::string&,const std::string&,const IInterface*);
      
      /**Virtual destructor*/
      virtual ~MultipleScatteringSamplerGeneralMixture();
      
      /** AlgTool initailize method.*/
      StatusCode initialize();
      
      /** AlgTool finalize method */
      StatusCode finalize();
      
      /** Calculate theta introduced by multiple scattering,
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

      //!< General mixture model: get parameters for single gaussian simulation
      static std::vector<double> getGaussian(double beta, double p,double dOverX0, double scale) ;
      //!< General mixture model: get parameters for gaussian mixture
      static std::vector<double> 	getGaussmix(double beta, double p,double dOverX0,double Z, double scale) ;
      //!< General mixture model: get parameters for semi-gaussian mixture
      static std::vector<double> 	getSemigauss(double beta,double p,double dOverX0,double Z, double scale) ;
      //!< General mixture model: simulate semi-gaussian mixture
      double 	simGaussmix(const std::vector<double>& scattering_params) const;
      //!< General mixture model: simulate gaussian mixture
      double 	simSemigauss(const std::vector<double>& scattering_params) const;
    };
  
  
} // end of namespace


#endif // ISF_FATRASTOOLS_MULTIPLESCATTERINGSAMPLERGENERALMIXTURE_H
