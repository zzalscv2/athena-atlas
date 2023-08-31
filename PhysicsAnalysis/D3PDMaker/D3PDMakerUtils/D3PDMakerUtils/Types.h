// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file D3PDMakerUtils/Types.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Jun, 2012
 * @brief A simple tuple of multiple types.
 *
 * This is used when declaring tools that can take one of a set of types.
 * For example,
 *
 *@code
 *   BlockFillerTool<Types<Obj1, Obj2> >
 @endcode
 *
 * declares a block filler that can take as input either @c Obj1 or @c Obj2.
 * (Note: the choice is made during configuration, not dynamically.)
 */


#ifndef D3PDMAKERUTILS_TYPES_H
#define D3PDMAKERUTILS_TYPES_H


#include <typeinfo>
#include <cstdlib>
#include <tuple>


namespace D3PD {


/**
 * @brief Placeholder for empty type.
 */
class NoType {};


/// Helper so that Types<> will be an empty class.
template <class T>
struct WrapType
{
  using type = T;
};


/**
 * @brief A simple tuple of multiple types.
 *
 * This can be used as the type argument of @c BlockFillerTool and
 * related templates in order to define a tool that can take one of
 * a set of types as input.  Eg,
 *
 *@code
 *   D3PD::Types<Obj1, Obj2>
 @endcode
 */
template <class... TYPES>
using Types = std::tuple<WrapType<TYPES>...>;


/**
 * @brief Select one type out of the tuple.
 *
 * If `T` is `Types<T0, ...>`, then `SelectType<T, N>::type` will be `TN`;
 * otherwise it will be `T`.
 */
template <class T, int N>
struct SelectType
{
  typedef T type;
};


/// SelectType used of Types.
template <int N, class... TYPES>
struct SelectType<Types<TYPES...>, N>
{
  using type = typename std::tuple_element_t<N, Types<TYPES...> >::type;
};


/**
 * @brief Return one @c type_info from a tuple.
 *
 * If `T` is `Types<T0, ...>`, then ` multiTypeInfo (T*, which)` will
 * return `typeid(Twhich)`.  Otherwise, it will return `typeid(T)`.
 */
template <class T0>
const std::type_info& multiTypeInfo (Types<T0>*, size_t which)
{
  if (which == 0) return typeid (T0);
  std::abort();
}


template <class T0, class... TYPES>
const std::type_info& multiTypeInfo (Types<T0, TYPES...>*, size_t which)
{
  if (which == 0) return typeid (T0);
  return multiTypeInfo (static_cast<Types<TYPES...>*>(nullptr), which-1);
}


template <class T>
const std::type_info& multiTypeInfo (T*, size_t /*which*/)
{
  return typeid (T);
}


template <class T>
struct ButFirstType
{
  using type = T;
};


template <class T0, class... TYPES>
struct ButFirstType<Types<T0, TYPES...> >
{
  using type = Types<TYPES...>;
};


template <class T>
using ButFirstType_t = typename ButFirstType<T>::type;


} // namespace D3PD



#endif // not D3PDMAKERUTILS_TYPES_H
