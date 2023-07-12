/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// DiscTrapezoidalBounds.cxx, (c) ATLAS Detector Software
///////////////////////////////////////////////////////////////////

// Trk
#include "TrkSurfaces/DiscTrapezoidalBounds.h"
// Gaudi
#include "GaudiKernel/MsgStream.h"
// STD
#include <iomanip>
#include <iostream>

Trk::DiscTrapezoidalBounds::DiscTrapezoidalBounds()
  : m_boundValues(DiscTrapezoidalBounds::bv_length, 0.)
{}

Trk::DiscTrapezoidalBounds::DiscTrapezoidalBounds(double minhalfx,
                                                  double maxhalfx,
                                                  double maxR,
                                                  double minR,
                                                  double avephi,
                                                  double stereo)
  : m_boundValues(DiscTrapezoidalBounds::bv_length, 0.)
{
  m_boundValues[DiscTrapezoidalBounds::bv_averagePhi] = avephi;
  m_boundValues[DiscTrapezoidalBounds::bv_stereo] = stereo;

  m_boundValues[DiscTrapezoidalBounds::bv_minHalfX] = minhalfx;
  m_boundValues[DiscTrapezoidalBounds::bv_maxHalfX] = maxhalfx;
  if (m_boundValues[DiscTrapezoidalBounds::bv_minHalfX] > m_boundValues[DiscTrapezoidalBounds::bv_maxHalfX])
    swap(m_boundValues[DiscTrapezoidalBounds::bv_minHalfX], m_boundValues[DiscTrapezoidalBounds::bv_maxHalfX]);

  m_boundValues[DiscTrapezoidalBounds::bv_rMax] = minR;
  m_boundValues[DiscTrapezoidalBounds::bv_rMin] = maxR;
  if (m_boundValues[DiscTrapezoidalBounds::bv_rMin] > m_boundValues[DiscTrapezoidalBounds::bv_rMax])
    swap(m_boundValues[DiscTrapezoidalBounds::bv_rMin], m_boundValues[DiscTrapezoidalBounds::bv_rMax]);

  m_boundValues[DiscTrapezoidalBounds::bv_halfPhiSector] =
    asin(m_boundValues[DiscTrapezoidalBounds::bv_maxHalfX] / m_boundValues[DiscTrapezoidalBounds::bv_rMax]);

  double hmax =
    sqrt(m_boundValues[DiscTrapezoidalBounds::bv_rMax] * m_boundValues[DiscTrapezoidalBounds::bv_rMax] -
         m_boundValues[DiscTrapezoidalBounds::bv_maxHalfX] * m_boundValues[DiscTrapezoidalBounds::bv_maxHalfX]);

  double hmin =
    sqrt(m_boundValues[DiscTrapezoidalBounds::bv_rMin] * m_boundValues[DiscTrapezoidalBounds::bv_rMin] -
         m_boundValues[DiscTrapezoidalBounds::bv_minHalfX] * m_boundValues[DiscTrapezoidalBounds::bv_minHalfX]);

  m_boundValues[DiscTrapezoidalBounds::bv_rCenter] = (hmax + hmin) / 2.;
  m_boundValues[DiscTrapezoidalBounds::bv_halfY] = (hmax - hmin) / 2.;
}

Trk::DiscTrapezoidalBounds::DiscTrapezoidalBounds(const DiscTrapezoidalBounds& disctrbo)
  : Trk::SurfaceBounds()
  , m_boundValues(disctrbo.m_boundValues)
{}


Trk::DiscTrapezoidalBounds&
Trk::DiscTrapezoidalBounds::operator=(const DiscTrapezoidalBounds& disctrbo)
{
  if (this != &disctrbo)
    m_boundValues = disctrbo.m_boundValues;
  return *this;
}

bool
Trk::DiscTrapezoidalBounds::operator==(const Trk::SurfaceBounds& sbo) const
{
  // check the type first not to compare apples with oranges
  const Trk::DiscTrapezoidalBounds* disctrbo = dynamic_cast<const Trk::DiscTrapezoidalBounds*>(&sbo);
  if (!disctrbo)
    return false;
  return (m_boundValues == disctrbo->m_boundValues);
}

bool
Trk::DiscTrapezoidalBounds::inside(const Amg::Vector2D& locpo, double, double) const
{
  double rMedium = m_boundValues[DiscTrapezoidalBounds::bv_rCenter];
  double phi = m_boundValues[DiscTrapezoidalBounds::bv_averagePhi];

  Amg::Vector2D cartCenter(rMedium * cos(phi), rMedium * sin(phi));

  Amg::Vector2D cartPos(locpo[Trk::locR] * cos(locpo[Trk::locPhi]),
                        locpo[Trk::locR] * sin(locpo[Trk::locPhi]));

  Amg::Vector2D Pos = cartPos - cartCenter;

  Amg::Vector2D DeltaPos(Pos[Trk::locX] * sin(phi) - Pos[Trk::locY] * cos(phi),
                         Pos[Trk::locY] * sin(phi) + Pos[Trk::locX] * cos(phi));

  bool insideX = (std::abs(DeltaPos[Trk::locX]) <
                  m_boundValues[DiscTrapezoidalBounds::bv_maxHalfX]);
  bool insideY = (std::abs(DeltaPos[Trk::locY]) <
                  m_boundValues[DiscTrapezoidalBounds::bv_halfY]);

  if (!insideX || !insideY)
    return false;

  if (std::abs(DeltaPos[Trk::locX]) <
      m_boundValues[DiscTrapezoidalBounds::bv_minHalfX])
    return true;

  double m = 2. * m_boundValues[DiscTrapezoidalBounds::bv_halfY] /
             (m_boundValues[DiscTrapezoidalBounds::bv_maxHalfX] -
              m_boundValues[DiscTrapezoidalBounds::bv_minHalfX]);

  double q = m_boundValues[DiscTrapezoidalBounds::bv_halfY] *
             (m_boundValues[DiscTrapezoidalBounds::bv_maxHalfX] +
              m_boundValues[DiscTrapezoidalBounds::bv_minHalfX]) /
             (m_boundValues[DiscTrapezoidalBounds::bv_minHalfX] -
              m_boundValues[DiscTrapezoidalBounds::bv_maxHalfX]);

  bool inside = (DeltaPos[Trk::locY] > (m * std::abs(DeltaPos[Trk::locX]) + q));

  return inside;
}

