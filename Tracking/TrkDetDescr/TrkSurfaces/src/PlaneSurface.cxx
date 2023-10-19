/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// PlaneSurface.cxx, (c) ATLAS Detector Software
///////////////////////////////////////////////////////////////////

// Trk
#include "TrkSurfaces/PlaneSurface.h"
#include "TrkEventPrimitives/CurvilinearUVT.h"
#include "TrkEventPrimitives/LocalDirection.h"
#include "TrkSurfaces/AnnulusBounds.h"
#include "TrkSurfaces/DiamondBounds.h"
#include "TrkSurfaces/EllipseBounds.h"
#include "TrkSurfaces/NoBounds.h"
#include "TrkSurfaces/RectangleBounds.h"
#include "TrkSurfaces/RotatedTrapezoidBounds.h"
#include "TrkSurfaces/TrapezoidBounds.h"
#include "TrkSurfaces/TriangleBounds.h"
// Identifier
#include "Identifier/Identifier.h"
// Gaudi
#include "GaudiKernel/MsgStream.h"
//CxxUtils
#include "CxxUtils/sincos.h"
#include "CxxUtils/inline_hints.h"
// STD
#include <iomanip>
#include <iostream>
#include <cmath>

const Trk::NoBounds Trk::PlaneSurface::s_boundless;

// default constructor
Trk::PlaneSurface::PlaneSurface()
  : Trk::Surface()
  , m_bounds(nullptr)
{}


// copy constructor with shift
Trk::PlaneSurface::PlaneSurface(const PlaneSurface& psf, const Amg::Transform3D& transf)
  : Trk::Surface(psf, transf)
  , m_bounds(psf.m_bounds)
{}

#if defined(FLATTEN)
// We compile this function with optimization, even in debug builds; otherwise,
// the heavy use of Eigen makes it too slow.  However, from here we may call
// to out-of-line Eigen code that is linked from other DSOs; in that case,
// it would not be optimized.  Avoid this by forcing all Eigen code
// to be inlined here if possible.
ATH_FLATTEN
#endif
// constructor from CurvilinearUVT
Trk::PlaneSurface::PlaneSurface(const Amg::Vector3D& position, const CurvilinearUVT& curvUVT)
  : Trk::Surface()
  , m_bounds(nullptr) // curvilinear surfaces are boundless
{
  Amg::Translation3D curvilinearTranslation(position.x(), position.y(), position.z());
  // create the rotation
  Amg::RotationMatrix3D curvilinearRotation;
  curvilinearRotation.col(0) = curvUVT.curvU();
  curvilinearRotation.col(1) = curvUVT.curvV();
  curvilinearRotation.col(2) = curvUVT.curvT();
  Amg::Transform3D transform{};
  transform = curvilinearRotation;
  transform.pretranslate(position);
  Trk::Surface::m_transforms = std::make_unique<Transforms>(transform);
}

// construct form TrkDetElementBase
Trk::PlaneSurface::PlaneSurface(const Trk::TrkDetElementBase& detelement, const Amg::Transform3D & transf)
  : Trk::Surface(detelement)
  , m_bounds(nullptr)
{
  Trk::Surface::m_transforms = std::make_unique<Transforms>(transf);
}

// construct form TrkDetElementBase
Trk::PlaneSurface::PlaneSurface(const Trk::TrkDetElementBase& detelement)
  : Trk::Surface(detelement)
  , m_bounds(nullptr)
{
  //
}

// construct from SiDetectorElement
Trk::PlaneSurface::PlaneSurface(const Trk::TrkDetElementBase& detelement,
                                const Identifier& id,
                                const Amg::Transform3D & transf)
  : Trk::Surface(detelement, id)
  , m_bounds(nullptr)
{
  Trk::Surface::m_transforms = std::make_unique<Transforms>(transf);
}

// construct from SiDetectorElement
Trk::PlaneSurface::PlaneSurface(const Trk::TrkDetElementBase& detelement,
                                const Identifier& id)
  : Trk::Surface(detelement, id)
  , m_bounds(nullptr)
{
  //
}

// construct planar surface without bounds
Trk::PlaneSurface::PlaneSurface(const Amg::Transform3D& htrans)
  : Trk::Surface(htrans)
  , m_bounds(nullptr)
{}

// construct rectangle module
Trk::PlaneSurface::PlaneSurface(const Amg::Transform3D & htrans, double halephi, double haleta)
  : Trk::Surface(htrans)
  , m_bounds(std::make_shared<Trk::RectangleBounds>(halephi, haleta))
{}

