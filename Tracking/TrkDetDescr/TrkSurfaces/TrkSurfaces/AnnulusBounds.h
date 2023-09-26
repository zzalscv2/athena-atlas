/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// AnnulusBounds.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKSURFACES_ANNULUSBOUNDS_H
#define TRKSURFACES_ANNULUSBOUNDS_H

#include "TrkSurfaces/SurfaceBounds.h"
//#include "TrkEventPrimitives/ParamDefs.h"
//#include <math.h>
#include <iosfwd> //ostream fwd declaration
#include <vector>

#include "GeoPrimitives/GeoPrimitives.h"

class MsgStream;

#ifdef TRKDETDESCR_USEFLOATPRECISON
typedef float TDD_real_t;
#else
typedef double TDD_real_t;
#endif

namespace Trk {

/**
 @class AnnulusBounds
 Bounds for a annulus-like, planar Surface.

 @internal
 @image html AnnulusBounds.gif
 @endinternal
    <br>

 @todo can be speed optimized, inner radius check in inside() can be optimized

 @author Marcin.Wolter@cern.ch
 */

class AnnulusBounds final: public SurfaceBounds
{

public:
  /** @enum BoundValues - for readability */
  /** NB bv_R is the radius of the wafer centre which may different from the assumed surface centre*/
  /** e.g. if a single wafer/sensor/mdodule has been split into multiple surfaces*/
  enum BoundValues
  {
    bv_minR = 0,
    bv_maxR = 1,
    bv_R = 2,
    bv_phi = 3,
    bv_phiS = 4,
    bv_length = 5
  };

  /**Default Constructor, needed for persistency*/
  AnnulusBounds();
  /**Constructor for AnnulusBounds*/
  AnnulusBounds(double minR, double maxR, double R, double phi, double phiS);

  /**Copy constructor*/
  AnnulusBounds(const AnnulusBounds& annbo) = default;
  /**Assignment operator*/
  AnnulusBounds& operator=(const AnnulusBounds& sbo) = default;
  /** Move constructor */
  AnnulusBounds(AnnulusBounds&& annbo) = default;
  /** Move assignment */
  AnnulusBounds& operator=(AnnulusBounds&& sbo) = default;

  /**Destructor*/
  virtual ~AnnulusBounds() = default;

  /**Virtual constructor*/
  virtual AnnulusBounds* clone() const override;

  /** Return the type of the bounds for persistency */
  virtual BoundsType type() const override { return SurfaceBounds::Annulus; }

  /**Equality operator*/
  bool operator==(const SurfaceBounds& annbo) const override;

  /**This method returns the smaller radius*/
  double minR() const;

  /**This method returns the bigger radius*/
  double maxR() const;

  /**This method returns the R-parameter from design of
    sensors, which is the radius that the original centre of a silicon
     wafer ends up at. It is therefore common to all surfaces on the same wafer
     and can be different from the assume surface centre*/
  double waferCentreR() const;

  /**This method returns the opening angle*/
  double phi() const;

  /**This method returns the tilt angle*/
  double phiS() const;

  /**
   * @brief Returns the four corners of the bounds
   * 
   * Returns the module corners starting from the upper right (max R, pos locX) and proceding clock-wise, 
   * i.e. (max R; pos locX), (min R; pos locX), (min R; neg loc X), (max R; neg locX).
   * 
   * This method is only intended for debug purposes. If used for production code, this should be changed to a
   * return-by-reference. This will necessitate the vector being stored in the class.
   * 
   * @return array of pairs of doubles giving the (R, x) of each corner clockwise from upper-right
   * */
  std::array<std::pair<double, double>, 4> corners() const;

  /**This method returns the maximal extension on the local plane*/
  virtual double r() const override;

  /**This method returns the opening angle alpha in point A (negative local phi) */
  //      double alpha() const;

  /**This method returns the opening angle beta in point B (positive local phi) */
  //      double beta() const;

  /** The orientation of the Trapezoid is according to the figure above,
   in words: the shorter of the two parallel sides of the trapezoid intersects
   with the negative @f$ y @f$ - axis of the local frame.

   <br>
   The cases are:<br>
   (0) @f$ y @f$ or @f$ x @f$ bounds are 0 || 0<br>
   (1) LocalPosition is outside @f$ y @f$ bounds <br>
   (2) LocalPosition is inside @f$ y @f$ bounds, but outside maximum @f$ x @f$ bounds  <br>
   (3) LocalPosition is inside @f$ y @f$ bounds AND inside minimum @f$ x @f$ bounds <br>
   (4) LocalPosition is inside @f$ y @f$ bounds AND inside maximum @f$ x @f$ bounds, so that
   it depends on the @f$ eta @f$ coordinate
   (5) LocalPosition fails test of (4) <br>

   The inside check is done using single equations of straight lines and one has to take care if a point
   lies on the positive @f$ x @f$ half area(I) or the negative one(II). Denoting @f$ |x_{min}| @f$ and
   @f$ | x_{max} | @f$ as \c minHalfX respectively \c maxHalfX, such as @f$ | y_{H} | @f$ as \c halfY,
   the equations for the straing lines in (I) and (II) can be written as:<br>
    <br>
   - (I):  @f$ y = \kappa_{I} x + \delta_{I} @f$ <br>
   - (II): @f$ y = \kappa_{II} x + \delta_{II} @f$ ,<br>
    <br>
   where @f$  \kappa_{I} = - \kappa_{II} = 2 \frac{y_{H}}{x_{max} - x_{min}} @f$ <br>
   and   @f$  \delta_{I} = \delta_{II} = - \frac{1}{2}\kappa_{I}(x_{max} + x_{min}) @f$  */
  virtual bool inside(const Amg::Vector2D& locpo, double tol1 = 0., double tol2 = 0.) const override final;
  virtual bool inside(const Amg::Vector2D& locpo, const BoundaryCheck& bchk) const override final;

