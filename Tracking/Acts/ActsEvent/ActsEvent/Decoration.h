/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ActsEvent_Decoration_h
#define ActsEvent_Decoration_h

namespace ActsTrk {
using IndexType = std::uint32_t;
namespace detail {
struct Decoration {
  using SetterType =
      std::function<std::any(ActsTrk::IndexType, const std::string&)>;
  using GetterType =
      std::function<const std::any(ActsTrk::IndexType, const std::string&)>;

  Decoration(const std::string& n, SetterType s, GetterType g)
      : name(n), hash(Acts::hashString(name)), setter(s), getter(g) {}

  std::string name;   // xAOD API needs this
  uint32_t hash;      // Acts API comes with this
  SetterType setter;  // type aware accessors
  GetterType getter;
};

template <typename T>
struct accepted_decoration_types {
  constexpr static bool value =
      std::is_same<T, float>::value or std::is_same<T, double>::value or
      std::is_same<T, short>::value or std::is_same<T, int>::value or
      std::is_same<T, std::uint32_t>::value;
};


}  // namespace detail
}  // namespace ActsTrk

#endif