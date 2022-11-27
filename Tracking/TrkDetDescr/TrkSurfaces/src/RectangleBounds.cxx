/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// RectangleBounds.cxx, (c) ATLAS Detector Software
///////////////////////////////////////////////////////////////////

// Trk
#include "TrkSurfaces/RectangleBounds.h"
// Gaudi
#include "GaudiKernel/MsgStream.h"
// STD
#include <iomanip>
#include <iostream>

// default constructor
Trk::RectangleBounds::RectangleBounds()
  : m_boundValues(RectangleBounds::bv_length, 0.)
{}

// rectangle constructor
Trk::RectangleBounds::RectangleBounds(double halex, double haley)
  : m_boundValues(RectangleBounds::bv_length, 0.)
{
  m_boundValues[RectangleBounds::bv_halfX] = halex;
  m_boundValues[RectangleBounds::bv_halfY] = haley;
}

// copy constructor
Trk::RectangleBounds::RectangleBounds(const RectangleBounds& recbo)
  : Trk::SurfaceBounds()
  , m_boundValues(recbo.m_boundValues)
{}


Trk::RectangleBounds&
Trk::RectangleBounds::operator=(const RectangleBounds& recbo)
{
  if (this != &recbo)
    m_boundValues = recbo.m_boundValues;
  return *this;
}

bool
Trk::RectangleBounds::operator==(const Trk::SurfaceBounds& sbo) const
{
  // check the type first not to compare apples with oranges
  const Trk::RectangleBounds* recbo = dynamic_cast<const Trk::RectangleBounds*>(&sbo);
  if (!recbo)
    return false;
  return (m_boundValues == recbo->m_boundValues);
}

bool
Trk::RectangleBounds::inside(const Amg::Vector2D& locpo,
                             const BoundaryCheck& bchk) const
{
  if (bchk.bcType == 0)
    return RectangleBounds::inside(
      locpo, bchk.toleranceLoc1, bchk.toleranceLoc2);

  // a fast FALSE
  double max_ell = bchk.lCovariance(0, 0) > bchk.lCovariance(1, 1)
                     ? bchk.lCovariance(0, 0)
                     : bchk.lCovariance(1, 1);
  double limit = bchk.nSigmas * sqrt(max_ell);
  if (!RectangleBounds::inside(locpo, limit, limit))
    return false;
  // a fast TRUE
  double min_ell = bchk.lCovariance(0, 0) < bchk.lCovariance(1, 1)
                     ? bchk.lCovariance(0, 0)
                     : bchk.lCovariance(1, 1);
  limit = bchk.nSigmas * sqrt(min_ell);
  if (RectangleBounds::inside(locpo, limit, limit))
    return true;

  // compute KDOP and axes for surface polygon
  std::vector<KDOP> elementKDOP(4);
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
  // ellipse is always at (0,0), surface is moved to ellipse position and then
  // rotated
  Amg::Vector2D p;
  p = Amg::Vector2D (m_boundValues[RectangleBounds::bv_halfX],
                     m_boundValues[RectangleBounds::bv_halfY]);
  elementP[0] = (rotMatrix * (p - locpo));
  p = Amg::Vector2D (m_boundValues[RectangleBounds::bv_halfX],
                     -m_boundValues[RectangleBounds::bv_halfY]);
  elementP[1] = (rotMatrix * (p - locpo));
  p = Amg::Vector2D (-m_boundValues[RectangleBounds::bv_halfX],
                     m_boundValues[RectangleBounds::bv_halfY]);
  elementP[2] = (rotMatrix * (p - locpo));
  p = Amg::Vector2D (-m_boundValues[RectangleBounds::bv_halfX],
                     -m_boundValues[RectangleBounds::bv_halfY]);
  elementP[3] = (rotMatrix * (p - locpo));
  std::vector<Amg::Vector2D> axis = { elementP[0] - elementP[1],
                                      elementP[0] - elementP[2],
                                      elementP[0] - elementP[3],
                                      elementP[1] - elementP[2] };
  bchk.ComputeKDOP(elementP, axis, elementKDOP);
  // compute KDOP for error ellipse
  std::vector<KDOP> errelipseKDOP(4);
  bchk.ComputeKDOP(bchk.EllipseToPoly(3), axis, errelipseKDOP);
  // check if KDOPs overlap and return result
  return bchk.TestKDOPKDOP(elementKDOP, errelipseKDOP);
}

double
Trk::RectangleBounds::minDistance(const Amg::Vector2D& pos) const
{
  double dx = std::abs(pos[0]) - m_boundValues[RectangleBounds::bv_halfX];
  double dy = std::abs(pos[1]) - m_boundValues[RectangleBounds::bv_halfY];

  if (dx <= 0. || dy <= 0.) {
    if (dx > dy){
      return dx;
    }
    return dy;
  }
  return sqrt(dx * dx + dy * dy);
}

// ostream operator overload
MsgStream&
Trk::RectangleBounds::dump(MsgStream& sl) const
{
  sl << std::setiosflags(std::ios::fixed);
  sl << std::setprecision(7);
  sl << "Trk::RectangleBounds:  (halflenghtX, halflengthY) = "
     << "(" << m_boundValues[RectangleBounds::bv_halfX] << ", " << m_boundValues[RectangleBounds::bv_halfY] << ")";
  sl << std::setprecision(-1);
  return sl;
}

std::ostream&
Trk::RectangleBounds::dump(std::ostream& sl) const
{
  sl << std::setiosflags(std::ios::fixed);
  sl << std::setprecision(7);
  sl << "Trk::RectangleBounds:  (halflenghtX, halflengthY) = "
     << "(" << m_boundValues[RectangleBounds::bv_halfX] << ", " << m_boundValues[RectangleBounds::bv_halfY] << ")";
  sl << std::setprecision(-1);
  return sl;
}
