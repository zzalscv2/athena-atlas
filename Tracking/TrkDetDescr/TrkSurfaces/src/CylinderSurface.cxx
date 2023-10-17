/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// CylinderSurface.cxx, (c) ATLAS Detector Software
///////////////////////////////////////////////////////////////////

// Trk
#include "TrkSurfaces/CylinderSurface.h"
#include "TrkSurfaces/RealQuadraticEquation.h"
// Gaudi
#include "GaudiKernel/MsgStream.h"
//CxxUtils
#include "CxxUtils/inline_hints.h"
// STD
#include <cassert>
#include <iomanip>
#include <iostream>

// default constructor
Trk::CylinderSurface::CylinderSurface()
  : Trk::Surface()
  , m_bounds(nullptr)
  , m_referencePoint(nullptr)
  , m_rotSymmetryAxis(nullptr)
{}

// copy constructor
Trk::CylinderSurface::CylinderSurface(const CylinderSurface& csf)
  : Trk::Surface(csf)
  , m_bounds(csf.m_bounds)
  , m_referencePoint(nullptr)
  , m_rotSymmetryAxis(nullptr)
{}

// copy constructor with shift
Trk::CylinderSurface::CylinderSurface(const CylinderSurface& csf,
                                      const Amg::Transform3D& transf)
  : Trk::Surface(csf, transf)
  , m_bounds(csf.m_bounds)
  , m_referencePoint(nullptr)
  , m_rotSymmetryAxis(nullptr)
{}

// constructor by radius and halflenght
Trk::CylinderSurface::CylinderSurface(const Amg::Transform3D& htrans,
                                      double radius,
                                      double hlength)
  : Trk::Surface(htrans)
  , m_bounds(std::make_shared<Trk::CylinderBounds>(radius, hlength))
  , m_referencePoint(nullptr)
  , m_rotSymmetryAxis(nullptr)
{}

// constructor by radius, halflenght and phisector
Trk::CylinderSurface::CylinderSurface(const Amg::Transform3D& htrans,
                                      double radius,
                                      double hphi,
                                      double hlength)
  : Trk::Surface(htrans)
  , m_bounds(std::make_shared<Trk::CylinderBounds>(radius, hphi, hlength))
  , m_referencePoint(nullptr)
  , m_rotSymmetryAxis(nullptr)
{}

// constructor by CylinderBounds
Trk::CylinderSurface::CylinderSurface(const Amg::Transform3D& htrans,
                                      Trk::CylinderBounds* cbounds)
  : Trk::Surface(htrans)
  , m_bounds(cbounds)
  , m_referencePoint(nullptr)
  , m_rotSymmetryAxis(nullptr)
{
  if (!cbounds) throw std::runtime_error("Cannot pass null CylinderBounds");
}

// constructor from transform
Trk::CylinderSurface::CylinderSurface(const Amg::Transform3D& htrans)
  : Trk::Surface(htrans)
  , m_bounds(nullptr)
  , m_referencePoint(nullptr)
  , m_rotSymmetryAxis(nullptr)
{}

// constructor by radius and halflength
Trk::CylinderSurface::CylinderSurface(double radius, double hlength)
  : Trk::Surface()
  , m_bounds(std::make_shared<Trk::CylinderBounds>(radius, hlength))
  , m_referencePoint(nullptr)
  , m_rotSymmetryAxis(nullptr)
{}

// constructor by radius, halflength and phisector
Trk::CylinderSurface::CylinderSurface(double radius,
                                      double hphi,
                                      double hlength)
  : Trk::Surface()
  , m_bounds(std::make_shared<Trk::CylinderBounds>(radius, hphi, hlength))
  , m_referencePoint(nullptr)
  , m_rotSymmetryAxis(nullptr)
{}

// constructor by CylinderBounds
Trk::CylinderSurface::CylinderSurface(Trk::CylinderBounds* cbounds)
  : Trk::Surface()
  , m_bounds(cbounds)
  , m_referencePoint(nullptr)
  , m_rotSymmetryAxis(nullptr)
{
  assert(cbounds);
}

