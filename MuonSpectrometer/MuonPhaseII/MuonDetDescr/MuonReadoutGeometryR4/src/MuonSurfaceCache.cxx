
/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include <MuonReadoutGeometryR4/MuonSurfaceCache.h>
#include <MuonReadoutGeometryR4/MuonTransformCache.h>

#include <Acts/Surfaces/Surface.hpp>
using namespace ActsTrk;
namespace MuonGMR4{

  MuonSurfaceCache::MuonSurfaceCache(const MuonTransformCache* transformCache, 
    ActsTrk::DetectorType type)
  : m_transformCache(transformCache),
    m_type(type) {}  

  const Acts::Transform3& MuonSurfaceCache::transform(const Acts::GeometryContext& anygctx) const  {

    const ActsGeometryContext *gctx = anygctx.get<const ActsGeometryContext *>();

    // This is needed for initial geometry construction. At that point, we don't
    // have a consistent view of the geometry yet, and thus we can't populate an
    // alignment store at that time.

    // unpack the alignment store from the context
    ActsGeometryContext::SubDetAlignments::const_iterator itr = gctx->alignmentStores.find(m_type);

    /// Does this mean that the alignment is not cached for this detector element?
    const AlignmentStore* store{itr == gctx->alignmentStores.end() ? nullptr : itr->second.get()};   
    return m_transformCache->getTransform(store);
  }

  const Acts::Surface& MuonSurfaceCache::surface() const  { return *m_surface; }
  Acts::Surface& MuonSurfaceCache::surface() { return *m_surface; }
  std::shared_ptr<Acts::Surface> MuonSurfaceCache::getSurface() const { return m_surface; }
  double MuonSurfaceCache::thickness() const { return 0.; }
  void MuonSurfaceCache::setSurface(std::shared_ptr<Acts::Surface> surface) { m_surface = surface; }

  IdentifierHash MuonSurfaceCache::hash() const { return m_transformCache->hash(); }


}