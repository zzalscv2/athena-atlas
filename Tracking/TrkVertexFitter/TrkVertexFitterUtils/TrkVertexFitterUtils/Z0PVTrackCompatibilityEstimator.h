/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRK_Z0PVTRACKCOMPATIBILITYESTIMATOR_H
#define TRK_Z0PVTRACKCOMPATIBILITYESTIMATOR_H


#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkVertexFitterInterfaces/ITrackToVertexIPEstimator.h"
#include "TrkVertexFitterInterfaces/IPVTrackCompatibilityEstimator.h"


class VxContainer;

namespace Trk{

  class Track;
  class TrackParticleBase;


/**
 * Z0PVTrackCompatibilityEstimator.h
 * 
 * Assigns tracks to the primary vertex based on their z0 significance. 
 * Tracks that are close to a pileup vertex are not assigned to the 
 * primary vertex.
 *
 * ruwiedel@physik.uni-bonn.de
 *
 */

  
  class Z0PVTrackCompatibilityEstimator : public AthAlgTool, virtual public IPVTrackCompatibilityEstimator{
    
  public:
    Z0PVTrackCompatibilityEstimator(const std::string& type, const std::string& name, const IInterface* parent); 
    virtual ~Z0PVTrackCompatibilityEstimator();
    
    virtual StatusCode initialize();
    virtual StatusCode finalize();
    
    static const InterfaceID& interfaceID();
    virtual StatusCode queryInterface( const InterfaceID& riid, void** ppvIf ) ;
    
    bool isCompatible(const Trk::Track* track,
		      const VxContainer* pvContainer, unsigned int pvIndex = 0) const;
    
    bool isCompatible(const Trk::TrackParticleBase* track,
		      const VxContainer* pvContainer, unsigned int pvIndex = 0) const;
    
    bool isCompatible(const TrackParameters* track,
		      const VxContainer* pvContainer, unsigned int pvIndex = 0) const;
    
  private:
    ToolHandle< Trk::ITrackToVertexIPEstimator > m_ipEstimator;
    
    double m_z0SignificanceMax;
    
  };
  

  inline const InterfaceID& Z0PVTrackCompatibilityEstimator::interfaceID(){
    static const InterfaceID IID_Z0PVTrackCompatibilityEstimator("Z0PVTrackCompatibilityEstimator", 1, 0); 
    return IID_Z0PVTrackCompatibilityEstimator;
  }

}

#endif 
