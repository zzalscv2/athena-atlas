/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ZDC_LUCRODDATACONTAINER_H
#define ZDC_LUCRODDATACONTAINER_H

#include "AthContainers/DataVector.h"
#include "AthenaKernel/CLASS_DEF.h"
#include "ZdcByteStream/ZdcLucrodData.h"

class ZdcLucrodDataContainer : public DataVector<ZdcLucrodData> {
  
 public:
  
  ZdcLucrodDataContainer() {}
  virtual ~ZdcLucrodDataContainer() {}
};

#ifndef __CINT__
CLASS_DEF(ZdcLucrodDataContainer, 1196064529, 1)
#endif

#endif // ZDC_LUCRODDATACONTAINER_H
