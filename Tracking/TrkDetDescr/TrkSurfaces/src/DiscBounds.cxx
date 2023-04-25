/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// DiscBounds.cxx, (c) ATLAS Detector Software
///////////////////////////////////////////////////////////////////

// Trk
#include "TrkSurfaces/DiscBounds.h"
// Gaudi
#include "GaudiKernel/MsgStream.h"
// STD
#include <iomanip>
#include <iostream>

Trk::DiscBounds::DiscBounds()
  : m_boundValues(DiscBounds::bv_length, 0.)
{}

Trk::DiscBounds::DiscBounds(double minrad, double maxrad, double hphisec)
  : m_boundValues(DiscBounds::bv_length, 0.)
{
  m_boundValues[DiscBounds::bv_rMin] = minrad;
  m_boundValues[DiscBounds::bv_rMax] = maxrad;
  m_boundValues[DiscBounds::bv_averagePhi] = 0.;
  m_boundValues[DiscBounds::bv_halfPhiSector] = hphisec;
  if (m_boundValues[DiscBounds::bv_rMin] > m_boundValues[DiscBounds::bv_rMax])
    swap(m_boundValues[DiscBounds::bv_rMin], m_boundValues[DiscBounds::bv_rMax]);
}

Trk::DiscBounds::DiscBounds(double minrad, double maxrad, double avephi, double hphisec)
  : m_boundValues(DiscBounds::bv_length, 0.)
{
  m_boundValues[DiscBounds::bv_rMin] = minrad;
  m_boundValues[DiscBounds::bv_rMax] = maxrad;
  m_boundValues[DiscBounds::bv_averagePhi] = avephi;
  m_boundValues[DiscBounds::bv_halfPhiSector] = hphisec;
  if (m_boundValues[DiscBounds::bv_rMin] > m_boundValues[DiscBounds::bv_rMax])
    swap(m_boundValues[DiscBounds::bv_rMin], m_boundValues[DiscBounds::bv_rMax]);
}


bool
Trk::DiscBounds::operator==(const Trk::SurfaceBounds& sbo) const
{
  // check the type first not to compare apples with oranges
  const Trk::DiscBounds* discbo = dynamic_cast<const Trk::DiscBounds*>(&sbo);
  if (!discbo)
    return false;
  return (m_boundValues == discbo->m_boundValues);
}

bool
Trk::DiscBounds::inside(const Amg::Vector2D& locpo, double tol1, double tol2) const
{
  double alpha = std::abs(locpo[locPhi] - m_boundValues[DiscBounds::bv_averagePhi]);
  if (alpha > M_PI)
    alpha = 2 * M_PI - alpha;
  bool insidePhi =
    (alpha <= (m_boundValues[DiscBounds::bv_halfPhiSector] + tol2));
  return (locpo[locR] > (m_boundValues[DiscBounds::bv_rMin] - tol1) &&
          locpo[locR] < (m_boundValues[DiscBounds::bv_rMax] + tol1) &&
          insidePhi);
}

