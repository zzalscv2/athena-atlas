/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// SimHitkCreatorID.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef FATRASTOOLSID_SIMHITCREATORID_H
#define FATRASTOOLSID_SIMHITCREATORID_H

// GaudiKernel & Athena
#include "AthenaBaseComps/AthAlgTool.h"
#include "AthenaKernel/IAtRndmGenSvc.h"
#include "CxxUtils/checker_macros.h"
#include "GaudiKernel/RndmGenerators.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"
#include "AthContainers/DataVector.h"

// Fatras
#include "ISF_FatrasInterfaces/ISimHitCreator.h"

// STL
#include <utility>

class AtlasDetectorID;

namespace iFatras {
  
  class IHitCreator;
      
  /**
     @class SimHitCreatorID
     
     Standard ATLAS hit creator for the Inner Detector,
     uses the ID helper to call the appropriate HitCreator
     
     @author sarka.todorova@cern.ch
  */
  
  class ATLAS_NOT_THREAD_SAFE SimHitCreatorID: public extends<AthAlgTool, ISimHitCreator>  // deprecated: ATLASSIM-6020
  {
  public:
    /**Constructor */
    SimHitCreatorID(const std::string&,const std::string&,const IInterface*);
    
    /**Destructor*/
    ~SimHitCreatorID();
    
    /** AlgTool initailize method.*/
    StatusCode initialize();
    
    /** AlgTool finalize method */
    StatusCode finalize();
    
    /** Loop over the hits and call the hit creator,
        provide the ISF::StackParticle to register the hits */
    void createHits(const ISF::ISFParticle& isp,
                    const std::vector<Trk::HitInfo>& hits) const;
    
  private:   
    /** Cluster creator AlgTool */
    ToolHandle<iFatras::IHitCreator>        m_pixelHitCreator;
    ToolHandle<iFatras::IHitCreator>        m_sctHitCreator;
    ToolHandle<iFatras::IHitCreator>        m_trtHitCreator;
    /** Used to find out the sub-det from */
    std::string                             m_idHelperName;             
    const AtlasDetectorID*                  m_idHelper;
             
  };
} // end of iFatras namespace

#endif 
