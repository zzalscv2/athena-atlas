// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file RootUtils/PyGetString.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Aug, 2019
 * @brief Convert python string -> C++ string for py2 and py3.
 */


#ifndef ROOTUTILS_PYGETSTRING_H
#define ROOTUTILS_PYGETSTRING_H

#ifdef _POSIX_C_SOURCE
# undef _POSIX_C_SOURCE
#endif
#ifdef _XOPEN_SOURCE
# undef _XOPEN_SOURCE
#endif
#include "Python.h"
#include <string>
#include <utility>


namespace RootUtils {


/**
 * @brief Convert python string -> C++ string for py2 and py3.
 *
 * Returns a pair (string, bool); the second element is true of the conversion
 * succeeded.
 *
 * This should be kept as an inline function to avoid having to have direct
 * dependencies on python where it's not really needed.
 */
inline
std::pair<std::string, bool> PyGetString (PyObject* s)
{
  const char* cstr = PyUnicode_AsUTF8AndSize (s, nullptr);
  if (!cstr && PyBytes_Check (s)) {
    PyErr_Clear();
    cstr = PyBytes_AsString (s);
  }
  if (cstr) {
    return std::make_pair (std::string (cstr), true);
  }
  return std::make_pair (std::string(), false);
}


} // namespace RootUtils


#endif // not ROOTUTILS_PYGETSTRING_H
