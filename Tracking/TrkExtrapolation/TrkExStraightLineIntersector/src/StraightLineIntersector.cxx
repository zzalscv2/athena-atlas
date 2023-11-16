/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

//////////////////////////////////////////////////////////////////////
// provide straight line intersection to a surface
// (useful for abstract interfaces in track/segment fitters)
// (c) ATLAS Tracking software
//////////////////////////////////////////////////////////////////////

#include <cmath>
#include "CLHEP/Units/SystemOfUnits.h"
#include "TrkExStraightLineIntersector/StraightLineIntersector.h"
#include "TrkExUtils/TrackSurfaceIntersection.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkSurfaces/CylinderSurface.h"
#include "TrkSurfaces/DiscSurface.h"
#include "TrkSurfaces/PerigeeSurface.h"
#include "TrkSurfaces/PlaneSurface.h"
#include "TrkSurfaces/StraightLineSurface.h"
#include "TrkSurfaces/Surface.h"

namespace Trk
{

StraightLineIntersector::StraightLineIntersector (const std::string&	type,
						  const std::string&	name, 
						  const IInterface*	parent)
    :	base_class		(type, name, parent),
    m_countExtrapolations	(0)
{
}

StatusCode
StraightLineIntersector::finalize()
{
    ATH_MSG_DEBUG( "finalized after " << m_countExtrapolations << " extrapolations" );
    return StatusCode::SUCCESS;
}

/**IIntersector interface method for general Surface type */
std::optional<Trk::TrackSurfaceIntersection>
StraightLineIntersector::intersectSurface(const Surface&	surface,
					  const TrackSurfaceIntersection&	trackIntersection,
					  const double      	qOverP) const
{

    const auto surfaceType = surface.type();
    if (surfaceType == Trk::SurfaceType::Plane) {
      return intersectPlaneSurface(static_cast<const PlaneSurface&>(surface),
                                   trackIntersection, qOverP);
    }
    if (surfaceType == Trk::SurfaceType::Line) {
      return approachStraightLineSurface(
          static_cast<const StraightLineSurface&>(surface), trackIntersection,
          qOverP);
    }
    if (surfaceType == Trk::SurfaceType::Cylinder) {
      return intersectCylinderSurface(
          static_cast<const CylinderSurface&>(surface), trackIntersection,
          qOverP);
    }
    if (surfaceType == Trk::SurfaceType::Disc) {
      return intersectDiscSurface(static_cast<const DiscSurface&>(surface),
                                  trackIntersection, qOverP);
    }
    if (surfaceType == Trk::SurfaceType::Perigee) {
      return approachPerigeeSurface(static_cast<const PerigeeSurface&>(surface),
                                    trackIntersection, qOverP);
    }

    ATH_MSG_WARNING( " unrecognized Surface" );
    return std::nullopt;
}
                                    
/**IIntersector interface method for specific Surface type : PerigeeSurface */
std::optional<Trk::TrackSurfaceIntersection>
StraightLineIntersector::approachPerigeeSurface(const PerigeeSurface&	surface,
						const TrackSurfaceIntersection&	trackIntersection,
						const double      	/*qOverP*/) const
{
  TrackSurfaceIntersection isect = trackIntersection;
  ++m_countExtrapolations;

  // straight line distance along track to closest approach to line
  const Amg::Vector3D&	lineDirection = surface.transform().rotation().col(2);
  double stepLength = distanceToLine (isect, surface.center(),lineDirection);
  step(isect, stepLength);
    
  return isect;
}
	
/**IIntersector interface method for specific Surface type : StraightLineSurface */
std::optional<Trk::TrackSurfaceIntersection>
StraightLineIntersector::approachStraightLineSurface(const StraightLineSurface& surface,
						     const TrackSurfaceIntersection&	trackIntersection,
						     const double      		/*qOverP*/) const
{
  TrackSurfaceIntersection isect = trackIntersection;
  ++m_countExtrapolations;

  // straight line distance along track to closest approach to line
  const Amg::Vector3D&	lineDirection = surface.transform().rotation().col(2);
  double stepLength = distanceToLine (isect, surface.center(),lineDirection);
  step(isect, stepLength);

  return isect;
}
            
/**IIntersector interface method for specific Surface type : CylinderSurface */
std::optional<Trk::TrackSurfaceIntersection>
StraightLineIntersector::intersectCylinderSurface (const CylinderSurface&	surface,
						   const TrackSurfaceIntersection&		trackIntersection,
						   const double      		/*qOverP*/) const
{
  TrackSurfaceIntersection isect = trackIntersection;
  ++m_countExtrapolations;
  
  // calculate straight line distance along track to intersect with cylinder radius
  double cylinderRadius = surface.globalReferencePoint().perp();
  double stepLength = distanceToCylinder(isect, cylinderRadius);
  step(isect, stepLength);

  return isect;
}

/**IIntersector interface method for specific Surface type : DiscSurface */
std::optional<Trk::TrackSurfaceIntersection>
StraightLineIntersector::intersectDiscSurface (const DiscSurface&	surface,
					       const TrackSurfaceIntersection&	trackIntersection,
					       const double      	/*qOverP*/) const
{
  TrackSurfaceIntersection isect = trackIntersection;
  ++m_countExtrapolations;

  // straight line distance along track to intersect with disc
  double stepLength = distanceToDisc(isect, surface.center().z());
  step(isect, stepLength);
  
  return isect;
}

/**IIntersector interface method for specific Surface type : PlaneSurface */
std::optional<Trk::TrackSurfaceIntersection>
StraightLineIntersector::intersectPlaneSurface(const PlaneSurface&	surface,
					       const TrackSurfaceIntersection&	trackIntersection,
					       const double      	/*qOverP*/) const
{
  TrackSurfaceIntersection isect = trackIntersection;
  ++m_countExtrapolations;

  // straight line distance along track to intersect with plane
  double stepLength = distanceToPlane (isect, surface.center(),surface.normal());
  step(isect, stepLength);
  stepLength = distanceToPlane (isect, surface.center(),surface.normal());

  return isect;
}

} // end of namespace
