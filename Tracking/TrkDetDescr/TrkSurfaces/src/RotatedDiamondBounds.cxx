/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// RotatedDiamondBounds.cxx, (c) ATLAS Detector Software
///////////////////////////////////////////////////////////////////

// Trk
#include "TrkSurfaces/RotatedDiamondBounds.h"
// Gaudi
#include "GaudiKernel/MsgStream.h"
// STD
#include <cmath>
#include <iomanip>
#include <iostream>

// default constructor
Trk::RotatedDiamondBounds::RotatedDiamondBounds()
  : m_boundValues(RotatedDiamondBounds::bv_length, 0.)
  , m_alpha1(0.)
  , m_alpha2(0.)
{}

// constructor from arguments I
Trk::RotatedDiamondBounds::RotatedDiamondBounds(double minhalex,
                                                double medhalex,
                                                double maxhalex,
                                                double haley1,
                                                double haley2)
  : m_boundValues(RotatedDiamondBounds::bv_length, 0.)
  , m_alpha1(0.)
  , m_alpha2(0.)
{
  m_boundValues[RotatedDiamondBounds::bv_minHalfX] = minhalex;
  m_boundValues[RotatedDiamondBounds::bv_medHalfX] = medhalex;
  m_boundValues[RotatedDiamondBounds::bv_maxHalfX] = maxhalex;
  m_boundValues[RotatedDiamondBounds::bv_halfY1] = haley1;
  m_boundValues[RotatedDiamondBounds::bv_halfY2] = haley2;
  if (minhalex > maxhalex)
    swap(m_boundValues[RotatedDiamondBounds::bv_minHalfX], m_boundValues[RotatedDiamondBounds::bv_maxHalfX]);
  RotatedDiamondBounds::initCache();
}

bool
Trk::RotatedDiamondBounds::operator==(const Trk::SurfaceBounds& sbo) const
{
  // check the type first not to compare apples with oranges
  const Trk::RotatedDiamondBounds* diabo = dynamic_cast<const Trk::RotatedDiamondBounds*>(&sbo);
  if (!diabo)
    return false;
  return (m_boundValues == diabo->m_boundValues);
}

