/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONREADOUTGEOMETRY_MUONSURFACECACHE_H
#define MUONREADOUTGEOMETRY_MUONSURFACECACHE_H

#include <MuonReadoutGeometryR4/MuonTransformCache.h>
#include <Acts/Geometry/DetectorElementBase.hpp>
#include <Acts/Surfaces/Surface.hpp>

namespace MuonGMR4{

  class MuonSurfaceCache: public Acts::DetectorElementBase {

  public:

      /** @brief: Helper class to connect the aligned transformations of each active sensor(layer) with the Acts::Surfaces.
    * It's actively used in the surface factories of the MuonReadoutElement. The MuonSurfaceCache takes a pointer to the
    * MuonTransformCache managing the local to global transformation of the corresponding layer and the DetectorType. The latter is needed
    * to pass the proper AlignmentStore from the Geometry context to the transform cache. Once constructed, the MuonTransformCache is parsed
    * to the surface construction mechanism of the Acts::Surface. The resulting shared_ptr is then parsed to the MuonSurfaceCache.
      */


    /** @brief: Standard constructor taking the tranasform cache of the element and the detector type. 
    **/

    MuonSurfaceCache(const MuonTransformCache* transformCache, ActsTrk::DetectorType type);

    /// Returns the transformation stored in the MuonTransformCache.
    const Acts::Transform3& transform(const Acts::GeometryContext& gctx) const override final;

    /// Returns the dereferenced pointer cache.
    const Acts::Surface& surface() const override final;

    Acts::Surface& surface() override final;

    /// Dummy override to satisfy the interface of the Acts::DetElementBase
    double thickness() const override final;

    /// Cache the pointer to the surface that's constructed from this cache
    void setSurface(const std::shared_ptr<Acts::Surface> surface);

    /// Returns the pointer to the cached surface.
    std::shared_ptr<Acts::Surface> getSurface() const;

    /// Hash of the MuonSurfaceCache which is the same as the one of the TransformCache.
    IdentifierHash hash() const;



  private:

    const MuonTransformCache* m_transformCache{nullptr};

    ActsTrk::DetectorType m_type;

    std::shared_ptr<Acts::Surface> m_surface{nullptr};


  };
  inline bool operator<(const std::unique_ptr<MuonSurfaceCache>& a,
                      const std::unique_ptr<MuonSurfaceCache>& b) {
    return a->hash() < b->hash();
  }
  inline bool operator<(const IdentifierHash& a,
                      const std::unique_ptr<MuonSurfaceCache>& b) {
    return a < b->hash();
  }
  inline bool operator<(const std::unique_ptr<MuonSurfaceCache>& a,
                      const IdentifierHash& b) {
      return a->hash() < b;
  }

  using MuonSurfaceSet = std::set<std::unique_ptr<MuonSurfaceCache>, std::less<>>;

 


}//namespace MuonGMR4
#endif