// construct trapezoidal module with parameters
Trk::PlaneSurface::PlaneSurface(const Amg::Transform3D & htrans, double minhalephi, double maxhalephi, double haleta)
  : Trk::Surface(htrans)
  , m_bounds(std::make_shared<Trk::TrapezoidBounds>(minhalephi, maxhalephi, haleta))
{}

// construct with bounds
Trk::PlaneSurface::PlaneSurface(const Amg::Transform3D & htrans, const Trk::SurfaceBounds* tbounds)
  : Trk::Surface(htrans)
  , m_bounds(tbounds)
{}

// construct module with shared boundaries
Trk::PlaneSurface::PlaneSurface(
    const Amg::Transform3D & htrans,
    const Trk::SharedObject<const Trk::SurfaceBounds>& tbounds)
    : Trk::Surface(htrans), m_bounds(tbounds) {}

bool
Trk::PlaneSurface::operator==(const Trk::Surface& sf) const
{
  // first check the type not to compare apples with oranges
  if (sf.type()!=Trk::SurfaceType::Plane){
      return false;
  }
  return (*this) == static_cast<const Trk::PlaneSurface&>(sf);
}

/** Use the Surface as a ParametersBase constructor, from local parameters -
 * charged */
Trk::Surface::ChargedTrackParametersUniquePtr
Trk::PlaneSurface::createUniqueTrackParameters(
  double l1,
  double l2,
  double phi,
  double theta,
  double qop,
  std::optional<AmgSymMatrix(5)> cov) const
{
  return std::make_unique<ParametersT<5, Charged, PlaneSurface>>(
    l1, l2, phi, theta, qop, *this, std::move(cov));
}
/** Use the Surface as a ParametersBase constructor, from global parameters -
 * charged*/
Trk::Surface::ChargedTrackParametersUniquePtr
Trk::PlaneSurface::createUniqueTrackParameters(
  const Amg::Vector3D& position,
  const Amg::Vector3D& momentum,
  double charge,
  std::optional<AmgSymMatrix(5)> cov) const
{
  return std::make_unique<ParametersT<5, Charged, PlaneSurface>>(
    position, momentum, charge, *this, std::move(cov));
}

/** Use the Surface as a ParametersBase constructor, from local parameters -
 * neutral */
Trk::Surface::NeutralTrackParametersUniquePtr
Trk::PlaneSurface::createUniqueNeutralParameters(
  double l1,
  double l2,
  double phi,
  double theta,
  double oop,
  std::optional<AmgSymMatrix(5)> cov) const
{
  return std::make_unique<ParametersT<5, Neutral, PlaneSurface>>(
    l1, l2, phi, theta, oop, *this, std::move(cov));
}

/** Use the Surface as a ParametersBase constructor, from global parameters -
 * neutral */
Trk::Surface::NeutralTrackParametersUniquePtr
Trk::PlaneSurface::createUniqueNeutralParameters(
  const Amg::Vector3D& position,
  const Amg::Vector3D& momentum,
  double charge,
  std::optional<AmgSymMatrix(5)> cov) const
{
  return std::make_unique<ParametersT<5, Neutral, PlaneSurface>>(
    position, momentum, charge, *this, std::move(cov));
}

// Avoid out-of-line Eigen calls
ATH_FLATTEN
void
Trk::PlaneSurface::localToGlobal(const Amg::Vector2D& locpos,
                                 const Amg::Vector3D&,
                                 Amg::Vector3D& glopos) const
{
  Amg::Vector3D loc3Dframe(locpos[Trk::locX], locpos[Trk::locY], 0.);
  glopos = transform() * loc3Dframe;
}

bool
Trk::PlaneSurface::globalToLocal(const Amg::Vector3D& glopos,
                                      const Amg::Vector3D&,
                                      Amg::Vector2D& locpos) const {
  Amg::Vector3D loc3Dframe = inverseTransformMultHelper(glopos);
  locpos = Amg::Vector2D(loc3Dframe.x(), loc3Dframe.y());
  return (loc3Dframe.z() * loc3Dframe.z() <=
          s_onSurfaceTolerance * s_onSurfaceTolerance);
}

