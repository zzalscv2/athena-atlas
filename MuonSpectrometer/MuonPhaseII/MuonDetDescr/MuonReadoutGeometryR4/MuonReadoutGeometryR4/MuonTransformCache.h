/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONREADOUTGEOMETRY_MUONTRANSFORMCACHE_H
#define MUONREADOUTGEOMETRY_MUONTRANSFORMCACHE_H

#include <MuonReadoutGeometryR4/MuonDetectorDefs.h>
#include <CxxUtils/CachedUniquePtr.h>

namespace MuonGMR4 {

class MuonTransformCache {
   public:
    /** @brief: Functional to initialize the cache for a given alignment. Ideally, this functional is defined
     *          by each of the MuonReadoutElement and calls the protected
     * 
     *              Amg::Transform3D toStation(ActsTrk::RawGeomAlignStore* alignStore) const
     *
     *          method which is applying the alignment transformations via the GeoModel kernel. The IdentifierHash
     *          encodes the active sensor number within the MuonReadoutElement. Hence, it  can be used 
     *          to find the transformation to go from the origin of  the GeoModel detector coordinate system 
     *          to the origin of the sensor coordinate system.
    */
    using TransformMaker = std::function<Amg::Transform3D(ActsTrk::RawGeomAlignStore* store, const IdentifierHash& hash)>;

    /** @brief: Standard constructor taking the hash of the sensor element and 
     *          and the TransformMaker expressed usually as a lambda function
    **/
    MuonTransformCache(const IdentifierHash& hash, TransformMaker maker);

    /** @brief Returns the matching transformation from the alignment store. 
     *         If a nullptr is given, then it's equivalent to the case that the transformation
     *         is pointing to a perfectly aligned surface. In this case, the internal nominal
     *         transformation cache is invoked.
     * */
    const Amg::Transform3D& getTransform(const ActsTrk::AlignmentStore* store) const;
    /** @brief Stores the aligned transformation into the store. */
    void storeAlignment(ActsTrk::RawGeomAlignStore& alignStore) const;
    /** @brief Returns the sensor hash of this transformation cache */
    IdentifierHash hash() const;
    /** @brief Returns the transform maker function of this transformation cache*/
    const TransformMaker& transformMaker() const;
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