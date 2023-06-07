/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/** @file MakeEventStreamInfo.cxx
 *  @brief This file contains the implementation for the MakeEventStreamInfo class.
 *  @author Peter van Gemmeren <gemmeren@anl.gov>
 **/

#include "MakeEventStreamInfo.h"

#include "GaudiKernel/IAlgorithm.h"

#include "PersistentDataModel/DataHeader.h"
#include "EventInfo/EventStreamInfo.h"
#include "EventInfo/EventInfo.h"
#include "EventInfo/EventID.h"
#include "StoreGate/StoreGateSvc.h"
#include "xAODEventInfo/EventInfo.h"
#include "EventInfoUtils/EventInfoFromxAOD.h"

//___________________________________________________________________________
MakeEventStreamInfo::MakeEventStreamInfo(const std::string& type,
	const std::string& name,
	const IInterface* parent) : base_class(type, name, parent)
{
}
//___________________________________________________________________________
MakeEventStreamInfo::~MakeEventStreamInfo() {
}
//___________________________________________________________________________
StatusCode MakeEventStreamInfo::initialize() {
   ATH_MSG_DEBUG("Initializing " << name());
   // Locate the MetaDataStore
   if (!m_metaDataSvc.retrieve().isSuccess()) {
      ATH_MSG_FATAL("Could not find MetaDataSvc");
      return(StatusCode::FAILURE);
   }
   if (!m_eventStore.retrieve().isSuccess()) {
      ATH_MSG_FATAL("Could not find EventStore");
      return(StatusCode::FAILURE);
   }

   // Autoconfigure data header key
   if (m_dataHeaderKey.empty()){
      const IAlgorithm* parentAlg = dynamic_cast<const IAlgorithm*>(this->parent());
      if (parentAlg == nullptr) {
         ATH_MSG_ERROR("Unable to get parent Algorithm");
         return(StatusCode::FAILURE);
      }
      m_dataHeaderKey.setValue(parentAlg->name());
   }

   m_filledEvent = false;

   return(StatusCode::SUCCESS);
}
//___________________________________________________________________________
StatusCode MakeEventStreamInfo::postInitialize() {
   // Remove EventStreamInfo with same key if it exists
   bool ignoreIfAbsent = true;
   if( !m_metaDataSvc->remove<EventStreamInfo>(m_key.value(), ignoreIfAbsent).isSuccess() ) {
      ATH_MSG_ERROR("Unable to remove EventStreamInfo with key " << m_key.value());
      return StatusCode::FAILURE;
   }
   return(StatusCode::SUCCESS);
}
//___________________________________________________________________________
StatusCode MakeEventStreamInfo::preExecute() {
   return(StatusCode::SUCCESS);
}
//___________________________________________________________________________
StatusCode MakeEventStreamInfo::preStream() {
   return(StatusCode::SUCCESS);
}
//___________________________________________________________________________
StatusCode MakeEventStreamInfo::postExecute() {
   SG::ReadHandle<DataHeader> dataHeader(m_dataHeaderKey);
   if (!dataHeader.isValid()) {
      return(StatusCode::SUCCESS);
   }
   // Retrieve the EventInfo object
   EventType evtype;
   unsigned long long runN = 0;
   unsigned lumiN = 0;
   SG::ReadHandle<xAOD::EventInfo> xEventInfo(m_eventInfoKey);
   if (xEventInfo.isValid()) {
      runN = xEventInfo->runNumber();
      lumiN = xEventInfo->lumiBlock();
      evtype = eventTypeFromxAOD(xEventInfo.get());
   } else {
      SG::ReadHandle<EventInfo> oEventInfo(m_oEventInfoKey);
      if (oEventInfo.isValid()) {
         runN = oEventInfo->event_ID()->run_number();
         lumiN = oEventInfo->event_ID()->lumi_block();
         evtype = *oEventInfo->event_type();
      } else {
         ATH_MSG_ERROR("Unable to retrieve EventInfo object");
         return(StatusCode::FAILURE);
      }
   }

   EventStreamInfo* pEventStream = m_metaDataSvc->tryRetrieve<EventStreamInfo>(m_key.value());
   if( !pEventStream ) {
      auto esinfo_up = std::make_unique<EventStreamInfo>();
      pEventStream = esinfo_up.get();
      if( m_metaDataSvc->record(std::move(esinfo_up), m_key.value()).isFailure() ) {
         ATH_MSG_ERROR("Could not register EventStreamInfo object");
         return(StatusCode::FAILURE);
      }
   }
   pEventStream->addEvent();
   pEventStream->insertProcessingTag(dataHeader->getProcessTag());
   pEventStream->insertLumiBlockNumber( lumiN );
   pEventStream->insertRunNumber( runN );
   for (const DataHeaderElement& dhe : *dataHeader) {
      pEventStream->insertItemList(dhe.getPrimaryClassID(), dhe.getKey());
   }
   pEventStream->insertEventType( evtype );

   m_filledEvent = true;

   return(StatusCode::SUCCESS);
}
//___________________________________________________________________________
StatusCode MakeEventStreamInfo::preFinalize() {
  EventStreamInfo* pEventStream = m_metaDataSvc->tryRetrieve<EventStreamInfo>(m_key.value());
  if (!pEventStream) {
    auto esinfo_up = std::make_unique<EventStreamInfo>();
    pEventStream = esinfo_up.get();
    ATH_CHECK(m_metaDataSvc->record(std::move(esinfo_up), m_key.value()));
  }
  if (!m_filledEvent) {
    // insert non-event information (processingTags)
    // to EventStreamInfo if we have not processed any event

    pEventStream->insertProcessingTag(m_dataHeaderKey.value());
  }
  return (StatusCode::SUCCESS);
}
//___________________________________________________________________________
StatusCode MakeEventStreamInfo::finalize() {
   ATH_MSG_DEBUG("in finalize()");
   // release the MetaDataStore
   if (!m_metaDataSvc.release().isSuccess()) {
      ATH_MSG_WARNING("Could not release MetaDataStore");
   }
   if (!m_eventStore.release().isSuccess()) {
      ATH_MSG_WARNING("Could not release EventStore");
   }
   return(StatusCode::SUCCESS);
}
