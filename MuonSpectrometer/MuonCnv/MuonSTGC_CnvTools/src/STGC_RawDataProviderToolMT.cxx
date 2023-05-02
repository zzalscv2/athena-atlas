/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// STGC_RawDataProviderToolMT.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#include "STGC_RawDataProviderToolMT.h"
#include "MuonRDO/STGC_RawDataContainer.h"
#include "eformat/SourceIdentifier.h"
using eformat::helper::SourceIdentifier;

//==============================================================================
Muon::STGC_RawDataProviderToolMT::STGC_RawDataProviderToolMT(const std::string& t, const std::string& n, const IInterface*  p) 
: STGC_RawDataProviderToolCore(t, n, p)
{
  declareInterface<IMuonRawDataProviderTool>(this);
}


//==============================================================================
StatusCode Muon::STGC_RawDataProviderToolMT::initialize()
{
  // generate all the Source Identifiers to request the fragments.
  // assume 16 RODs per side (one per sector) and that ROB ID = ROD ID.
  for (uint32_t detID : {eformat::MUON_STGC_ENDCAP_A_SIDE, eformat::MUON_STGC_ENDCAP_C_SIDE}) { //0x6D, 0x6E
    for (uint8_t sectorID(0); sectorID < 16; ++sectorID) {
       // for now lets build all the possible ROB ids of all possible readout configurations
       // maybe later we can come up with a smart way to detect which readout sheme is running and only request the relevant ROB ids from the ROBDataProviderSvc
       // reference: slide 6 of https://indico.cern.ch/event/1260377/contributions/5294286/attachments/2603399/4495810/NSW-SwRod-Felix-v3.pdf
 
       uint16_t moduleID = (0x0 << 8) | sectorID; // combined/single ROB
       SourceIdentifier sid(static_cast<eformat::SubDetector>(detID), moduleID);
       m_allRobIds.push_back(sid.simple_code());
       
       moduleID = (0x1 << 8) | sectorID; // full device ROB (split configuration)
       sid = SourceIdentifier(static_cast<eformat::SubDetector>(detID), moduleID);
       m_allRobIds.push_back(sid.simple_code());
       
       moduleID = (0x2 << 8) | sectorID; // shared device ROB (split configuration)
       sid = SourceIdentifier(static_cast<eformat::SubDetector>(detID), moduleID);
       m_allRobIds.push_back(sid.simple_code());
       
       moduleID = (0x3 << 8) | sectorID; // spare device ROB (split configuration)
       sid = SourceIdentifier(static_cast<eformat::SubDetector>(detID), moduleID);
       m_allRobIds.push_back(sid.simple_code());
    }
  }

  ATH_CHECK(m_rdoContainerCacheKey.initialize(!m_rdoContainerCacheKey.key().empty()));
  ATH_CHECK(STGC_RawDataProviderToolCore::initialize());
  return StatusCode::SUCCESS;
}