Trk::CylinderSurface&
Trk::CylinderSurface::operator=(const CylinderSurface& csf)
{
  if (this != &csf) {
    Trk::Surface::operator=(csf);
    m_bounds = csf.m_bounds;
    m_referencePoint.store(nullptr);
    m_rotSymmetryAxis.store(nullptr);
  }
  return *this;
}

/** Use the Surface as a ParametersBase constructor, from local parameters -
 * charged */
Trk::Surface::ChargedTrackParametersUniquePtr
Trk::CylinderSurface::createUniqueTrackParameters(
    double l1, double l2, double phi, double theta, double qop,
    std::optional<AmgSymMatrix(5)> cov) const {
  return std::make_unique<ParametersT<5, Charged, CylinderSurface>>(
      l1, l2, phi, theta, qop, *this, std::move(cov));
}

/** Use the Surface as a ParametersBase constructor, from global parameters -
 * charged*/
Trk::Surface::ChargedTrackParametersUniquePtr
Trk::CylinderSurface::createUniqueTrackParameters(
    const Amg::Vector3D& position, const Amg::Vector3D& momentum, double charge,
    std::optional<AmgSymMatrix(5)> cov) const {
  return std::make_unique<ParametersT<5, Charged, CylinderSurface>>(
      position, momentum, charge, *this, std::move(cov));
}

/** Use the Surface as a ParametersBase constructor, from local parameters -
 * neutral */
Trk::Surface::NeutralTrackParametersUniquePtr
Trk::CylinderSurface::createUniqueNeutralParameters(
    double l1, double l2, double phi, double theta, double qop,
    std::optional<AmgSymMatrix(5)> cov) const {
  return std::make_unique<ParametersT<5, Neutral, CylinderSurface>>(
      l1, l2, phi, theta, qop, *this, std::move(cov));
}

/** Use the Surface as a ParametersBase constructor, from global parameters -
 * neutral */
Trk::Surface::NeutralTrackParametersUniquePtr
Trk::CylinderSurface::createUniqueNeutralParameters(
    const Amg::Vector3D& position, const Amg::Vector3D& momentum, double charge,
    std::optional<AmgSymMatrix(5)> cov) const {
  return std::make_unique<ParametersT<5, Neutral, CylinderSurface>>(
      position, momentum, charge, *this, std::move(cov));
}

const Amg::Vector3D&
Trk::CylinderSurface::globalReferencePoint() const
{
  if (!m_referencePoint) {
    double rMedium = bounds().r();
    double phi = bounds().averagePhi();
    Amg::Vector3D gp(rMedium * cos(phi), rMedium * sin(phi), 0.);
    m_referencePoint.set(std::make_unique<Amg::Vector3D>(transform() * gp));
  }
  return (*m_referencePoint);
}

bool
Trk::CylinderSurface::operator==(const Trk::Surface& sf) const
{
  // first check the type not to compare apples with oranges
  if (sf.type()!=Trk::SurfaceType::Cylinder){
      return false;
  }
  return (*this) == static_cast<const Trk::CylinderSurface&>(sf);
}

// return the measurement frame: it's the tangential plane
Amg::RotationMatrix3D
Trk::CylinderSurface::measurementFrame(const Amg::Vector3D& pos,
                                       const Amg::Vector3D&) const
{
  Amg::RotationMatrix3D mFrame;
  // construct the measurement frame
  Amg::Vector3D measY(
    transform().rotation().col(2)); // measured Y is the z axis
  Amg::Vector3D measDepth =
    Amg::Vector3D(pos.x(), pos.y(), 0.)
      .unit(); // measured z is the position transverse normalized
  Amg::Vector3D measX(
    measY.cross(measDepth).unit()); // measured X is what comoes out of it
  // the columnes
  mFrame.col(0) = measX;
  mFrame.col(1) = measY;
  mFrame.col(2) = measDepth;
  // return the rotation matrix
  return mFrame;
}