bool
Trk::DiscTrapezoidalBounds::inside(const Amg::Vector2D& locpo,
                                   const BoundaryCheck& bchk) const
{
  if (bchk.bcType == 0 || bchk.nSigmas == 0 ||
      m_boundValues[DiscTrapezoidalBounds::bv_rMin] != 0)
    return DiscTrapezoidalBounds::inside(
      locpo, bchk.toleranceLoc1, bchk.toleranceLoc2);
  double alpha =
    std::abs(locpo[locPhi] - m_boundValues[DiscTrapezoidalBounds::bv_averagePhi]);
  if (alpha > M_PI)
    alpha = 2 * M_PI - alpha;
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
  if (locpo(0, 0) >
      (m_boundValues[DiscTrapezoidalBounds::bv_rMax] *
         cos(m_boundValues[DiscTrapezoidalBounds::bv_halfPhiSector]) /
         cos(alpha) +
       max_ell))
    return false;
  // a fast TRUE
  double min_ell = dx < dy ? dx : dy;
  if (locpo(0, 0) <
      (m_boundValues[DiscTrapezoidalBounds::bv_rMax] *
         cos(m_boundValues[DiscTrapezoidalBounds::bv_halfPhiSector]) /
         cos(alpha) +
       min_ell))
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
        innerPolygonCoef[t] = 0.5 / cos(2.0f*M_PI / numNodes);
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
    explicit EllipseCollisionTest(int maxIterations) : m_maxIterations(maxIterations)
    {
      
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
  double r = m_boundValues[DiscTrapezoidalBounds::bv_rMax];
  // check if ellipse and circle overlap and return result
  return test.collide(x0, y0, w, h, x1, y1, r);
}


double
Trk::DiscTrapezoidalBounds::minDistance(const Amg::Vector2D& pos) const
{
  const double pi2 = 2. * M_PI;
  double alpha = std::abs(pos[locPhi]);
  if (alpha > M_PI)
    alpha = pi2 - alpha;

  double r = pos[locR];
  if (r == 0.)
    return m_boundValues[DiscTrapezoidalBounds::bv_rMin] * cos(m_boundValues[DiscTrapezoidalBounds::bv_halfPhiSector]) /
           cos(alpha);

  // check if it is external in R
  double sr0 = m_boundValues[DiscTrapezoidalBounds::bv_rMin] *
                 cos(m_boundValues[DiscTrapezoidalBounds::bv_halfPhiSector]) / cos(alpha) -
               r;
  if (sr0 > 0.)
    return sr0;
  double sr1 = r - m_boundValues[DiscTrapezoidalBounds::bv_rMax] *
                     cos(m_boundValues[DiscTrapezoidalBounds::bv_halfPhiSector]) / cos(alpha);
  if (sr1 > 0.)
    return sr1;

  // check if it is external in phi
  if ((alpha - m_boundValues[DiscTrapezoidalBounds::bv_halfPhiSector]) > 0.)
    return r * std::abs(sin(alpha - m_boundValues[DiscTrapezoidalBounds::bv_halfPhiSector]));

  // if here it is inside:
  // Evaluate the distance from the 4 segments
  double dist[4] = { sr0,
                     sr1,
                     -r * sin(m_boundValues[DiscTrapezoidalBounds::bv_halfPhiSector] - alpha),
                     -r * sin(m_boundValues[DiscTrapezoidalBounds::bv_halfPhiSector] + alpha) };

  return *std::max_element(dist, dist + 4);
}

// ostream operator overload

MsgStream&
Trk::DiscTrapezoidalBounds::dump(MsgStream& sl) const
{
  sl << std::setiosflags(std::ios::fixed);
  sl << std::setprecision(7);
  sl << "Trk::DiscTrapezoidalBounds:  (innerRadius, outerRadius, hMinX, hMaxX, hlengthY, hPhiSector, averagePhi, "
        "rCenter, stereo) = ";
  sl << "(" << this->rMin() << ", " << this->rMax() << ", " << this->minHalflengthX() << ", " << this->maxHalflengthX()
     << ", " << this->halflengthY() << ", " << this->halfPhiSector() << ", " << this->averagePhi() << ", "
     << this->rCenter() << ", " << this->stereo() << ")";
  sl << std::setprecision(-1);
  return sl;
}

std::ostream&
Trk::DiscTrapezoidalBounds::dump(std::ostream& sl) const
{
  sl << std::setiosflags(std::ios::fixed);
  sl << std::setprecision(7);
  sl << "Trk::DiscTrapezoidalBounds:  (innerRadius, outerRadius, hMinX, hMaxX, hlengthY, hPhiSector, averagePhi, "
        "rCenter, stereo) = ";
  sl << "(" << this->rMin() << ", " << this->rMax() << ", " << this->minHalflengthX() << ", " << this->maxHalflengthX()
     << ", " << this->halflengthY() << ", " << this->halfPhiSector() << ", " << this->averagePhi() << ", "
     << this->rCenter() << ", " << this->stereo() << ")";
  sl << std::setprecision(-1);
  return sl;
}
