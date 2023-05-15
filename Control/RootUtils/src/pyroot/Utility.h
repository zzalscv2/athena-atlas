// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file RootUtils/src/pyroot/Utility.h
 * @author scott snyder, Wim Lavrijsen
 * @date Jul, 2015
 * @brief Utility code originally from pyroot.
 */


#ifndef ROOTUTILS_UTILITY_H
#define ROOTUTILS_UTILITY_H


#include "Python.h"
#include "TPython.h"
#include "TClass.h"


#include "CPyCppyy/PyException.h"
namespace RootUtils {
  using PyException = CPyCppyy::PyException;
}



namespace RootUtils {


int GetBuffer( PyObject* pyobject, char tc, int size, void*& buf, Bool_t check = kTRUE );
TClass* objectIsA (PyObject* obj);
bool setOwnership (PyObject* obj, bool flag);


}


#endif // not ROOTUTILS_UTILITY_H