  /** This method checks inside bounds in loc1
  - loc1/loc2 correspond to the natural coordinates of the surface
  - As loc1/loc2 are correlated the single check doesn't make sense :
     -> check is done on enclosing Rectangle ! */

  virtual bool insideLoc1(const Amg::Vector2D& locpo, double tol1 = 0.) const override final;

  /** This method checks inside bounds in loc2
  - loc1/loc2 correspond to the natural coordinates of the surface
  - As loc1/loc2 are correlated the single check doesn't make sense :
     -> check is done on enclosing Rectangle !  */
  virtual bool insideLoc2(const Amg::Vector2D& locpo, double tol2 = 0.) const override final;

  /** Minimal distance to boundary ( > 0 if outside and <=0 if inside) */
  virtual double minDistance(const Amg::Vector2D& pos) const override final;

  /** Output Method for MsgStream*/
  virtual MsgStream& dump(MsgStream& sl) const override;

  /** Output Method for std::ostream */
  virtual std::ostream& dump(std::ostream& sl) const override;

  /**
   * @brief Returns the gradient and y-intercept of the left and right module edges.
   * 
   * This method is only intended for debug purposes. If used for production code, this should be changed to a
   * return-by-reference. This will necessitate the vector being stored in the class.
   * 
   * @return Vector with the gradients (m) and intercepts (c) of the left (_L) and right (_R) edges. [m_L, m_R, c_L, c_R] 
   */
  std::array<TDD_real_t,4> getEdgeLines() const;

  const std::vector<TDD_real_t>& getBoundsValues();

private:
  //      bool m_forceCovEllipse;

  /** isAbove() method for checking whether a point lies above or under a straight line */

  static bool isAbove(const Amg::Vector2D& locpo, double tol1, double tol2, double x1, double y1, double x2, double y2) ;

  static bool isRight(const Amg::Vector2D& locpo, double tol1, double tol2, double x1, double y1, double x2, double y2) ;

  static bool isLeft(const Amg::Vector2D& locpo, double tol1, double tol2, double x1, double y1, double x2, double y2) ;

  // check whether an ellipse intersects a line
  static bool EllipseIntersectLine(const Amg::Vector2D& locpo, double h, double k, double x1, double y1, double x2, double y2)
    ;

  /** Distance to line */
  static double distanceToLine(const Amg::Vector2D& locpo, const std::vector<TDD_real_t>& P1, const std::vector<TDD_real_t>& P2) ;

  /** Distance to arc */
  static double distanceToArc(const Amg::Vector2D& locpo,
                       double R,
                       const std::vector<TDD_real_t>& sL,
                       const std::vector<TDD_real_t>& sR) ;

  /** 
   * @brief Circle and line intersection. \n 
   * 
   * Circle is of radius R and centred at the origin. Line takes the form y = kx + d
   * 
   * @param R Radius of the circle
   * @param k Gradient of the line
   * @param d Intercept of the line
   * @return Co-ordinates of the intercept with highest y
   **/
  static std::vector<double> circleLineIntersection(double R, double k, double d) ;

  std::vector<TDD_real_t> m_boundValues;

  TDD_real_t m_maxYout;
  TDD_real_t m_minYout;
  TDD_real_t m_maxXout;
  TDD_real_t m_minXout;

  TDD_real_t m_maxYin;
  TDD_real_t m_minYin;
  TDD_real_t m_maxXin;
  TDD_real_t m_minXin;

  TDD_real_t m_k_L;
  TDD_real_t m_k_R;
  TDD_real_t m_d_L;
  TDD_real_t m_d_R;

  std::vector<TDD_real_t> m_solution_L_min;
  std::vector<TDD_real_t> m_solution_L_max;
  std::vector<TDD_real_t> m_solution_R_min;
  std::vector<TDD_real_t> m_solution_R_max;
};

} // end of namespace

#include "TrkSurfaces/AnnulusBounds.icc"
#endif // TRKSURFACES_ANNULUSBOUNDS_H
