// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
 */
/**
 * @file DataModelTestDataRead/versions/AllocTestAuxContainer_v1.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Nov, 2022
 * @brief Testing an xAOD object with a non-standard memory allocator.
 */


#ifndef DATAMODELTESTDATAREAD_ALLOCTESTAUXCONTAINER_V1_H
#define DATAMODELTESTDATAREAD_ALLOCTESTAUXCONTAINER_V1_H


#include "xAODCore/AuxContainerBase.h"
#include "AthenaKernel/BaseInfo.h"
#include "TestTools/TestAlloc.h"
#include <vector>
#include <memory>


namespace DMTest {


/**
 * @brief Testing an xAOD object with a non-standard memory allocator.
 *
 * This class uses non-default allocators.
 * The identically-named class in DataModelTestDataWrite is the same
 * except for not using the non-default allocators.  That will be used
 * for testing forwards/backwards compatibility.
 */
class AllocTestAuxContainer_v1
  : public xAOD::AuxContainerBase
{
public:
  AllocTestAuxContainer_v1();
  AllocTestAuxContainer_v1 (std::pmr::memory_resource* r);


  // For this one, the dictionary will always be present (from AthContainers).
  std::vector<int, std::pmr::polymorphic_allocator<int> > atInt1;

  // For this one, the dictionary is only present in this package.
  std::vector<int, Athena_test::TestAlloc<int> > atInt2;
};


} // namespace DMTest


SG_BASE (DMTest::AllocTestAuxContainer_v1, xAOD::AuxContainerBase);


#endif // not DATAMODELTESTDATAREAD_ALLOCTESTAUXCONTAINER_V1_H
