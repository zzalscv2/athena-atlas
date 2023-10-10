/*                                                                             
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration                              
*/

///////////////////////////////////////////////////////////////////////////                          
// Utils for the main sTGCRawDataMonAlg.cxx                                                            
// Part of StgcRawDataMonAlg.h                                                                         
// see StgcRawDataMonAlg.cxx                                
///////////////////////////////////////////////////////////////////////////                      
         
#include "StgcRawDataMonitoring/StgcRawDataMonAlg.h"

int sTgcRawDataMonAlg::getSectors(const Identifier& id) const { 
  return m_idHelperSvc -> sector(id)*(m_idHelperSvc -> stationEta(id) > 0 ? 1. : -1.);
}

int sTgcRawDataMonAlg::getLayer(int multiplet, int gasGap) const {
  return 4*(multiplet -1 ) + gasGap;
}

int32_t sTgcRawDataMonAlg::sourceidToSector(uint32_t sourceid, bool isSideA) const {
  uint32_t sectorNumber = sourceid & 0xf;
  return (isSideA) ? sectorNumber + 1: -sectorNumber - 1;
}

int sTgcRawDataMonAlg::getSignedPhiId(const uint32_t phiid) const {
  // 1 bit of sign (0 = positive) followed by 5 bits of phiid
  constexpr size_t nbitsPhi{5};
  constexpr size_t mask{(1 << nbitsPhi) - 1};
  return std::pow(-1, phiid >> nbitsPhi) * (phiid & mask);
}

std::optional<Identifier> sTgcRawDataMonAlg::getPadId(uint32_t sourceid, uint32_t pfeb, uint32_t tdschan) const {
  bool isValid = false;
  const int side = (decoder::isA(sourceid)) ? 1 : -1;
  const auto vmm = tdschan / NVMMCHAN + FIRSTPFEBVMM;
  const auto vmmchan = tdschan % NVMMCHAN;
  const auto sec = decoder::sector(sourceid);
  const auto& help = m_idHelperSvc -> stgcIdHelper();
  const auto pad_id = help.channelID(help.elementID(decoder::offlineStationName(sec),
						    decoder::offlineStationAbsEta(pfeb) * side,
						    decoder::offlineStationPhi(sourceid)),
				     decoder::offlineMultilayer(pfeb), 
				     decoder::offlineGasgap(pfeb),
				     Muon::nsw::OFFLINE_CHANNEL_TYPE_PAD,
				     decoder::offlineChannelNumber(sec, pfeb, vmm, vmmchan), isValid);
  
  if (!isValid) {
    ATH_MSG_WARNING("Pad Identifier not valid, skipping");
    return std::nullopt;
  }
  
  return std::make_optional(pad_id);
}

std::optional<std::tuple<Identifier, const Trk::RIO_OnTrack*>> sTgcRawDataMonAlg::getRotIdAndRotObject(const Trk::TrackStateOnSurface* trkState) const {
  if (!trkState->type(Trk::TrackStateOnSurface::Measurement)) return std::nullopt;

  Identifier surfaceId = (trkState) -> surface().associatedDetectorElementIdentifier();
  if(!m_idHelperSvc -> issTgc(surfaceId)) return std::nullopt;
  
  const Trk::MeasurementBase* meas = trkState->measurementOnTrack();
  if(!meas) return std::nullopt;
  
  const Trk::RIO_OnTrack* rot = dynamic_cast<const Trk::RIO_OnTrack*>(meas);
  if(!rot) return std::nullopt;
  
  Identifier rot_id = rot -> identify();

  if(!rot_id.is_valid()) {
    ATH_MSG_WARNING("Invalid identifier found in Trk::RIO_OnTrack");
    return std::nullopt;
  }
  
  return std::make_tuple(rot_id, rot);
}
    
std::optional<Identifier> sTgcRawDataMonAlg::getRotId(const Trk::TrackStateOnSurface* trkState) const {
  std::optional<std::tuple<Identifier, const Trk::RIO_OnTrack*>> status = getRotIdAndRotObject(trkState);
  if (!status.has_value()) return std::nullopt;
  std::tuple<Identifier, const Trk::RIO_OnTrack*> rotIDtuple = status.value();
  
  return std::make_optional(std::get<Identifier>(rotIDtuple));
}
  
