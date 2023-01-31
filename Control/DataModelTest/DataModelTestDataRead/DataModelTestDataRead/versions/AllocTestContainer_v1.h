// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file DataModelTestDataRead/versions/AllocTestContainer_v1.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Nov, 2022
 * @brief Testing an xAOD object with a non-standard memory allocator.
 */


#ifndef DATAMODELTESTDATAREAD_ALLOCTESTCONTAINER_V1_H
#define DATAMODELTESTDATAREAD_ALLOCTESTCONTAINER_V1_H


#include "DataModelTestDataRead/versions/AllocTest_v1.h"
#include "AthContainers/DataVector.h"


namespace DMTest {


typedef DataVector<AllocTest_v1> AllocTestContainer_v1;


} // namespace DMTest


#endif // not DATAMODELTESTDATAREAD_ALLOCTESTCONTAINER_V1_H