Trk::Intersection
Trk::PlaneSurface::straightLineIntersection(const Amg::Vector3D& pos,
                                            const Amg::Vector3D& dir,
                                            bool forceDir,
                                            Trk::BoundaryCheck bchk) const {
  double denom = dir.dot(normal());
  if (denom) {
    double u = (normal().dot((center() - pos))) / (denom);
    Amg::Vector3D intersectPoint(pos + u * dir);
    // evaluate the intersection in terms of direction
    bool isValid = forceDir ? (u > 0.) : true;
    // evaluate (if necessary in terms of boundaries)
    isValid = bchk ? (isValid && isOnSurface(intersectPoint)) : isValid;
    // return the result
    return Trk::Intersection(intersectPoint, u, isValid);
  }
  return Trk::Intersection(pos, 0., false);
}

void
Trk::PlaneSurface::localToGlobalDirection(const Trk::LocalDirection& locdir, Amg::Vector3D& globdir) const
{

  CxxUtils::sincos scXZ(locdir.angleXZ());
  CxxUtils::sincos scYZ(locdir.angleYZ());

  double norm = 1. / std::sqrt(scYZ.cs * scYZ.cs * scXZ.sn * scXZ.sn + scYZ.sn * scYZ.sn);

  // decide on the sign
  double sign = (scXZ.sn < 0.) ? -1. : 1.;

  // now calculate the GlobalDirection in the global frame
  globdir =
    transform().linear() *
    Amg::Vector3D(sign * scXZ.cs * scYZ.sn * norm, sign * scXZ.sn * scYZ.cs * norm, sign * scXZ.sn * scYZ.sn * norm);
}

void
Trk::PlaneSurface::globalToLocalDirection(const Amg::Vector3D& glodir, Trk::LocalDirection& ldir) const
{
  // bring the global direction into the surface frame
  Amg::Vector3D d(inverseTransformHelper().linear() * glodir);
  ldir = Trk::LocalDirection(std::atan2(d.z(), d.x()), std::atan2(d.z(), d.y()));
}

bool
Trk::PlaneSurface::isOnSurface(const Amg::Vector3D& glopo,
                               const Trk::BoundaryCheck& bchk,
                               double tol1, double tol2) const
{
  Amg::Vector3D loc3Dframe = inverseTransformMultHelper(glopo);
  if (std::abs(loc3Dframe(2)) > (s_onSurfaceTolerance + tol1)){
    return false;
  }
  return (bchk ? bounds().inside(Amg::Vector2D(loc3Dframe(0), loc3Dframe(1)), tol1, tol2) : true);
}

/** distance to surface */
Trk::DistanceSolution
Trk::PlaneSurface::straightLineDistanceEstimate(const Amg::Vector3D& pos, const Amg::Vector3D& dir) const
{
  static const double tol = 0.001;

  const Amg::Vector3D& N = normal();

  const double d = (pos - center()).dot(N);

  const double A = dir.dot(N); // ignore sign
  if (A == 0.) {               // direction parallel to surface
    if (std::abs(d) < tol) {
      return {1, 0., true, 0.};
    }
      return {0, d, true, 0.};

  }

  return {1, d, true, -d / A};
}

Trk::DistanceSolution
Trk::PlaneSurface::straightLineDistanceEstimate(const Amg::Vector3D& pos, const Amg::Vector3D& dir, bool bound) const
{
  const Amg::Transform3D& T = transform();
  double Az[3] = { T(0, 2), T(1, 2), T(2, 2) };

  // Transformation to plane system coordinates
  //
  double dx = pos[0] - T(0, 3);
  double dy = pos[1] - T(1, 3);
  double dz = pos[2] - T(2, 3);
  double z = dx * Az[0] + dy * Az[1] + dz * Az[2];
  double az = dir[0] * Az[0] + dir[1] * Az[1] + dir[2] * Az[2];

  // Step to surface
  //
  int ns = 0;
  double s = 0.;
  if (az != 0.) {
    s = -z / az;
    ns = 1;
  }
  double dist = std::abs(z);
  if (!bound)
    return {ns, std::abs(z), true, s};

  // Min distance to surface
  //
  double x = dx * T(0, 0) + dy * T(1, 0) + dz * T(2, 0);
  double y = dx * T(0, 1) + dy * T(1, 1) + dz * T(2, 1);

  Amg::Vector2D lp(x, y);

  double d = bounds().minDistance(lp);
  if (d > 0.)
    dist = std::sqrt(dist * dist + d * d);

  return {ns, dist, true, s};
}
