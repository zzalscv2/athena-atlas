/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONRDO_NSW_MMTP_RAWDATAHIT_H
#define MUONRDO_NSW_MMTP_RAWDATAHIT_H

#include <inttypes.h>
#include <vector>
#include <iosfwd>

namespace Muon {
  class NSW_MMTP_RawDataHit {
    friend class NSW_MMTP_RawDataHitCnv_p1;
  public:
    NSW_MMTP_RawDataHit (uint16_t art_BCID, uint8_t art_layer, uint16_t art_channel);
    virtual ~NSW_MMTP_RawDataHit() = default ; 

    uint16_t art_BCID    () const {return m_art_BCID;};
    uint8_t  art_layer   () const {return m_art_layer;};
    uint16_t art_channel () const {return m_art_channel;};

  private:
    uint16_t m_art_BCID{0};
    uint8_t  m_art_layer{0};
    uint16_t m_art_channel{0};
  };
}

#endif
