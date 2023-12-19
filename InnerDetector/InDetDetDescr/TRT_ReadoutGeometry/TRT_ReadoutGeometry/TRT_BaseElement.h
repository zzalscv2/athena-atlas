/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TRT_BaseElement.h
///////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRT_BaseElement_h
#define TRT_BaseElement_h 1


//include the Amg packages they include the Eigen plug-ins
#include "EventPrimitives/EventPrimitives.h"
#include "GeoPrimitives/GeoPrimitives.h"

#include "TrkSurfaces/Surface.h"
#include "TrkSurfaces/StraightLineSurface.h"

#include "TrkDetElementBase/TrkDetElementBase.h"
#include "InDetReadoutGeometry/SurfaceCache.h"
#include "Identifier/IdentifierHash.h"
#include "Identifier/Identifier.h"

#include "CLHEP/Geometry/Transform3D.h"
#include "CLHEP/Geometry/Point3D.h"

#include "CxxUtils/CachedUniquePtr.h"
#include "CxxUtils/CachedValue.h"
#include <vector>



class TRT_ID;
class GeoAlignmentStore;

namespace InDetDD {

  class SurfaceCacheBase;
  class SurfaceCache;
  class TRT_Conditions;

  /** @class TRT_BaseElement

      Virtual base class of TRT readout elements.
      Differently to the Silicon readout elements, the TRT readoutelements describe multiple readout surfaces,
      i.e. several straws that are described by a surface, such as e.g. a planar sector surface in the barrel,
      or a disc surface in the endcap.

      @author Grant Gorfine
      - modified& maintained: Andreas Salzburger, Nick Styles

      */

  class TRT_BaseElement : public Trk::TrkDetElementBase  {

  public:

    enum Type {BARREL, ENDCAP};

    /** Constructor: */
    TRT_BaseElement(const GeoVFullPhysVol* volume,
                    const Identifier& id,
                    const TRT_ID* idHelper,
                    const TRT_Conditions* conditions);

    TRT_BaseElement(const TRT_BaseElement& right);

    /** Destructor: */
    virtual ~TRT_BaseElement() = default;

    /** Type information: returns BARREL or ENDCAP */
    virtual TRT_BaseElement::Type type() const = 0;

    /** identifier of this detector element: */
    virtual Identifier identify() const override final;

    /** identifier hash */
    virtual IdentifierHash identifyHash() const override final;

    // --- GeoModel transformation forwards ----------------------------------------------------- //

    /** Get Default Transform (of module in barrel, layer in endcap) from GeoModel before alignment corrections */
    const GeoTrf::Transform3D& defTransform() const;

    /** This is an alias to strawTransform(int straw) */
    const HepGeom::Transform3D getAbsoluteTransform(int straw) const;

    // ---- Surface & Tracking information cache ------------------------------------------------ //
    // (a) Element Surface section - accesses the m_surfaceCache store

    /** Element Surface: access to the Surface (straw layer) */
    virtual const Trk::Surface& surface () const override final;

    /** Straw layer bounds */
    virtual const Trk::SurfaceBounds& bounds() const override final;

    /** Element Surface: Get Transform of element in Tracking frame: Amg */
    virtual const Amg::Transform3D& transform() const override final;

    /** Element Surface: center of a straw layer. */
    virtual const Amg::Vector3D& center() const override final;

    /** Element Surface: normal of a straw layer */
    virtual const Amg::Vector3D& normal() const override final;

    /** TrkDetElementBase interface detectorTyoe */
    virtual Trk::DetectorElemType detectorType() const override final;

    // (b) Straw Surface section - accesses the vector<SurfaceCache> m_strawSurfacesCache store

    /** Straw Surface: access to the surface via identifier */
    virtual const Trk::Surface& surface (const Identifier& id) const override final;

    /** Returns the full list of all detection surfaces associated to this detector element */
    const std::vector<const Trk::Surface*>& surfaces() const;

    /** Straw Surface: access to the bounds via Identifier */
    virtual const Trk::SurfaceBounds& bounds(const Identifier& id) const override final;

