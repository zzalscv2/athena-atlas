/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUPATTRACK_H
#define MUPATTRACK_H

#include <mutex>
#include <set>
#include <string>
#include <vector>

#include "MuPatPrimitives/MuPatCandidateBase.h"
#include "MuPatPrimitives/MuPatSegment.h"
#include "MuonStationIndex/MuonStationIndex.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkTrack/Track.h"
#include "TrkEventPrimitives/TrkObjectCounter.h"

namespace Muon {

    /**
        @brief track candidate object.
        The purpose of the track candidate is three folded:
        - provide the generic MuPatCandidateBase interface for tracks
        - keep track of segments used to build track
        - cache additional information that cannot be stored on the track

        The following information is cached:
        - the stage during the pattern recognition the candidate was made (Moore legacy)
        - the pointer to the track
        - a vector of the segments that were used to form the track
        - a vector of the segments that were already tested against the candidate but failed.
          This information can be used to speed up the pat-rec code by avoiding fits that
          were already tried.
     */
    class MuPatTrack : public MuPatCandidateBase, public Trk::ObjectCounter<MuPatTrack> {
        friend class MuPatCandidateTool;

    public:
        /** enum to keep track of the life of candidates */
        enum ProcessingStage {
            Unknown,
            InitialLoop,
            LayerRecovery,
            ExtendedWithSegment,
            SegmentRecovery,
            FitRemovedSegment,
            RefitRemovedSegment,
            AmbiguityCreateCandidateFromSeeds,
            AmbiguitySelectCandidates,
            MatchFail,
            FitFail,
            FitWorse,
            UnassociatedEM,
            FitRemovedLayer,
            TrackSelector,
            KeptUntilEndOfCombi,
            NumberOfProcessingStages
        };

        /** Convert enum to string */
        static const std::string& processingStageString(ProcessingStage stage);

        /** maximum width of the strings corresponding to the ProcessingStage */
        static unsigned int processingStageStringMaxLen();

        /** @brief constructor taking a vector of MuPatSegment object, the candidate takes ownership of the track
                   It will increase the usedInFit counter of the MuPatSegment objects by one. */
        MuPatTrack(const std::vector<MuPatSegment*>& segments, std::unique_ptr<Trk::Track>& track, MuPatSegment* seedSeg = 0);

        /** @brief constructor taking a MuPatSegment object, the candidate takes ownership of the track
                   It will increase the usedInFit counter of the MuPatSegment objects by one. */
        MuPatTrack(MuPatSegment* segment, std::unique_ptr<Trk::Track>& track);

        /** @brief constructor taking two MuPatSegment objects, the candidate takes ownership of the track
             It will increase the usedInFit counter of the MuPatSegment objects by one. */
        MuPatTrack(MuPatSegment* segment1, MuPatSegment* segment2, std::unique_ptr<Trk::Track>& track, MuPatSegment* seedSeg = 0);

        /** @brief destructor, decrease the usedInFit counter of all MuPatSegment objects by one */
        ~MuPatTrack();

        /** copying constructor. It will not copy the track, just its pointer (lazy). It will increase the usedInFit counter of the
         * MuPatSegment objects by one. */
        MuPatTrack(const MuPatTrack& can);

        /** assignment operator. It will not copy the track, just its pointer (lazy). It will increase the usedInFit counter of the
         * MuPatSegment objects by one. */
        MuPatTrack& operator=(const MuPatTrack& can);

        /** @brief access to track */
        Trk::Track& track() const;
        /** @brief update track. Candidate takes ownership of track. */
        void updateTrack(std::unique_ptr<Trk::Track>& newTrack);

        /** @brief access to segments */
        const std::vector<MuPatSegment*>& segments() const;

        /** @brief Return pointer to the seed segment */
        MuPatSegment* seedSegment() const;

        /** @brief access to segments */
        const std::vector<MuPatSegment*>& excludedSegments() const;

