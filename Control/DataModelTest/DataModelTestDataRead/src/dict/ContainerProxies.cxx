/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "xAODCore/AddDVProxy.h"
#include "DataModelTestDataRead/GVec.h"
#include "DataModelTestDataRead/versions/HVec_v1.h"
#include "DataModelTestDataRead/versions/HVec_v2.h"
#include "DataModelTestDataRead/versions/AllocTestContainer_v1.h"

ADD_NS_DV_PROXY (DMTest, GVec);
ADD_NS_DV_PROXY (DMTest, HVec_v1);
ADD_NS_DV_PROXY (DMTest, HVec_v2);
ADD_NS_DV_PROXY (DMTest, AllocTestContainer_v1);