//==============================================================================
StatusCode Muon::STGC_RawDataProviderToolMT::initRdoContainer(const EventContext& ctx, STGC_RawDataContainer*& rdoContainer) const
{
  // Create the identifiable RdoContainer in StoreGate to be filled with decoded fragment contents.
  SG::WriteHandle<STGC_RawDataContainer> rdoContainerHandle(m_rdoContainerKey, ctx); 

  const bool externalCacheRDO = !m_rdoContainerCacheKey.key().empty();
  if(!externalCacheRDO){
    ATH_CHECK(rdoContainerHandle.record(std::make_unique<STGC_RawDataContainer>(m_maxhashtoUse)));
    ATH_MSG_DEBUG("Created STGC RDO container");
  } else {
    SG::UpdateHandle<STGC_RawDataCollection_Cache> update(m_rdoContainerCacheKey, ctx);
    ATH_CHECK(update.isValid());
    ATH_CHECK(rdoContainerHandle.record(std::make_unique<STGC_RawDataContainer>(update.ptr())));
    ATH_MSG_DEBUG("Created STGC RDO container using cache for " << m_rdoContainerCacheKey.key());
  }

  // this should never happen, but since we dereference the pointer, we should check
  if (!(rdoContainer = rdoContainerHandle.ptr())) {
    ATH_MSG_ERROR("The STGC RDO container is null, cannot decode STGC data");
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}


//==============================================================================
StatusCode Muon::STGC_RawDataProviderToolMT::convert(const std::vector<IdentifierHash>& rdoIdhVect, const EventContext& ctx) const
{
  // method for RoI-seeded mode. we don't let empty hash containers reach the decoder, 
  // since an empty container means unseeded mode (decode everything).

  STGC_RawDataContainer* rdoContainer{nullptr};
  ATH_CHECK(initRdoContainer(ctx, rdoContainer));

  if (rdoIdhVect.empty() || m_skipDecoding) return StatusCode::SUCCESS;

  std::vector<const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment*> vecRobf;
  m_robDataProvider->getROBData(m_allRobIds, vecRobf);

  return convertIntoContainer(ctx, vecRobf, rdoIdhVect, *rdoContainer);
}


//==============================================================================
StatusCode  Muon::STGC_RawDataProviderToolMT::convert(const EventContext& ctx) const
{
  // method for unseeded mode. just decode everything.

  STGC_RawDataContainer* rdoContainer{nullptr};
  ATH_CHECK(initRdoContainer(ctx, rdoContainer));
  if(m_skipDecoding) return StatusCode::SUCCESS;

  std::vector<const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment*> vecRobf;
  m_robDataProvider->getROBData(m_allRobIds, vecRobf);
  
  // dummy hashID vector for the decoder (empty = unseeded mode)
  const std::vector<IdentifierHash> rdoIdhVect;

  return convertIntoContainer(ctx, vecRobf, rdoIdhVect, *rdoContainer);
}

StatusCode Muon::STGC_RawDataProviderToolMT::convert(const std::vector<uint32_t>& robIds, const EventContext& ctx) const
{
  STGC_RawDataContainer* rdoContainer{nullptr};
  ATH_CHECK(initRdoContainer(ctx, rdoContainer));
  
  if (robIds.empty() || m_skipDecoding) return StatusCode::SUCCESS;
  
  std::vector<const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment*> vecRobf;

  m_robDataProvider->getROBData(robIds, vecRobf);

  // pass empty list of ID hashes, every ROB ID in list will be decoded
  const std::vector<IdentifierHash> hashIDList; 

  return convertIntoContainer(ctx, vecRobf, hashIDList, *rdoContainer);

}

//===============================================================================
StatusCode  Muon::STGC_RawDataProviderToolMT::convert() const
{
  ATH_MSG_ERROR("STGC_RawDataProviderToolMT::convert() Not implemented.");
  return StatusCode::FAILURE;
}

StatusCode Muon::STGC_RawDataProviderToolMT::convert(const ROBFragmentList& ) const
{    
  ATH_MSG_ERROR("STGC_RawDataProviderToolMT::convert(const ROBFragmentList& vecRobs) Not implemented.");
  return StatusCode::FAILURE;
}

StatusCode  Muon::STGC_RawDataProviderToolMT::convert(const std::vector<IdentifierHash>& ) const
{
  ATH_MSG_ERROR("STGC_RawDataProviderToolMT::convert(const std::vector<IdentifierHash>& rdoIdhVect) Not implemented.");
  return StatusCode::FAILURE;
}

StatusCode  Muon::STGC_RawDataProviderToolMT::convert(const ROBFragmentList&, const std::vector<IdentifierHash>&) const
{
  ATH_MSG_ERROR("STGC_RawDataProviderToolMT::convert(const ROBFragmentList& vecRobs, const std::vector<IdentifierHash>&) Not implemented.");
  return StatusCode::FAILURE;
}