bool
Trk::RotatedDiamondBounds::inside(const Amg::Vector2D& locpo,
                                  const BoundaryCheck& bchk) const
{
  // locX and locY are interchanged wrt DiamondBounds
  if (bchk.bcType == 0)
    return RotatedDiamondBounds::inside(
      locpo, bchk.toleranceLoc1, bchk.toleranceLoc2);
  // a fast FALSE
  double max_ell = bchk.lCovariance(0, 0) > bchk.lCovariance(1, 1)
                     ? bchk.lCovariance(0, 0)
                     : bchk.lCovariance(1, 1);
  double limit = bchk.nSigmas * sqrt(max_ell);
  if (locpo[Trk::locX] <
      -2 * m_boundValues[RotatedDiamondBounds::bv_halfY1] - limit)
    return false;
  if (locpo[Trk::locX] >
      2 * m_boundValues[RotatedDiamondBounds::bv_halfY2] + limit)
    return false;
  // a fast FALSE
  double fabsX = std::abs(locpo[Trk::locY]);
  if (fabsX > (m_boundValues[RotatedDiamondBounds::bv_medHalfX] + limit))
    return false;
  // a fast TRUE
  double min_ell = bchk.lCovariance(0, 0) < bchk.lCovariance(1, 1)
                     ? bchk.lCovariance(0, 0)
                     : bchk.lCovariance(1, 1);
  limit = bchk.nSigmas * sqrt(min_ell);
  if (fabsX < (fmin(m_boundValues[RotatedDiamondBounds::bv_minHalfX],
                    m_boundValues[RotatedDiamondBounds::bv_maxHalfX]) -
               limit))
    return true;
  // a fast TRUE
  if (std::abs(locpo[Trk::locX]) <
      (fmin(m_boundValues[RotatedDiamondBounds::bv_halfY1],
            m_boundValues[RotatedDiamondBounds::bv_halfY2]) -
       limit))
    return true;

  // compute KDOP and axes for surface polygon
  std::vector<KDOP> elementKDOP(5);
  std::vector<Amg::Vector2D> elementP(6);
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
  // rotated exchange locX and locY
  Amg::Vector2D locpoF;
  locpoF[0] = locpo[Trk::locY];
  locpoF[1] = locpo[Trk::locX];
  Amg::Vector2D p =
    Amg::Vector2D(-m_boundValues[RotatedDiamondBounds::bv_minHalfX],
                  -2. * m_boundValues[RotatedDiamondBounds::bv_halfY1]);
  elementP[0] = (rotMatrix * (p - locpoF));
  p = Amg::Vector2D (-m_boundValues[RotatedDiamondBounds::bv_medHalfX], 0.);
  elementP[1] = (rotMatrix * (p - locpoF));
  p = Amg::Vector2D (-m_boundValues[RotatedDiamondBounds::bv_maxHalfX],
                     2. * m_boundValues[RotatedDiamondBounds::bv_halfY2]);
  elementP[2] = (rotMatrix * (p - locpoF));
  p = Amg::Vector2D (m_boundValues[RotatedDiamondBounds::bv_maxHalfX],
                     2. * m_boundValues[RotatedDiamondBounds::bv_halfY2]);
  elementP[3] = (rotMatrix * (p - locpoF));
  p = Amg::Vector2D (m_boundValues[RotatedDiamondBounds::bv_medHalfX], 0.);
  elementP[4] = (rotMatrix * (p - locpoF));
  p = Amg::Vector2D (m_boundValues[RotatedDiamondBounds::bv_minHalfX],
                     -2. * m_boundValues[RotatedDiamondBounds::bv_halfY1]);
  elementP[5] = (rotMatrix * (p - locpoF));
  std::vector<Amg::Vector2D> axis = { normal * (elementP[1] - elementP[0]),
                                      normal * (elementP[2] - elementP[1]),
                                      normal * (elementP[3] - elementP[2]),
                                      normal * (elementP[4] - elementP[3]),
                                      normal * (elementP[5] - elementP[4]) };
  bchk.ComputeKDOP(elementP, axis, elementKDOP);
  // compute KDOP for error ellipse
  std::vector<KDOP> errelipseKDOP(5);
  bchk.ComputeKDOP(bchk.EllipseToPoly(3), axis, errelipseKDOP);
  // check if KDOPs overlap and return result
  return bchk.TestKDOPKDOP(elementKDOP, errelipseKDOP);
}

// checking if inside bounds (Full symmetrical Diamond)
bool
Trk::RotatedDiamondBounds::inside(const Amg::Vector2D& locpo, double tol1, double tol2) const
{
  return this->insideFull(locpo, tol1, tol2);
}

// checking if inside bounds (Full symmetrical Diamond)
bool
Trk::RotatedDiamondBounds::insideFull(const Amg::Vector2D& locpo, double tol1, double tol2) const
{
  // validity check
  if (!m_boundValues[RotatedDiamondBounds::bv_halfY1] && !m_boundValues[RotatedDiamondBounds::bv_minHalfX]) return false;

  // quick False (radial direction)
  if (locpo[Trk::locX] < -2. * m_boundValues[RotatedDiamondBounds::bv_halfY1] - tol1) return false;
  if (locpo[Trk::locX] >  2. * m_boundValues[RotatedDiamondBounds::bv_halfY2] + tol1) return false;

  double  absY = std::abs(locpo[Trk::locY]);

  // quick False (transverse direction)
  if (absY > (m_boundValues[RotatedDiamondBounds::bv_medHalfX] + tol2)) return false;
  
  // quick True
  if (absY < std::min(m_boundValues[RotatedDiamondBounds::bv_minHalfX], m_boundValues[RotatedDiamondBounds::bv_maxHalfX]) + tol2) return true;

  /** use basic calculation of a straight line */
  const double& halfBaseUp = locpo[Trk::locX] < 0 ? m_boundValues[RotatedDiamondBounds::bv_medHalfX] : m_boundValues[RotatedDiamondBounds::bv_maxHalfX];
  const double& halfBaseLo = locpo[Trk::locX] < 0 ? m_boundValues[RotatedDiamondBounds::bv_minHalfX] : m_boundValues[RotatedDiamondBounds::bv_medHalfX];
  const double& halfH      = locpo[Trk::locX] < 0 ? m_boundValues[RotatedDiamondBounds::bv_halfY1]   : m_boundValues[RotatedDiamondBounds::bv_halfY2];
  double k = halfH ? 0.5*(halfBaseUp - halfBaseLo)/halfH : 0.;
  double sign = (k < 0) ? -1. : 1.; 
  return (absY - tol2 <= m_boundValues[RotatedDiamondBounds::bv_medHalfX] + k * (locpo[Trk::locX] + sign*tol1));
}

