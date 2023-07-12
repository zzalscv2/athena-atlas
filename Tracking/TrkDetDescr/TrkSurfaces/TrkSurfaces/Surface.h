/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// Surface.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKSURFACES_SURFACE_H
#define TRKSURFACES_SURFACE_H

// Amg
// Ensure Eigen plugin comes first
#include "EventPrimitives/EventPrimitives.h"
#include "GeoPrimitives/GeoPrimitives.h"

// Trk
#include "TrkDetDescrUtils/GeometryStatics.h"
#include "TrkDetDescrUtils/Intersection.h"
#include "TrkDetElementBase/TrkDetElementBase.h"
//
#include "TrkEventPrimitives/LocalParameters.h"
#include "TrkEventPrimitives/ParamDefs.h"
#include "TrkEventPrimitives/PropDirection.h"
#include "TrkEventPrimitives/SurfaceTypes.h"
#include "TrkEventPrimitives/SurfaceUniquePtrT.h"
#include "TrkEventPrimitives/TrkObjectCounter.h"
//
#include "TrkParametersBase/Charged.h"
#include "TrkParametersBase/Neutral.h"
#include "TrkParametersBase/ParametersBase.h"
#include "TrkSurfaces/BoundaryCheck.h"
#include "TrkSurfaces/DistanceSolution.h"
// Identifier
#include "Identifier/Identifier.h"
//
#include "CxxUtils/CachedUniquePtr.h"
#include "CxxUtils/checker_macros.h"
#include <atomic>
#include <memory>
#include <optional>

class MsgStream;
class SurfaceCnv_p1;

namespace Trk {

class TrkDetElementBase;
class SurfaceBounds;
class Layer;

enum SurfaceOwner
{
  noOwn = 0,
  TGOwn = 1,
  DetElOwn = 2,
  userOwn = 3
};

/**
 @class Surface

 Abstract Base Class for tracking surfaces

 For all isOnSurface, or positionOnSurface and insideBounds methods two
 tolerance parameters can be given which correspond to the two local natural
 coordinates of the surface loc1, loc2.

 @author Andreas.Salzburger@cern.ch
 @author Christos Anastopoulos (Thread safety and interface cleanup)
 @author Shaun Roe (interface cleanup)
 */

class Surface: public Trk::ObjectCounter<Trk::Surface>
{

public:
  /*
   * struct holding the transform, center, normal,
   * needed when by surfaces when not delegating
   * to a detector element
   */
  struct Transforms
  {
    // constructor with just a Amg::Transform3D input
    inline Transforms(const Amg::Transform3D& atransform)
      : transform(atransform)
      , center(transform.translation())
      , normal(transform.rotation().col(2))
    {
    }

    // constructor with  Amg::Transform3D and center input
    inline Transforms(const Amg::Transform3D& atransform,
                      const Amg::Vector3D& acenter)
      : transform(atransform)
      , center(acenter)
      , normal(transform.rotation().col(2))
    {
    }

    // constructor with  Amg::Transform3D , center and normal input
    inline Transforms(const Amg::Transform3D& atransform,
                      const Amg::Vector3D& acenter,
                      const Amg::Vector3D& anormal)
      : transform(atransform)
      , center(acenter)
      , normal(anormal)
    {
    }
    Transforms(const Transforms&) = default;
    Transforms(Transforms&&) = default;
    Transforms& operator=(const Transforms&) = default;
    Transforms& operator=(Transforms&&) = default;
    ~Transforms() = default;
    //!< Transform3D to orient surface w.r.t to global frame
    Amg::Transform3D transform;
    //!< center position of the surface
    Amg::Vector3D center;
    //!< normal vector of the surface
    Amg::Vector3D normal;
  };
  /** Unique ptr types**/
  using ChargedTrackParametersUniquePtr =
    std::unique_ptr<ParametersBase<5, Trk::Charged>>;
  using NeutralTrackParametersUniquePtr =
    std::unique_ptr<ParametersBase<5, Trk::Neutral>>;

  /**Default Constructor
   for inheriting classes */
  Surface();

  /**Virtual Destructor*/
  virtual ~Surface();

