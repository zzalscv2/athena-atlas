/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// RotatedTrapezoidBounds.cxx, (c) ATLAS Detector Software
///////////////////////////////////////////////////////////////////

// Trk
#include "TrkSurfaces/RotatedTrapezoidBounds.h"
// Gaudi
#include "GaudiKernel/MsgStream.h"
// STD
#include <cmath>
#include <iomanip>
#include <iostream>

// default constructor
Trk::RotatedTrapezoidBounds::RotatedTrapezoidBounds()
  : m_boundValues(RotatedTrapezoidBounds::bv_length, 0.)  
{}

// constructor from arguments I
Trk::RotatedTrapezoidBounds::RotatedTrapezoidBounds(double halex, double minhalex, double maxhalex, double alpha):
    RotatedTrapezoidBounds(halex, minhalex, maxhalex){
    m_rotMat = Eigen::Rotation2D{alpha};
}
Trk::RotatedTrapezoidBounds::RotatedTrapezoidBounds(double halex, double minhalex, double maxhalex)
  : m_boundValues(RotatedTrapezoidBounds::bv_length, 0.)
  
{
  m_boundValues[RotatedTrapezoidBounds::bv_halfX] = std::abs(halex);
  m_boundValues[RotatedTrapezoidBounds::bv_minHalfY] = std::abs(minhalex);
  m_boundValues[RotatedTrapezoidBounds::bv_maxHalfY] = std::abs(maxhalex);
  // swap if needed
  if (m_boundValues[RotatedTrapezoidBounds::bv_minHalfY] > m_boundValues[RotatedTrapezoidBounds::bv_maxHalfY])
    swap(m_boundValues[RotatedTrapezoidBounds::bv_minHalfY], m_boundValues[RotatedTrapezoidBounds::bv_maxHalfY]);
  // assign kappa and delta
  RotatedTrapezoidBounds::initCache();
}


bool
Trk::RotatedTrapezoidBounds::operator==(const Trk::SurfaceBounds& sbo) const
{
  // check the type first not to compare apples with oranges
  const Trk::RotatedTrapezoidBounds* trabo = dynamic_cast<const Trk::RotatedTrapezoidBounds*>(&sbo);
  if (!trabo)
    return false;
  return (m_boundValues == trabo->m_boundValues);
}

