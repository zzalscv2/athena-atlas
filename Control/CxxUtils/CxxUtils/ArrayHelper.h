/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef CXXUTILS_ARRAY_HELPER
#define CXXUTILS_ARRAY_HELPER
#include <array>
#include <stddef.h>
 
/// Helper function to initialize in-place arrays with non-zero values
template<class T, size_t N>  constexpr std::array<T, N> make_array(const T& def_val) {
    std::array<T,N> arr{};
    arr.fill(def_val);
    return arr;
}
#endif
