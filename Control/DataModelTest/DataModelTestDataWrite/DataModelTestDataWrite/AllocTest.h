// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file DataModelTestDataWrite/AllocTest.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Nov, 2022
 * @brief Testing an xAOD object with a non-standard memory allocator.
 */


#ifndef DATAMODELTESTDATAWRITE_ALLOCTEST_H
#define DATAMODELTESTDATAWRITE_ALLOCTEST_H


#include "DataModelTestDataWrite/versions/AllocTest_v1.h"


namespace DMTest {


typedef AllocTest_v1 AllocTest;


} // namespace DMTest


#include "xAODCore/CLASS_DEF.h"
CLASS_DEF (DMTest::AllocTest, 9844, 1)


#endif // not DATAMODELTESTDATAWRITE_ALLOCTEST_H
