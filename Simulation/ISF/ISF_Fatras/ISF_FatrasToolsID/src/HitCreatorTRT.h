/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// HitCreatorTRT.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef ISF_FATRASTOOLSID_HITCREATORTRT_H
#define ISF_FATRASTOOLSID_HITCREATORTRT_H

// Fatras
#include "ISF_FatrasInterfaces/IHitCreator.h"
// Gaudi
#include "AthenaBaseComps/AthAlgTool.h"
#include "AthenaKernel/IAtRndmGenSvc.h"
#include "GaudiKernel/RndmGenerators.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/IIncidentListener.h"
#include "CxxUtils/checker_macros.h"
// Trk
#include "TrkParameters/TrackParameters.h"
// CLHEP
#include "CLHEP/Random/RandomEngine.h"
// Identifier
#include "Identifier/Identifier.h"
// InDetSimEvent
#include "InDetSimEvent/TRTUncompressedHitCollection.h"

class ITRT_StrawStatusSummaryTool;
class TRT_ID;
class StoreGateSvc;

namespace ISF {
    class ISFParticle;
}

namespace iFatras {
      
  /** 
   @class HitCreatorTRT

   RIOOnTrack creation, starting from intersection on an active surface

   @author Andreas.Salzburger -at- cern.ch 
   */
      
  class ATLAS_NOT_THREAD_SAFE HitCreatorTRT : public extends<AthAlgTool, IHitCreator, IIncidentListener>  // deprecated: ATLASSIM-6020
  {
    public:

      /**Constructor */
      HitCreatorTRT(const std::string&,const std::string&,const IInterface*);
      
      /**Destructor*/
      ~HitCreatorTRT();
      
      /** AlgTool initailize method.*/
      StatusCode initialize();
      
      /** AlgTool finalize method */
      StatusCode finalize();

      /** handle for incident service */
      void handle(const Incident& inc);
     
      /** Record the hit */
      void createSimHit(const ISF::ISFParticle& isp, const Trk::TrackParameters& , double ) const;
      
      /** Return the cluster on Track -- the PrepRawData is contained in this one */       
      const ParametersROT* createHit(const ISF::ISFParticle&, const Trk::TrackParameters&  ) const { return 0; };
      
      /** Return the cluster on Track -- the PrepRawData is contained in this one */       
      const std::vector< ParametersROT >* createHits(const ISF::ISFParticle&, const ParametersLayer& ) const { return 0; }

    private:
        
      /* Incident Service */  
      ServiceHandle<IIncidentSvc>         m_incidentSvc;
              
      /*  SiHit collection and collection helper */
      TRTUncompressedHitCollection             *m_hitColl;                  //!< the sim hit collection
      std::string                               m_collectionName;           //!< name of the collection on storegate

      /** Pointer to the random number generator service */
      ServiceHandle<IAtRndmGenSvc>              m_randomSvc;                    //!< Random Svc  
      std::string                               m_randomEngineName;             //!< Name of the random number stream
      CLHEP::HepRandomEngine*                   m_randomEngine;                 //!< Random Engine 
      
      std::string                               m_trtIdHelperName;              //!< where to find the SCT helper
      const TRT_ID*                             m_trtIdHelper;                  //!< TRT ID helper 
      
      ToolHandle<ITRT_StrawStatusSummaryTool> m_trtStatusSummaryTool;          //!< Handle to TRT conditions service
      bool                                      m_useConditionsSvc;
                                                                                    
        
 
  };

} // end of namespace

#endif 