// opening angle in point A
double
Trk::RotatedDiamondBounds::alpha1() const
{
  return m_alpha1;
}

// opening angle in point A'
double
Trk::RotatedDiamondBounds::alpha2() const
{
  return m_alpha2;
}

double
Trk::RotatedDiamondBounds::minDistance(const Amg::Vector2D& pos) const
{
  const int Np = 6;

  double y1 = 2. * m_boundValues[RotatedDiamondBounds::bv_halfY1];
  double y2 = 2. * m_boundValues[RotatedDiamondBounds::bv_halfY2];

  double X[6] = { -m_boundValues[RotatedDiamondBounds::bv_minHalfX], -m_boundValues[RotatedDiamondBounds::bv_medHalfX],
                  -m_boundValues[RotatedDiamondBounds::bv_maxHalfX], m_boundValues[RotatedDiamondBounds::bv_maxHalfX],
                  m_boundValues[RotatedDiamondBounds::bv_medHalfX],  m_boundValues[RotatedDiamondBounds::bv_minHalfX] };
  double Y[6] = { -y1, 0., y2, y2, 0., -y1 };

  double dm = 1.e+20;
  double Ao = 0.;
  bool in = true;

  for (int i = 0; i != Np; ++i) {

    int j = (i == Np-1 ? 0 : i+1);

    // interchange locx and locy
    double x = X[i] - pos[1];
    double y = Y[i] - pos[0];
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
  if (in)
    return -sqrt(dm);
  return sqrt(dm);
}

// ostream operator overload

MsgStream&
Trk::RotatedDiamondBounds::dump(MsgStream& sl) const
{
  sl << std::setiosflags(std::ios::fixed);
  sl << std::setprecision(7);
  sl << "Trk::RotatedDiamondBounds:  (minHlenghtX, medHlengthX, maxHlengthX, hlengthY1, hlengthY2 ) = ";
  sl << "(" << m_boundValues[RotatedDiamondBounds::bv_minHalfX] << ", "
     << m_boundValues[RotatedDiamondBounds::bv_medHalfX] << ", " << m_boundValues[RotatedDiamondBounds::bv_maxHalfX]
     << ", " << m_boundValues[RotatedDiamondBounds::bv_halfY1] << ", " << m_boundValues[RotatedDiamondBounds::bv_halfY2]
     << ")";
  sl << std::setprecision(-1);
  return sl;
}

std::ostream&
Trk::RotatedDiamondBounds::dump(std::ostream& sl) const
{
  sl << std::setiosflags(std::ios::fixed);
  sl << std::setprecision(7);
  sl << "Trk::RotatedDiamondBounds:  (minHlenghtX, medHlengthX, maxHlengthX, hlengthY1, hlengthY2 ) = ";
  sl << "(" << m_boundValues[RotatedDiamondBounds::bv_minHalfX] << ", "
     << m_boundValues[RotatedDiamondBounds::bv_medHalfX] << ", " << m_boundValues[RotatedDiamondBounds::bv_maxHalfX]
     << ", " << m_boundValues[RotatedDiamondBounds::bv_halfY1] << ", " << m_boundValues[RotatedDiamondBounds::bv_halfY2]
     << ")";
  sl << std::setprecision(-1);
  return sl;
}
