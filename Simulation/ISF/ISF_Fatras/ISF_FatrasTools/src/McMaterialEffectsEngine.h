/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/


///////////////////////////////////////////////////////////////////
// McMaterialEffectsEngine.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef ISF_FATRASTOOLS_MCMATERIALEFFECTENIGINE_H
#define ISF_FATRASTOOLS_MCMATERIALEFFECTENIGINE_H

// GaudiKernel & Athena
#include "AthenaBaseComps/AthAlgTool.h"
#include "AthenaKernel/IAtRndmGenSvc.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"
#include "CxxUtils/checker_macros.h"

// Trk
#include "TrkExInterfaces/IMaterialEffectsEngine.h"
#include "TrkExInterfaces/IEnergyLossUpdator.h"
#include "TrkExUtils/ExtrapolationCell.h"
#include "TrkEventPrimitives/PropDirection.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkExUtils/MaterialUpdateMode.h"
#include "TrkEventPrimitives/PdgToParticleHypothesis.h"

// ISF
#include "ISF_Event/ITruthIncident.h"
#include "ISF_Event/ISFParticleContainer.h"

// Barcode
#include "BarcodeEvent/PhysicsProcessCode.h"

class StoreGateSvc;

namespace Trk{
  class EnergyLoss;
  class CylinderVolumeBounds;
  class TrackingGeometry;
}

namespace ISF {
  class IParticleBroker;
  class ISFParticle;
  class ITruthSvc;
}

namespace iFatras {

  class IPhysicsValidationTool;
  class IEnergyLossSampler;
  class IMultipleScatteringSampler;
  class IHadronicInteractionProcessor;
  class IPhotonConversionSampler; 
  class IParticleDecayHelper;
  class IProcessSamplingTool;
  
  /** @class McMaterialEffectsEngine
      
      Mc Material effects engine for charged and neutral (fast track simulation) used inside the ExtrapolatorEngine. It extends the IMaterialEffectsEngine.
      
      The McMaterialEffectsEngine uses both an extended EnergyLossSampler
      and the standard ATLAS MultipleScatteringSampler configured for the Gaussian-Mixture-Model.
      
      @author Andreas.Salzburger@cern.ch, Noemi.Calace@cern.ch
  */
  
  class ATLAS_NOT_THREAD_SAFE McMaterialEffectsEngine : public extends<AthAlgTool, Trk::IMaterialEffectsEngine> {  // deprecated: ATLASSIM-6020
  public:      
    /**AlgTool constructor for McMaterialEffectsEngine*/
    McMaterialEffectsEngine(const std::string&,const std::string&,const IInterface*);
    /**Destructor*/
    virtual ~McMaterialEffectsEngine();
    
    /** AlgTool initailize method.*/
    StatusCode initialize();
    /** AlgTool finalize method */
    StatusCode finalize();
    
    /** charged extrapolation */
    virtual Trk::ExtrapolationCode handleMaterial(Trk::ExCellCharged& ecCharged,
						  Trk::PropDirection dir=Trk::alongMomentum,
						  Trk::MaterialUpdateStage matupstage=Trk::fullUpdate) const;

    /** charged extrapolation */
    virtual Trk::ExtrapolationCode handleMaterial(Trk::ExCellNeutral& ecNeutral,
						  Trk::PropDirection dir= Trk::alongMomentum,
						  Trk::MaterialUpdateStage matupstage=Trk::fullUpdate) const;
    
  private:

    template <class T> Trk::ExtrapolationCode handleMaterialT( Trk::ExtrapolationCell<T>& eCell,
							       Trk::PropDirection dir=Trk::alongMomentum,
							       Trk::MaterialUpdateStage matupstage=Trk::fullUpdate) const;
    
    Trk::ExtrapolationCode processMaterialOnLayer(const ISF::ISFParticle* isp,
				Trk::ExCellCharged& eCell,
				Trk::PropDirection dir,
				float& mFraction) const;

    Trk::ExtrapolationCode processMaterialOnLayer(const ISF::ISFParticle* isp,
				Trk::ExCellNeutral& eCell,
				Trk::PropDirection dir,
				float& mFraction) const;

