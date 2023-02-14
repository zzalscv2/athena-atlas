// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file CxxUtils/releasing_iterator.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Feb, 2023
 * @brief Adapter to retrieve elements from a unique_ptr iterator via release().
 */


#ifndef CXXUTILS_RELEASING_ITERATOR_H
#define CXXUTILS_RELEASING_ITERATOR_H


#include <iterator>


namespace CxxUtils {


/**
 * @brief Adapter to retrieve elements from a unique_ptr iterator via release().
 *
 * This allows, say,
 *@code
 *   std::vector<std::unique_ptr<Foo> > c1 = ...;
 *   std::vector<Foo*> c2;
 *   c2.assign (releasing_iterator(c1.begin()), releasing_iterator(c1.end()));
 @endcode
 *
 * All elements in @c c1 will be release'd.
 *
 * Since this implies that a given iterator can only be dereferenced once,
 * this iterator models @c input_iterator.
 */
template <class ITERATOR>
class releasing_iterator
{
public:
  using value_type = typename std::iterator_traits<ITERATOR>::value_type::pointer;
  using reference = value_type&;
  using pointer = value_type*;
  using difference_type = typename std::iterator_traits<ITERATOR>::difference_type;
  using iterator_category = std::input_iterator_tag;

  releasing_iterator (ITERATOR it) : m_it (it) {}
  value_type operator*() { return m_it->release(); }
  releasing_iterator& operator++() { ++m_it; return *this; }
  bool operator!= (const releasing_iterator& other) const { return m_it != other.m_it; }


private:
  ITERATOR m_it;
};


} // namespace CxxUtils


#endif // not CXXUTILS_RELEASING_ITERATOR_H
