/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonHoughPatternEvent/MuonHoughHitContainer.h"


void MuonHoughHitContainer::addHit(const std::shared_ptr<MuonHoughHit>& hit) {
    if (hit->getId() == -1) { hit->setId(size()); }
    m_hit.push_back(hit);
}

void MuonHoughHitContainer::removeHit(unsigned int hitno) {
    if (hitno >= m_hit.size()) { throw std::runtime_error("MuonHoughHitContainer::range error!"); }
    
    m_hit.erase(m_hit.begin() + hitno);
}

int MuonHoughHitContainer::getRPChitno() const {
    int rpchitno = 0;
    for (unsigned int i = 0; i < m_hit.size(); i++) { rpchitno += getDetectorId(i) == MuonHough::RPC; }
    return rpchitno;
}

int MuonHoughHitContainer::getMDThitno() const {
    int mdthitno = 0;
    for (unsigned int i = 0; i < m_hit.size(); i++) { mdthitno += getDetectorId(i) == MuonHough::MDT; }
    return mdthitno;
}

int MuonHoughHitContainer::getRPCetahitno() const {
    int rpchitno = 0;
    for (unsigned int i = 0; i < m_hit.size(); i++) { rpchitno += getDetectorId(i) == MuonHough::RPC && !getMeasuresPhi(i); }
    return rpchitno;
}

int MuonHoughHitContainer::getCSChitno() const {
    int cschitno = 0;
    for (unsigned int i = 0; i < m_hit.size(); i++) { cschitno += getDetectorId(i) == MuonHough::CSC; }
    return cschitno;
}

int MuonHoughHitContainer::getTGChitno() const {
    int tgchitno = 0;
    for (unsigned int i = 0; i < m_hit.size(); i++) { tgchitno += getDetectorId(i) == MuonHough::TGC; }
    return tgchitno;
}
