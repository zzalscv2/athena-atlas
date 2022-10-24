/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "PixelDeadMapCondAlg.h"
#include "GaudiKernel/EventIDRange.h"
#include "StringUtilities.h"


using PixelConditionsAlgorithms::parseDeadMapString;

PixelDeadMapCondAlg::PixelDeadMapCondAlg(const std::string& name, ISvcLocator* pSvcLocator):
  ::AthReentrantAlgorithm(name, pSvcLocator)
{
}

StatusCode PixelDeadMapCondAlg::initialize() {
  ATH_MSG_DEBUG("PixelDeadMapCondAlg::initialize()");

  ATH_CHECK(m_readKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_writeKey.initialize());

  return StatusCode::SUCCESS;
}

StatusCode PixelDeadMapCondAlg::execute(const EventContext& ctx) const {
  ATH_MSG_DEBUG("PixelDeadMapCondAlg::execute()");

  SG::WriteCondHandle<PixelDeadMapCondData> writeHandle(m_writeKey, ctx);
  if (writeHandle.isValid()) {
    ATH_MSG_DEBUG("CondHandle " << writeHandle.fullKey() << " is already valid.. In theory this should not be called, but may happen if multiple concurrent events are being processed out of order.");
    return StatusCode::SUCCESS; 
  }

  // Construct the output Cond Object and fill it in
  std::unique_ptr<PixelDeadMapCondData> writeCdo(std::make_unique<PixelDeadMapCondData>());

  const EventIDBase start{EventIDBase::UNDEFNUM, EventIDBase::UNDEFEVT,                     0,                       
                                              0, EventIDBase::UNDEFNUM, EventIDBase::UNDEFNUM};
  const EventIDBase stop {EventIDBase::UNDEFNUM,   EventIDBase::UNDEFEVT, EventIDBase::UNDEFNUM-1, 
                          EventIDBase::UNDEFNUM-1, EventIDBase::UNDEFNUM, EventIDBase::UNDEFNUM};

  EventIDRange rangeW{start, stop};
  if (!m_readKey.empty()) {
    SG::ReadCondHandle<CondAttrListCollection> readHandle(m_readKey, ctx);
    const CondAttrListCollection* readCdo = *readHandle; 
    if (readCdo==nullptr) {
      ATH_MSG_FATAL("Null pointer to the read conditions object");
      return StatusCode::FAILURE;
    }
    // Get the validitiy range
    if (not readHandle.range(rangeW)) {
      ATH_MSG_FATAL("Failed to retrieve validity range for " << readHandle.key());
      return StatusCode::FAILURE;
    }
    ATH_MSG_INFO("Size of AthenaAttributeList " << readHandle.fullKey() << " readCdo->size()= " << readCdo->size());
    ATH_MSG_INFO("Range of input is " << rangeW);

    for (const auto & attrList : *readCdo) {
      const CondAttrListCollection::AttributeList &payload = attrList.second;
      // RUN-3 format
      if (payload.exists("data_array") and not payload["data_array"].isNull()) {
        const std::string &stringStatus = payload["data_array"].data<std::string>();
        const auto & hashStatusVector = parseDeadMapString(stringStatus);
        for (const auto & [hash, status] : hashStatusVector){
          //status ==0 means its the module status to be set to '1'
          if (status==0) writeCdo->setModuleStatus(hash, 1);
          //...any other status will set the chip status
          else writeCdo->setChipStatus(hash, status);
        }
      }
    }
  }

  if (rangeW.stop().isValid() and rangeW.start()>rangeW.stop()) {
    ATH_MSG_FATAL("Invalid intersection rangeW: " << rangeW);
    return StatusCode::FAILURE;
  }

  if (writeHandle.record(rangeW, std::move(writeCdo)).isFailure()) {
    ATH_MSG_FATAL("Could not record PixelDeadMapCondData " << writeHandle.key() << " with EventRange " << rangeW << " into Conditions Store");
    return StatusCode::FAILURE;
  }
  ATH_MSG_INFO("recorded new CDO " << writeHandle.key() << " with range " << rangeW << " into Conditions Store");

  return StatusCode::SUCCESS;
}

