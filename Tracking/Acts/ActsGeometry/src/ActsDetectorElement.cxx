/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "ActsGeometry/ActsDetectorElement.h"

// ATHENA
#include "ActsInterop/IdentityHelper.h"
#include "SCT_ReadoutGeometry/StripBoxDesign.h"
#include "SCT_ReadoutGeometry/StripStereoAnnulusDesign.h"
#include "TRT_ReadoutGeometry/TRT_BarrelElement.h"
#include "TRT_ReadoutGeometry/TRT_EndcapElement.h"
#include "TrkSurfaces/AnnulusBounds.h"
#include "TrkSurfaces/RectangleBounds.h"
#include "TrkSurfaces/Surface.h"
#include "TrkSurfaces/SurfaceBounds.h"
#include "TrkSurfaces/TrapezoidBounds.h"

// PACKAGE
#include "ActsGeometryInterfaces/ActsGeometryContext.h"
// ACTS
#include "Acts/Definitions/Units.hpp"
#include "Acts/Geometry/GeometryContext.hpp"
#include "Acts/Surfaces/AnnulusBounds.hpp"
#include "Acts/Surfaces/DiscSurface.hpp"
#include "Acts/Surfaces/LineBounds.hpp"
#include "Acts/Surfaces/PlaneSurface.hpp"
#include "Acts/Surfaces/RectangleBounds.hpp"
#include "Acts/Surfaces/StrawSurface.hpp"
#include "Acts/Surfaces/TrapezoidBounds.hpp"
#include "Acts/Visualization/ObjVisualization3D.hpp"
#include "Acts/Visualization/PlyVisualization3D.hpp"


// BOOST
#include <boost/variant.hpp>
#include <boost/variant/get.hpp>

using Acts::Surface;
using Acts::Transform3;

using namespace Acts::UnitLiterals;
using namespace ActsTrk;
using SubDetAlignments = ActsGeometryContext::SubDetAlignments;



constexpr double length_unit = 1_mm;

ActsDetectorElement::ActsDetectorElement(const InDetDD::SiDetectorElement &detElem) : m_detElement{&detElem} {
  m_type = detElem.isPixel() ? DetectorType::Pixel : DetectorType::Sct;

  auto boundsType = detElem.bounds().type();

  m_thickness = detElem.thickness();

  Acts::Transform3 recoToHit = Amg::CLHEPTransformToEigen(detElem.recoToHitTransform());

  m_extraTransform = recoToHit; // default to recoToHit

  if (boundsType == Trk::SurfaceBounds::Rectangle) {

    const InDetDD::SiDetectorDesign &design = detElem.design();
    double hlX = design.width() / 2. * length_unit;
    double hlY = design.length() / 2. * length_unit;

    auto rectangleBounds =
        std::make_shared<const Acts::RectangleBounds>(hlX, hlY);

    m_bounds = rectangleBounds;

    if (const auto *bd = dynamic_cast<const InDetDD::StripBoxDesign *>(&design);
        bd != nullptr) {
      // extra shift for split row modules
      m_extraTransform = bd->moduleShift() * recoToHit;
    }

    m_surface =
        Acts::Surface::makeShared<Acts::PlaneSurface>(rectangleBounds, *this);



  } else if (boundsType == Trk::SurfaceBounds::Trapezoid) {

    const InDetDD::SiDetectorDesign &design = detElem.design();

    double minHlX = design.minWidth() / 2. * length_unit;
    double maxHlX = design.maxWidth() / 2. * length_unit;
    double hlY = design.length() / 2. * length_unit;

    auto trapezoidBounds =
        std::make_shared<const Acts::TrapezoidBounds>(minHlX, maxHlX, hlY);

    m_bounds = trapezoidBounds;

    m_surface =
        Acts::Surface::makeShared<Acts::PlaneSurface>(trapezoidBounds, *this);


  } else if (boundsType == Trk::SurfaceBounds::Annulus) {

    const InDetDD::SiDetectorDesign &design = detElem.design();
    const auto *annulus =
        dynamic_cast<const InDetDD::StripStereoAnnulusDesign *>(&design);
    if (annulus == nullptr) {
      throw std::domain_error("ActsDetectorElement got inconsistent surface");
    }

    double phi = annulus->phiWidth();
    double phiS = annulus->stereo();
    double R = annulus->waferCentreR();
    double maxR = annulus->maxR();
    double minR = annulus->minR();

    double phiAvg =
        0; // phiAvg is the bounds-internal local rotation. We don't want one

    // phi is the total opening angle, set up symmetric phi bounds
    double phiMax = phi / 2.;
    double phiMin = -phiMax;

    // need to rotate pi/2 to reproduce ABXY orientation, phiS so that phi=0
    // is center and symmetric
    double phiShift = M_PI / 2. - phiS;

    Amg::Vector2D originStripXYRotated(R * (1 - std::cos(phiS)),
                                       R * std::sin(-phiS));

    auto annulusBounds = std::make_shared<Acts::AnnulusBounds>(
        minR, maxR, phiMin, phiMax, originStripXYRotated, phiAvg);
    m_bounds = annulusBounds;

    m_surface =
        Acts::Surface::makeShared<Acts::DiscSurface>(annulusBounds, *this);

    Amg::Vector2D origin2D = annulusBounds->moduleOrigin();
    Amg::Translation3D transl(Amg::Vector3D(origin2D.x(), origin2D.y(), 0));
    Amg::Rotation3D rot(Amg::AngleAxis3D(-phiShift, Amg::Vector3D::UnitZ()));
    Amg::Transform3D originTrf;
    originTrf = transl * rot;
    m_extraTransform = recoToHit * originTrf.inverse();

  } else {
    std::cout << boundsType << std::endl;
    throw std::domain_error(
        "ActsDetectorElement does not support this surface type");
  }
}

