/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuPatPrimitives/MuPatTrack.h"

#include <algorithm>

#include "TrkParameters/TrackParameters.h"
#include "TrkTrack/Track.h"

namespace Muon {

    // Static functions

    unsigned int MuPatTrack::processingStageStringMaxLen() {
        static const unsigned int maxlen = []() -> unsigned int {
          const std::vector<std::string>& pss = processingStageStrings();
          auto it = std::max_element(pss.begin(), pss.end(),
                                     [](const std::string& lhs, const std::string& rhs) { return lhs.size() < rhs.size(); });

          if (it != pss.end())
            return it->size();
          return 0u;
        }();

        return maxlen;
    }

    std::vector<std::string> MuPatTrack::initProcessingStageStrings() {
        std::vector<std::string> pss;
        pss.resize(NumberOfProcessingStages + 1);
        pss[Unknown] = "Unknown";
        pss[InitialLoop] = "InitialLoop";
        pss[LayerRecovery] = "LayerRecov";
        pss[ExtendedWithSegment] = "ExtWSegment";
        pss[SegmentRecovery] = "SegmentRecov";
        pss[FitRemovedSegment] = "FitRmSegment";
        pss[RefitRemovedSegment] = "RefitRmSegment";
        pss[AmbiguityCreateCandidateFromSeeds] = "AmbiCreate";
        pss[AmbiguitySelectCandidates] = "AmbiSelect";
        pss[MatchFail] = "MatchFail";
        pss[FitFail] = "FitFail";
        pss[FitWorse] = "FitWorse";
        pss[UnassociatedEM] = "UnassocEM";
        pss[FitRemovedLayer] = "FitRmLayer";
        pss[TrackSelector] = "TrackSelect";
        pss[KeptUntilEndOfCombi] = "--KEPT--";
        pss[NumberOfProcessingStages] = "OutOfBounds";

        return pss;
    }

    const std::vector<std::string>& MuPatTrack::processingStageStrings()
    {
      static const std::vector<std::string> processingStrings =
        initProcessingStageStrings();
      return processingStrings;
    }

    const std::string& MuPatTrack::processingStageString(MuPatTrack::ProcessingStage stage) {
        const std::vector<std::string>& pss = processingStageStrings();

        if (static_cast<size_t>(stage) < pss.size()) {
            return pss[static_cast<size_t>(stage)];
        } else {
            return pss[NumberOfProcessingStages];
        }
    }

    // member functions
    MuPatTrack::MuPatTrack(const std::vector<MuPatSegment*>& segments, std::unique_ptr<Trk::Track>& track, MuPatSegment* seedSeg) :
        MuPatCandidateBase(), m_segments(segments) {

        m_track.swap(track);
        // increase segment counters
        modifySegmentCounters(+1);
        m_hasMomentum = hasMomentum();

        // now update links between tracks and segments
        updateSegments(true);
       
        if (seedSeg){
            m_seedSeg = seedSeg;
            addToTrash(seedSeg->garbage());
        } else if (!segments.empty())
            m_seedSeg = segments[0];
        for (MuPatSegment* seg : segments){
            addToTrash(seg->garbage());
        }
    }

    MuPatTrack::MuPatTrack(MuPatSegment* segment, std::unique_ptr<Trk::Track>& track) :
        MuPatCandidateBase(){
        m_track.swap(track);

        m_segments.reserve(3);
        m_segments.push_back(segment);
        addToTrash(segment->garbage());
        // increase segment counters
        modifySegmentCounters(+1);
        m_hasMomentum = hasMomentum();

        // now update links between tracks and segments
        updateSegments(true);
        
    }

    MuPatTrack::MuPatTrack(MuPatSegment* segment1, MuPatSegment* segment2, std::unique_ptr<Trk::Track>& track, MuPatSegment* seedSeg) :
        MuPatCandidateBase() {
        m_track.swap(track);

        m_segments.reserve(3);
        m_segments.push_back(segment1);
        m_segments.push_back(segment2);
        // increase segment counters
        modifySegmentCounters(+1);
        m_hasMomentum = hasMomentum();

        // now update links between tracks and segments
        updateSegments(true);
        
        m_excludedSegments.clear();
        if (seedSeg)
            m_seedSeg = seedSeg;
        else
            m_seedSeg = segment1 ? segment1 : segment2;
        for (MuPatSegment* seg : {segment1, segment2, seedSeg}) {
            if (seg) addToTrash(seg->garbage());
        }
    }

    MuPatTrack::~MuPatTrack() {
        // now update links between tracks and segments
        updateSegments(false);
    }

    MuPatTrack::MuPatTrack(const MuPatTrack& can) :
        MuPatCandidateBase(can),
        Trk::ObjectCounter<MuPatTrack>(can),
        created(can.created),
        lastSegmentChange(can.lastSegmentChange),
        m_segments(can.m_segments),
        m_excludedSegments(can.m_excludedSegments),
        m_track (std::make_unique<Trk::Track>(can.track())),
        m_seedSeg(can.m_seedSeg) {
        m_hasMomentum = can.m_hasMomentum;
        // increase segment counters
        modifySegmentCounters(+1);

        // now update links between tracks and segments
        updateSegments(true);
    }

