// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file DataModelTestDataRead/versions/AllocTest_v1.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Nov, 2022
 * @brief Testing an xAOD object with a non-standard memory allocator.
 */


#ifndef DATAMODELTESTDATAREAD_ALLOCTEST_V1_H
#define DATAMODELTESTDATAREAD_ALLOCTEST_V1_H


#include "AthContainers/AuxElement.h"
#include "AthenaKernel/BaseInfo.h"


namespace DMTest {


/**
 * @brief Testing an xAOD object with a non-standard memory allocator.
 *
 * This is the counterpart of AllocTest from DataModelTestDataWrite.
 * This version uses non-default allocators.
 */
class AllocTest_v1
  : public SG::AuxElement
{
public:
  int atInt1() const;
  void setAtInt1 (int i);
  int atInt2() const;
  void setAtInt2 (int i);
};


} // namespace DMTest


SG_BASE (DMTest::AllocTest_v1, SG::AuxElement);


#endif // not DATAMODELTESTDATAREAD_ALLOCTEST_V1_H
