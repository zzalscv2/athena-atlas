/*
 * Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
 */

#ifndef NSW_TRIGRAWDATASEGMENT_P1_H
#define NSW_TRIGRAWDATASEGMENT_P1_H

#include <utility>
#include <vector>

namespace Muon {
  class NSW_TrigRawDataSegment_p1 {
    public:
      NSW_TrigRawDataSegment_p1() = default;

      void addChannel(uint8_t layer, uint16_t channel) { m_channels.emplace_back( std::pair<uint8_t,uint16_t>(layer,channel) ); }

      uint8_t m_deltaTheta{0};
      uint8_t m_phiIndex{0};
      uint8_t m_rIndex{0};
      uint8_t m_spare{0};
      bool m_lowRes{false};
      bool m_phiRes{false};
      bool m_monitor{false};
      std::vector< std::pair<uint8_t,uint16_t> > m_channels{};
  };
}

#endif