bool
Trk::DiscBounds::inside(const Amg::Vector2D& locpo, const BoundaryCheck& bchk) const
{
  if (bchk.bcType == 0 || bchk.nSigmas == 0 ||
      m_boundValues[DiscBounds::bv_rMin] != 0 ||
      m_boundValues[DiscBounds::bv_halfPhiSector] != M_PI)
    return DiscBounds::inside(locpo, bchk.toleranceLoc1, bchk.toleranceLoc2);

  // a fast FALSE
  sincosCache scResult = bchk.FastSinCos(locpo(1, 0));
  double dx = bchk.nSigmas * sqrt(bchk.lCovariance(0, 0));
  double dy =
    bchk.nSigmas * sqrt(scResult.sinC * scResult.sinC * bchk.lCovariance(0, 0) +
                        locpo(0, 0) * locpo(0, 0) * scResult.cosC *
                          scResult.cosC * bchk.lCovariance(1, 1) +
                        2 * scResult.cosC * scResult.sinC * locpo(0, 0) *
                          bchk.lCovariance(0, 1));
  double max_ell = dx > dy ? dx : dy;
  if (locpo(0, 0) > (m_boundValues[DiscBounds::bv_rMax] + max_ell))
    return false;
  // a fast TRUE
  double min_ell = dx < dy ? dx : dy;
  if (locpo(0, 0) < (m_boundValues[DiscBounds::bv_rMax] + min_ell))
    return true;

  // we are not using the KDOP approach here but rather a highly optimized one
  class EllipseCollisionTest
  {
  private:
    int m_maxIterations;
    bool iterate(double x,
                 double y,
                 double c0x,
                 double c0y,
                 double c2x,
                 double c2y,
                 double rr) const
    {
      std::vector<double> innerPolygonCoef(m_maxIterations + 1);
      std::vector<double> outerPolygonCoef(m_maxIterations + 1);
      /*
         t2______t4
     --_     	\
          --_	 \	              				    /¨¨¨ ¨¨\
                    t1 = (0, 0)   	           (     t   )
                        | \ \__ _ /
                        |   \
                        |    t3
                    |   /
                    | /
                 t0
      */
      for (int t = 1; t <= m_maxIterations; t++) {
        int numNodes = 4 << t;
        innerPolygonCoef[t] = 0.5 / cos(2.0*M_PI / numNodes);
        double c1x = (c0x + c2x) * innerPolygonCoef[t];
        double c1y = (c0y + c2y) * innerPolygonCoef[t];
        double tx = x - c1x; // t indicates a translated coordinate
        double ty = y - c1y;
        if (tx * tx + ty * ty <= rr) {
          return true; // collision with t1
        }
        double t2x = c2x - c1x;
        double t2y = c2y - c1y;
        if (tx * t2x + ty * t2y >= 0 &&
            tx * t2x + ty * t2y <= t2x * t2x + t2y * t2y &&
            (ty * t2x - tx * t2y >= 0 ||
             rr * (t2x * t2x + t2y * t2y) >=
               (ty * t2x - tx * t2y) * (ty * t2x - tx * t2y))) {
          return true; // collision with t1---t2
        }
        double t0x = c0x - c1x;
        double t0y = c0y - c1y;
        if (tx * t0x + ty * t0y >= 0 &&
            tx * t0x + ty * t0y <= t0x * t0x + t0y * t0y &&
            (ty * t0x - tx * t0y <= 0 ||
             rr * (t0x * t0x + t0y * t0y) >=
               (ty * t0x - tx * t0y) * (ty * t0x - tx * t0y))) {
          return true; // collision with t1---t0
        }
        outerPolygonCoef[t] =
          0.5 / (std::cos(M_PI / numNodes) * std::cos(M_PI / numNodes));
        double c3x = (c0x + c1x) * outerPolygonCoef[t];
        double c3y = (c0y + c1y) * outerPolygonCoef[t];
        if ((c3x - x) * (c3x - x) + (c3y - y) * (c3y - y) < rr) {
          c2x = c1x;
          c2y = c1y;
          continue; // t3 is inside circle
        }
        double c4x = c1x - c3x + c1x;
        double c4y = c1y - c3y + c1y;
        if ((c4x - x) * (c4x - x) + (c4y - y) * (c4y - y) < rr) {
          c0x = c1x;
          c0y = c1y;
          continue; // t4 is inside circle
        }
        double t3x = c3x - c1x;
        double t3y = c3y - c1y;
        if (ty * t3x - tx * t3y <= 0 ||
            rr * (t3x * t3x + t3y * t3y) >
              (ty * t3x - tx * t3y) * (ty * t3x - tx * t3y)) {
          if (tx * t3x + ty * t3y > 0) {
            if (std::abs(tx * t3x + ty * t3y) <= t3x * t3x + t3y * t3y ||
                (x - c3x) * (c0x - c3x) + (y - c3y) * (c0y - c3y) >= 0) {
              c2x = c1x;
              c2y = c1y;
              continue; // circle center is inside t0---t1---t3
            }
          } else if (-(tx * t3x + ty * t3y) <= t3x * t3x + t3y * t3y ||
                     (x - c4x) * (c2x - c4x) + (y - c4y) * (c2y - c4y) >= 0) {
            c0x = c1x;
            c0y = c1y;
            continue; // circle center is inside t1---t2---t4
          }
        }
        return false; // no collision possible
      }
      return false; // out of iterations so it is unsure if there was a
                    // collision. But have to return something.
    }

  public:
    // test for collision between an ellipse of horizontal radius w and vertical
    // radius h at (x0, y0) and a circle of radius r at (x1, y1)
    bool collide(double x0,
                 double y0,
                 double w,
                 double h,
                 double x1,
                 double y1,
                 double r) const
    {
      double x = std::abs(x1 - x0);
      double y = std::abs(y1 - y0);
      if (x * x + (h - y) * (h - y) <= r * r ||
          (w - x) * (w - x) + y * y <= r * r ||
          x * h + y * w <= w * h // collision with (0, h)
          || ((x * h + y * w - w * h) * (x * h + y * w - w * h) <=
                r * r * (w * w + h * h) &&
              x * w - y * h >= -h * h &&
              x * w - y * h <= w * w)) { // collision with (0, h)---(w, 0)
        return true;
      } else {
        if ((x - w) * (x - w) + (y - h) * (y - h) <= r * r ||
            (x <= w && y - r <= h) || (y <= h && x - r <= w)) {
          return iterate(
            x, y, w, 0, 0, h, r * r); // collision within triangle (0, h) (w, h)
                                      // (0, 0) is possible
        }
        return false;
      }
    }
    explicit EllipseCollisionTest(int maxIterations)
    {
      this->m_maxIterations = maxIterations;
    }
  };

  EllipseCollisionTest test(4);
  // convert to cartesian coordinates
  AmgMatrix(2, 2) covRotMatrix;
  // cppcheck-suppress constStatement
  covRotMatrix << scResult.cosC, -locpo(0, 0) * scResult.sinC, scResult.sinC,
    locpo(0, 0) * scResult.cosC;
  AmgMatrix(2, 2) lCovarianceCar =
    covRotMatrix * bchk.lCovariance * covRotMatrix.transpose();
  Amg::Vector2D locpoCar(covRotMatrix(1, 1), -covRotMatrix(0, 1));

  // ellipse is always at (0,0), surface is moved to ellipse position and then
  // rotated
  double w = bchk.nSigmas * sqrt(lCovarianceCar(0, 0));
  double h = bchk.nSigmas * sqrt(lCovarianceCar(1, 1));
  double x0 = 0;
  double y0 = 0;
  float theta =
    (lCovarianceCar(1, 0) != 0 &&
     (lCovarianceCar(1, 1) - lCovarianceCar(0, 0)) != 0)
      ? .5 * bchk.FastArcTan(2 * lCovarianceCar(1, 0) /
                             (lCovarianceCar(1, 1) - lCovarianceCar(0, 0)))
      : 0.;
  scResult = bchk.FastSinCos(theta);
  AmgMatrix(2, 2) rotMatrix;
  rotMatrix << scResult.cosC, scResult.sinC, -scResult.sinC, scResult.cosC;
  Amg::Vector2D tmp = rotMatrix * (-locpoCar);
  double x1 = tmp(0, 0);
  double y1 = tmp(1, 0);
  double r = m_boundValues[DiscBounds::bv_rMax];
  // check if ellipse and circle overlap and return result
  return test.collide(x0, y0, w, h, x1, y1, r);
}

