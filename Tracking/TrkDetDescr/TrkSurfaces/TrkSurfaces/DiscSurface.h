/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// DiscSurface.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKSURFACES_DISCSURFACE_H
#define TRKSURFACES_DISCSURFACE_H

// Trk
#include "TrkDetDescrUtils/SharedObject.h"
#include "TrkEventPrimitives/LocalParameters.h"
#include "TrkParametersBase/ParametersT.h"
#include "TrkSurfaces/NoBounds.h"
#include "TrkSurfaces/Surface.h"
#include "TrkSurfaces/SurfaceBounds.h"
// Amg
#include "EventPrimitives/EventPrimitives.h"
#include "GeoPrimitives/GeoPrimitives.h"
// std
#include <cmath> //for cos, sin etc

class MsgStream;
template<class SURFACE, class BOUNDS_CNV>
class BoundSurfaceCnv_p1;
template<class SURFACE, class BOUNDS_CNV>
class BoundSurfaceCnv_p2;

namespace Trk {

class DiscBounds;
class DiscTrapezoidalBounds;
class AnnulusBounds;
class AnnulusBoundsPC;
class TrkDetElementBase;
class LocalParameters;

template<int DIM, class T, class S>
class ParametersT;

/**
 @class DiscSurface
 Class for a DiscSurface in the ATLAS detector.
 It inherits from Surface.

 @author Andreas.Salzburger@cern.ch
 @author Christos Anastopoulos (Thread safety and interface cleanup)
 @author Shaun Roe (interface cleanup)
 */

class DiscSurface : public Surface
{

public:
  static constexpr SurfaceType staticType = SurfaceType::Disc;
  /**Default Constructor*/
  DiscSurface();

  /**Copy Constructor*/
  DiscSurface(const DiscSurface& psf);

  /**Assignement operator*/
  DiscSurface& operator=(const DiscSurface& dsf);

  /**Copy Constructor*/
  DiscSurface(DiscSurface&& psf) noexcept = default;

  /**Assignement operator*/
  DiscSurface& operator=(DiscSurface&& dsf) noexcept = default;

  /**Destructor*/
  virtual ~DiscSurface() = default;

  /**Constructor for Discs from HepGeom::Transform3D, \f$ r_{min}, r_{max} \f$
   */
  DiscSurface(const Amg::Transform3D& htrans, double rmin, double rmax);

  /**Constructor for Discs from HepGeom::Transform3D, \f$ r_{min}, r_{max},
   * \phi_{hsec} \f$ */
  DiscSurface(const Amg::Transform3D& htrans,
              double rmin,
              double rmax,
              double hphisec);

  /**Constructor for Discs from HepGeom::Transform3D, \f$ r_{min}, r_{max},
     hx_{min}, hx_{max} \f$ In this case you have DiscTrapezoidalBounds*/
  DiscSurface(const Amg::Transform3D& htrans,
              double minhalfx,
              double maxhalfx,
              double maxR,
              double minR,
              double avephi,
              double stereo = 0.);

  /**Constructor for Discs from HepGeom::Transform3D and DiscBounds
     - ownership of bounds is passed */
  DiscSurface(const Amg::Transform3D& htrans, DiscBounds* dbounds);

  /**Constructor for Discs from HepGeom::Transform3D and DiscTrapezoidalBounds
     - ownership of bounds is passed */
  DiscSurface(const Amg::Transform3D& htrans, DiscTrapezoidalBounds* dtbounds);

  /**Constructor for Discs from HepGeom::Transform3D and AnnulusBoundsPC
	   - ownership of bounds is passed */
  DiscSurface(const Amg::Transform3D& htrans, AnnulusBoundsPC* annpcbounds);

  /// @brief Constructor for Discs from Transform3D and AnnulusBoundsPC
  /// This will use the converting factory in @c AnnulusBoundsPC to
  /// build an equivalent PC version.
  /// The overall transform of this object will be @c htrans and an additional
  /// rotation to overlay the two bounds implementations exactly.
  /// @param htrans Base transform. This should be the transform of the @c PlaneSurface
  ///               on which @c annbounds is located
  /// @param annbounds Cartesian annulus bounds to convert
  /// @param detElem Additional explicit detector element
  /// @note During testing, no conversion of the underlying detector element was implemented.
  ///       This polar coordinate surface will therefore link back to the cartesian detector
  ///       element.
  DiscSurface(const Amg::Transform3D& htrans, std::unique_ptr<AnnulusBounds> annbounds, const TrkDetElementBase* detElem = nullptr);

  /**Constructor for Discs from HepGeom::Transform3D by unique_ptr
   - bounds is not set */
  DiscSurface(const Amg::Transform3D& htrans);

  /**Constructor for DiscSegment from DetectorElement*/
  DiscSurface(const TrkDetElementBase& dmnt);

  /**Copy Constructor with shift*/
  DiscSurface(const DiscSurface& psf, const Amg::Transform3D& transf);

  /**Equality operator*/
  virtual bool operator==(const Surface& sf) const override;
  bool operator==(const DiscSurface& cf) const;

  /** Virtual constructor*/
  virtual DiscSurface* clone() const override;

  /** Use the Surface as a ParametersBase constructor, from local parameters -
   * charged */
  virtual Surface::ChargedTrackParametersUniquePtr createUniqueTrackParameters(
    double l1,
    double l2,
    double phi,
    double theta,
    double qop,
    std::optional<AmgSymMatrix(5)> cov = std::nullopt) const override final;

  /** Use the Surface as a ParametersBase constructor, from global parameters -
   * charged*/
  virtual Surface::ChargedTrackParametersUniquePtr createUniqueTrackParameters(
    const Amg::Vector3D& position,
    const Amg::Vector3D& momentum,
    double charge,
    std::optional<AmgSymMatrix(5)> cov = std::nullopt) const override final;

