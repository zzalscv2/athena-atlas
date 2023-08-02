/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include <vector>
#include <exception>
#include <sstream>
#include <string>
#include <algorithm>
#include <tuple>

#include "MuonNSWCommonDecode/MMTrigPacket.h"
#include "MuonNSWCommonDecode/NSWMMTPDecodeBitmaps.h"

Muon::nsw::MMTrigPacket::MMTrigPacket (std::vector<uint32_t>& payload){

  std::size_t readPointer{0};
  CxxUtils::span<const std::uint32_t> data{payload.data(), 3};

  if (payload.size()!=2) {
    throw std::runtime_error( Muon::nsw::format( "MM Trigger packet size not as expected: expected exactly 2 uint32_t, got {}", payload.size() ));
  }

  m_trig_padding =  Muon::nsw::decode_and_advance<uint64_t>(data, readPointer, Muon::nsw::MMTRIG::size_trig_padding);
  m_trig_BCID =     Muon::nsw::decode_and_advance<uint64_t>(data, readPointer, Muon::nsw::MMTRIG::size_trig_BCID);
  m_trig_reserved = Muon::nsw::decode_and_advance<uint64_t>(data, readPointer, Muon::nsw::MMTRIG::size_trig_reserved);
  m_trig_dTheta =   Muon::nsw::decode_and_advance<uint64_t>(data, readPointer, Muon::nsw::MMTRIG::size_trig_dTheta);
  m_trig_phiBin =   Muon::nsw::decode_and_advance<uint64_t>(data, readPointer, Muon::nsw::MMTRIG::size_trig_phiBin);
  m_trig_rBin =     Muon::nsw::decode_and_advance<uint64_t>(data, readPointer, Muon::nsw::MMTRIG::size_trig_rBin);

}
