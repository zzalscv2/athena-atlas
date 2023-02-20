// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file CxxUtils/SizedUInt.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Jan, 2023
 * @brief Generate an unsigned integer type of a specified size.
 *
 * CxxUtils::detail::SizedUInt<N>::type will give an unsigned type N bytes long.
 */


#ifndef CXXUTILS_SIZEDUINT_H
#define CXXUTILS_SIZEDUINT_H


#include <cstdint>


namespace CxxUtils {
namespace detail {


template <unsigned SIZE>
struct SizedUInt
{
};


template <>
struct SizedUInt<1>
{
  using type = uint8_t;
};


template <>
struct SizedUInt<2>
{
  using type = uint16_t;
};


template <>
struct SizedUInt<4>
{
  using type = uint32_t;
};


template <>
struct SizedUInt<8>
{
  using type = uint64_t;
};


} // namespace detail
} // namespace CxxUtils


#endif // not CXXUTILS_SIZEDUINT_H
