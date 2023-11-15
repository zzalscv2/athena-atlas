/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// SolenoidalIntersector.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#include "GaudiKernel/SystemOfUnits.h"
#include "TrkExSolenoidalIntersector/SolenoidalIntersector.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkSurfaces/CylinderSurface.h"
#include "TrkSurfaces/DiscSurface.h"
#include "TrkSurfaces/PerigeeSurface.h"
#include "TrkSurfaces/PlaneSurface.h"
#include "TrkSurfaces/StraightLineSurface.h"
#include "TrkSurfaces/Surface.h"

namespace Trk
{

SolenoidalIntersector::Constants::Constants (const SolenoidParametrization& solpar,
                                             const TrackSurfaceIntersection& trackTrackSurfaceIntersection,
                                             const double qOverP)
  : m_sinTheta (trackTrackSurfaceIntersection.direction().perp()),
    m_oneOverSinTheta (1./m_sinTheta),
    m_cotTheta (trackTrackSurfaceIntersection.direction().z() * m_oneOverSinTheta),
    m_qOverPt (qOverP * m_oneOverSinTheta),
    m_solPar (solpar),
    m_lastPosition (trackTrackSurfaceIntersection.position()),
    m_solParams (solpar,
                 trackTrackSurfaceIntersection.position().perp(),
                 trackTrackSurfaceIntersection.position().z(),
                 m_cotTheta)
{
}

SolenoidalIntersector::SolenoidalIntersector(const std::string& type,
                                             const std::string& name,
                                             const IInterface* parent)
    : base_class(type, name, parent),
      m_rungeKuttaIntersector(
          "Trk::RungeKuttaIntersector/RungeKuttaIntersector", this),
      m_deltaPhiTolerance(0.01),  // upper limit for small angle approx
      m_surfaceTolerance(2.0 * Gaudi::Units::micrometer),
      m_countExtrapolations(0),
      m_countRKSwitches(0) {
  declareProperty("RungeKuttaIntersector", m_rungeKuttaIntersector);
  declareProperty("SurfaceTolerance", m_surfaceTolerance);
}

StatusCode
SolenoidalIntersector::initialize()
{
    ATH_CHECK( m_solenoidParametrizationKey.initialize() );
    ATH_CHECK(m_rungeKuttaIntersector.retrieve());
    return StatusCode::SUCCESS;
}

StatusCode
SolenoidalIntersector::finalize()
{
    ATH_MSG_DEBUG( "finalized after " << m_countExtrapolations << " extrapolations with "
		  << m_countRKSwitches << " switches to RK integration");

    return StatusCode::SUCCESS;
}


/**IIntersector interface method for general Surface type */
std::optional<Trk::TrackSurfaceIntersection>
SolenoidalIntersector::intersectSurface(const Surface&		surface,
					const TrackSurfaceIntersection&	trackIntersection,
					const double      	qOverP) const
{
    const PlaneSurface* plane			= dynamic_cast<const PlaneSurface*>(&surface);
    if (plane)		return intersectPlaneSurface(*plane,trackIntersection,qOverP);
    
    const StraightLineSurface* straightLine	= dynamic_cast<const StraightLineSurface*>(&surface);
    if (straightLine)	return m_rungeKuttaIntersector->approachStraightLineSurface
			    (*straightLine, trackIntersection, qOverP);
    
    const CylinderSurface* cylinder		= dynamic_cast<const CylinderSurface*>(&surface);
    if (cylinder)	return intersectCylinderSurface(*cylinder,trackIntersection,qOverP);

    const DiscSurface* disc			= dynamic_cast<const DiscSurface*>(&surface);
    if (disc)		return intersectDiscSurface(*disc,trackIntersection,qOverP);

    const PerigeeSurface* perigee		= dynamic_cast<const PerigeeSurface*>(&surface);
    if (perigee)	return m_rungeKuttaIntersector->approachPerigeeSurface
			    (*perigee, trackIntersection, qOverP);
    
    ATH_MSG_WARNING( " unrecognized Surface" );
    return std::nullopt;
}
	           
/**IIntersector interface method for specific Surface type : PerigeeSurface */
std::optional<Trk::TrackSurfaceIntersection>
SolenoidalIntersector::approachPerigeeSurface(const PerigeeSurface&	surface,
					      const TrackSurfaceIntersection&	trackIntersection,
					      const double      	qOverP) const
{ return m_rungeKuttaIntersector->approachPerigeeSurface(surface, trackIntersection, qOverP); }
	
/**IIntersector interface method for specific Surface type : StraightLineSurface */
std::optional<Trk::TrackSurfaceIntersection>
SolenoidalIntersector::approachStraightLineSurface(const StraightLineSurface& surface,
						   const TrackSurfaceIntersection&	trackIntersection,
						   const double      	qOverP) const
{ return m_rungeKuttaIntersector->approachStraightLineSurface(surface, trackIntersection, qOverP); }
              
/**IIntersector interface method for specific Surface type : CylinderSurface */
std::optional<Trk::TrackSurfaceIntersection>
SolenoidalIntersector::intersectCylinderSurface(const CylinderSurface&	surface,
						const TrackSurfaceIntersection&	trackIntersection,
						const double      	qOverP) const
{
    const SolenoidParametrization* solenoidParametrization =
      getSolenoidParametrization();

    double endRadius		= surface.globalReferencePoint().perp();
    if (!solenoidParametrization || endRadius > solenoidParametrization->maximumR())
      return m_rungeKuttaIntersector->intersectCylinderSurface(surface, trackIntersection, qOverP);

    Constants* com = nullptr;
    std::optional<TrackSurfaceIntersection> isect = newIntersection(
        trackIntersection, *solenoidParametrization, qOverP, com);
    ++m_countExtrapolations;

    double radius2 = isect->position().perp2();
    if (std::abs(endRadius - sqrt(radius2)) > m_surfaceTolerance
        && ! extrapolateToR(*isect, radius2, *com, endRadius))	return std::nullopt;

    return intersection(std::move(*isect), *com, surface);
}

/**IIntersector interface method for specific Surface type : DiscSurface */
std::optional<Trk::TrackSurfaceIntersection>
SolenoidalIntersector::intersectDiscSurface (const DiscSurface&		surface,
                                             const TrackSurfaceIntersection&	trackIntersection,
                                             const double      		qOverP) const
{
    const SolenoidParametrization* solenoidParametrization =
      getSolenoidParametrization();

    double endZ	= surface.center().z();
    if (!solenoidParametrization || std::abs(endZ) > solenoidParametrization->maximumZ())
      return m_rungeKuttaIntersector->intersectDiscSurface(surface, trackIntersection, qOverP);

    Constants* com = nullptr;
    std::optional<TrackSurfaceIntersection> isect =
      newIntersection (trackIntersection,
                       *solenoidParametrization,
                       qOverP,
                       com);
                                                             
    ++m_countExtrapolations;

    if (std::abs(endZ -trackIntersection.position().z()) > m_surfaceTolerance
        && ! extrapolateToZ(*isect, *com, endZ))
    {
      return std::nullopt;
    }
    
    return intersection(std::move(*isect), *com, surface);
}

/**IIntersector interface method for specific Surface type : PlaneSurface */
std::optional<Trk::TrackSurfaceIntersection>
SolenoidalIntersector::intersectPlaneSurface(const PlaneSurface&	surface,
					     const TrackSurfaceIntersection&	trackIntersection,
					     const double      		qOverP) const
{
    const SolenoidParametrization* solenoidParametrization =
      getSolenoidParametrization();

    if (!solenoidParametrization ||
        std::abs(surface.center().z())	> solenoidParametrization->maximumZ()
    	|| surface.center().perp()	> solenoidParametrization->maximumR())
      return m_rungeKuttaIntersector->intersectPlaneSurface(surface, trackIntersection, qOverP);

    Constants* com = nullptr;
    std::optional<TrackSurfaceIntersection> isect =
      newIntersection (trackIntersection,
                       *solenoidParametrization,
                       qOverP,
                       com);
    Amg::Vector3D& pos = isect->position();
    Amg::Vector3D& dir = isect->direction();
    ++m_countExtrapolations;
    double radius2 = pos.perp2();

    // step until sufficiently near to plane surface
    // this gives wrong answer!
    // DistanceSolution solution	= surface.straightLineDistanceEstimate(pos,dir);
    // if (! solution.signedDistance())						return 0;
    // double distance		= solution.currentDistance(true);

    int numberSteps	= 0;
    double dot		= surface.normal().dot(dir);
    double offset	= surface.normal().dot(surface.center() - pos);

    while (std::abs(offset) > m_surfaceTolerance * std::abs(dot)) {
      // take care if grazing incidence - quit if looper
      if (std::abs(dot) < 0.0001)
        return std::nullopt;
      double distance = offset / dot;

      // extrapolate
      if (com->m_sinTheta < 0.9) {
        if (!extrapolateToZ(*isect, *com, pos.z() + distance * dir.z()))
          return std::nullopt;
        radius2 = pos.perp2();
      } else {
        if (!extrapolateToR(*isect, radius2, *com,
                            (pos + distance * dir).perp()))
          return std::nullopt;
      }

      // check we are getting closer to the plane, switch to RK in case of
      // difficulty
      dot = surface.normal().dot(dir);
      offset = surface.normal().dot(surface.center() - pos);
      if (std::abs(offset) > m_surfaceTolerance * std::abs(dot) &&
          (++numberSteps > 5 ||
           std::abs(offset) > 0.5 * std::abs(distance * dot))) {
        ++m_countRKSwitches;
        ATH_MSG_DEBUG(" switch to RK after "
                      << numberSteps << " steps at offset " << offset
                      << "   dot " << surface.normal().dot(dir));

        return m_rungeKuttaIntersector->intersectPlaneSurface(
            surface, trackIntersection, qOverP);
      }
    };

    return intersection(std::move(*isect), *com, surface);
}

/**IIntersector interface method to check validity of parametrization within extrapolation range */
bool
SolenoidalIntersector::isValid (Amg::Vector3D startPosition,
                                Amg::Vector3D endPosition) const
{
    const SolenoidParametrization* solenoidParametrization =
      getSolenoidParametrization();

    // check cylinder bounds for valid parametrization
    return solenoidParametrization &&

        std::abs(endPosition.z())	< solenoidParametrization->maximumZ()

	&& endPosition.perp()		< solenoidParametrization->maximumR()

	&& getSolenoidParametrization()->validOrigin(startPosition);
}

// private methods


bool
SolenoidalIntersector::extrapolateToR(TrackSurfaceIntersection& isect,
                                      double& radius2,
                                      Constants&        com,
                                      const double 	endR) const
{
    Amg::Vector3D& pos = isect.position();
    Amg::Vector3D& dir = isect.direction();

    double fieldComponent =
        com.m_solPar.fieldComponent(pos.z(), com.m_solParams);
    double curvature = fieldComponent * com.m_qOverPt;
    double arcLength = linearArcLength(isect, com, radius2, endR);
    if (std::abs(arcLength * curvature) > m_deltaPhiTolerance) {
      double radiusOfCurvature = 1. / curvature;
      double xCentre =
          pos.x() - radiusOfCurvature * dir.y() * com.m_oneOverSinTheta;
      double yCentre =
          pos.y() + radiusOfCurvature * dir.x() * com.m_oneOverSinTheta;
      double cosPhi;
      double sinPhi;
      arcLength =
          circularArcLength(endR, radiusOfCurvature, xCentre, yCentre,
                            dir.x() * com.m_oneOverSinTheta,
                            dir.y() * com.m_oneOverSinTheta, cosPhi, sinPhi);
      if (std::abs(arcLength * com.m_cotTheta) < m_surfaceTolerance) {
          pos.x() = xCentre + radiusOfCurvature * sinPhi;
          pos.y() = yCentre - radiusOfCurvature * cosPhi;
          pos.z() += arcLength * com.m_cotTheta;
          radius2 = endR * endR;
          dir.x() = cosPhi * com.m_sinTheta;
          dir.y() = sinPhi * com.m_sinTheta;
          isect.pathlength() += arcLength * com.m_oneOverSinTheta;
          arcLength = 0.;
      }
    }

    double deltaZ = arcLength * com.m_cotTheta;
    if (std::abs(deltaZ) < m_surfaceTolerance)  // avoid FPE with constant
                                                // curvature parabolic approx
    {
      double cosPhi = dir.x() * com.m_oneOverSinTheta;
      double sinPhi = dir.y() * com.m_oneOverSinTheta;
      if (std::abs(arcLength) > m_surfaceTolerance) {
          double sinDPhi = 0.5 * arcLength * curvature;
          double cosDPhi =
              1. - 0.5 * sinDPhi * sinDPhi * (1.0 + 0.25 * sinDPhi * sinDPhi);
          double temp = cosPhi;
          cosPhi = temp * cosDPhi - sinPhi * sinDPhi;
          sinPhi = temp * sinDPhi + sinPhi * cosDPhi;
          dir.x() = (cosPhi * cosDPhi - sinPhi * sinDPhi) * com.m_sinTheta;
          dir.y() = (sinPhi * cosDPhi + cosPhi * sinDPhi) * com.m_sinTheta;
      }

      pos.x() += arcLength * cosPhi;
      pos.y() += arcLength * sinPhi;
      pos.z() += arcLength * com.m_cotTheta;
      radius2 = endR * endR;
      isect.pathlength() += arcLength * com.m_oneOverSinTheta;
    } else {
      extrapolateToZ(isect, com, pos.z() + deltaZ);
      radius2 = pos.perp2();
      if (std::abs(endR - std::sqrt(radius2)) > m_surfaceTolerance) {
          deltaZ = linearArcLength(isect, com, radius2, endR) * com.m_cotTheta;
          extrapolateToZ(isect, com, pos.z() + deltaZ);
          radius2 = pos.perp2();
      }
    }

    return true;
}

bool
SolenoidalIntersector::extrapolateToZ(TrackSurfaceIntersection& isect,
                                      Constants&   com,
                                      const double endZ) 
{
    Amg::Vector3D& pos = isect.position();
    Amg::Vector3D& dir = isect.direction();

    double firstIntegral = 0.;
    double secondIntegral = 0.;
    com.m_solPar.fieldIntegrals(firstIntegral, secondIntegral, pos.z(), endZ,
                                com.m_solParams);
    double DFiMax = 0.1;
    double cosPhi = dir.x() * com.m_oneOverSinTheta;
    double sinPhi = dir.y() * com.m_oneOverSinTheta;
    double cosBeta;
    double sinBeta;
    double deltaPhi2 =
        secondIntegral * com.m_qOverPt / std::abs(com.m_cotTheta);
    if (std::abs(deltaPhi2) < DFiMax) {
      double deltaPhi2Squared = deltaPhi2 * deltaPhi2;
      sinBeta = 1. - 0.166667 * deltaPhi2Squared;
      cosBeta = -0.5 * deltaPhi2 * (1. - 0.083333 * deltaPhi2Squared);
    } else if (2. * std::abs(deltaPhi2) < M_PI) {
      sinBeta = std::sin(deltaPhi2) / deltaPhi2;
      cosBeta = (std::cos(deltaPhi2) - 1.) / deltaPhi2;
    } else {
      return false;
    }

    double radialDistance = (endZ - pos.z()) / com.m_cotTheta;
    pos.x() += radialDistance * (sinPhi * cosBeta + cosPhi * sinBeta);
    pos.y() += radialDistance * (sinPhi * sinBeta - cosPhi * cosBeta);
    pos.z() = endZ;
    isect.pathlength() += radialDistance * com.m_oneOverSinTheta;

    double cosAlpha;
    double sinAlpha;
    double deltaPhi1 = firstIntegral * com.m_qOverPt / std::abs(com.m_cotTheta);
    if (std::abs(deltaPhi1) < DFiMax) {
      double deltaPhi1Squared = deltaPhi1 * deltaPhi1;
      sinAlpha = deltaPhi1 * (1. - 0.166667 * deltaPhi1Squared);
      cosAlpha =
          1. - 0.5 * deltaPhi1Squared * (1. - 0.083333 * deltaPhi1Squared);
    } else {
      sinAlpha = std::sin(deltaPhi1);
      cosAlpha = std::cos(deltaPhi1);
    }
    dir.x() = (cosPhi * cosAlpha - sinPhi * sinAlpha) * com.m_sinTheta;
    dir.y() = (sinPhi * cosAlpha + cosPhi * sinAlpha) * com.m_sinTheta;
    return true;
}

std::optional<TrackSurfaceIntersection>
SolenoidalIntersector::newIntersection (const TrackSurfaceIntersection& isect,
                                        const SolenoidParametrization& solpar,
                                        const double qOverP,
                                        Constants*& com) 
{
  const TrackSurfaceIntersection::IIntersectionCache* oldCache = isect.cache();
  std::unique_ptr<Constants> cache;
  const Amg::Vector3D* lastPosition = nullptr;
  if (oldCache && typeid(*oldCache) == typeid(Constants)) {
    cache = std::make_unique<Constants> (*static_cast<const Constants*> (oldCache));
    lastPosition = &cache->m_lastPosition;
  }
  else {
    cache = std::make_unique<Constants> (solpar, isect, qOverP);
  }

  com = cache.get();
  auto newIsect =
      std::make_optional<TrackSurfaceIntersection>(isect, std::move(cache));
  if (lastPosition) {
    newIsect->position() = *lastPosition;
  }
  return newIsect;
}

void SolenoidalIntersector::throwMissingCondData() const {
   std::stringstream msg;
   msg << "Invalid read handle for SolenoidParametrization  " << m_solenoidParametrizationKey.key();
   throw std::logic_error(msg.str());
}

} // end of namespace