  /**Copy constructor with shift */
  Surface(const Surface& sf, const Amg::Transform3D& transf);

  /**Constructor with Amg::Transform3D reference */
  Surface(const Amg::Transform3D& htrans);

  /**Constructor from TrkDetElement*/
  Surface(const TrkDetElementBase& detelement);

  /**Constructor form TrkDetElement and Identifier*/
  Surface(const TrkDetElementBase& detelement, const Identifier& id);

  /**Equality operator*/
  virtual bool operator==(const Surface& sf) const = 0;

  /**Non-equality operator*/
  bool operator!=(const Surface& sf) const;

  /**Implicit constructor - uses the copy constructor */
  virtual Surface* clone() const = 0;

  /** NVI method returning unique_ptr clone */
  std::unique_ptr<Surface> uniqueClone() const;

  /** Returns the Surface type to avoid dynamic casts */
  virtual SurfaceType type() const = 0;

  /** Return the cached transformation directly.  Don't try to make
      a new transform if it's not here. */
  const Amg::Transform3D* cachedTransform() const;

  /** Returns HepGeom::Transform3D by reference */
  const Amg::Transform3D& transform() const;

  /** Returns the center position of the Surface */
  const Amg::Vector3D& center() const;

  /** Returns the normal vector of the Surface (i.e. in generall z-axis of
   * rotation) */
  virtual const Amg::Vector3D& normal() const;

  /** Returns a normal vector at a specific local position
   */
  virtual Amg::Vector3D normal(const Amg::Vector2D& lp) const;

  /** Returns a global reference point on the surface,
     for PlaneSurface, StraightLineSurface, PerigeeSurface this is equal to
     center(), for CylinderSurface and DiscSurface this is a new member
     */
  virtual const Amg::Vector3D& globalReferencePoint() const;

  /** return associated Detector Element */
  const TrkDetElementBase* associatedDetectorElement() const;

  /** return Identifier of the associated Detector Element */
  Identifier associatedDetectorElementIdentifier() const;

  /** return the associated Layer */
  const Trk::Layer* associatedLayer() const;

  /** return the material Layer */
  const Trk::Layer* materialLayer() const;
  Trk::Layer* materialLayer();

  /** return the base surface (simplified for persistification) */
  virtual const Trk::Surface* baseSurface() const;

  /** Use the Surface as a ParametersBase constructor, from local parameters -
   * charged.
   */
  virtual ChargedTrackParametersUniquePtr createUniqueTrackParameters(
    double l1,
    double l2,
    double phi,
    double theat,
    double qop,
    std::optional<AmgSymMatrix(5)> cov = std::nullopt) const = 0;

  /** Use the Surface as a ParametersBase constructor, from global parameters -
   * charged.
   */
  virtual ChargedTrackParametersUniquePtr createUniqueTrackParameters(
    const Amg::Vector3D&,
    const Amg::Vector3D&,
    double,
    std::optional<AmgSymMatrix(5)> cov = std::nullopt) const = 0;

  /** Use the Surface as a ParametersBase constructor, from local parameters -
   * neutral.
   */
  virtual NeutralTrackParametersUniquePtr createUniqueNeutralParameters(
    double l1,
    double l2,
    double phi,
    double theat,
    double qop,
    std::optional<AmgSymMatrix(5)> cov = std::nullopt) const = 0;

  /** Use the Surface as a ParametersBase constructor, from global parameters -
   * neutral.
   */
  virtual NeutralTrackParametersUniquePtr createUniqueNeutralParameters(
    const Amg::Vector3D&,
    const Amg::Vector3D&,
    double charge = 0.,
    std::optional<AmgSymMatrix(5)> cov = std::nullopt) const = 0;

  /** positionOnSurface() returns the  LocalPosition on the
    Surface,<br>
    If BoundaryCheck==false it just returns the value of
    globalToLocal (including nullptr possibility),
    if BoundaryCheck==true
    it checks whether the point is inside bounds or not (returns
    std::nullopt in this case).
    */
  std::optional<Amg::Vector2D> positionOnSurface(
    const Amg::Vector3D& glopo,
    const BoundaryCheck& bchk = true,
    double tol1 = 0.,
    double tol2 = 0.) const;

