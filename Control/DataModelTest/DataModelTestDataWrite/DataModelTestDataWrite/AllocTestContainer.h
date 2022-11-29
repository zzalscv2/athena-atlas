// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file DataModelTestDataWrite/AllocTestContainer.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Nov, 2022
 * @brief Testing an xAOD object with a non-standard memory allocator.
 */


#ifndef DATAMODELTESTDATAWRITE_ALLOCTESTCONTAINER_H
#define DATAMODELTESTDATAWRITE_ALLOCTESTCONTAINER_H


#include "DataModelTestDataWrite/versions/AllocTestContainer_v1.h"


namespace DMTest {


typedef AllocTestContainer_v1 AllocTestContainer;


} // namespace DMTest


#include "xAODCore/CLASS_DEF.h"
CLASS_DEF (DMTest::AllocTestContainer, 9841, 1)


#endif // not DATAMODELTESTDATAWRITE_ALLOCTESTCONTAINER_H