const Amg::Vector3D&
Trk::CylinderSurface::rotSymmetryAxis() const
{
  if (!m_rotSymmetryAxis) {
    Amg::Vector3D zAxis(transform().rotation().col(2));
    m_rotSymmetryAxis.set(std::make_unique<Amg::Vector3D>(zAxis));
  }
  return (*m_rotSymmetryAxis);
}

// Avoid out-of-line-eigen calls
ATH_FLATTEN
void
Trk::CylinderSurface::localToGlobal(const Amg::Vector2D& locpos,
                                    const Amg::Vector3D&,
                                    Amg::Vector3D& glopos) const
{
  // create the position in the local 3d frame
  double r = bounds().r();
  double phi = locpos[Trk::locRPhi] / r;
  glopos = Amg::Vector3D(r * cos(phi), r * sin(phi), locpos[Trk::locZ]);
  // transform it to the globalframe: CylinderSurfaces are allowed to have 0
  // pointer transform
  if (Trk::Surface::m_transforms)
    glopos = transform() * glopos;
}

bool
Trk::CylinderSurface::globalToLocal(const Amg::Vector3D& glopos,
                                    const Amg::Vector3D&,
                                    Amg::Vector2D& locpos) const
{
  // get the transform & transform global position into cylinder frame
  // transform it to the globalframe: CylinderSurfaces are allowed to have 0
  // pointer transform
  double radius = 0.;
  double inttol = bounds().r() * 0.0001;
  if (inttol < 0.01)
    inttol = 0.01;
  // do the transformation or not
  if (Trk::Surface::m_transforms) {
    Amg::Vector3D loc3Dframe(inverseTransformMultHelper(glopos));
    locpos = Amg::Vector2D(bounds().r() * loc3Dframe.phi(), loc3Dframe.z());
    radius = loc3Dframe.perp();
  } else {
    locpos = Amg::Vector2D(bounds().r() * glopos.phi(), glopos.z());
    radius = glopos.perp();
  }
  // return true or false
  return (fabs(radius - bounds().r()) <= inttol);
}

bool
Trk::CylinderSurface::isOnSurface(const Amg::Vector3D& glopo,
                                  const Trk::BoundaryCheck& bchk,
                                  double tol1,
                                  double tol2) const
{
  Amg::Vector3D loc3Dframe =
    Trk::Surface::m_transforms ? inverseTransformMultHelper(glopo) : glopo;
  return (bchk ? bounds().inside3D(loc3Dframe,
                                   tol1 + s_onSurfaceTolerance,
                                   tol2 + s_onSurfaceTolerance)
               : true);
}

