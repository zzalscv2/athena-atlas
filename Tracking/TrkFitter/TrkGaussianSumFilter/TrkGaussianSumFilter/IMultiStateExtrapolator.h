/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file   IMultiStateExtrapolator.h
 * @date   Tuesday 25th January 2005
 * @author Anthony Morley, Christos Anastopoulos
 * Abstract base class for extrapolation of a MultiComponentState
 */


#ifndef TrkIMultiStateExtrapolator_H
#define TrkIMultiStateExtrapolator_H

#include "TrkMultiComponentStateOnSurface/MultiComponentState.h"
#include "TrkGaussianSumFilter/IMultiStateMaterialEffects.h"

#include "TrkEventPrimitives/ParticleHypothesis.h"
#include "TrkEventPrimitives/PropDirection.h"

#include "TrkParameters/TrackParameters.h"
#include "TrkSurfaces/BoundaryCheck.h"

#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/IAlgTool.h"

#include <memory>
#include <vector>

namespace Trk {

class IPropagator;
class Surface;
class Track;
class TrackingVolume;
class TrackStateOnSurface;

/** @struct StateAtBoundarySurface
  - Structure to contain information about a state at the interface between
  tracking volumes
  */

struct StateAtBoundarySurface
{

  /** Data members */
  const MultiComponentState* stateAtBoundary = nullptr;
  const TrackParameters* navigationParameters = nullptr;
  const TrackingVolume* trackingVolume = nullptr;


  /** Update State at Boundary Surface Information */
  void updateBoundaryInformation(const MultiComponentState* boundaryState,
                                 const TrackParameters* navParameters,
                                 const TrackingVolume* nextVolume)
  {
    stateAtBoundary = boundaryState;
    navigationParameters = navParameters;
    trackingVolume = nextVolume;
  }
};


static const InterfaceID IID_IMultiStateExtrapolator("IMultiStateExtrapolator",
                                                     1,
                                                     0);

class IMultiStateExtrapolator : virtual public IAlgTool
{
public:
  /** @brief MultiStateExtrapolator cache class.
   *  This object holds information regarding the state of the extrpolation process 
   *  as well as a large store for  material effects properties.
   *  
   *  The is owned by the tools that call the extrapolatator (Currently the GSFSmoother or Forward fitter)
   *  and it not exposed to the caller of the track fitter.
   *                 */
  struct Cache
  {
    bool  m_recall = false;                   //!< Flag the recall solution
    const Surface* m_recallSurface = nullptr; //!< Surface for recall
    const Layer* m_recallLayer = nullptr;     //!< Layer for recall
    const TrackingVolume*
      m_recallTrackingVolume = nullptr; //!< Tracking volume for recall
    StateAtBoundarySurface
      m_stateAtBoundarySurface; //!< Instance of structure describing the state
                                //!< at a boundary of tracking volumes
    std::unique_ptr<std::vector<const Trk::TrackStateOnSurface*>> m_matstates;
    std::vector<std::unique_ptr<const MultiComponentState>>
      m_mcsGarbageBin; //!< Garbage bin for MultiComponentState objects
    std::vector<std::unique_ptr<const TrackParameters>>
      m_tpGarbageBin; //!< Garbage bin for TrackParameter objects

    std::vector<Trk::IMultiStateMaterialEffects::Cache> 
      m_materialEffectsCaches; //!<  Large cache for material effects


    Cache()
    {
      m_materialEffectsCaches.reserve(72);
    }

    void reset(){
      m_recall = false;
      m_recallSurface = nullptr;
      m_recallLayer = nullptr;
      m_recallTrackingVolume = nullptr;
      m_matstates.reset(nullptr);
      m_stateAtBoundarySurface.updateBoundaryInformation(nullptr,nullptr,nullptr);
      m_mcsGarbageBin.clear();
      m_tpGarbageBin.clear();
      m_materialEffectsCaches.clear();
    };
  };
 
 /** Virtual destructor */
  virtual ~IMultiStateExtrapolator() = default;

  /** AlgTool interface method */
  static const InterfaceID& interfaceID()
  {
    return IID_IMultiStateExtrapolator;
  };

  /** Configured AlgTool extrapolation method (1) */
  virtual MultiComponentState extrapolate(
    const EventContext& ctx,
    Cache&,
    const MultiComponentState&,
    const Surface&,
    PropDirection direction = anyDirection,
    const BoundaryCheck& boundaryCheck = true,
    ParticleHypothesis particleHypothesis = nonInteracting) const = 0;

  /** Configured AlgTool extrapolation without material effects method (2) */
  virtual MultiComponentState extrapolateDirectly(
    const EventContext& ctx,
    const MultiComponentState&,
    const Surface&,
    PropDirection direction = anyDirection,
    const BoundaryCheck& boundaryCheck = true,
    ParticleHypothesis particleHypothesis = nonInteracting) const = 0;

  virtual std::unique_ptr<std::vector<const Trk::TrackStateOnSurface*>>
  extrapolateM(const EventContext& ctx,
               const MultiComponentState&,
               const Surface&,
               PropDirection dir = anyDirection,
               const BoundaryCheck& bcheck = true,
               ParticleHypothesis particle = nonInteracting) const = 0;
};

} // end trk namespace

#endif
