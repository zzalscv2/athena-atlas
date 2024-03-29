/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TransportTool.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef ISF_FATRASTOOLS_TRANSPORTTOOL_H
#define ISF_FATRASTOOLS_TRANSPORTTOOL_H

// Athena Base
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ITHistSvc.h"
#include "AthenaKernel/IAtRndmGenSvc.h"
#include "CxxUtils/checker_macros.h"
#include "TrkExInterfaces/ITimedExtrapolator.h"

// IFS
#include "ISF_Interfaces/IParticleProcessor.h"

// Tracking
#include "TrkEventPrimitives/PdgToParticleHypothesis.h"
#include "TrkParameters/TrackParameters.h"
// ROOT forward declarations
class TTree;

//class AtlasDetectorID;

namespace ISF {
    class ISFParticle;
    class IParticleHelper;
    class IParticleFilter;
}

namespace iFatras 
{
 
  class IParticleDecayHelper;
  class ISimHitCreator;    
  class IPhysicsValidationTool;
  class IProcessSamplingTool;
    
  /** @class TransportTool 
      
      Fatras AlgTool to create a ISFParticle at a volume entry/exit
      - universal transport tool
      
      @author Sharka Todorova Sarka.Todorova -at- cern.ch
      @author Andreas.Salzburger -at- cern.ch
  */  
  
  class ATLAS_NOT_THREAD_SAFE TransportTool : public extends<AthAlgTool, ISF::IParticleProcessor>  // deprecated: ATLASSIM-6020
  {
  public:
    /** Constructor */
    TransportTool(const std::string&,const std::string&,const IInterface*);
    
    /** Destructor */
    virtual ~TransportTool ();
    
    /** AlgTool initialize method */
    virtual StatusCode initialize() override;
    
    /** AlgTool finalize method */

    virtual StatusCode finalize() override;
     
    /** Creates a new ParticleState from a given ParticleState, universal transport tool */
    virtual
    ISF::ISFParticle* process(const ISF::ISFParticle& isp, CLHEP::HepRandomEngine*) const override;
    
  private:
     /** templated Tool retrieval - gives unique handling & look and feel */
     template <class T> StatusCode retrieveTool(ToolHandle<T>& thandle){
       if (!thandle.empty() && thandle.retrieve().isFailure()){
         ATH_MSG_FATAL( "[ fatras setup ] Cannot retrieve " << thandle << ". Abort.");
         return StatusCode::FAILURE;
       }
       else {
         ATH_MSG_DEBUG("[ fatras setup ] Successfully retrieved " << thandle);
       }
       return StatusCode::SUCCESS;
     }
      
    /*---------------------------------------------------------------------
     *  Private members
     *---------------------------------------------------------------------*/

     /** Random Generator service  */
     ServiceHandle<IAtRndmGenSvc>                 m_rndGenSvc;
     CLHEP::HepRandomEngine*                             m_randomEngine;
     std::string                                  m_randomEngineName;         //!< Name of the random number stream


    /** Validation output with histogram service */
    bool                                                         m_validationOutput; //!< turn validation mode on/off
    ToolHandle<IPhysicsValidationTool>                           m_validationTool;   //!< the ntuple

    /** Particle Decay */
    ToolHandle<IParticleDecayHelper>    m_particleDecayHelper;

    /** The SimHit Creation */
    ToolHandle<ISimHitCreator>          m_simHitCreatorID;
    ToolHandle<ISimHitCreator>          m_simHitCreatorMS;

    /** The Extrapolator setup */
    ToolHandle<Trk::ITimedExtrapolator>      m_extrapolator;          
    
    /** Filtering setup & other ISF stuff*/
    ToolHandle<ISF::IParticleFilter>    m_trackFilter;
    ToolHandle<ISF::IParticleFilter>    m_neutralHadronFilter;
    ToolHandle<ISF::IParticleFilter>    m_photonFilter;
    ToolHandle<ISF::IParticleHelper>    m_iparticleHelper;
    ToolHandle<IProcessSamplingTool>    m_samplingTool;

    //const AtlasDetectorID*              m_idHelper;     //*> hit type identification   
    Trk::PdgToParticleHypothesis        m_pdgToParticleHypothesis;

    bool                                m_errorPropagation;    // error propagation for eloss validation
    bool                                m_hitsOff;             // steering of sim hit production

  }; 
}

#endif // FATRASTOOLS_TRANSPORTTOOL_H
