
/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include <MuonReadoutGeometryR4/MuonTransformCache.h>
#include <GeoModelKernel/GeoFullPhysVol.h>

namespace MuonGMR4 {
MuonTransformCache::MuonTransformCache(const IdentifierHash& hash,
                                       TransformMaker maker)
    : m_hash{hash},
      m_transform{maker} {}

const Amg::Transform3D& MuonTransformCache::getTransform(const ActsTrk::AlignmentStore* alignStore) const {    
    /// Valid alignment store is given -> Take the transformation from the cache there
    if (alignStore){
        const Amg::Transform3D* cache = alignStore->getTransform(this);
        if (cache) return *cache;
        throw std::runtime_error("The Alignment store does not contain an aligned transformation.");
    }
    /// Fall back solution to go onto the nominal cache    
    if (!m_nomCache) {
        return (*m_nomCache.set(std::make_unique<Amg::Transform3D>(m_transform(nullptr, m_hash))));
    }
    return (*m_nomCache);
}

void MuonTransformCache::storeAlignment(ActsTrk::RawGeomAlignStore& alignStore) const {
    if (!alignStore.trackingAlignment) return;
    if (alignStore.trackingAlignment->getTransform(this)){
        throw std::runtime_error("Transformation has already been cached. Being called twice");
    }
    alignStore.trackingAlignment->setTransform(this, m_transform(&alignStore, m_hash));
}
IdentifierHash MuonTransformCache::hash() const {
    return m_hash;
}
const MuonTransformCache::TransformMaker& MuonTransformCache::transformMaker() const {
    return m_transform;
}

}  // namespace MuonGMR4