  /** The templated Parameters OnSurface method - checks on surface pointer
   * first */
  template<class T>
  bool onSurface(const T& parameters,
                 const BoundaryCheck& bchk = BoundaryCheck(true)) const;

  /** This method returns true if the GlobalPosition is on the Surface for both,
    within or without check of whether the local position is inside boundaries
    or not */
  virtual bool isOnSurface(const Amg::Vector3D& glopo,
                           const BoundaryCheck& bchk = true,
                           double tol1 = 0.,
                           double tol2 = 0.) const;

  /**  virtual methods to be overwritten by the inherited surfaces */
  virtual bool insideBounds(const Amg::Vector2D& locpos,
                            double tol1 = 0.,
                            double tol2 = 0.) const = 0;

  virtual bool insideBoundsCheck(const Amg::Vector2D& locpos,
                                 const BoundaryCheck& bchk) const = 0;

  /** Specified by each surface type: LocalToGlobal method without dynamic
   * memory allocation */
  virtual void localToGlobal(const Amg::Vector2D& locp,
                             const Amg::Vector3D& mom,
                             Amg::Vector3D& glob) const = 0;

  /** This method returns the GlobalPosition from a LocalPosition
   * uses the per surface localToGlobal.
   */
  Amg::Vector3D localToGlobal(const Amg::Vector2D& locpos) const;

  /** This method returns the GlobalPosition from a LocalPosition
   * The LocalPosition can be outside Surface bounds - for generality with
   * momentum
   */
  Amg::Vector3D localToGlobal(const Amg::Vector2D& locpos,
                              const Amg::Vector3D& glomom) const;

  /** This method returns the GlobalPosition from LocalParameters
   * The LocalParameters can be outside Surface bounds.
   */
  Amg::Vector3D localToGlobal(const LocalParameters& locpars) const;

  /** This method returns the GlobalPosition from LocalParameters
   * The LocalParameters can be outside Surface bounds - for generality with
   * momentum
   */
  Amg::Vector3D localToGlobal(const LocalParameters& locpars,
                              const Amg::Vector3D& glomom) const;

  /** Specified by each surface type: GlobalToLocal method without dynamic
   * memory allocation - boolean checks if on surface */
  virtual bool globalToLocal(const Amg::Vector3D& glob,
                             const Amg::Vector3D& mom,
                             Amg::Vector2D& loc) const = 0;

  /** This method returns the LocalPosition from a provided GlobalPosition.
    If the GlobalPosition is not on the Surface, it returns nullopt
    This method does not check if the calculated LocalPosition is inside surface
    bounds. If this check is needed, use positionOnSurface - only for planar,
    cylinder surface fully defined*/
  std::optional<Amg::Vector2D> globalToLocal(const Amg::Vector3D& glopos,
                                             double tol = 0.) const;

  /** This method returns the LocalPosition from a provided GlobalPosition.
      If the GlobalPosition is not on the Surface, it returns a nullopt
      This method does not check if the calculated LocalPosition is inside
     surface bounds. If this check is needed, use positionOnSurface - for
     generality with momentum */
  std::optional<Amg::Vector2D> globalToLocal(const Amg::Vector3D& glopos,
                                             const Amg::Vector3D& glomom) const;

  /** Optionally specified by each surface type : LocalParameters to Vector2D */
  virtual Amg::Vector2D localParametersToPosition(
    const LocalParameters& locpars) const;

  /** the pathCorrection for derived classes with thickness - it reflects if the
   * direction projection is positive or negative */
  virtual double pathCorrection(const Amg::Vector3D& pos,
                                const Amg::Vector3D& mom) const;

  /** Return the measurement frame - this is needed for alignment, in particular
     for StraightLine and Perigee Surface
       - the default implementation is the the RotationMatrix3D of the transform
   */
  virtual Amg::RotationMatrix3D measurementFrame(
    const Amg::Vector3D& glopos,
    const Amg::Vector3D& glomom) const;

  /** fst straight line intersection schema - templated for charged and neutral
   * parameters */
  template<class T>
  Intersection straightLineIntersection(
    const T& pars,
    bool forceDir = false,
    const Trk::BoundaryCheck& bchk = false) const
  {
    return straightLineIntersection(
      pars.position(), pars.momentum().unit(), forceDir, bchk);
  }