bool
Trk::RotatedTrapezoidBounds::inside(const Amg::Vector2D& pos,
                                    const BoundaryCheck& bchk) const
{
  const Amg::Vector2D locpo = m_rotMat * pos;
  if (bchk.bcType == 0)
    return RotatedTrapezoidBounds::inside(
      locpo, bchk.toleranceLoc1, bchk.toleranceLoc2);

  // a fast FALSE
  const double fabsX = std::abs(locpo[Trk::locX]);
  double max_ell = bchk.lCovariance(0, 0) > bchk.lCovariance(1, 1)
                     ? bchk.lCovariance(0, 0)
                     : bchk.lCovariance(1, 1);
  double limit = bchk.nSigmas * sqrt(max_ell);
  if (fabsX > (m_boundValues[RotatedTrapezoidBounds::bv_halfX] + limit))
    return false;
  // a fast FALSE
  const double fabsY = std::abs(locpo[Trk::locY]);
  if (fabsY > (m_boundValues[RotatedTrapezoidBounds::bv_maxHalfY] + limit))
    return false;
  // a fast TRUE
  double min_ell = bchk.lCovariance(0, 0) < bchk.lCovariance(1, 1)
                     ? bchk.lCovariance(0, 0)
                     : bchk.lCovariance(1, 1);
  limit = bchk.nSigmas * sqrt(min_ell);
  if (fabsY < (m_boundValues[RotatedTrapezoidBounds::bv_minHalfY] + limit) &&
      fabsX < (m_boundValues[RotatedTrapezoidBounds::bv_halfX] + limit))
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
  Amg::Vector2D p =
    Amg::Vector2D(-m_boundValues[RotatedTrapezoidBounds::bv_halfX],
                  m_boundValues[RotatedTrapezoidBounds::bv_minHalfY]);
  elementP[0] = (rotMatrix * (p - locpo));
  p = Amg::Vector2D (-m_boundValues[RotatedTrapezoidBounds::bv_halfX],
                     -m_boundValues[RotatedTrapezoidBounds::bv_minHalfY]);
  elementP[1] = (rotMatrix * (p - locpo));
  p = Amg::Vector2D (m_boundValues[RotatedTrapezoidBounds::bv_halfX],
                     m_boundValues[RotatedTrapezoidBounds::bv_maxHalfY]);
  elementP[2] = (rotMatrix * (p - locpo));
  p = Amg::Vector2D (m_boundValues[RotatedTrapezoidBounds::bv_halfX],
                     -m_boundValues[RotatedTrapezoidBounds::bv_maxHalfY]);
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
Trk::RotatedTrapezoidBounds::inside(const Amg::Vector2D& pos, double tol1, double tol2) const
{

  // the cases:
  const Amg::Vector2D locpo = m_rotMat * pos;
  double fabsX = std::abs(locpo[Trk::locX]);
  double fabsY = std::abs(locpo[Trk::locY]);
  // (1) a fast FALSE
  if (fabsX > (m_boundValues[RotatedTrapezoidBounds::bv_halfX] + tol1))
    return false;
  // (2) a fast FALSE
  if (fabsY > (m_boundValues[RotatedTrapezoidBounds::bv_maxHalfY] + tol2))
    return false;
  // (3) a fast TRUE
  if (fabsY < (m_boundValues[RotatedTrapezoidBounds::bv_minHalfY] + tol2))
    return true;
  // (4) it is inside the rectangular shape solve the isBelow
  return (isBelow(locpo[Trk::locX], fabsY, tol1, tol2));
}

// checking if local point lies above a line
bool
Trk::RotatedTrapezoidBounds::isBelow(double locX, double fabsLocY, double tol1, double tol2) const
{
  // the most tolerant approach for tol1 and tol2
  return ((m_kappa * (locX + tol1) + m_delta) > fabsLocY - tol2);
}

double
Trk::RotatedTrapezoidBounds::minDistance(const Amg::Vector2D& locpo) const
{
  const Amg::Vector2D pos = m_rotMat * locpo;
  constexpr int Np = 4;

  const std::array<double,4> X{ -m_boundValues[RotatedTrapezoidBounds::bv_halfX],
                  -m_boundValues[RotatedTrapezoidBounds::bv_halfX],
                  m_boundValues[RotatedTrapezoidBounds::bv_halfX],
                  m_boundValues[RotatedTrapezoidBounds::bv_halfX] };
  const std::array<double,4> Y{ -m_boundValues[RotatedTrapezoidBounds::bv_minHalfY],
                  m_boundValues[RotatedTrapezoidBounds::bv_minHalfY],
                  m_boundValues[RotatedTrapezoidBounds::bv_maxHalfY],
                  -m_boundValues[RotatedTrapezoidBounds::bv_maxHalfY] };

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
    return -std::sqrt(dm);
  }
  return std::sqrt(dm);
}

// ostream operator overload

MsgStream&
Trk::RotatedTrapezoidBounds::dump(MsgStream& sl) const
{
  sl << std::setiosflags(std::ios::fixed);
  sl << std::setprecision(7);
  sl << "Trk::RotatedTrapezoidBounds:  (halfX, minHalfX, maxHalfY) = "
     << "(" << m_boundValues[RotatedTrapezoidBounds::bv_halfX] << ", "
     << m_boundValues[RotatedTrapezoidBounds::bv_minHalfY] << ", " << m_boundValues[RotatedTrapezoidBounds::bv_maxHalfY]
     << ")";
  sl << std::setprecision(-1);
  return sl;
}

std::ostream&
Trk::RotatedTrapezoidBounds::dump(std::ostream& sl) const
{
  sl << std::setiosflags(std::ios::fixed);
  sl << std::setprecision(7);
  sl << "Trk::RotatedTrapezoidBounds:  (halfX, minHalfX, maxHalfY) = "
     << "(" << m_boundValues[RotatedTrapezoidBounds::bv_halfX] << ", "
     << m_boundValues[RotatedTrapezoidBounds::bv_minHalfY] << ", " << m_boundValues[RotatedTrapezoidBounds::bv_maxHalfY]
     << ")";
  sl << std::setprecision(-1);
  return sl;
}
