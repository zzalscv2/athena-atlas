// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file CxxUtils/starts_with.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Mar, 2023
 * @brief C++20-like starts_with/ends_with for strings.
 */


#ifndef CXXUTILS_STARTS_WITH_H
#define CXXUTILS_STARTS_WITH_H


#include <string>


namespace CxxUtils {


/**
 * @brief Test whether one string starts with another.
 * @param s String in which to search.
 * @param prefix Prefix for which to search.
 *
 * Returns true if string @c s starts with @c prefix.
 */
bool starts_with (const std::string& s, const std::string& prefix);


/**
 * @brief Test whether one string ends with another.
 * @param s String in which to search.
 * @param suffix Suffix for which to search.
 *
 * Returns true if string @c s ends with @c suffix.
 */
bool ends_with   (const std::string& s, const std::string& suffix);


} // namespace CxxUtils


#include "CxxUtils/starts_with.icc"


#endif // not CXXUTILS_STARTS_WITH_H
