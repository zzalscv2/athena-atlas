/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GEOMODELUTILITIES_TRANSFORMMAP_H
#define GEOMODELUTILITIES_TRANSFORMMAP_H

#include <memory>
#include <unordered_map>

template <typename T, typename X> class TransformMap {
public:
    TransformMap() = default;
    ~TransformMap() = default;

    bool setTransform(const T *obj, const X &xf);
    const X *getTransform(const T *obj) const;

    bool append(const TransformMap &other);

private:
    std::unordered_map<const T *, std::shared_ptr<const X>> m_container;
};

template <typename T, typename X> bool TransformMap<T, X>::setTransform(const T *obj, const X &xf) {
    std::shared_ptr<const X> &newObj = m_container[obj];
    bool good = !newObj;
    newObj = std::make_shared<X>(xf);
    return good;
}

template <typename T, typename X> const X *TransformMap<T, X>::getTransform(const T *obj) const {
    auto it = m_container.find(obj);
    if (it != m_container.end()) return it->second.get();
    return nullptr;
}

template <typename T, typename X> bool TransformMap<T, X>::append(const TransformMap &other) {
    bool allGood{true};
    for (const auto &[key, value] : other.m_container) {
        std::shared_ptr<const X> &newObj = m_container[key];
        allGood &= !newObj;
        newObj = value;
    }
    return allGood;
}
#endif
