// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file DataModelTestDataRead/src/AllocTestWriteWithAlloc.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Nov, 2022
 * @brief Test writing AllocTest with a non-default allocator.
 */



#ifndef DATAMODELTESTDATAREAD_ALLOCTESTWRITEWITHALLOC_H
#define DATAMODELTESTDATAREAD_ALLOCTESTWRITEWITHALLOC_H


#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/WriteHandleKey.h"
#include "DataModelTestDataRead/AllocTestContainer.h"
#include "DataModelTestDataRead/AllocTest.h"


namespace DMTest {


/**
 * @brief Test writing AllocTest with a non-default allocator.
 *
 * This is the counterpart of AllocTestWiteWithoutAlloc.  It writes
 * AllocTest using a custom allocator.
 */
class AllocTestWriteWithAlloc
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
  SG::WriteHandleKey<AllocTestContainer> m_containerKey
  { this, "ContainerKey", "AllocTest" };
};


} // namespace DMTest


#endif // not DATAMODELTESTDATAREAD_ALLOCTESTWRITEWITHALLOC_H
