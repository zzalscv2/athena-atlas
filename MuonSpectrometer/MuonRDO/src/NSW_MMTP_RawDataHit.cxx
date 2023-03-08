/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonRDO/NSW_MMTP_RawDataHit.h"

Muon::NSW_MMTP_RawDataHit::NSW_MMTP_RawDataHit(uint16_t art_BCID, uint8_t art_layer, uint16_t art_channel):
  m_art_BCID(art_BCID),
  m_art_layer(art_layer),
  m_art_channel(art_channel)
{ }
