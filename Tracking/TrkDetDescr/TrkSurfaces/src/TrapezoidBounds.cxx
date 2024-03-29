/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TrapezoidBounds.cxx, (c) ATLAS Detector Software
///////////////////////////////////////////////////////////////////

// Trk
#include "TrkSurfaces/TrapezoidBounds.h"
// Gaudi
#include "GaudiKernel/MsgStream.h"
// STD
#include <cmath>
#include <iomanip>
#include <iostream>

// default constructor
Trk::TrapezoidBounds::TrapezoidBounds()
  : m_boundValues(TrapezoidBounds::bv_length, 0.)
  , m_alpha(0.)
  , m_beta(0.)
{}

// constructor from arguments I
Trk::TrapezoidBounds::TrapezoidBounds(double minhalex, double maxhalex, double haley)
  : m_boundValues(TrapezoidBounds::bv_length, 0.)
  , m_alpha(0.)
  , m_beta(0.)
{
  m_boundValues[TrapezoidBounds::bv_minHalfX] = std::abs(minhalex);
  m_boundValues[TrapezoidBounds::bv_maxHalfX] = std::abs(maxhalex);
  m_boundValues[TrapezoidBounds::bv_halfY] = std::abs(haley);
  if (m_boundValues[TrapezoidBounds::bv_minHalfX] > m_boundValues[TrapezoidBounds::bv_maxHalfX])
    swap(m_boundValues[TrapezoidBounds::bv_minHalfX], m_boundValues[TrapezoidBounds::bv_maxHalfX]);
}

// constructor from arguments II
Trk::TrapezoidBounds::TrapezoidBounds(double minhalex, double haley, double alpha, double beta)
  : m_boundValues(TrapezoidBounds::bv_length, 0.)
  , m_alpha(alpha)
  , m_beta(beta)
{
  double gamma = (alpha > beta) ? (alpha - 0.5 * M_PI) : (beta - 0.5 * M_PI);
  // now fill them
  m_boundValues[TrapezoidBounds::bv_minHalfX] = std::abs(minhalex);
  m_boundValues[TrapezoidBounds::bv_maxHalfX] = minhalex + (2. * m_boundValues[TrapezoidBounds::bv_halfY]) * tan(gamma);
  m_boundValues[TrapezoidBounds::bv_halfY] = std::abs(haley);
}


bool
Trk::TrapezoidBounds::operator==(const Trk::SurfaceBounds& sbo) const
{
  // check the type first not to compare apples with oranges
  const Trk::TrapezoidBounds* trabo = dynamic_cast<const Trk::TrapezoidBounds*>(&sbo);
  if (!trabo)
    return false;
  return (m_boundValues == trabo->m_boundValues);
}

