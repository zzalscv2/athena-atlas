/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONREVENTATHENAPOOL_MDTAMTHIT_P1_H
#define MUONREVENTATHENAPOOL_MDTAMTHIT_P1_H

#include <inttypes.h>

#include<vector>

// MDT RDO's : data from a single channel of an AMT 
// Atlas Muon TDC
// Stefano Rosati, Feb 2003

class MdtAmtHit_p1
{
public:
    friend class  MdtAmtHitCnv_p1;
    MdtAmtHit_p1() = default;
 private:
  
  /** TDC Id in the CSM (TDC online Id)*/
  uint16_t m_tdcId{0};
  /** Channel number */
  uint16_t m_channelId{0};

  // Leading edge boolean flag
  bool m_leading{false};
  
  // Decoded time of the first leading edge
  uint16_t m_coarse{0};
  uint16_t m_fine{0};
  // Decoded width in case of a combined measurement
  uint16_t m_width{0};

  // Masked channel flag
  bool m_isMasked{false};
  
  // All the datawords (no headers and footers) coming from this channel 
  // to be decoded on demand using MdtReadOut methods
  std::vector<uint32_t> m_dataWords{};
};

#endif // MUONREVENTATHENAPOOL_MDTAMTHIT_P1_H


