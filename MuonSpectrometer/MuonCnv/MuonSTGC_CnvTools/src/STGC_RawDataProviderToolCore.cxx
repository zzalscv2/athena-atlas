/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#include <atomic>
#include "STGC_RawDataProviderToolCore.h"
using OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment;

//================ Constructor =================================================
Muon::STGC_RawDataProviderToolCore::STGC_RawDataProviderToolCore(const std::string& t, const std::string& n, const IInterface* p) 
: AthAlgTool(t, n, p)
, m_robDataProvider("ROBDataProviderSvc",n) 
{ }

//================ Initialisation ==============================================
StatusCode Muon::STGC_RawDataProviderToolCore::initialize()
{
  ATH_CHECK(m_idHelperSvc.retrieve());
  ATH_CHECK(m_decoder.retrieve());
  ATH_CHECK(m_robDataProvider.retrieve()); // ROBDataProviderSvc
  ATH_CHECK(m_rdoContainerKey.initialize());

  m_maxhashtoUse = m_idHelperSvc->stgcIdHelper().module_hash_max();  
  return StatusCode::SUCCESS;
}

//==============================================================================
StatusCode Muon::STGC_RawDataProviderToolCore::convertIntoContainer(const std::vector<const ROBFragment*>& vecRobs, const std::vector<IdentifierHash>& rdoIdhVect, STGC_RawDataContainer& stgcRdoContainer) const
{
  // Since there can be multiple ROBFragments contributing to the same RDO collection a temporary cache is setup and passed to fillCollection by reference. Once all ROBFragments are processed the collections are added into the rdo container
  std::unordered_map<IdentifierHash, std::unique_ptr<STGC_RawDataCollection>> rdo_map;


  // Loop on the passed ROB fragments, and call the decoder for each one to fill the RDO container.
  for (const ROBFragment* fragment : vecRobs)
    ATH_CHECK( m_decoder->fillCollection(*fragment, rdoIdhVect, rdo_map) ); // always returns StatusCode::SUCCESS

  
  // error counters
  int nerr_duplicate{0}, nerr_rdo{0};

  // add the RDO collections created from the data of this ROB into the identifiable container.
  for (auto& [hash, collection]: rdo_map) {

    if (!collection->size()) continue; // skip empty collections

    STGC_RawDataContainer::IDC_WriteHandle lock = stgcRdoContainer.getWriteHandle(hash);

    if (lock.alreadyPresent()) {
      ++nerr_duplicate;
    } else if (!lock.addOrDelete(std::move(collection)).isSuccess()) {
      // since we prevent duplicates above, this error should never happen.
      ++nerr_rdo;
    }
  }


  // error summary (to reduce the number of messages)
  if (nerr_duplicate) ATH_MSG_WARNING(nerr_duplicate << " elinks skipped since the same module hash has been added by a previous ROB fragment");
  if (nerr_rdo) {
      ATH_MSG_ERROR("Failed to add "<<nerr_rdo<<" RDOs into the identifiable container");
      return StatusCode::FAILURE;
   }

  ATH_MSG_DEBUG("Size of sTgcRdoContainer is " << stgcRdoContainer.size());
  return StatusCode::SUCCESS;
}

