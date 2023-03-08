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

Muon::nsw::MMTrigPacket::MMTrigPacket (uint32_t bs){
  uint pp = 0;

  m_trig_BCID =     bit_slice<uint64_t,uint32_t>(&bs, pp, pp+Muon::nsw::MMTRIG::size_trig_BCID-1);       pp+= Muon::nsw::MMTRIG::size_trig_BCID;
  m_trig_reserved = bit_slice<uint64_t,uint32_t>(&bs, pp, pp+Muon::nsw::MMTRIG::size_trig_reserved-1);   pp+= Muon::nsw::MMTRIG::size_trig_reserved;
  m_trig_dTheta =   bit_slice<uint64_t,uint32_t>(&bs, pp, pp+Muon::nsw::MMTRIG::size_trig_dTheta-1);     pp+= Muon::nsw::MMTRIG::size_trig_dTheta;
  m_trig_phiBin =   bit_slice<uint64_t,uint32_t>(&bs, pp, pp+Muon::nsw::MMTRIG::size_trig_phiBin-1);     pp+= Muon::nsw::MMTRIG::size_trig_phiBin;
  m_trig_rBin =     bit_slice<uint64_t,uint32_t>(&bs, pp, pp+Muon::nsw::MMTRIG::size_trig_rBin-1);       pp+= Muon::nsw::MMTRIG::size_trig_rBin;

}
