// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file DataModelTestDataRead/AllocTestAuxContainer.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Nov, 2022
 * @brief Testing an xAOD object with a non-standard memory allocator.
 */


#ifndef DATAMODELTESTDATAREAD_ALLOCTESTAUXCONTAINER_H
#define DATAMODELTESTDATAREAD_ALLOCTESTAUXCONTAINER_H


#include "DataModelTestDataRead/versions/AllocTestAuxContainer_v1.h"


namespace DMTest {


typedef AllocTestAuxContainer_v1 AllocTestAuxContainer;


} // namespace DMTest


#include "xAODCore/CLASS_DEF.h"
CLASS_DEF (DMTest::AllocTestAuxContainer, 9842, 1)


#endif // not DATAMODELTESTDATAREAD_ALLOCTESTAUXCONTAINER_H
