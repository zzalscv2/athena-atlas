/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONRDO_NSW_MMTP_RAWDATACOLLECTION_H
#define MUONRDO_NSW_MMTP_RAWDATACOLLECTION_H

#include "MuonRDO/NSW_MMTP_RawDataHit.h"
#include "MuonRDO/NSW_MMTP_RawDataSegment.h"
#include "AthContainers/DataVector.h"
#include "AthenaKernel/CLASS_DEF.h"
#include "Identifier/IdentifierHash.h"

namespace Muon
{
  class NSW_MMTP_RawDataCollection
  {
    friend class NSW_MMTP_RawDataContainerCnv_p1;
  public:
    //NSW_MMTP_RawDataCollection(IdentifierHash hash) : m_idHash(hash) {}
    NSW_MMTP_RawDataCollection(uint32_t sourceID, uint32_t L1ID, uint16_t BCID, uint16_t L1Arequest, uint16_t L1Aopen, uint16_t L1Aclose); 
    //const IdentifierHash& identifierHash() const { return m_idHash; }

    uint32_t sourceID () const {return m_sourceID;};
    uint32_t L1ID     () const {return m_L1ID;};
    uint16_t BCID     () const {return m_BCID;};
    uint16_t L1Arequest () const {return m_L1Arequest;};
    uint16_t L1Aopen    () const {return m_L1Aopen;};
    uint16_t L1Aclose   () const {return m_L1Aclose;};

    const DataVector<NSW_MMTP_RawDataHit>& hits () const { return m_hits; }
    const DataVector<NSW_MMTP_RawDataSegment>& segments () const { return m_segments; }

    void addHit     (NSW_MMTP_RawDataHit* hit)     {m_hits.push_back(hit);}
    void addSegment (NSW_MMTP_RawDataSegment* seg) {m_segments.push_back(seg);}

  private:
    //IdentifierHash m_idHash;
    uint32_t m_sourceID{0};
    uint32_t m_L1ID{0};
    uint16_t m_BCID{0};
    uint16_t m_L1Arequest{0};
    uint16_t m_L1Aopen{0};
    uint16_t m_L1Aclose{0};

    DataVector<NSW_MMTP_RawDataHit> m_hits {};
    DataVector<NSW_MMTP_RawDataSegment> m_segments {};

  };
}

#endif
