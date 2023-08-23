/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IDVPM_XIncludeErrHandler_h
#define IDVPM_XIncludeErrHandler_h
#include "CxxUtils/checker_macros.h"
#include <xercesc/dom/DOMErrorHandler.hpp>


class ATLAS_NOT_THREAD_SAFE XIncludeErrHandler : public xercesc::DOMErrorHandler {
public:
  XIncludeErrHandler();
  ~XIncludeErrHandler();
  // no copy
  XIncludeErrHandler(const XIncludeErrHandler&) = delete;
  // no assignment
  void operator = (const XIncludeErrHandler&) = delete;
  bool
  getSawErrors() const {
    return m_errors;
  }

  bool handleError(const xercesc::DOMError& domError);
  void resetErrors();
private:
  bool m_errors;
};
#endif
