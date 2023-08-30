// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file DataModelTestDataCommon/xAODTestWriteFwdLink1.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Aug, 2023
 * @brief Testing writing of forward-declared DataLink.
 */


#ifndef DATAMODELTESTDATACOMMON_XAODTESTWRITEFWDLINK1_H
#define DATAMODELTESTDATACOMMON_XAODTESTWRITEFWDLINK1_H


#include "DataModelTestDataCommon/CVec.h"
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/WriteHandleKey.h"


namespace DMTest {


class xAODTestWriteFwdLink1
  : public AthReentrantAlgorithm
{
public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  virtual StatusCode initialize() override;
  virtual StatusCode execute (const EventContext& ctx) const override;


private:
  SG::WriteHandleKey<DMTest::CVec> m_cvecKey
  { this, "CVecKey", "CVecFwdLink", "" };
};


} // namespace DMTest


#endif // not DATAMODELTESTDATACOMMON_XAODTESTWRITEFWDLINK1_H
