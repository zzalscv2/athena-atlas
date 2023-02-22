/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonRDO/NSW_MMTP_RawDataSegment.h"

Muon::NSW_MMTP_RawDataSegment::NSW_MMTP_RawDataSegment (uint16_t trig_BCID, uint8_t trig_dTheta, uint8_t trig_rBin, uint8_t trig_phiBin):
  m_trig_BCID(trig_BCID),
  m_trig_dTheta(trig_dTheta),
  m_trig_rBin(trig_rBin),
  m_trig_phiBin(trig_phiBin & 0b11111),
  m_trig_phiSign((trig_phiBin >> 5) & 0b1)
{ }
