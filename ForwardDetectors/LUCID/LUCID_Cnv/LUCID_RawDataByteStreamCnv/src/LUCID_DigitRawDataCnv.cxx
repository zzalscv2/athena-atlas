/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "LUCID_RawDataByteStreamCnv/LUCID_DigitRawDataCnv.h"

#include "GaudiKernel/ThreadLocalContext.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"
LUCID_DigitRawDataCnv::LUCID_DigitRawDataCnv(const std::string& name,
                                             ISvcLocator* pSvcLocator)
  : AthAlgorithm(name, pSvcLocator)
{
}

LUCID_DigitRawDataCnv::~LUCID_DigitRawDataCnv() {}

StatusCode
LUCID_DigitRawDataCnv::initialize()
{
  ATH_CHECK(m_digitContainerKey.initialize());
  ATH_CHECK(m_lucid_RawDataContainerKey.initialize());
  return StatusCode::SUCCESS;
}

StatusCode
LUCID_DigitRawDataCnv::execute()
{

  const EventContext& ctx = Gaudi::Hive::currentContext();
  SG::ReadHandle<LUCID_DigitContainer> digitContainer(m_digitContainerKey, ctx);

  if (!digitContainer.isValid()) {
    ATH_MSG_WARNING("BAD DATA!!! Input data does not include "
                    << m_digitContainerKey);
    return StatusCode::SUCCESS;
  }

  for (const LUCID_Digit* digit : *digitContainer) {
    m_rodEncoder.addDigit(digit);
  }

  std::vector<uint32_t> data_block;
  m_rodEncoder.encode(data_block, msg());
  data_block.push_back(0); // add status word

  auto container = std::make_unique<LUCID_RawDataContainer>();
  container->push_back(new LUCID_RawData(data_block));
  ATH_CHECK(SG::makeHandle(m_lucid_RawDataContainerKey, ctx)
              .record(std::move(container)));

  return StatusCode::SUCCESS;
}