ActsDetectorElement::ActsDetectorElement(const Acts::Transform3 &trf, const InDetDD::TRT_BaseElement &detElem, const Identifier &id) :
    m_type{DetectorType::Trt}, m_detElement{&detElem}, m_defTransform{trf}, m_explicitIdentifier(id) {

  // we know this is a straw
  double length = detElem.strawLength() * 0.5 * length_unit;

  // we need to find the radius
  auto ecElem = dynamic_cast<const InDetDD::TRT_EndcapElement *>(&detElem);
  auto brlElem = dynamic_cast<const InDetDD::TRT_BarrelElement *>(&detElem);
  double innerTubeRadius{0.};
  if (ecElem) {
    innerTubeRadius = ecElem->getDescriptor()->innerTubeRadius() * length_unit;
  } else {
    if (brlElem) {
      innerTubeRadius =
          brlElem->getDescriptor()->innerTubeRadius() * length_unit;
    } else {
      throw std::runtime_error(
          "Cannot get tube radius for element in ActsDetectorElement c'tor");
    }
  }

  auto lineBounds =
      std::make_shared<const Acts::LineBounds>(innerTubeRadius, length);
  m_bounds = lineBounds;

  m_surface = Acts::Surface::makeShared<Acts::StrawSurface>(lineBounds, *this);
}

ActsDetectorElement::ActsDetectorElement(const InDetDD::HGTD_DetectorElement &detElem, const Identifier &id) :
    m_type{DetectorType::Hgtd}, m_detElement{&detElem}, m_thickness{detElem.thickness()}, m_explicitIdentifier{id} {

  auto boundsType = detElem.bounds().type();

  if (boundsType == Trk::SurfaceBounds::Rectangle) {

    const InDetDD::HGTD_ModuleDesign &design = detElem.design();
    double hlX = design.width() / 2. * length_unit;
    double hlY = design.length() / 2. * length_unit;

    auto rectangleBounds =
        std::make_shared<const Acts::RectangleBounds>(hlX, hlY);

    m_bounds = rectangleBounds;

    m_surface =
        Acts::Surface::makeShared<Acts::PlaneSurface>(rectangleBounds, *this);
        
  } else {
    throw std::domain_error(
        "ActsDetectorElement: the surface type of HGTD is not does not Rectangle, it is wrong");
  }
}

IdentityHelper ActsDetectorElement::identityHelper() const {
  if (detectorType() == DetectorType::Pixel || detectorType() == DetectorType::Sct) {
        return IdentityHelper(static_cast<const InDetDD::SiDetectorElement *>(m_detElement));
  } else {
    throw std::domain_error("Cannot get IdentityHelper for TRT element");
  }
}