    /** Straw Surface: access to the transform of individual straw in Tracking frame: Amg */
    virtual const Amg::Transform3D& transform(const Identifier& id) const override final;

    /** Straw transform - fast access in array, in Tracking frame: Amg */
    /** Straw Surface: access to the transform of individual straw in Tracking frame: Amg */
    const Amg::Transform3D& strawTransform(unsigned int straw) const;

    /** Straw Surface: Center of a straw using Identifier
        Straw center and straw axis can be obtained by the following:
        (The straw center is the center of the active region)
        Amg::Transform3D& transform = element->strawTransform(straw);
        Amb::Vector3D& center = element->strawCenter();
        double r = element->strawCenter()->perp();
        double phi = element->strawCenter()->phi();
        Amg::Vector3D strawAxis =  element->strawTransform(straw)* Vector3D(0,0,1) * strawDirection() */

    virtual const Amg::Vector3D& center(const Identifier& id) const override final;

    /** Normal of a straw. (Not very meaningful). */
    virtual const Amg::Vector3D& normal(const Identifier& id) const override final;

    /** Straw Surface: access to the surface via integer */
    const Trk::StraightLineSurface& strawSurface(int straw) const;

    /** Straw Surface: Local -> global transform of the straw via integer */
    const Amg::Transform3D& strawTransform(int straw) const;

    /** Straw Surface: Local -> global transform of the straw via integer */
    const Amg::Vector3D& strawCenter(int straw) const;

    /** Straw axis. Always in direction of increasing eta.
        +ve z direction in barrel (for both +ve and -ve half)
        Away from beam pipe in -ve z endcap,
        Towards beam pipe in +ve endcap. */
    Amg::Vector3D strawAxis(int straw) const;

    /** Number of straws in the element. */
    unsigned int nStraws() const;

    /** Active straw length */
    virtual const double& strawLength() const = 0;

    /** StrawDirection. +1 if axis is in same direction as local z axis, -1 otherwise. */
    virtual int strawDirection() const = 0;

    /** Invalidate cache */
    void invalidate();

    /** Update all caches */
    void updateAllCaches();

    /** Return the TRT_Conditions object associated to this Detector element */
    const TRT_Conditions* conditions() const;

    /** to be overloaded by the extended classes */
    virtual   HepGeom::Transform3D calculateStrawTransform(int straw) const = 0;

    /** the straw bounds */
    virtual const Trk::SurfaceBounds&  strawBounds() const = 0;

    /** creates surface for detector element, to be implemented in derived class */
    virtual const Trk::Surface&  elementSurface() const = 0;

    /** create the surface cache of the detector element, to be implementd in the deried class */
    virtual void createSurfaceCache() const = 0;

    /** create the surface & surface cache for the straw */
    void createSurfaceCache(Identifier id) const;

    /** invalidate action on the cache */
    void invalidateOther() const {};

  private:

    /** Illegal operations: */
    const TRT_BaseElement& operator=(const TRT_BaseElement&right);
    /** Helper method for cache dealing */
    void deleteCache();
    std::unique_ptr<SurfaceCacheBase> createSurfaceCacheHelper(int straw) const;

  protected:
    Identifier                                          m_id;
    IdentifierHash                                      m_idHash;
    /*
     * The number of straws and the vector below need to
     * initialosed in the derived constructors for now.
     * This should fine as this is pure virtual class
     */
    unsigned int m_nstraws = 0;
    const TRT_ID*                                       m_idHelper=nullptr;
    const TRT_Conditions*                               m_conditions=nullptr;
    CxxUtils::CachedUniquePtr<Trk::Surface> m_surface;
   // Amg cache for the straw surfaces
    std::vector<CxxUtils::CachedUniquePtr<Trk::StraightLineSurface>> m_strawSurfaces{};
    std::vector<CxxUtils::CachedUniquePtr<SurfaceCacheBase>> m_strawSurfacesCache{};
    //!< helper element surface for the cache
    CxxUtils::CachedValue<std::vector<const Trk::Surface*>> m_surfaces;
    CxxUtils::CachedValue<SurfaceCache> m_surfaceCache;

  };

}
#include "TRT_ReadoutGeometry/TRT_BaseElement.icc"
#endif


