/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LUCID_DIGITRAWDATACNV_H
#define LUCID_DIGITRAWDATACNV_H

#include <stdint.h>
#include <string>

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "LUCID_RawEvent/LUCID_DigitContainer.h"
#include "LUCID_RawEvent/LUCID_RawData.h"
#include "LUCID_RawEvent/LUCID_RawDataContainer.h"

#include "LUCID_RawDataByteStreamCnv/LUCID_RodEncoder.h"

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
class LUCID_RodEncoder;

class LUCID_DigitRawDataCnv : public AthReentrantAlgorithm
{

public:
  LUCID_DigitRawDataCnv(const std::string& name, ISvcLocator* pSvcLocator);
  ~LUCID_DigitRawDataCnv();

  StatusCode initialize() override;
  StatusCode execute(const EventContext& ctx) const override;

private:
  SG::WriteHandleKey<LUCID_RawDataContainer> m_lucid_RawDataContainerKey{
    this,
    "lucid_RawDataContainerKey",
    "Lucid_RawData",
    ""
  };
  SG::ReadHandleKey<LUCID_DigitContainer>
    m_digitContainerKey{ this, "lucid_DigitContainerKey", "Lucid_Digits", "" };

  LUCID_RodEncoder m_rodEncoder;
};

#endif