  /** Use the Surface as a ParametersBase constructor, from local parameters -
   * neutral */
  virtual NeutralTrackParametersUniquePtr createUniqueNeutralParameters(
    double l1,
    double l2,
    double phi,
    double theta,
    double qop,
    std::optional<AmgSymMatrix(5)> cov = std::nullopt) const override final;

  /** Use the Surface as a ParametersBase constructor, from global parameters -
   * neutral */
  virtual NeutralTrackParametersUniquePtr createUniqueNeutralParameters(
    const Amg::Vector3D& position,
    const Amg::Vector3D& momentum,
    double charge,
    std::optional<AmgSymMatrix(5)> cov = std::nullopt) const override final;

  /** Use the Surface as a ParametersBase constructor, from local parameters */
  template<int DIM, class T>
  std::unique_ptr<ParametersT<DIM, T, DiscSurface>> createUniqueParameters(
    double l1,
    double l2,
    double phi,
    double theta,
    double qop,
    std::optional<AmgSymMatrix(DIM)> cov = std::nullopt) const;

  /** Use the Surface as a ParametersBase constructor, from global parameters */
  template<int DIM, class T>
  std::unique_ptr<ParametersT<DIM, T, DiscSurface>> createUniqueParameters(
    const Amg::Vector3D& position,
    const Amg::Vector3D& momentum,
    double charge,
    std::optional<AmgSymMatrix(DIM)> cov = std::nullopt) const;

  /** Return the surface type */
  virtual SurfaceType type() const override final;

  /** Returns a global reference point:
    For the Disc this is @f$ (R*cos(\phi), R*sin(\phi),0)*transform() @f$
    Where  @f$ r,  \phi @f$ denote the r(), averagePhi() of the Bounds.
  */
  virtual const Amg::Vector3D& globalReferencePoint() const override final;

  /**This method returns the bounds by reference*/
  const SurfaceBounds& bounds() const override final;

  /**This method calls the inside method of the bounds*/
  virtual bool insideBounds(const Amg::Vector2D& locpos,
                            double tol1 = 0.,
                            double tol2 = 0.) const override;
  virtual bool insideBoundsCheck(
    const Amg::Vector2D& locpos,
    const BoundaryCheck& bchk) const override final;

  /** This method returns true if the GlobalPosition is on the Surface for both,
    within or without check of whether the local position is inside boundaries
    or not */
  virtual bool isOnSurface(const Amg::Vector3D& glopo,
                           const BoundaryCheck& bchk = true,
                           double tol1 = 0.,
                           double tol2 = 0.) const override;

  /** Specialized for DiscSurface : LocalParameters to Vector2D */
  virtual Amg::Vector2D localParametersToPosition(
    const LocalParameters& locpars) const override final;

  /** Specialized for DiscSurface: LocalToGlobal method without dynamic memory
   * allocation */
  virtual void localToGlobal(const Amg::Vector2D& locp,
                             const Amg::Vector3D& mom,
                             Amg::Vector3D& glob) const override;

  /** Specialized for DiscSurface: GlobalToLocal method without dynamic memory
   * allocation - boolean checks if on surface
   */
  virtual bool globalToLocal(const Amg::Vector3D& glob,
                             const Amg::Vector3D& mom,
                             Amg::Vector2D& loc) const override;

  /** fast straight line intersection schema - standard: provides closest
     intersection and (signed) path length forceDir is to provide the closest
     forward solution

      <b>mathematical motivation:</b>

      the equation of the plane is given by: <br>
      @f$ \vec n \cdot \vec x = \vec n \cdot \vec p,@f$ <br>
      where @f$ \vec n = (n_{x}, n_{y}, n_{z})@f$ denotes the normal vector of
     the plane,
      @f$ \vec p = (p_{x}, p_{y}, p_{z})@f$ one specific point on the plane and
     @f$ \vec x = (x,y,z) @f$ all possible points on the plane.<br> Given a line
     with:<br>
      @f$ \vec l(u) = \vec l_{1} + u \cdot \vec v @f$, <br>
      the solution for @f$ u @f$ can be written:
      @f$ u = \frac{\vec n (\vec p - \vec l_{1})}{\vec n \vec v}@f$ <br>
      If the denominator is 0 then the line lies:
      - either in the plane
      - perpenticular to the normal of the plane
   */
  virtual Intersection straightLineIntersection(
    const Amg::Vector3D& pos,
    const Amg::Vector3D& dir,
    bool forceDir = false,
    Trk::BoundaryCheck bchk = false) const override final;

  /** fast straight line distance evaluation to Surface */
  virtual DistanceSolution straightLineDistanceEstimate(
    const Amg::Vector3D& pos,
    const Amg::Vector3D& dir) const override;

  /** fast straight line distance evaluation to Surface - with bound option*/
  virtual DistanceSolution straightLineDistanceEstimate(
    const Amg::Vector3D& pos,
    const Amg::Vector3D& dir,
    bool Bound) const override;

  /** Return properly formatted class name for screen output */
  virtual std::string name() const override;

protected: //!< data members
  template<class SURFACE, class BOUNDS_CNV>
  friend class ::BoundSurfaceCnv_p1;
  template<class SURFACE, class BOUNDS_CNV>
  friend class ::BoundSurfaceCnv_p2;
  //!< bounds (shared)
  SharedObject<const SurfaceBounds> m_bounds;
  //!< reference Point on the Surface
  CxxUtils::CachedUniquePtr<Amg::Vector3D> m_referencePoint;
  //!< static member for boundless approach
  static const NoBounds s_boundless;
};

} // end of namespace
#include "TrkSurfaces/DiscSurface.icc"

#endif // TRKSURFACES_DISCSURFACE_H
