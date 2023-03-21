/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonNSWCommonDecode/NSWOfflineHelper.h"
#include "MuonNSWCommonDecode/NSWResourceId.h"
#include "MuonNSWCommonDecode/MapperSTG.h"
#include "MuonNSWCommonDecode/MapperMMG.h"

//=====================================================================
uint8_t Muon::nsw::helper::NSWOfflineHelper::channel_type ()
{
  uint8_t det_id = m_elinkId->detId ();
  uint8_t chan_type = Muon::nsw::OFFLINE_CHANNEL_TYPE_STRIP;

  if (det_id == eformat::MUON_STGC_ENDCAP_A_SIDE || det_id == eformat::MUON_STGC_ENDCAP_C_SIDE)
    if (m_elinkId->resourceType () == Muon::nsw::NSW_RESOURCE_PAD)
      chan_type = m_vmm == 0 ? Muon::nsw::OFFLINE_CHANNEL_TYPE_WIRE : Muon::nsw::OFFLINE_CHANNEL_TYPE_PAD;

  return chan_type;
}


//=====================================================================
uint16_t Muon::nsw::helper::NSWOfflineHelper::channel_number ()
{
  uint8_t  det_id = m_elinkId->detId ();
  uint8_t  radius = m_elinkId->radius();

  if (det_id == eformat::MUON_MMEGA_ENDCAP_A_SIDE || det_id == eformat::MUON_MMEGA_ENDCAP_C_SIDE) {
    static const Muon::nsw::MapperMMG m;
    return m.channel_number (radius, m_vmm, m_chan);

  } else if (det_id == eformat::MUON_STGC_ENDCAP_A_SIDE || det_id == eformat::MUON_STGC_ENDCAP_C_SIDE) {
  
    static const Muon::nsw::MapperSTG m;
    uint8_t is_large = m_elinkId->is_large_station () ? 1 : 0;
    uint8_t layer    = m_elinkId->layer (); // in [0, 7]
    return m.channel_number (this->channel_type(), is_large, radius, layer, m_vmm, m_chan);
  }

  return 0;
}


//=====================================================================
Muon::nsw::helper::NSWOfflineRobId::NSWOfflineRobId (const std::string &station_name, int8_t station_eta, uint8_t station_phi)
{
  bool is_large = station_name.substr (2, 1) == "L";
  std::string detectorString = station_name.substr(0,2);
  const std::pair <std::string, bool> name_and_side = {detectorString, station_eta > 0};

  uint8_t detId  = Muon::nsw::helper::s_station_to_detector_map.at (name_and_side);
  uint8_t sector = (station_phi - 1) * 2 + (is_large ? 0 : 1);

  // for now lets build all possible ROB ids for all readout configurations
  // reference: slide 6 of https://indico.cern.ch/event/1260377/contributions/5294286/attachments/2603399/4495810/NSW-SwRod-Felix-v3.pdf
  for(uint8_t splitConfig = 0; splitConfig<4; splitConfig++){
    if( detectorString=="MM" && splitConfig == 3) continue; // MM does not have spare devices
    uint32_t sourceId = (detId << 16) | (splitConfig << 8)  | sector;
    m_sourceIds.push_back(sourceId);
  }

}