Trk::Intersection
Trk::CylinderSurface::straightLineIntersection(const Amg::Vector3D& pos,
                                               const Amg::Vector3D& dir,
                                               bool forceDir,
                                               Trk::BoundaryCheck bchk) const
{
  bool needsTransform = m_transforms || m_associatedDetElement;
  // create the hep points
  Amg::Vector3D point1 = pos;
  Amg::Vector3D direction = dir;
  if (needsTransform) {
    Amg::Transform3D invTrans = inverseTransformHelper();
    point1 = invTrans * pos;
    direction = invTrans.linear() * dir;
  }
  // the bounds radius
  double R = bounds().r();
  double t1 = 0.;
  double t2 = 0.;
  if (direction.x()) {
    // get line and circle constants
    double idirx = 1. / direction.x();
    double k = direction.y() * idirx;
    double d = point1.y() - point1.x() * k;
    // and solve the qaudratic equation
    Trk::RealQuadraticEquation pquad(1 + k * k, 2 * k * d, d * d - R * R);
    if (pquad.solutions != Trk::none) {
      // the solutions in the 3D frame of the cylinder
      t1 = (pquad.first - point1.x()) * idirx;
      t2 = (pquad.second - point1.x()) * idirx;
    } else // bail out if no solution exists
      return {pos, 0., false};
  } else if (direction.y()) {
    // x value is the one of point1
    // x^2 + y^2 = R^2
    // y = sqrt(R^2-x^2)
    double x = point1.x();
    double r2mx2 = R * R - x * x;
    // bail out if no solution
    if (r2mx2 < 0.)
      return {pos, 0., false};
    double y = sqrt(r2mx2);
    // assign parameters and solutions
    double idiry = 1. / direction.y();
    t1 = (y - point1.y()) * idiry;
    t2 = (-y - point1.y()) * idiry;
  } else {
    return {pos, 0., false};
  }
  Amg::Vector3D sol1raw(point1 + t1 * direction);
  Amg::Vector3D sol2raw(point1 + t2 * direction);
  // now reorder and return
  Amg::Vector3D solution(0, 0, 0);
  double path = 0.;

  // first check the validity of the direction
  bool isValid = true;

  // both solutions are of same sign, take the smaller, but flag as false if not
  // forward
  if (t1 * t2 > 0 || !forceDir) {
    // asign validity
    isValid = forceDir ? (t1 > 0.) : true;
    // assign the right solution
    if (t1 * t1 < t2 * t2) {
      solution = sol1raw;
      path = t1;
    } else {
      solution = sol2raw;
      path = t2;
    }
  } else {
    if (t1 > 0.) {
      solution = sol1raw;
      path = t1;
    } else {
      solution = sol2raw;
      path = t2;
    }
  }
  // the solution is still in the local 3D frame, direct check
  isValid =
    bchk ? (isValid && m_bounds->inside3D(solution,
                                          Trk::Surface::s_onSurfaceTolerance,
                                          Trk::Surface::s_onSurfaceTolerance))
         : isValid;

  // now return
  return needsTransform ? Intersection(transform() * solution, path, isValid)
                        : Intersection(solution, path, isValid);
}

/** distance to surface */

#if defined(FLATTEN)
// We compile this function with optimization, even in debug builds; otherwise,
// the heavy use of Eigen makes it too slow.  However, from here we may call
// to out-of-line Eigen code that is linked from other DSOs; in that case,
// it would not be optimized.  Avoid this by forcing all Eigen code
// to be inlined here if possible.
ATH_FLATTEN
#endif
Trk::DistanceSolution
Trk::CylinderSurface::straightLineDistanceEstimate(
  const Amg::Vector3D& pos,
  const Amg::Vector3D& dir) const
{
  double tol = 0.001;

  const Amg::Vector3D& X = center(); // point
  const Amg::Vector3D& S = normal(); // vector

  double radius = bounds().r();
  double sp = pos.dot(S);
  double sc = X.dot(S);
  double dp = dir.dot(S);
  Amg::Vector3D dx = X - pos - (sc - sp) * S; // vector
  Amg::Vector3D ax = dir - dp * S;            // vector

  double A = ax.dot(ax); // size of projected direction (squared)
  double B = ax.dot(dx); // dot product (->cos angle)
  double C = dx.dot(dx); // distance to axis (squared)
  double currDist = radius - sqrt(C);

  if (A == 0.) { // direction parallel to cylinder axis
    if (fabs(currDist) < tol) {
      return {1, 0., true, 0.}; // solution at surface
    }
    return {
      0, currDist, true, 0.}; // point of closest approach without intersection
  }

  // minimal distance to cylinder axis
  // The [[maybe_unused]] declaration is to suppress redundant division checking
  // here. Even a tiny change in rmin (~1e-13) can cause huge changes in the
  // reconstructed output, so don't change how it's evaluated.
  [[maybe_unused]] const double rmin_tmp = B * B / A;
  const double rmin2 = C - rmin_tmp;
  const double rmin = rmin2 < 0 ? 0 : sqrt(rmin2);

  if (rmin > radius) { // no intersection
    double first = B / A;
    return {
      0,
      currDist,
      true,
      first}; // point of closest approach without intersection
  }
  if (fabs(rmin - radius) <
      tol) { // tangential 'intersection' - return double solution
    double first = B / A;
    return {2, currDist, true, first, first};
  }
  // The [[maybe_unused]] declaration here suppresses redundant division
  // checking. We don't want to rewrite how this is evaluated due to
  // instabilities.
  [[maybe_unused]] const double b_a = B / A;
  const double x = sqrt((radius - rmin) * (radius + rmin) / A);
  double first = b_a - x;
  double second = b_a + x;
  if (first >= 0.) {
    return {2, currDist, true, first, second};
  }
  if (second <= 0.) {
    return {2, currDist, true, second, first};
  } // inside cylinder
  return {2, currDist, true, second, first};
}

