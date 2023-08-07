/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef I_H5_FILE_SVC_H
#define I_H5_FILE_SVC_H

#include "GaudiKernel/IService.h"

// This service gives access to an H5::Group, so that subgroups within
// the same file can be created by mulitple clients.
//
// Note that this inherits all the normal IO related issues to the
// client: if multiple clients try to create the same dataset,
// especially from different threads, there's no garentee that things
// will work out well!
//

namespace H5 {
  class Group;
}

class IH5GroupSvc : virtual public IService
{
public:
  virtual ~IH5GroupSvc() {};
  static const InterfaceID& interfaceID();
  virtual H5::Group* group() = 0;
};

inline const InterfaceID& IH5GroupSvc::interfaceID() {
  static const InterfaceID id("IH5GroupSvc", 1, 0);
  return id;
}

#endif
