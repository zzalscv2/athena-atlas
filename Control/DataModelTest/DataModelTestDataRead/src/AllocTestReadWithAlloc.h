// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file DataModelTestDataRead/src/AllocTestReadWithAlloc.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Nov, 2022
 * @brief Test reading AllocTest with a non-default allocator.
 */


#ifndef DATAMODELTESTDATAREAD_ALLOCTESTREADWITHALLOC_H
#define DATAMODELTESTDATAREAD_ALLOCTESTREADWITHALLOC_H


#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "DataModelTestDataRead/AllocTestContainer.h"
#include "DataModelTestDataRead/AllocTest.h"


namespace DMTest {


/**
 * @brief Test reading AllocTest with a non-default allocator.
 *
 * This is the counterpart of AllocTestReadWithoutAlloc.  It reads
 * AllocTest using a custom allocator.
 */
class AllocTestReadWithAlloc
  : public AthReentrantAlgorithm
{
public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;


  /**
   * @brief Gaudi initialize method.
   */
  virtual StatusCode initialize() override;


  /**
   * @brief Algorithm event processing.
   */
  virtual StatusCode execute (const EventContext& ctx) const override;


private:
  SG::ReadHandleKey<AllocTestContainer> m_containerKey
  { this, "ContainerKey", "AllocTest" };
};


} // namespace DMTest


#endif // not DATAMODELTESTDATAREAD_ALLOCTESTREADWITHALLOC_H
