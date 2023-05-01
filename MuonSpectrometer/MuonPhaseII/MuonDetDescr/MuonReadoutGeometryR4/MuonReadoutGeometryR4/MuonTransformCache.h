/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONREADOUTGEOMETRY_MUONTRANSFORMCACHE_H
#define MUONREADOUTGEOMETRY_MUONTRANSFORMCACHE_H

#include <CxxUtils/CachedUniquePtr.h>
#include <MuonReadoutGeometryR4/MuonDetectorDefs.h>

/// The Muon transform cache is used to hold all geometry 
namespace MuonGMR4 {

class MuonTransformCache {
   public:
    using TransformMaker = std::function<Amg::Transform3D(ActsTrk::RawGeomAlignStore* store, const IdentifierHash& hash)>;

    MuonTransformCache(const IdentifierHash& hash, TransformMaker maker);

    const Amg::Transform3D& getTransform(const ActsTrk::AlignmentStore* store) const;

    void storeAlignment(ActsTrk::RawGeomAlignStore& alignStore) const;

    IdentifierHash hash() const;

   private:
    IdentifierHash m_hash{0};
    TransformMaker m_transform{};
    CxxUtils::CachedUniquePtr<Amg::Transform3D> m_nomCache{};

    
};

inline bool operator<(const std::unique_ptr<MuonTransformCache>& a,
                      const std::unique_ptr<MuonTransformCache>& b) {
    return a->hash() < b->hash();
}
inline bool operator<(const IdentifierHash& a,
                      const std::unique_ptr<MuonTransformCache>& b) {
    return a < b->hash();
}
inline bool operator<(const std::unique_ptr<MuonTransformCache>& a,
                      const IdentifierHash& b) {
    return a->hash() < b;
}
using MuonTransformSet =
    std::set<std::unique_ptr<MuonTransformCache>, std::less<>>;
}  // namespace MuonGMR4
#endif