  /** fast straight line intersection schema - standard: provides closest
     intersection and (signed) path length forceFwd is to provide the closest
     forward solution
   */
  virtual Intersection straightLineIntersection(
    const Amg::Vector3D& pos,
    const Amg::Vector3D& dir,
    bool forceDir = false,
    Trk::BoundaryCheck bchk = false) const = 0;

  /** fast straight line distance evaluation to Surface */
  virtual DistanceSolution straightLineDistanceEstimate(
    const Amg::Vector3D& pos,
    const Amg::Vector3D& dir) const = 0;

  /** fast straight line distance evaluation to Surface - with bound option*/
  virtual DistanceSolution straightLineDistanceEstimate(
    const Amg::Vector3D& pos,
    const Amg::Vector3D& dir,
    bool Bound) const = 0;

  /** Surface Bounds method */
  virtual const SurfaceBounds& bounds() const = 0;

  /** Returns 'true' if this surface is 'free', i.e. it does not belong to a
   * detector element (and returns false otherwise*/
  bool isFree() const;

  /** Return 'true' if this surface is own by the detector element */
  bool isActive() const;

  /** Set the transform updates center and normal*/
  void setTransform(const Amg::Transform3D& trans);

  /** set Ownership */
  void setOwner(SurfaceOwner x);

  /** return ownership */
  SurfaceOwner owner() const;

  /** set material layer */
  void setMaterialLayer(Layer* mlay);

  /** Output Method for MsgStream, to be overloaded by child classes */
  virtual MsgStream& dump(MsgStream& sl) const;

  /** Output Method for std::ostream, to be overloaded by child classes */
  virtual std::ostream& dump(std::ostream& sl) const;

  /** Return properly formatted class name */
  virtual std::string name() const = 0;

  /** method to associate the associated Trk::Layer which is alreay owned
     - only allowed by LayerBuilder
     - only done if no Layer is set already  */
  void associateLayer(const Layer& lay);

protected:
 /**Copy operators for inheriting classes
  They  resets the associated
  detector element to nullptr and the identifier to invalid,
  as the copy cannot be owned by the same detector element as the original */
 Surface(const Surface& sf);
 Surface& operator=(const Surface& sf);
 // Move operators for inheriting classes
 Surface(Surface&& sf) noexcept = default;
 Surface& operator=(Surface&& sf) noexcept = default;

 /** Helper method to factorize in one place common operations
 calculate inverse transofrm and multiply with position */
 Amg::Transform3D inverseTransformHelper() const;
 Amg::Vector3D inverseTransformMultHelper(const Amg::Vector3D& glopos) const;

 friend class ::SurfaceCnv_p1;

 //!< Unique Pointer to the Transforms struct*/
 std::unique_ptr<Transforms> m_transforms = nullptr;

 /** Not owning Pointer to the TrkDetElementBase*/
 const TrkDetElementBase* m_associatedDetElement = nullptr;

 /** Identifier for the TrkDetElementBase*/
 Identifier m_associatedDetElementId;

 /**The associated layer Trk::Layer
  - layer in which the Surface is embedded
  (not owning pointed)
  */
 const Layer* m_associatedLayer = nullptr;
 /** Possibility to attach a material descrption
 - potentially given as the associated material layer
   (not owning pointer)
 */
 Layer* m_materialLayer = nullptr;
 /** enum for surface owner : 0  free surface */
 SurfaceOwner m_owner;

 /**Tolerance for being on Surface */
 static constexpr double s_onSurfaceTolerance = 10e-5;  // 0.1 * micron
};
/**Overload of << operator for both, MsgStream and std::ostream for debug
 * output*/
MsgStream&
operator<<(MsgStream& sl, const Surface& sf);
std::ostream&
operator<<(std::ostream& sl, const Surface& sf);

typedef SurfaceUniquePtrT<Trk::Surface> SurfaceUniquePtr;
typedef SurfaceUniquePtrT<const Trk::Surface> ConstSurfaceUniquePtr;
} //
#include "Surface.icc"
#endif // TRKSURFACES_SURFACE_H
