/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// BoundaryPlaneSurface.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKVOLUMES_BOUNDARYPLANESURFACE_H
#define TRKVOLUMES_BOUNDARYPLANESURFACE_H

// Trk
#include "GeoPrimitives/GeoPrimitives.h"
#include "TrkEventPrimitives/PropDirection.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkSurfaces/PlaneSurface.h"
#include "TrkVolumes/BoundarySurface.h"

namespace Trk {

// class TrackParameters;
class Volume;

/**
 @class BoundaryPlaneSurface

 BoundaryPlaneSurface description inside the tracking realm,
 it extends the PlaneSurface description to make a surface being a boundary of a
 Trk::Volume (used for all volume shapes).
 It inherits from BoundarySurface to get the interface of boundaries.

 @author Andreas.Salzburger@cern.ch
 @author Christos Anastopoulos (Athena  MT modifications)
*/

template <class Tvol>
class BoundaryPlaneSurface final : virtual public BoundarySurface<Tvol>,
                                   public PlaneSurface {
  /** typedef the BinnedArray */
  typedef BinnedArray<Tvol> VolumeArray;

 public:
  /** Default Constructor - needed for pool and inherited classes */
  BoundaryPlaneSurface() = default;

  /** Copy constructor */
  BoundaryPlaneSurface(const BoundaryPlaneSurface<Tvol>& bps) = default;

  /**Assignment operator*/
  BoundaryPlaneSurface& operator=(const BoundaryPlaneSurface& vol) = default;

  /**Virtual Destructor*/
  virtual ~BoundaryPlaneSurface() = default;

  /** Constructor for a Boundary with exact two Volumes attached to it*/
  BoundaryPlaneSurface(const Tvol* inside, const Tvol* outside,
                       const PlaneSurface& psf)
      : BoundarySurface<Tvol>(inside, outside), PlaneSurface(psf) {}

  /** Constructor for a Boundary with two VolumeArrays attached to it*/
  BoundaryPlaneSurface(SharedObject<VolumeArray> insideArray,
                       SharedObject<VolumeArray> outsideArray,
                       const PlaneSurface& psf)
      : BoundarySurface<Tvol>(insideArray, outsideArray), PlaneSurface(psf) {}

  /** Copy constructor with a shift */
  BoundaryPlaneSurface(const Tvol* inside, const Tvol* outside,
                       const PlaneSurface& psf, const Amg::Transform3D& tr)
      : BoundarySurface<Tvol>(inside, outside), PlaneSurface(psf, tr) {}

  /** Get the next Volume depending on the TrackParameters and the requested
   direction, gives back 0 if there's no volume attached to the requested
   direction
   */
  virtual const Tvol* attachedVolume(const TrackParameters& parms,
                                     PropDirection dir) const override final;

  /** Get the next Volume depending on GlobalPosition, GlobalMomentum, dir
   on the TrackParameters and the requested direction */
  virtual const Tvol* attachedVolume(const Amg::Vector3D& pos,
                                     const Amg::Vector3D& mom,
                                     PropDirection dir) const override final;

  /** The Surface Representation of this */
  virtual const Surface& surfaceRepresentation() const override final;
  virtual Surface& surfaceRepresentation() override final;
};

// Hash include inline functions
#include "TrkVolumes/BoundaryPlaneSurface.icc"

}  // end of namespace Trk

#endif  // TRKVOLUMES_BOUNDARYPLANESURFACE_H