    void multipleScatteringUpdate(const Trk::TrackParameters& pars,
				  AmgVector(5)& parameters,
				  double simTheta, 
				  double num_deltaPhi) const;

    void recordBremPhoton(const ISF::ISFParticle* parent,
			  double time,
			  double pElectron,
			  double gammaE,
			  const Amg::Vector3D& vertex,
			  Amg::Vector3D& particleDir,Trk::ParticleHypothesis  ) const; 

    void recordBremPhotonLay(const ISF::ISFParticle* parent,
			     double time,
			     double pElectron,
			     double gammaE,
			     const Amg::Vector3D& vertex,
			     Amg::Vector3D& particleDir,Trk::ParticleHypothesis, 
			     Trk::PropDirection dir,float mFraction ) const; 

    ISF::ISFParticle* bremPhoton(const ISF::ISFParticle* parent,
				 double time,
				 double pElectron,
				 double gammaE,
				 const Amg::Vector3D& vertex,
				 Amg::Vector3D& particleDir, Trk::ParticleHypothesis ) const; 
    
    void radiate(const ISF::ISFParticle* parent, AmgVector(5)& parm ,
		 Trk::ExCellCharged& eCell, float pathlim, float mFr,Trk::PropDirection dir,float refX) const;
    
    /** IProcessSamplig */
    ToolHandle<IProcessSamplingTool>             m_samplingTool;

    /** IEnergyLossSampler */
    bool                                             m_eLoss;
    ToolHandle<Trk::IEnergyLossUpdator>              m_eLossSampler;
    
    /** Boolean switch for use of a dedicated eloss updator */
    bool                                             m_dedicatedElectronSampler;     
    /** Pointer to the energy loss sampler - electrons */
    ToolHandle<iFatras::IEnergyLossSampler>          m_elEnergyLossSampler;          
   
    /** IMultipleScatteringSampler */
    bool                                             m_ms;
    ToolHandle<iFatras::IMultipleScatteringSampler>  m_msSampler;

    /** Particle Decay */
    ToolHandle<IParticleDecayHelper>             m_particleDecayHelper; 
    
    /** describe deflection parametric/do real deflection */
    bool                                             m_parametricScattering;

    /** use the relativistic hertz dipole for brem photon radiation */
    bool                                             m_uniformHertzDipoleAngle;
    
    /** MCTruth process code for TruthIncidents created by this tool */
    Barcode::PhysicsProcessCode                      m_processCode;
     
    /** Minimum momentum cut */
    double                                           m_minimumMomentum;
    
    /** Minimum momentum for brem photons */
    double                                           m_minimumBremPhotonMomentum;
    
    /** Create brem photons flag */
    bool                                             m_createBremPhoton;
     
    /** Switch to use bending correction */
    bool                                             m_bendingCorrection;

    /** Random Generator service  */
    ServiceHandle<IAtRndmGenSvc>                     m_rndGenSvc;
    /** Random engine  */
    CLHEP::HepRandomEngine*                          m_randomEngine;
    /** Name of the random number stream */
    std::string                                      m_randomEngineName;                 
    
    Trk::PdgToParticleHypothesis                     m_pdgToParticleHypothesis;

    /** Validation section */
    bool                          m_validationMode;
    ToolHandle<IPhysicsValidationTool>  m_validationTool;

    ServiceHandle<ISF::IParticleBroker>              m_particleBroker;
    ServiceHandle<ISF::ITruthSvc>                    m_truthRecordSvc;

    /** useful for the angle calculation of the brem photon */ 
    double                                           m_oneOverThree;

    /** cache incoming particle */
    mutable const ISF::ISFParticle*                  m_isp;
    
    /** cache layer properties */
    mutable const Trk::Layer*                       m_layer;
    mutable const Trk::MaterialProperties*          m_matProp;           
    mutable float                                   m_thicknessInX0;
    mutable float                                   m_thicknessInL0;
     
	double                                          m_projectionFactor;
  };
        
}

//!< define the templated function   
#include "McMaterialEffectsEngine.icc" 

#endif // ISF_FATRASTOOLS_MCMATERIALEFFECTENIGINE_H
