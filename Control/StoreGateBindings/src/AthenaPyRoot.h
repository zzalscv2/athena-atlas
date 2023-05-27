// -*- C++ -*-

/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef STOREGATEBINDINGS_ATHENAPYROOT_H
#define STOREGATEBINDINGS_ATHENAPYROOT_H 1

#include "Python.h"
#include "RootUtils/PyGetString.h"

// PyROOT includes
#include <TPython.h>
#include "CPyCppyy/PyException.h"
#ifndef ROOT_TPyException
# define ROOT_TPyException 1 /* there was a typo in TPyException-v20882 */
#endif

#define CPPInstance_ASVOIDPTR(o) (TPython::CPPInstance_AsVoidPtr(o))

namespace PyROOT {

inline
void throw_py_exception (bool display = true)
{
  if (display) {
    // fetch error
    PyObject* pytype = 0, *pyvalue = 0, *pytrace = 0;
    PyErr_Fetch (&pytype, &pyvalue, &pytrace);
    Py_XINCREF  (pytype);
    Py_XINCREF  (pyvalue);
    Py_XINCREF  (pytrace);
    // restore...
    PyErr_Restore (pytype, pyvalue, pytrace);
    // and print
    PyErr_Print();
  }
  throw CPyCppyy::PyException();
}

} //> namespace PyROOT

#endif //> STOREGATEBINDINGS_ATHENAPYROOT_H
