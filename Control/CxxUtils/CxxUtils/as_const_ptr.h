// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file CxxUtils/as_const_ptr.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Feb, 2023
 * @brief Helper for getting a const version of a pointer.
 */


#ifndef CXXUTILS_AS_CONST_PTR_H
#define CXXUTILS_AS_CONST_PTR_H


namespace CxxUtils {


/**
 * @brief Helper for getting a const version of a pointer.
 * @param p Pointer to convert.
 *
 * Given <code>T* p</code>, <code>as_const_ptr(p)</code> will return @c p
 * as a <code>const T*</code>.
 *
 * This is similar in spirit to @c std::as_const (which doesn't really
 * do what you might expect for pointers).
 */
template <class T>
inline
const T* as_const_ptr (const T* p)
{
  return p;
}


} // namespace CxxUtils


#endif // not CXXUTILS_AS_CONST_PTR_H