const Acts::Transform3 &ActsDetectorElement::transform(const Acts::GeometryContext &anygctx) const {
    // any cast to known context type
    const ActsGeometryContext *gctx = anygctx.get<const ActsGeometryContext *>();

    // This is needed for initial geometry construction. At that point, we don't
    // have a consistent view of the geometry yet, and thus we can't populate an
    // alignment store at that time.

    // unpack the alignment store from the context
    SubDetAlignments::const_iterator itr = gctx->alignmentStores.find(detectorType());

    /// Does this mean that the alignment is not cached for this detector element?
    if (itr == gctx->alignmentStores.end()) return getDefaultTransform();
    const GeoModel::TransientConstSharedPtr<AlignmentStore> &alignmentStore = itr->second;

    // get the correct cached transform
    // units should be fine here since we converted at construction
    const Acts::Transform3 *cachedTrf = alignmentStore->getTransform(this);

    assert(cachedTrf != nullptr);
    return *cachedTrf;

}

bool ActsDetectorElement::storeAlignment(RawGeomAlignStore &store) const {
  if (store.detType != detectorType()) return false;

    Amg::Transform3D trf{Amg::Transform3D::Identity()};
    static constexpr std::array<DetectorType, 3> useGeoModel{DetectorType::Pixel, DetectorType::Sct, DetectorType::Hgtd};
    if (std::find(useGeoModel.begin(), useGeoModel.end(), detectorType()) != useGeoModel.end()) {
        Amg::Transform3D l2g = m_detElement->getMaterialGeom()->getAbsoluteTransform(store.geoModelAlignment.get()) * m_extraTransform;
        // need to make sure translation has correct units
        l2g.translation() *= 1.0 / CLHEP::mm * length_unit;

        trf = l2g;
    } else if (detectorType() == DetectorType::Trt && m_defTransform.isValid()) {
        // So far: NO ALIGNMENT for the ACTS TRT version. Default transform set in
        // constructor, should be safe to access without mutex.
        trf = *m_defTransform.ptr();
  } else {
    throw std::runtime_error{"Unknown detector element type "+to_string(detectorType())};
  }

  store.trackingAlignment->setTransform(this, trf);
#ifndef NDEBUG
    if (!store.trackingAlignment->getTransform(this)) { throw std::runtime_error("Detector element was unable to store transform in GAS"); }
#endif
    return true;

}

const Acts::Transform3 &
ActsDetectorElement::getDefaultTransform() const {
  if (!m_defTransform.isValid()) {
    // transform not yet set
    static constexpr std::array<DetectorType, 3> useGeoModel{DetectorType::Pixel, DetectorType::Sct, DetectorType::Hgtd};
    if (std::find(useGeoModel.begin(), useGeoModel.end(), detectorType()) != useGeoModel.end()) {
        Amg::Transform3D l2g = m_detElement->getMaterialGeom()->getAbsoluteTransform() * m_extraTransform;
            // need to make sure translation has correct units
            l2g.translation() *= 1.0 / CLHEP::mm * length_unit;

            m_defTransform.set(std::move(l2g));
        } else if (const auto *detElem = dynamic_cast<const InDetDD::TRT_BaseElement *>(m_detElement); detElem != nullptr) {
            throw std::logic_error{"TRT transform should have been set in the constructor"};
        } else {
            throw std::runtime_error{"Unknown detector element type"};
        }
    }
    return *m_defTransform.ptr();
}

const Acts::Surface &ActsDetectorElement::surface() const {
  return (*m_surface);
}

Acts::Surface &ActsDetectorElement::surface() {
  return (*m_surface);
}

const Trk::Surface &ActsDetectorElement::atlasSurface() const {
  if (const auto *detElem =
          dynamic_cast<const InDetDD::SiDetectorElement *>(m_detElement);
      detElem != nullptr) {
    return detElem->surface();
  } else {
    throw std::domain_error("Cannot get surface for TRT element");
  }
}

double ActsDetectorElement::thickness() const { return m_thickness; }

Identifier ActsDetectorElement::identify() const {
  if (const auto *detElem =
          dynamic_cast<const InDetDD::SiDetectorElement *>(m_detElement);
      detElem != nullptr) {
    return detElem->identify();
  } else if (dynamic_cast<const InDetDD::TRT_BaseElement *>(m_detElement) !=
             nullptr) {
    return m_explicitIdentifier;
  } else if (dynamic_cast<const InDetDD::HGTD_DetectorElement *>(m_detElement) !=
             nullptr) {
    return m_explicitIdentifier;
  } else {
    throw std::runtime_error{"Unknown detector element type"};
  }
}

const GeoVDetectorElement *
ActsDetectorElement::upstreamDetectorElement() const {
  return m_detElement;
}
DetectorType ActsDetectorElement::detectorType() const { return m_type; }
