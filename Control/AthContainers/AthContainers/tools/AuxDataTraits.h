// This file's extension implies that it's C, but it's really -*- C++ -*-.

/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file AthContainers/tools/AuxDataTraits.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Oct, 2014
 * @brief Allow customizing how aux data types are treated.
 */


#ifndef ATHCONTAINERS_AUXDATATRAITS_H
#define ATHCONTAINERS_AUXDATATRAITS_H


#include "CxxUtils/span.h"
#include <vector>
#include <cstdlib>


namespace SG {


/**
 * @brief Allow customizing how aux data types are treated.
 *
 * @c T here is the type that the user requests, eg in the template
 * argument of a decorator.
 */
template <class T, class ALLOC = std::allocator<T> >
class AuxDataTraits
{
public:
  /// The type the user sees.
  using element_type = T;

  /// Reference types returned by aux data accessors.
  using reference_type = element_type&;
  using const_reference_type =  const element_type&;

  /// Allocator type used to store this variable.
  using allocator_type = ALLOC;

  /// Container type used to store this variable.
  using vector_type = std::vector<T, allocator_type>;
  using container_value_type = typename vector_type::value_type;

  /// Pointers to the data within the container.
  using container_pointer_type = typename vector_type::pointer;
  using const_container_pointer_type = typename vector_type::const_pointer;

  /// Look up an element in the container by index.
  /// ptr is a pointer to the start of the container's data.
  static reference_type index (void* ptr, size_t ndx)
  {
    return reinterpret_cast<container_pointer_type>(ptr)[ndx];
  }
    
  /// Look up an element in the container by index.
  /// ptr is a pointer to the start of the container's data.
  static const_reference_type index (const void* ptr, size_t ndx)
  {
    return reinterpret_cast<const_container_pointer_type>(ptr)[ndx];
  }

  using span = CxxUtils::span<container_value_type>;
  using const_span = CxxUtils::span<const container_value_type>;
};


/**
 * @brief Allow customizing how aux data types are treated.
 *
 * Special case for bool variables.
 *
 * @c T here is the type that the user requests, eg in the template
 * argument of a decorator.
 */
template <class ALLOC>
class AuxDataTraits<bool, ALLOC>
{
public:
  /// The type the user sees.
  using element_type = bool;

  /// Reference types returned by aux data accessors.
  using reference_type = element_type&;
  using const_reference_type =  const element_type&;

  /// Allocator type used to store this variable.
  using allocator_type = typename std::allocator_traits<ALLOC>::template rebind_alloc<char>;

  /// Container type used to store this variable.
  using vector_type = std::vector<char, allocator_type>;
  using container_value_type = typename vector_type::value_type;

  /// Pointers to the data within the container.
  using container_pointer_type = typename vector_type::pointer;
  using const_container_pointer_type = typename vector_type::const_pointer;

  /// Look up an element in the container by index.
  /// ptr is a pointer to the start of the container's data.
  static reference_type index (void* ptr, size_t ndx)
  {
    return reinterpret_cast<bool*>(ptr)[ndx];
  }
    
  /// Look up an element in the container by index.
  /// ptr is a pointer to the start of the container's data.
  static const_reference_type index (const void* ptr, size_t ndx)
  {
    return reinterpret_cast<const bool*>(ptr)[ndx];
  }

  using span = CxxUtils::span<container_value_type>;
  using const_span = CxxUtils::span<const container_value_type>;
};


/**
 * @brief Helper to specialize how to make an initialized instance of @c T.
 *
 * SG::Zero<T>::zero() should return a fully-initialized instance of @c T.
 */
template <class T>
struct Zero
{
  static T zero()
  {
    return T();
  }
};


} // namespace SG


#endif // not ATHCONTAINERS_AUXDATATRAITS_H