Trk::DistanceSolution
Trk::CylinderSurface::straightLineDistanceEstimate(const Amg::Vector3D& pos,
                                                   const Amg::Vector3D& dir,
                                                   bool bound) const
{
  const double tolb = .01;

  const Amg::Transform3D& T = transform();
  //  double                Ax[3] = {T.xx(),T.yx(),T.zx()};
  //  double                Ay[3] = {T.xy(),T.yy(),T.zy()};
  //  double                Az[3] = {T.xz(),T.yz(),T.zz()};

  // Transformation to cylinder system coordinates
  //

  // BEGIN here is what i guess this might mean: BEGIN
  Amg::Vector3D Ax = T.rotation().col(0);
  Amg::Vector3D Ay = T.rotation().col(1);
  Amg::Vector3D Az = T.rotation().col(2);

  Amg::Vector3D dxyz = pos - T.translation();
  double x = dxyz.dot(Ax);
  double y = dxyz.dot(Ay);
  double z = dxyz.dot(Az);
  double ax = dir.dot(Ax);
  double ay = dir.dot(Ay);
  double at = ax * ax + ay * ay;
  double r = sqrt(x * x + y * y);
  double R = bounds().r();

  // END here is what i guessed this means END

  //  double dx  = pos[0]-T.dx()                         ;
  //  double dy  = pos[1]-T.dy()                         ;
  //  double dz  = pos[2]-T.dz()                         ;
  //  double x   = dx*Ax[0]+dy*Ax[1]+dz*Ax[2]            ;
  //  double y   = dx*Ay[0]+dy*Ay[1]+dz*Ay[2]            ;
  //  double z   = dx*Az[0]+dy*Az[1]+dz*Az[2]            ;
  //  double ax  = dir[0]*Ax[0]+dir[1]*Ax[1]+dir[2]*Ax[2];
  //  double ay  = dir[0]*Ay[0]+dir[1]*Ay[1]+dir[2]*Ay[2];
  //  double at  = ax*ax+ay*ay                           ;
  //  double r   = sqrt(x*x+y*y)                         ;
  //  double R   = bounds().r()                          ;

  // Step to surface
  //
  int ns = 0;
  double s1 = 0.;
  double s2 = 0.;

  if (at != 0.) {

    const double inv_at = 1. / at;
    double A = -(ax * x + ay * y) * inv_at;
    double B = A * A + (R - r) * (R - r) * inv_at;

    if (B >= 0.) {

      B = sqrt(B);
      if (B > tolb) {
        if (A > 0.) {
          s1 = A - B;
          s2 = A + B;
        } else {
          s1 = A + B;
          s2 = A - B;
        }
        ns = 2;
      } else {
        s1 = A;
        ns = 1;
      }
    }
  }
  double sr = r - R;
  if (!bound)
    return {ns, fabs(sr), true, s1, s2};

  // Min distance to surface
  //
  Amg::Vector2D lp(atan2(y, x) * R, 0.);

  double d = bounds().minDistance(lp);
  double sz = fabs(z) - bounds().halflengthZ();
  if (sz <= 0.)
    sz = 0.;
  double dist = sr * sr + sz * sz;
  if (d > 0.)
    dist += ((d * d) * (sr / R + 1.));

  return {ns, sqrt(dist), true, s1, s2};
}
