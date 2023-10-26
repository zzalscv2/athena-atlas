/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// ExtrapolationEngine.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKEXINTERFACES_EXTRAPOLATIONENGINE_H
#define TRKEXINTERFACES_EXTRAPOLATIONENGINE_H

// Gaudi
#include "AthenaBaseComps/AthAlgTool.h"
#include "AthenaBaseComps/AthCheckedComponent.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"
// Trk
#include "TrkDetDescrInterfaces/ITrackingGeometrySvc.h"
#include "TrkExInterfaces/IExtrapolationEngine.h"
#include "TrkExInterfaces/ExtrapolationMacros.h"
#include "TrkExUtils/ExtrapolationCell.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkNeutralParameters/NeutralParameters.h"
// throw GaudiExceptions where necessary
#include "GaudiKernel/GaudiException.h"

#include "StoreGate/ReadCondHandleKey.h"
#include "TrkGeometry/TrackingGeometry.h"
#include "CxxUtils/checker_macros.h"

namespace Trk {
  
  class IPropagationEngine;    
  class INavigationEngine;        

  /** @class ExtrapolationEngine 
      
      Master extrapolation engine for extrapolation through the TrackingGeometry,
      it delegates the extrapolation to optimised engines, handing over the ExtrapolationCell
      as internal cache.
  
      There are identical interfaces for charged and neutral track parameters.
      Providing a destination surface is optional, if no destination surface is given the extrapolation 
      process can be stopped by other directives, e.g. stopping at a certain path limit, material limit
      or with a change of detector signature.
  
      @author Andreas.Salzburger -at- cern.ch 
  */
  class ExtrapolationEngine : public AthCheckedComponent<AthAlgTool>, virtual public IExtrapolationEngine {
      
      friend class NavigationInitTest;
      
      public:
        /** Constructor */
        ExtrapolationEngine(const std::string&,const std::string&,const IInterface*);
        
        /** Destructor */
        ~ExtrapolationEngine();

        /** AlgTool initialize method */
        StatusCode initialize();
        
        /** AlgTool finalize method */
        StatusCode finalize();
        
        using IExtrapolationEngine::extrapolate;
        
        /** charged extrapolation - public interface */
        virtual ExtrapolationCode extrapolate(ExCellCharged& ecCharged,
                                              const Surface* sf = 0,
                                              const BoundaryCheck& bcheck = true) const;

        /** neutral extrapolation - public interface */
        virtual ExtrapolationCode extrapolate(ExCellNeutral& ecNeutral,
                                              const Surface* sf = 0,
                                              const BoundaryCheck& bcheck = true) const;
                         
                         
        /** define for which GeometrySignature this extrapolator is valid - this is GLOBAL */
        GeometryType geometryType() const;                           
                         
     private:
        /** main loop extrapolation method */
        template <class T> ExtrapolationCode extrapolateT(ExtrapolationCell<T>& eCell,
                                                          const Surface* sf = 0,
                                                          PropDirection dir=alongMomentum,
                                                          const BoundaryCheck& bcheck = true) const;
            
        /** initialization method */                                      
        template <class T>  ExtrapolationCode initNavigation(ExtrapolationCell<T>& eCell,
                                                             const Surface* sf = 0,
                                                             PropDirection dir=alongMomentum) const;
                
                
        //!< return and retrieve
        const TrackingGeometry& trackingGeometry() const;

        void throwFailedToGetTrackingGeomtry() const;
        const TrackingGeometry* retrieveTrackingGeometry(const EventContext& ctx) const {
           SG::ReadCondHandle<TrackingGeometry>  handle(m_trackingGeometryReadKey,ctx);
           if (!handle.isValid()) {
              EX_MSG_FATAL("", "updateGeo", "", "Could not load TrackingGeometry with name '" << m_trackingGeometryReadKey.key() << "'. Aborting." );
              throwFailedToGetTrackingGeomtry();
           }
           return handle.cptr();
        }

        SG::ReadCondHandleKey<TrackingGeometry>   m_trackingGeometryReadKey
           {this, "TrackingGeometryReadKey", "", "Key of the TrackingGeometry conditions data."};

        //!< the tool handle array for static / dense / detached
        ToolHandleArray<IExtrapolationEngine> m_extrapolationEngines{ this, "ExtrapolationEngines", {} };   
        ToolHandle<IPropagationEngine> m_propagationEngine{this, "PropagationEngine", "Trk::PropagationEngine/AtlasStaticPropagation"};         //!< the used propagation engine for navigation initialization
        ToolHandle<INavigationEngine>  m_navigationEngine{this, "NavigationEngine", "Trk::StaticNavigationEngine/AtlasStaticNavigation"};          //!< access to tracking geometry (unique?)
        std::vector<const IExtrapolationEngine*>            m_eeAccessor;                //!< the extrapolation engines for 


        //!< forces a global search for the initialization, allows to switch TrackingGeometries in one job
        bool                                                m_forceSearchInit; 
    
    };

  inline GeometryType  ExtrapolationEngine::geometryType() const 
      { return Trk::Master; }


  inline const Trk::TrackingGeometry& ExtrapolationEngine::trackingGeometry() const {
      const Trk::TrackingGeometry *tracking_geometry = retrieveTrackingGeometry(Gaudi::Hive::currentContext());
      if (!tracking_geometry){
          EX_MSG_FATAL("", "updateGeo", "", "Did not get valid TrackingGeometry. Aborting." );
          throw GaudiException("ExtrapolationEngine", "Problem with TrackingGeometry loading.", StatusCode::FAILURE);
      }
      return *tracking_geometry;
  }
   
   

} // end of namespace

//!< define the templated function    
#include "ExtrapolationEngine.icc"  

#endif // TRKEXINTERFACES_IEXTRAPOLATIONENGINE_H

