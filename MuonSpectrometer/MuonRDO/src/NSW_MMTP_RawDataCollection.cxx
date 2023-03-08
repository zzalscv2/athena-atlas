/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonRDO/NSW_MMTP_RawDataCollection.h"

Muon::NSW_MMTP_RawDataCollection::NSW_MMTP_RawDataCollection(uint32_t sourceID, uint32_t L1ID, uint16_t BCID, uint16_t L1Arequest, uint16_t L1Aopen, uint16_t L1Aclose):
  m_sourceID(sourceID),
  m_L1ID(L1ID),
  m_BCID(BCID),
  m_L1Arequest(L1Arequest),
  m_L1Aopen(L1Aopen),
  m_L1Aclose(L1Aclose)
{ }

