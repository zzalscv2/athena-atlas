/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONRDO_NSW_MMTP_RAWDATASEGMENT_H
#define MUONRDO_NSW_MMTP_RAWDATASEGMENT_H

#include <inttypes.h>
#include <vector>
#include <iosfwd>

namespace Muon {
  class NSW_MMTP_RawDataSegment {
    friend class NSW_MMTP_RawDataSegmentCnv_p1;
  public:
    NSW_MMTP_RawDataSegment (uint16_t trig_BCID, uint8_t trig_dTheta, uint8_t trig_rBin, uint8_t trig_phiBin);
    virtual ~NSW_MMTP_RawDataSegment() = default;

    uint16_t trig_BCID    () const {return m_trig_BCID;};
    uint8_t  trig_dTheta  () const {return m_trig_dTheta;};
    uint8_t  trig_rBin    () const {return m_trig_rBin;};
    uint8_t  trig_phiBin  () const {return m_trig_phiBin;};
    bool     trig_phiSign () const {return m_trig_phiSign;};

  private:
    uint16_t m_trig_BCID{0};
    uint8_t  m_trig_dTheta{0};
    uint8_t  m_trig_rBin{0};
    uint8_t  m_trig_phiBin{0}; 
    bool     m_trig_phiSign{0};  // trig_phiBin from constructor is from decoder: 6 bits => 1 bit is sign, 5 bits for magnitude 

  };
}

#endif
