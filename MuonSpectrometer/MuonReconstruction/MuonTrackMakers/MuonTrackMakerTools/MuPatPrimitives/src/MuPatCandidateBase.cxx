/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuPatPrimitives/MuPatCandidateBase.h"
#include <set>

namespace Muon {

    MuPatCandidateBase::MuPatCandidateBase(const MeasVec& etaHits, const MeasVec& phiHits, const MeasVec& fakePhiHits,
                                           const MeasVec& allHits) :
        m_etaHits(etaHits), m_phiHits(phiHits), m_fakePhiHits(fakePhiHits), m_allHits(allHits) {}

    void MuPatCandidateBase::setChambers(const std::set<MuonStationIndex::ChIndex>& chambers) {
        m_chambers = chambers;
        m_stations.clear();
        for (const MuonStationIndex::ChIndex it : chambers) m_stations.insert(MuonStationIndex::toStationIndex(it));
    }

    bool MuPatCandidateBase::shareChambers(const MuPatCandidateBase& entry) const {
        std::vector<Identifier> intersection;
        std::set_intersection(entry.chamberIds().begin(), entry.chamberIds().end(), chamberIds().begin(), chamberIds().end(),
                              std::back_inserter(intersection));

        unsigned int intersectionSize = intersection.size();
        return intersectionSize != 0;
    }

    const std::set<MuonStationIndex::StIndex>& MuPatCandidateBase::stations() const { return m_stations; }

    const std::set<MuonStationIndex::ChIndex>& MuPatCandidateBase::chambers() const { return m_chambers; }

    const std::set<Identifier>& MuPatCandidateBase::chamberIds() const { return m_chamberIds; }

    /** @brief returns set with contained chamber ids */
    std::set<Identifier>& MuPatCandidateBase::chamberIds() { return m_chamberIds; }

    void MuPatCandidateBase::addChamber(MuonStationIndex::ChIndex chIndex) {
        m_chambers.insert(chIndex);
        m_stations.insert(MuonStationIndex::toStationIndex(chIndex));
    }

    bool MuPatCandidateBase::hasSmallChamber() const { return m_hasSmallChamber; }

    bool MuPatCandidateBase::hasLargeChamber() const { return m_hasLargeChamber; }

    bool MuPatCandidateBase::hasSLOverlap() const { return m_hasSLOverlap; }

    void MuPatCandidateBase::hasSmallChamber(bool hasSmall) { m_hasSmallChamber = hasSmall; }

    void MuPatCandidateBase::hasLargeChamber(bool hasLarge) { m_hasLargeChamber = hasLarge; }

    void MuPatCandidateBase::hasSLOverlap(bool hasSL) { m_hasSLOverlap = hasSL; }

    const MuPatCandidateBase::MeasVec& MuPatCandidateBase::MuPatCandidateBase::etaHits() const { return m_etaHits; }

    const MuPatCandidateBase::MeasVec& MuPatCandidateBase::phiHits() const { return m_phiHits; }

    const MuPatCandidateBase::MeasVec& MuPatCandidateBase::fakePhiHits() const { return m_fakePhiHits; }

    const MuPatCandidateBase::MeasVec& MuPatCandidateBase::hits() const { return m_allHits; }

    void MuPatCandidateBase::setEtaHits(const MuPatCandidateBase::MeasVec& hits) { m_etaHits = hits; }

    void MuPatCandidateBase::setPhiHits(const MuPatCandidateBase::MeasVec& hits) { m_phiHits = hits; }

    void MuPatCandidateBase::setFakePhiHits(const MuPatCandidateBase::MeasVec& hits) { m_fakePhiHits = hits; }

    void MuPatCandidateBase::setAllHits(const MuPatCandidateBase::MeasVec& hits) { m_allHits = hits; }

    bool MuPatCandidateBase::hasEndcap() const { return m_hasEndcap; }

    void MuPatCandidateBase::hasEndcap(bool hasEC) { m_hasEndcap = hasEC; }

    bool MuPatCandidateBase::containsChamber(MuonStationIndex::ChIndex chIndex) const {
        return m_chambers.find(chIndex) != m_chambers.end();
    }

    bool MuPatCandidateBase::containsStation(MuonStationIndex::StIndex stIndex) const {
        return m_stations.find(stIndex) != m_stations.end();
    }

    void MuPatCandidateBase::clearChambers() {
        m_chambers.clear();
        m_stations.clear();
        m_chamberIds.clear();
    }

    bool MuPatCandidateBase::hasMomentum() const { return m_hasMomentum; }

    void MuPatCandidateBase::addToTrash(std::unique_ptr<const Trk::MeasurementBase> meas) {m_garbage.push_back(std::move(meas));}
    void MuPatCandidateBase::addToTrash(const std::vector<std::shared_ptr<const Trk::MeasurementBase>>& measurements) {
        if (m_garbage.capacity() < measurements.size() + m_garbage.size()){
            m_garbage.reserve(measurements.size() + m_garbage.size());
        }
        m_garbage.insert(m_garbage.end(), measurements.begin(), measurements.end());
    }
    const std::vector<std::shared_ptr<const Trk::MeasurementBase>>& MuPatCandidateBase::garbage()const {return m_garbage;}

}  // namespace Muon