double
Trk::DiscBounds::minDistance(const Amg::Vector2D& pos) const
{
  const double pi2 = 2. * M_PI;

  double r = pos[locR];
  if (r == 0.)
    return m_boundValues[DiscBounds::bv_rMin];
  double sf = 0.;
  double dF = 0.;

  if (m_boundValues[DiscBounds::bv_halfPhiSector] < M_PI) {

    dF = std::abs(pos[locPhi] - m_boundValues[DiscBounds::bv_averagePhi]);
    if (dF > M_PI)
      dF = pi2 - dF;
    dF -= m_boundValues[DiscBounds::bv_halfPhiSector];
    sf = r * sin(dF);
    if (dF > 0.)
      r *= cos(dF);

  } else {
    sf = -1.e+10;
  }

  if (sf <= 0.) {

    double sr0 = m_boundValues[DiscBounds::bv_rMin] - r;
    if (sr0 > 0.)
      return sr0;
    double sr1 = r - m_boundValues[DiscBounds::bv_rMax];
    if (sr1 > 0.)
      return sr1;
    if (sf < sr0)
      sf = sr0;
    if (sf < sr1)
      sf = sr1;
    return sf;
  }

  double sr0 = m_boundValues[DiscBounds::bv_rMin] - r;
  if (sr0 > 0.)
    return sqrt(sr0 * sr0 + sf * sf);
  double sr1 = r - m_boundValues[DiscBounds::bv_rMax];
  if (sr1 > 0.)
    return sqrt(sr1 * sr1 + sf * sf);
  return sf;
}

// ostream operator overload

MsgStream&
Trk::DiscBounds::dump(MsgStream& sl) const
{
  sl << std::setiosflags(std::ios::fixed);
  sl << std::setprecision(7);
  sl << "Trk::DiscBounds:  (innerRadius, outerRadius, averagePhi, hPhiSector) = ";
  sl << "(" << this->rMin() << ", " << this->rMax() << ", " << this->averagePhi() << ", " << this->halfPhiSector()
     << ")";
  sl << std::setprecision(-1);
  return sl;
}

std::ostream&
Trk::DiscBounds::dump(std::ostream& sl) const
{
  sl << std::setiosflags(std::ios::fixed);
  sl << std::setprecision(7);
  sl << "Trk::DiscBounds:  (innerRadius, outerRadius, hPhiSector) = ";
  sl << "(" << this->rMin() << ", " << this->rMax() << ", " << this->averagePhi() << ", " << this->halfPhiSector()
     << ")";
  sl << std::setprecision(-1);
  return sl;
}
