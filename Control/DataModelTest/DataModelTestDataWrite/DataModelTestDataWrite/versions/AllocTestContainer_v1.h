// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file DataModelTestDataWrite/versions/AllocTestContainer_v1.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Nov, 2022
 * @brief Testing an xAOD object with a non-standard memory allocator.
 */


#ifndef DATAMODELTESTDATAWRITE_ALLOCTESTCONTAINER_V1_H
#define DATAMODELTESTDATAWRITE_ALLOCTESTCONTAINER_V1_H


#include "DataModelTestDataWrite/versions/AllocTest_v1.h"
#include "AthContainers/DataVector.h"


namespace DMTest {


typedef DataVector<AllocTest_v1> AllocTestContainer_v1;


} // namespace DMTest


#endif // not DATAMODELTESTDATAWRITE_ALLOCTESTCONTAINER_V1_H
