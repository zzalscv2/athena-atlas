/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// IPhotonConversionTool.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef ISF_FATRASINTERFACES_IPHOTONCONVERSIONTOOL_H
#define ISF_FATRASINTERFACES_IPHOTONCONVERSIONTOOL_H

// Gaudi
#include "GaudiKernel/IAlgTool.h"
#include "CxxUtils/checker_macros.h"
// Event primitives
#include "TrkEventPrimitives/ParticleHypothesis.h"
#include "TrkNeutralParameters/NeutralParameters.h"
// ISF
#include "ISF_Event/ISFParticleContainer.h"

namespace Trk {
  class MaterialProperties;
  class ExtendedMaterialProperties;
}

/*-------------------------------------------------------------------------
 *  class IPhotonConversionTool
 *-------------------------------------------------------------------------*/
namespace iFatras
{
  /** 
      @class IPhotonConversionTool
      
      Interface definition for the handling of nuclear/hadronic interactions,
      to be used by the MC based material effects updater
      
      @author Sarka.Todorova@cern.ch
  */
  class ATLAS_NOT_THREAD_SAFE IPhotonConversionTool : virtual public IAlgTool {  // deprecated: ATLASSIM-6020

  public:
    
    /** Virtual destructor */    virtual ~IPhotonConversionTool(){}
    
    /// Creates the InterfaceID and interfaceID() method
    DeclareInterfaceID(IPhotonConversionTool, 1, 0);
    
    /** interface for processing of the pair production */
    virtual bool pairProduction(const Trk::MaterialProperties& mprop,
				double pathCorrection,
				double p) const = 0;

    /** interface for processing of the presampled pair production */
    virtual bool doConversion(double time, const Trk::NeutralParameters& parm,
			      const Trk::ExtendedMaterialProperties* extMatProp=0) const;

    /** interface for processing of the presampled conversion on layer*/
    virtual ISF::ISFParticleVector doConversionOnLayer(const ISF::ISFParticle* parent, 
						       double time, const Trk::NeutralParameters& parm,
						       const Trk::ExtendedMaterialProperties *ematprop=0) const;

  };

  inline bool IPhotonConversionTool::doConversion(double /*time*/, const Trk::NeutralParameters& /*parm*/,
						  const Trk::ExtendedMaterialProperties* /*ematprop*/) const
 {
    return false;
  }

  inline ISF::ISFParticleVector IPhotonConversionTool::doConversionOnLayer(const ISF::ISFParticle* /*parent*/,
									   double /*time*/, const Trk::NeutralParameters& /*parm*/,
									   const Trk::ExtendedMaterialProperties* /*ematprop*/) const
  {
    ISF::ISFParticleVector cv(0); return cv;
  }

}

#endif // ISF_FATRASINTERFACES_IPHOTONCONVERSIONTOOL_H