        /** @brief add segment + the associated new track. Takes ownership of the track. */
        void addSegment(MuPatSegment* segment, std::unique_ptr<Trk::Track>& newTrack);

        /** @brief add segment that does not match the track */
        void addExcludedSegment(MuPatSegment* segment);
        /** @brief loops over the excluded segment collection and checks whether the pointer is in there */
        bool isSegmentExcluded(const MuPatSegment* segment) const;

        /** @brief returns first track parameters */
        const Trk::TrackParameters& entryPars() const;

        /** @brief returns whether canditate has a momentum measurement */
        bool hasMomentum() const;

        /** @brief returns vector with contained stationIndices in the order they were added */
        std::vector<MuonStationIndex::StIndex> stationsInOrder();

        /** @brief reset chambers on the candidate. Return whether segments were removed. */
        bool resetChambersOnCandidate(const std::set<MuonStationIndex::ChIndex>& chambers);

        /** @brief string containing the names of the segments on the candidate */
        std::string segmentNames() const;

    private:       
        //
        // private member functions
        //
        /** @brief Initialize s_processingStageStrings & s_processingStageStringMaxLen */
        static std::vector<std::string> initProcessingStageStrings();
        /** @brief Return list of processing stage strings. */
        static const std::vector<std::string>& processingStageStrings();

        /** @brief update segment/track association, if add == true ,will add track to segments else remove it */
        void updateSegments(bool add);

    public:
        //
        // public data members
        //
        ProcessingStage created{Unknown};
        ProcessingStage lastSegmentChange{Unknown};

    private:       

        
        /** @brief increase the segment counters by the passed number */
        void modifySegmentCounters(int change);

        /** @brief check whether track measures momentum */
        static bool hasMomentum(const Trk::Track& track) ;

        std::vector<MuPatSegment*> m_segments{};          //<! list of associated segments
        std::vector<MuPatSegment*> m_excludedSegments{};  //<! list of associated segments
        std::unique_ptr<Trk::Track> m_track{};            //<! associated track
        MuPatSegment* m_seedSeg{nullptr};                        //!< The special segment for this track

    };  // class MuPatTrack


    //
    // inline member functions implementations
    //
    inline const std::vector<MuPatSegment*>& MuPatTrack::segments() const { return m_segments; }

    inline MuPatSegment* MuPatTrack::seedSegment() const {
        return m_seedSeg;  // could be a null pointer - do not dereference immediately!!
    }

    inline const std::vector<MuPatSegment*>& MuPatTrack::excludedSegments() const { return m_excludedSegments; }

    inline Trk::Track& MuPatTrack::track() const { return *m_track; }

    inline const Trk::TrackParameters& MuPatTrack::entryPars() const {
        const Trk::Perigee* pp = m_track->perigeeParameters();
        if (pp) { return *pp; }
        assert(!m_track->trackParameters() || m_track->trackParameters()->empty());
        return *m_track->trackParameters()->front();
    }

    class SortMuPatTrackByQuality {
    public:
        bool operator()(const MuPatTrack* c1, const MuPatTrack* c2) {
            // prefer candidates with more segments
            if (c1->segments().size() > c2->segments().size()) return true;
            if (c1->segments().size() < c2->segments().size()) return false;

            // prefer tracks with fit quality (always expected)
            const Trk::FitQuality* fq1 = c1->track().fitQuality();
            const Trk::FitQuality* fq2 = c2->track().fitQuality();
            if (!fq1) return false;
            if (!fq2) return true;

            if (fq1->numberDoF() > fq2->numberDoF()) return true;
            if (fq1->numberDoF() < fq2->numberDoF()) return false;

            // select candidate with smallest chi2
            double chi2Ndof1 = fq1->chiSquared() / fq1->numberDoF();
            double chi2Ndof2 = fq2->chiSquared() / fq2->numberDoF();
            return chi2Ndof1 < chi2Ndof2;
        }
    };

}  // namespace Muon

#endif
