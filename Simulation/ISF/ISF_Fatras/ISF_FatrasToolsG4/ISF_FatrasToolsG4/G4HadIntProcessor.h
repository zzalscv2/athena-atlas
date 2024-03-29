/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////
// G4HadIntProcessor.h, (c) ATLAS Detector software
/////////////////////////////////////////////////////////////////// 
#ifndef ISF_FATRASTOOLSG4_G4HADINTPROCESSOR_H
#define ISF_FATRASTOOLSG4_G4HADINTPROCESSOR_H

// GaudiKernel & Athena
#include "AthenaBaseComps/AthAlgTool.h"
#include "AthenaKernel/IAtRndmGenSvc.h"
#include "CxxUtils/checker_macros.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"

// Fatras
#include "ISF_FatrasInterfaces/IHadronicInteractionProcessor.h"

//Barcode
#include "BarcodeEvent/PhysicsProcessCode.h"

// Geant4
#include "G4ThreeVector.hh"

// Trk
#include "TrkEventPrimitives/ParticleHypothesis.h"

// ISF
#include "ISF_Event/ITruthIncident.h"
#include "ISF_Event/ISFParticleContainer.h"

// Forward Declarations
class TTree;
class G4DynamicParticle;
class G4VProcess;
class G4RunManager;
class G4VUserPhysicsList;
class G4LayerDetectorConstruction;
class G4LayerPrimaryGeneratorAction;
class G4LayerTrackingAction;
class G4Step;
class G4StepPoint;
class G4AtlasRunManager;
class G4Material;
class G4MaterialCutsCouple;

namespace Trk {
  class Material;
  class MaterialProperties;       // TODO: get rid of MatProp dependence
}

namespace ISF {
  class IParticleBroker;
  class ITruthSvc;
  class IG4RunManagerHelper;
}

namespace iFatras {

  class IPhysicsValidationTool;

  /** @class G4HadIntProcessor

    Wrapper class for multiple scattering, energyloss, hadronic interaction
    tool from Geant4.

    @author Andreas.Salzburger@cern.ch
    */

  class ATLAS_NOT_THREAD_SAFE G4HadIntProcessor : public extends<AthAlgTool, iFatras::IHadronicInteractionProcessor> {  // deprecated: ATLASSIM-6020
    public:      
      /** AlgTool constructor for G4HadIntProcessor*/
      G4HadIntProcessor(const std::string&,const std::string&,const IInterface*);
      /**Destructor*/
      virtual ~G4HadIntProcessor();

      /** AlgTool initailize method.*/
      StatusCode initialize();
      /** AlgTool finalize method */
      StatusCode finalize();

      /** interface for processing of the nuclear interactions */
      bool hadronicInteraction(const Amg::Vector3D& position, const Amg::Vector3D& momentum, 
			       double p, double E, double charge, 
                               const Trk::MaterialProperties& mprop, double pathCorrection,
                               Trk::ParticleHypothesis particle=Trk::pion) const;

      bool doHadronicInteraction(double time, const Amg::Vector3D& position, const Amg::Vector3D& momentum,
                                 const Trk::Material *ematprop,
				 Trk::ParticleHypothesis particle=Trk::pion,
                                 bool  processSecondaries=true) const;

      /** interface for processing of the presampled nuclear interactions on layer*/
      ISF::ISFParticleVector doHadIntOnLayer(const ISF::ISFParticle* parent, double time, 
					     const Amg::Vector3D& position, const Amg::Vector3D& momentum,
					     const Trk::Material *ematprop,
					     Trk::ParticleHypothesis particle=Trk::pion) const ;

    private:
      /** initialize G4RunManager on first call if not done by then */
      StatusCode initG4RunManager ATLAS_NOT_THREAD_SAFE ();

      /** collect secondaries for layer material update */                           
      ISF::ISFParticleVector getHadState(const ISF::ISFParticle* parent,
					 double time, const Amg::Vector3D& position, const Amg::Vector3D& momentum, 
					 const Trk::Material *ematprop) const;

      //!< Initialize inleastic hadronic Geant4 processes 
      std::map<int,G4VProcess*>::const_iterator  initProcessPDG(int pdg);

      //!< choose for list of predefined (pure) materials
      unsigned int retrieveG4MaterialIndex(const Trk::Material* ematprop) const;

      //!< random number service
      ServiceHandle<IAtRndmGenSvc>         m_rndGenSvc;

      ToolHandle<ISF::IG4RunManagerHelper> m_g4RunManagerHelper;

      //!< steering: enable elastic interactions?
      bool                                 m_doElastic;

      /* scale factors for hadronic/electromagnetic interactions */
      double                               m_hadIntProbScale;

      // internal steering : clone type
      double                               m_minMomentum;

      //!< Geant4 processes <PDGcode, process>  TODO : fission, capture
      std::map<int, G4VProcess*>   m_g4HadrInelasticProcesses;
      std::map<int, G4VProcess*>   m_g4HadrElasticProcesses;

      std::vector<std::pair<float,std::pair< G4Material*, G4MaterialCutsCouple> > > m_g4Material;

      /** ISF services & Tools */
      ServiceHandle<ISF::IParticleBroker>  m_particleBroker;
      ServiceHandle<ISF::ITruthSvc>        m_truthRecordSvc;

      /** Random engine  */
      CLHEP::HepRandomEngine*              m_randomEngine;
      std::string                          m_randomEngineName;       //!< Name of the random number stream

      // ------------------------ Validation section ------------------------------------
      bool                          m_validationMode;
      ToolHandle<IPhysicsValidationTool>  m_validationTool;
      std::string                   m_validationTreeName;        //!< validation tree name - to be acessed by this from root
      std::string                   m_validationTreeDescription; //!< validation tree description - second argument in TTree
      std::string                   m_validationTreeFolder;      //!< stream/folder to for the TTree to be written out


      std::string                   m_bremValidationTreeName;        //!< validation tree name - to be acessed by this from root
      std::string                   m_bremValidationTreeDescription; //!< validation tree description - second argument in TTree
      std::string                   m_bremValidationTreeFolder;      //!< stream/folder to for the TTree to be written out


      // --------------------------------------------------------------------------------

      std::string                   m_edValidationTreeName;        //!< validation tree name - to be acessed by this from root
      std::string                   m_edValidationTreeDescription; //!< validation tree description - second argument in TTree
      std::string                   m_edValidationTreeFolder;      //!< stream/folder to for the TTree to be written out
  };

}

#endif