bool
Trk::TrapezoidBounds::inside(const Amg::Vector2D& locpo,
                             const BoundaryCheck& bchk) const
{
  if (bchk.bcType == 0)
    return TrapezoidBounds::inside(
      locpo, bchk.toleranceLoc1, bchk.toleranceLoc2);

  // a fast FALSE
  double fabsY = std::abs(locpo[Trk::locY]);
  double max_ell = bchk.lCovariance(0, 0) > bchk.lCovariance(1, 1)
                     ? bchk.lCovariance(0, 0)
                     : bchk.lCovariance(1, 1);
  double limit = bchk.nSigmas * sqrt(max_ell);
  if (fabsY > (m_boundValues[TrapezoidBounds::bv_halfY] + limit))
    return false;
  // a fast FALSE
  double fabsX = std::abs(locpo[Trk::locX]);
  if (fabsX > (m_boundValues[TrapezoidBounds::bv_maxHalfX] + limit))
    return false;
  // a fast TRUE
  double min_ell = bchk.lCovariance(0, 0) < bchk.lCovariance(1, 1)
                     ? bchk.lCovariance(0, 0)
                     : bchk.lCovariance(1, 1);
  limit = bchk.nSigmas * sqrt(min_ell);
  if (fabsX < (m_boundValues[TrapezoidBounds::bv_minHalfX] + limit) &&
      fabsY < (m_boundValues[TrapezoidBounds::bv_halfY] + limit))
    return true;

  // compute KDOP and axes for surface polygon
  std::vector<KDOP> elementKDOP(3);
  std::vector<Amg::Vector2D> elementP(4);
  float theta =
    (bchk.lCovariance(1, 0) != 0 &&
     (bchk.lCovariance(1, 1) - bchk.lCovariance(0, 0)) != 0)
      ? .5 * bchk.FastArcTan(2 * bchk.lCovariance(1, 0) /
                             (bchk.lCovariance(1, 1) - bchk.lCovariance(0, 0)))
      : 0.;
  sincosCache scResult = bchk.FastSinCos(theta);
  AmgMatrix(2, 2) rotMatrix;
  rotMatrix << scResult.cosC, scResult.sinC, -scResult.sinC, scResult.cosC;
  AmgMatrix(2, 2) normal;
  // cppcheck-suppress constStatement
  normal << 0, -1, 1, 0;
  // ellipse is always at (0,0), surface is moved to ellipse position and then
  // rotated
  Amg::Vector2D p = Amg::Vector2D(m_boundValues[TrapezoidBounds::bv_minHalfX],
                                  -m_boundValues[TrapezoidBounds::bv_halfY]);
  elementP[0] = (rotMatrix * (p - locpo));
  p = Amg::Vector2D (-m_boundValues[TrapezoidBounds::bv_minHalfX],
                     -m_boundValues[TrapezoidBounds::bv_halfY]);
  elementP[1] = (rotMatrix * (p - locpo));
  scResult = bchk.FastSinCos(m_beta);
  p = Amg::Vector2D (m_boundValues[TrapezoidBounds::bv_minHalfX] +
                     (2. * m_boundValues[TrapezoidBounds::bv_halfY]) *
                     (scResult.sinC / scResult.cosC),
                     m_boundValues[TrapezoidBounds::bv_halfY]);
  elementP[2] = (rotMatrix * (p - locpo));
  scResult = bchk.FastSinCos(m_alpha);
  p = Amg::Vector2D (-(m_boundValues[TrapezoidBounds::bv_minHalfX] +
                       (2. * m_boundValues[TrapezoidBounds::bv_halfY]) *
                       (scResult.sinC / scResult.cosC)),
                     m_boundValues[TrapezoidBounds::bv_halfY]);
  elementP[3] = (rotMatrix * (p - locpo));
  std::vector<Amg::Vector2D> axis = { normal * (elementP[1] - elementP[0]),
                                      normal * (elementP[3] - elementP[1]),
                                      normal * (elementP[2] - elementP[0]) };
  bchk.ComputeKDOP(elementP, axis, elementKDOP);
  // compute KDOP for error ellipse
  std::vector<KDOP> errelipseKDOP(3);
  bchk.ComputeKDOP(bchk.EllipseToPoly(3), axis, errelipseKDOP);
  // check if KDOPs overlap and return result
  return bchk.TestKDOPKDOP(elementKDOP, errelipseKDOP);
}

// checking if inside bounds
bool
Trk::TrapezoidBounds::inside(const Amg::Vector2D& locpo, double tol1, double tol2) const
{
  if (m_alpha == 0.)
    return insideFull(locpo, tol1, tol2);
  return (insideFull(locpo, tol1, tol2) && !insideExclude(locpo, tol1, tol2));
}

// checking if inside bounds (Full symmetrical Trapezoid)
bool
Trk::TrapezoidBounds::insideFull(const Amg::Vector2D& locpo, double tol1, double tol2) const
{
  // the cases:
  double fabsX = std::abs(locpo[Trk::locX]);
  double fabsY = std::abs(locpo[Trk::locY]);
  // (1) a fast FALSE
  if (fabsY > (m_boundValues[TrapezoidBounds::bv_halfY] + tol2))
    return false;
  // (2) a fast FALSE
  if (fabsX > (m_boundValues[TrapezoidBounds::bv_maxHalfX] + tol1))
    return false;
  // (3) a fast TRUE
  if (fabsX < (m_boundValues[TrapezoidBounds::bv_minHalfX] + tol1))
    return true;
  // (4) particular case - a rectangle
  if (m_boundValues[TrapezoidBounds::bv_maxHalfX] == m_boundValues[TrapezoidBounds::bv_minHalfX])
    return true;
  // (5) /** use basic calculation of a straight line */
  double k = 2.0 * m_boundValues[TrapezoidBounds::bv_halfY] /
             (m_boundValues[TrapezoidBounds::bv_maxHalfX] - m_boundValues[TrapezoidBounds::bv_minHalfX]) *
             ((locpo[Trk::locX] > 0.) ? 1.0 : -1.0);
  double d =
    -std::abs(k) * 0.5 * (m_boundValues[TrapezoidBounds::bv_maxHalfX] + m_boundValues[TrapezoidBounds::bv_minHalfX]);
  return (isAbove(locpo, tol1, tol2, k, d));
}

