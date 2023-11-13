/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ActsEvent_Decoration_h
#define ActsEvent_Decoration_h
#include "AthContainers/AuxElement.h"

namespace ActsTrk {
using IndexType = std::uint32_t;  // TODO take from a common header
namespace detail {

template <typename STORE>
struct Decoration {
  using SetterType =
      std::function<std::any(STORE*, ActsTrk::IndexType, const std::string&)>;
  using GetterType = std::function<const std::any(
      const STORE*, ActsTrk::IndexType, const std::string&)>;
  using CopierType =
      std::function<void(STORE*, ActsTrk::IndexType, const std::string&,
                         const STORE*, ActsTrk::IndexType)>;

  Decoration(const std::string& n, GetterType g, CopierType c,
             SetterType s = static_cast<SetterType>(nullptr))
      : name(n),
        hash(Acts::hashString(name)),
        getter(g),
        copier(c),
        setter(s) {}

  std::string name;  // xAOD API needs this
  uint32_t hash;     // Acts API comes with this
  // TODO add here the aux ID to save on lookup
  GetterType getter;  // type aware accessors
  CopierType copier;
  SetterType setter;
};

template <typename T>
struct accepted_decoration_types {
  constexpr static bool value =
      std::is_same<T, float>::value or std::is_same<T, double>::value or
      std::is_same<T, short>::value or std::is_same<T, int>::value or
      std::is_same<T, std::uint32_t>::value;
};

// getter that is good for non-mutable containers
template <typename STORE, typename T>
const std::any constDecorationGetter(const STORE* container,
                                     ActsTrk::IndexType idx,
                                     const std::string& name) {
  const SG::ConstAuxElement el(container, idx);
  return &(el.auxdataConst<T>(name));
}

// getter that is good for mutable containers (returns const ptr wrapped in
// std::any but allow adding decorations to store)
template <typename STORE, typename T>
const std::any decorationGetter(const STORE* container, ActsTrk::IndexType idx,
                                const std::string& name) {
  const SG::AuxElement* el = (*container)[idx];
  return const_cast<const T*>(&(el->auxdecor<T>(name)));
}

// setter for mutable containers (i.e. provides non const ptr wrapped in
// std::any)
template <typename STORE, typename T>
std::any decorationSetter(STORE* container, ActsTrk::IndexType idx,
                          const std::string& name) {
  const SG::AuxElement* el = (*container)[idx];
  return &(el->auxdecor<T>(name));
}

template <typename STORE, typename T>
void decorationCopier(STORE* dst, ActsTrk::IndexType dst_idx,
                      const std::string& name, const STORE* src,
                      ActsTrk::IndexType src_idx) {
  *std::any_cast<T*>(decorationSetter<STORE, T>(dst, dst_idx, name)) =
      *std::any_cast<const T*>(
          constDecorationGetter<STORE, T>(src, src_idx, name));
}

}  // namespace detail
}  // namespace ActsTrk

#endif