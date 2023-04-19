/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/


#include "MM_RawDataProviderToolCore.h"
#include "Identifier/IdentifierHash.h"
#include <atomic>
#include <memory> //for unique_ptr
#include <unordered_map>

using OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment;

//================ Constructor =================================================
Muon::MM_RawDataProviderToolCore::MM_RawDataProviderToolCore(const std::string& t, const std::string& n, const IInterface* p)
: AthAlgTool(t, n, p)
, m_robDataProvider("ROBDataProviderSvc",n) 
{ }

//================ Initialisation ==============================================
StatusCode 
Muon::MM_RawDataProviderToolCore::initialize()
{
  ATH_CHECK(AthAlgTool::initialize());
  ATH_CHECK(m_idHelperSvc.retrieve());
  ATH_CHECK(m_decoder.retrieve());
  ATH_CHECK(m_robDataProvider.retrieve());
  ATH_CHECK(m_rdoContainerKey.initialize());

  m_maxhashtoUse = m_idHelperSvc->mmIdHelper().module_hash_max();  

  return StatusCode::SUCCESS;
}

//==============================================================================
StatusCode 
Muon::MM_RawDataProviderToolCore::convertIntoContainer(const EventContext& ctx, const std::vector<const ROBFragment*>& vecRobs, const std::vector<IdentifierHash>& rdoIdhVect, MM_RawDataContainer& mmRdoContainer) const
{
  // Since there can be multiple ROBFragments contributing to the same RDO collection a temporary cache is setup and passed to fillCollection by reference. Once all ROBFragments are processed the collections are added into the rdo container


  std::unordered_map<IdentifierHash, std::unique_ptr<MM_RawDataCollection>> rdo_map;


  // Loop on the passed ROB fragments, and call the decoder for each one to fill the RDO container.
  for (const ROBFragment* fragment : vecRobs)
    ATH_CHECK( m_decoder->fillCollection(ctx, *fragment, rdoIdhVect, rdo_map) ); // always returns StatusCode::SUCCESS

  // error counters
  int nerr_duplicate{0}, nerr_rdo{0};

  // add the RDO collections created from the data of this ROB into the identifiable container.
  for (auto& [hash, collection]: rdo_map) {

    if ((!collection) or collection->empty()) continue; // skip empty collections

    MM_RawDataContainer::IDC_WriteHandle lock = mmRdoContainer.getWriteHandle(hash);

    if (lock.alreadyPresent()) {
      ++nerr_duplicate;
    } else if (!lock.addOrDelete(std::move(collection)).isSuccess()) {
      // since we prevent duplicates above, this error should never happen.
      ++nerr_rdo;
    }
  }


  // error summary (to reduce the number of messages)
  if (nerr_duplicate) ATH_MSG_WARNING(nerr_duplicate << " elinks skipped since the same module hash has been added by a previous ROB fragment");
  if (nerr_rdo){
     ATH_MSG_ERROR("Failed to add "<<nerr_rdo<<" RDOs into the identifiable container");
     return StatusCode::FAILURE;
  }

  ATH_MSG_DEBUG("Size of mmRdoContainer is " << mmRdoContainer.size());
  return StatusCode::SUCCESS;
}

