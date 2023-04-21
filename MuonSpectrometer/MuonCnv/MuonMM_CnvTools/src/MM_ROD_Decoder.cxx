/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include <memory>
#include <unordered_map>

#include "MuonRDO/MM_RawDataContainer.h"
#include "MuonIdHelpers/MmIdHelper.h"
#include "StoreGate/ReadCondHandle.h"

#include "MuonNSWCommonDecode/NSWCommonDecoder.h"
#include "MuonNSWCommonDecode/NSWElink.h"
#include "MuonNSWCommonDecode/VMMChannel.h"
#include "MuonNSWCommonDecode/NSWResourceId.h"

#include "Identifier/IdentifierHash.h"
#include "eformat/Issue.h"
#include "MM_ROD_Decoder.h"

//===============================================================================
Muon::MM_ROD_Decoder::MM_ROD_Decoder( const std::string& type, const std::string& name,const IInterface* parent )
: AthAlgTool(type, name, parent)
{
  declareInterface<IMM_ROD_Decoder>( this );
}


//===============================================================================
StatusCode Muon::MM_ROD_Decoder::initialize() 
{
  ATH_CHECK(detStore()->retrieve(m_MmIdHelper, "MMIDHELPER"));
  ATH_CHECK(m_mmCablingMap.initialize());
  return StatusCode::SUCCESS;
}


//===============================================================================
// Processes a ROB fragment and fills the RDO container. 
// If the vector of IdentifierHashes is not empty, then seeded mode is assumed 
// (only requested modules are decoded). This must be made here, because the 
// trigger granularity is a module, whereas ROB granularity is a whole sector. 
// Therefore, refined selection is needed with decoded information.
StatusCode Muon::MM_ROD_Decoder::fillCollection(const EventContext& ctx, const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment& robFrag, const std::vector<IdentifierHash>& rdoIdhVect, std::unordered_map<IdentifierHash, std::unique_ptr<MM_RawDataCollection>> &rdo_map) const
{
  // check fragment for errors
  try {
    robFrag.check();
  } catch (eformat::Issue &ex) {
    ATH_MSG_WARNING(ex.what());
    return StatusCode::SUCCESS;
  }

  SG::ReadCondHandle<MicroMega_CablingMap>  mmCablingMap{m_mmCablingMap, ctx};
  if(! mmCablingMap.isValid()){
    ATH_MSG_ERROR("Cannot find Micromegas cabling map!");
    return StatusCode::FAILURE;
  }

  // if the vector of hashes is not empty, then we are in seeded mode
  bool seeded_mode(!rdoIdhVect.empty());

  // have the NSWCommonDecoder take care of the decoding
  Muon::nsw::NSWCommonDecoder common_decoder(robFrag);  
  const std::vector<Muon::nsw::NSWElink *>&   elinks = common_decoder.get_elinks();  
  ATH_MSG_DEBUG("Retrieved "<<elinks.size()<<" elinks");
  if (!elinks.size()) return StatusCode::SUCCESS;


  // loop on elinks. we need an RDO (collection) per quadruplet!
  for (auto* elink : elinks) {

    // skip null packets
    if (elink->isNull()) continue;

    // get the offline ID hash (module ctx) to be passed to the RDO 
    // also specifies the index of the RDO in the container.
    const char*  station_name = elink->elinkId()->is_large_station() ? "MML" : "MMS";
    int          station_eta  = (int)elink->elinkId()->station_eta();
    unsigned int station_phi  = (unsigned int)elink->elinkId()->station_phi();
    unsigned int multi_layer  = (unsigned int)elink->elinkId()->multi_layer();
    unsigned int gas_gap      = (unsigned int)elink->elinkId()->gas_gap();
    Identifier   module_ID    = m_MmIdHelper->elementID(station_name, station_eta, station_phi);

    IdentifierHash module_hashID;
    m_MmIdHelper->get_module_hash(module_ID, module_hashID);

    // if we are in ROI-seeded mode, check if this hashID is requested
    if (seeded_mode && std::find(rdoIdhVect.begin(), rdoIdhVect.end(), module_hashID) == rdoIdhVect.end()) continue;

    if (!rdo_map[module_hashID]) rdo_map[module_hashID] = std::make_unique<MM_RawDataCollection>(module_hashID);
    MM_RawDataCollection* rdo = rdo_map[module_hashID].get();

    // loop on all channels of this elink to fill the collection
    const std::vector<Muon::nsw::VMMChannel *>& channels = elink->get_channels();
    for (auto channel : channels) {
       unsigned int channel_number = channel->channel_number();
       if (channel_number == 0) continue; // skip disconnected vmm channels
       Identifier channel_ID = m_MmIdHelper->channelID(module_ID, multi_layer, gas_gap, channel_number);  // not validating the IDs (too slow)
       
       // now we have to check if for this channel there is a correction of the cabling needed
       std::optional<Identifier> correctedChannelId = mmCablingMap.cptr()->correctChannel(channel_ID, msgStream());
       if(correctedChannelId){
          // for data the time and charge are in counts
          bool timeAndChargeInCounts = true;
          uint correctedChannelNumber = m_MmIdHelper->channel(*correctedChannelId);
          rdo->push_back(new MM_RawData(*correctedChannelId, correctedChannelNumber, channel->tdo(), channel->pdo(), channel->rel_bcid(),timeAndChargeInCounts)); // isDead = false (ok?)
       } else {
          ATH_MSG_DEBUG("Channel was shifted outside its connector and is therefore not decoded into and RDO");
       }
    }
  }

  return StatusCode::SUCCESS;
}

