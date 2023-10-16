// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file CxxUtils/no_unique_address.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Oct, 2023
 * @brief Wrapper for C++20 no_unique_address attribute.
 *
 * This header provides a macro @c ATH_NO_UNIQUE_ADDRESS that provides
 * the C++20 [[no_unique_address]] attribute if it is supported
 * by the compiler.
 *
 * (In practice, this attribute is supported by all our current compilers,
 * even in c++17 mode, but to be safe, use this until we switch to c++20.)
 */


#ifndef CXXUTILS_NO_UNIQUE_ADDRESS_H
#define CXXUTILS_NO_UNIQUE_ADDRESS_H


#if !defined(HAVE_NO_UNIQUE_ADDRESS) && defined(__has_cpp_attribute)
# if __has_cpp_attribute(no_unique_address)
#  define HAVE_NO_UNIQUE_ADDRESS 1
# endif
#endif
#if !defined(HAVE_NO_UNIQUE_ADDRESS)
# define HAVE_NO_UNIQUE_ADDRESS 0
#endif

#if HAVE_NO_UNIQUE_ADDRESS
# define ATH_NO_UNIQUE_ADDRESS [[no_unique_address]]
#else
# define ATH_NO_UNIQUE_ADDRESS
#endif


#endif // not CXXUTILS_NO_UNIQUE_ADDRESS_H