// checking if local point is inside the exclude area
bool
Trk::TrapezoidBounds::insideExclude(const Amg::Vector2D& locpo, double tol1, double tol2) const
{

  // line a
  bool alphaBiggerBeta(m_alpha > m_beta);
  double ka = alphaBiggerBeta ? tan(M_PI - m_alpha) : tan(m_alpha);
  double kb = alphaBiggerBeta ? tan(M_PI - m_beta) : tan(m_beta);
  double sign = alphaBiggerBeta ? -1. : 1.;
  double da = -m_boundValues[TrapezoidBounds::bv_halfY] + sign * ka * m_boundValues[TrapezoidBounds::bv_minHalfX];
  double db = -m_boundValues[TrapezoidBounds::bv_halfY] + sign * kb * m_boundValues[TrapezoidBounds::bv_minHalfX];

  return (isAbove(locpo, tol1, tol2, ka, da) && isAbove(locpo, tol1, tol2, kb, db));
}

// checking if local point lies above a line
bool
Trk::TrapezoidBounds::isAbove(const Amg::Vector2D& locpo, double tol1, double tol2, double k, double d) 
{
  // the most tolerant approach for tol1 and tol2
  double sign = k > 0. ? -1. : +1.;
  return (locpo[Trk::locY] + tol2 > (k * (locpo[Trk::locX] + sign * tol1) + d));
}

double
Trk::TrapezoidBounds::minDistance(const Amg::Vector2D& pos) const
{
  const int Np = 4;

  double xl = -m_boundValues[TrapezoidBounds::bv_maxHalfX];
  double xr = m_boundValues[TrapezoidBounds::bv_maxHalfX];
  if (m_alpha != 0.) {
    xl = -m_boundValues[TrapezoidBounds::bv_minHalfX] - 2. * tan(m_alpha) * m_boundValues[TrapezoidBounds::bv_halfY];
  } else if (m_beta != 0.) {
    xr = m_boundValues[TrapezoidBounds::bv_minHalfX] + 2. * tan(m_beta) * m_boundValues[TrapezoidBounds::bv_halfY];
  }
  double X[4] = { -m_boundValues[TrapezoidBounds::bv_minHalfX], xl, xr, m_boundValues[TrapezoidBounds::bv_minHalfX] };
  double Y[4] = { -m_boundValues[TrapezoidBounds::bv_halfY],
                  m_boundValues[TrapezoidBounds::bv_halfY],
                  m_boundValues[TrapezoidBounds::bv_halfY],
                  -m_boundValues[TrapezoidBounds::bv_halfY] };

  double dm = 1.e+20;
  double Ao = 0.;
  bool in = true;

  for (int i = 0; i != Np; ++i) {

    int j = (i == Np-1 ? 0 : i+1);

    double x = X[i] - pos[0];
    double y = Y[i] - pos[1];
    double dx = X[j] - X[i];
    double dy = Y[j] - Y[i];
    double A = x * dy - y * dx;
    double S = -(x * dx + y * dy);

    if (S <= 0.) {
      double d = x * x + y * y;
      if (d < dm)
        dm = d;
    } else {
      double a = dx * dx + dy * dy;
      if (S <= a) {
        double d = (A * A) / a;
        if (d < dm)
          dm = d;
      }
    }
    if (i && in && Ao * A < 0.)
      in = false;
    Ao = A;
  }
  if (in){
    return -sqrt(dm);
  }
  return sqrt(dm);
}

// ostream operator overload

MsgStream&
Trk::TrapezoidBounds::dump(MsgStream& sl) const
{
  sl << std::setiosflags(std::ios::fixed);
  sl << std::setprecision(7);
  sl << "Trk::TrapezoidBounds:  (minHlenghtX, maxHlengthX, hlengthY) = "
     << "(" << m_boundValues[TrapezoidBounds::bv_minHalfX] << ", " << m_boundValues[TrapezoidBounds::bv_maxHalfX]
     << ", " << m_boundValues[TrapezoidBounds::bv_halfY] << ")";
  sl << std::setprecision(-1);
  return sl;
}

std::ostream&
Trk::TrapezoidBounds::dump(std::ostream& sl) const
{
  sl << std::setiosflags(std::ios::fixed);
  sl << std::setprecision(7);
  sl << "Trk::TrapezoidBounds:  (minHlenghtX, maxHlengthX, hlengthY) = "
     << "(" << m_boundValues[TrapezoidBounds::bv_minHalfX] << ", " << m_boundValues[TrapezoidBounds::bv_maxHalfX]
     << ", " << m_boundValues[TrapezoidBounds::bv_halfY] << ")";
  sl << std::setprecision(-1);
  return sl;
}
