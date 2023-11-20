/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// INavigator.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKDETDESCRINTERFACES_INAVIGATOR_H
#define TRKDETDESCRINTERFACES_INAVIGATOR_H

// Gaudi
#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/ThreadLocalContext.h"
// GeoPrimitives
#include "GeoPrimitives/GeoPrimitives.h"
// Trk
#include "TrkEventPrimitives/ParticleHypothesis.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkVolumes/BoundarySurface.h"
#include "TrkVolumes/BoundarySurfaceFace.h"
// STL
#include <utility>

namespace Trk {

class IPropagator;
class Surface;
class Track;
// class TrackParameters;
class TrackingVolume;
class TrackingGeometry;
class DetachedTrackingVolume;

/**useful struct for a single navigation cell*/
struct NavigationCell
{
  //Not Ownig ptr to volume from geometry
  const TrackingVolume* nextVolume;
  //Owning ptr for parameters
  std::unique_ptr<TrackParameters> parametersOnBoundary;
  BoundarySurfaceFace exitFace;
  /** Constructor */
  NavigationCell(const TrackingVolume* nVol,
                 std::unique_ptr<TrackParameters> lPar,
                 BoundarySurfaceFace face = undefinedFace)
    : nextVolume(nVol)
    , parametersOnBoundary(std::move(lPar))
    , exitFace(face)
  {}
};

typedef std::pair<int, const NavigationCell*> IdNavigationCell;

/** Interface ID for INavigator */
static const InterfaceID IID_INavigator("INavigator", 1, 0);

/** @class INavigator
   Interface class for the navigation AlgTool, it inherits from IAlgTool
   Detailed information about private members and member functions can be
   found in the actual implementation class Trk::Navigator which inherits from
   this one.

   @author Andreas.Salzburger@cern.ch
  */
class INavigator : virtual public IAlgTool
{
public:
  /**Virtual destructor*/
  virtual ~INavigator() {}

  /** AlgTool and IAlgTool interface methods */
  static const InterfaceID& interfaceID() { return IID_INavigator; }

  /** INavigator interface method - returns the TrackingGeometry used for
   * navigation */
  virtual const TrackingGeometry* trackingGeometry(
    const EventContext& ctx) const = 0;

  /** INavigator interface method - global search for the Volume one is in */
  virtual const TrackingVolume* volume(const EventContext& ctx,
                                       const Amg::Vector3D& gp) const = 0;

  /** INavigator interface method - forward hightes TrackingVolume */
  virtual const TrackingVolume* highestVolume(
    const EventContext& ctx) const = 0;

  /** INavigator interface method - getting the closest TrackParameters from a
   * Track to a Surface */
  virtual const TrackParameters* closestParameters(
    const Track& trk,
    const Surface& sf) const = 0;

  /** INavigator method to resolve navigation at boundary */
  virtual bool atVolumeBoundary(const Trk::TrackParameters* parms,
                                const Trk::TrackingVolume* vol,
                                Trk::PropDirection dir,
                                const Trk::TrackingVolume*& nextVol,
                                double tol) const = 0;

  /** INavigator interface method - getting the next BoundarySurface not knowing
   * the Volume*/
  virtual const BoundarySurface<TrackingVolume>* nextBoundarySurface(
    const EventContext& ctx,
    const IPropagator& prop,
    const TrackParameters& parms,
    PropDirection dir) const = 0;

  /** INavigator interface method - getting the next BoundarySurface when
   * knowing the Volume*/
  virtual const BoundarySurface<TrackingVolume>* nextBoundarySurface(
    const EventContext& ctx,
    const IPropagator& prop,
    const TrackParameters& parms,
    PropDirection dir,
    const TrackingVolume& vol) const = 0;

  /** INavigator interface method - - getting the next Volume and the parameter
   * for the next Navigation */
  virtual NavigationCell nextTrackingVolume(
    const EventContext& ctx,
    const IPropagator& prop,
    const TrackParameters& parms,
    PropDirection dir,
    const TrackingVolume& vol) const = 0;


};
} // end of namespace

#endif // TRKDETDESCRINTERFACES_INAVIGATOR_H