    MuPatTrack& MuPatTrack::operator=(const MuPatTrack& can) {
        if (&can != this) {

            // now update links between tracks and segments, remove old links
            updateSegments(false);

            // decrease old segment counters
            modifySegmentCounters(-1);

            // copy members
            MuPatCandidateBase::operator=(can);
            created = can.created;
            lastSegmentChange = can.lastSegmentChange;
            m_segments = can.m_segments;
            m_excludedSegments = can.m_excludedSegments;

            m_track = std::make_unique<Trk::Track>(can.track());
            m_chambers = can.m_chambers;
            m_stations = can.m_stations;
            m_hasMomentum = can.m_hasMomentum;
            m_seedSeg = can.m_seedSeg;

            // increase new segment counters
            modifySegmentCounters(+1);

            // now update links between tracks and segments, add new segments
            updateSegments(true);
        }
        return *this;
    }

    bool MuPatTrack::hasMomentum() const {
        if (!m_track) return false;
        return hasMomentum(*m_track);
    }

    bool MuPatTrack::hasMomentum(const Trk::Track& track) {
        // use track info if set properly
        if (track.info().trackProperties(Trk::TrackInfo::StraightTrack)) return false;

        bool hasMom = false;
        const Trk::Perigee* pp = track.perigeeParameters();
        if (pp) {
            const AmgSymMatrix(5)* cov = pp->covariance();
            if (cov) {
                // sum covariance terms of momentum, use it to determine whether fit was SL fit
                double momCov = 0.;
                for (int i = 0; i < 4; ++i) momCov += std::abs((*cov)(4, i));
                for (int i = 0; i < 4; ++i) momCov += std::abs((*cov)(i, 4));
                if (momCov > 1e-10) { hasMom = true; }
            }
        }
        return hasMom;
    }

    void MuPatTrack::updateTrack(std::unique_ptr<Trk::Track>& track) { m_track.swap(track); }

    void MuPatTrack::addExcludedSegment(MuPatSegment* segment) { m_excludedSegments.push_back(segment); }
    bool MuPatTrack::isSegmentExcluded(const MuPatSegment* segment) const {
        return std::find(m_excludedSegments.begin(), m_excludedSegments.end(), segment) != m_excludedSegments.end();
    }
    void MuPatTrack::addSegment(MuPatSegment* segment, std::unique_ptr<Trk::Track>& newTrack) {
        // add segment and increase counter
        m_segments.push_back(segment);
        addToTrash(segment->garbage());
        segment->addTrack(this);
        ++segment->usedInFit;
        for (const MuonStationIndex::ChIndex& chit : segment->chambers()) addChamber(chit);

        if (newTrack) {
            // delete old track, assign new
            m_track.swap(newTrack);
        }
        m_hasMomentum = hasMomentum();
    }

    void MuPatTrack::modifySegmentCounters(int change) {
        // modify usedInFit counter of segment
        for (MuPatSegment* seg : m_segments) {
            for (const MuonStationIndex::ChIndex& chit : seg->chambers()) addChamber(chit);
            seg->usedInFit += change;
        }
    }

    bool MuPatTrack::resetChambersOnCandidate(const std::set<MuonStationIndex::ChIndex>& chambers) {
        // loop over input chambers, check whether segments have a chamber in the list
        // remove segment if not in the list
        setChambers(chambers);  // also updates station list

        bool bRemovedSegments = false;
        std::vector<MuPatSegment*>::iterator it = m_segments.begin();
        // NOTE: can not cache m_segments.end() because it may change in the loop
        while (it != m_segments.end()) {
            bool inChamberSet = false;
            for (const MuonStationIndex::ChIndex& chit : (*it)->chambers()) {
                if (containsChamber(chit)) {
                    inChamberSet = true;
                    break;
                }
            }
            if (inChamberSet) {
                ++it;
            } else {
                (*it)->removeTrack(this);
                bRemovedSegments = true;
                it = m_segments.erase(it);  // it points to next element
            }
        }

        return bRemovedSegments;
    }

    std::vector<MuonStationIndex::StIndex> MuPatTrack::stationsInOrder() {
        std::vector<MuonStationIndex::StIndex> stations;
        stations.reserve(m_segments.size());
        for (MuPatSegment* seg : m_segments) stations.push_back(seg->stIndex);
        return stations;
    }

    std::string MuPatTrack::segmentNames() const {
        std::string names;
        // rest with spaces
        for (const MuPatSegment* seg : m_segments) {
            names += seg->name;
            names += "  ";
        }
        /// Remove the trailing white space
        return names.substr(0, names.size() - 1);
    }

    void MuPatTrack::updateSegments(bool add) {
        for (MuPatSegment* seg : m_segments) {
            if (add) {
                seg->addTrack(this);
            } else {
                seg->removeTrack(this);
            }
        }
    }

}  // namespace Muon
