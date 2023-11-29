
/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MooTrackBuilder.h"

#include <set>

#include "AthenaKernel/Timeout.h"
#include "MuPatPrimitives/MuPatSegment.h"
#include "MuPatPrimitives/MuPatTrack.h"
#include "MuPatPrimitives/SortMuPatHits.h"
#include "MuonCompetingRIOsOnTrack/CompetingMuonClustersOnTrack.h"
#include "MuonRIO_OnTrack/MMClusterOnTrack.h"
#include "MuonRIO_OnTrack/MdtDriftCircleOnTrack.h"
#include "MuonRIO_OnTrack/MuonClusterOnTrack.h"
#include "MuonSegmentMakerUtils/CompareMuonSegmentKeys.h"
#include "MuonSegmentMakerUtils/MuonSegmentKey.h"
#include "MuonTrackMakerUtils/MuonGetClosestParameters.h"
#include "MuonTrackMakerUtils/MuonTSOSHelper.h"
#include "MuonTrackMakerUtils/SortMeasurementsByPosition.h"
#include "MuonTrackMakerUtils/SortTracksByHitNumber.h"
#include "TrkEventPrimitives/ResidualPull.h"
#include "TrkSegment/SegmentCollection.h"

namespace Muon {
    static const MooTrackBuilder::PrepVec emptyPhiHits{};

    MooTrackBuilder::MooTrackBuilder(const std::string& t, const std::string& n, const IInterface* p) : AthAlgTool(t, n, p) {
        declareInterface<IMuonSegmentTrackBuilder>(this);
        declareInterface<MooTrackBuilder>(this);
        declareInterface<IMuonTrackRefiner>(this);
        declareInterface<IMuonTrackBuilder>(this);
    }

    StatusCode MooTrackBuilder::initialize() {
        ATH_CHECK(m_fitter.retrieve());
        ATH_CHECK(m_slFitter.retrieve());
        ATH_CHECK(m_fieldCacheCondObjInputKey.initialize());
        ATH_CHECK(m_errorOptimisationTool.retrieve(DisableTool{m_errorOptimisationTool.empty()}));
        ATH_CHECK(m_candidateHandler.retrieve());
        ATH_CHECK(m_candidateMatchingTool.retrieve());
        ATH_CHECK(m_muonChamberHoleRecoverTool.retrieve());
        ATH_CHECK(m_trackExtrapolationTool.retrieve());
        ATH_CHECK(m_idHelperSvc.retrieve());
        ATH_CHECK(m_edmHelperSvc.retrieve());
        ATH_CHECK(m_printer.retrieve());
        ATH_CHECK(m_trackToSegmentTool.retrieve());
        ATH_CHECK(m_seededSegmentFinder.retrieve());
        ATH_CHECK(m_mdtRotCreator.retrieve());
        ATH_CHECK(m_compRotCreator.retrieve());
        ATH_CHECK(m_propagator.retrieve());
        ATH_CHECK(m_pullCalculator.retrieve());
        ATH_CHECK(m_trackSummaryTool.retrieve());

        return StatusCode::SUCCESS;
    }

    StatusCode MooTrackBuilder::finalize() {
        if (m_nTimedOut > 0 && m_ncalls > 0) {
            double scale = 1. / m_ncalls;
            ATH_MSG_INFO(" Number of calls that timed out " << m_nTimedOut << " fraction of total calls " << scale * m_nTimedOut);
        }
        return StatusCode::SUCCESS;
    }

    std::unique_ptr<Trk::Track> MooTrackBuilder::refit(const EventContext& ctx, Trk::Track& track) const {
        // use slFitter for straight line fit, or toroid off, otherwise use normal Fitter

        if (m_edmHelperSvc->isSLTrack(track)) return m_slFitter->refit(ctx, track);

        // Also check if toriod is off:
        MagField::AtlasFieldCache fieldCache;
        // Get field cache object
        SG::ReadCondHandle<AtlasFieldCacheCondObj> readHandle{m_fieldCacheCondObjInputKey, ctx};
        const AtlasFieldCacheCondObj* fieldCondObj{*readHandle};

        if (!fieldCondObj) {
            ATH_MSG_ERROR("refit: Failed to retrieve AtlasFieldCacheCondObj with key " << m_fieldCacheCondObjInputKey.key());
            return nullptr;
        }
        fieldCondObj->getInitializedCache(fieldCache);
        if (!fieldCache.toroidOn()) return m_slFitter->refit(ctx, track);

        // if not refit tool specified do a pure refit
        if (m_errorOptimisationTool.empty()) return m_fitter->refit(ctx, track);
        std::unique_ptr<Trk::Track> optTrack = m_errorOptimisationTool->optimiseErrors(track, ctx);
        return optTrack;
    }

    void MooTrackBuilder::refine(const EventContext& ctx, MuPatTrack& track) const {

        ATH_MSG_VERBOSE("refine: before recovery " << std::endl
                                                  << m_printer->print(track.track()) << std::endl
                                                  << m_printer->print(track.track().measurementsOnTrack()->stdcont()));
        
        std::unique_ptr<Trk::Track> finalTrack(m_muonChamberHoleRecoverTool->recover(track.track(), ctx));
        if (!finalTrack) { ATH_MSG_WARNING(" final track lost, this should not happen "); }
        ATH_MSG_VERBOSE("refine: after recovery " << std::endl
                                                  << m_printer->print(*finalTrack) << std::endl
                                                  << m_printer->print(finalTrack->measurementsOnTrack()->stdcont()));

        // generate a track summary for this track
        if (m_trackSummaryTool.isEnabled()) { m_trackSummaryTool->computeAndReplaceTrackSummary(*finalTrack, false); }

        bool recalibrateMDTHits = m_recalibrateMDTHits;
        bool recreateCompetingROTs = true;
        std::unique_ptr<Trk::Track> recalibratedTrack = recalibrateHitsOnTrack(ctx, *finalTrack, recalibrateMDTHits, recreateCompetingROTs);
        if (!recalibratedTrack) {
            ATH_MSG_WARNING(" failed to recalibrate hits on track " << std::endl << m_printer->print(*finalTrack));
        } else
            finalTrack.swap(recalibratedTrack);

        std::unique_ptr<Trk::Track> refittedTrack = refit(ctx, *finalTrack);
        if (!refittedTrack) {
            ATH_MSG_VERBOSE(" failed to refit track " << std::endl
                                                      << m_printer->print(*finalTrack) << std::endl
                                                      << m_printer->printStations(*finalTrack));
        } else
            finalTrack.swap(refittedTrack);

        // redo holes as they are dropped in the fitter
        std::unique_ptr<Trk::Track> finalTrackWithHoles(m_muonChamberHoleRecoverTool->recover(*finalTrack, ctx));
        if (!finalTrackWithHoles) {
            ATH_MSG_WARNING(" failed to add holes to final track, this should not happen ");
        } else
            finalTrack.swap(finalTrackWithHoles);

        std::unique_ptr<Trk::Track> entryRecordTrack(m_trackExtrapolationTool->extrapolate(*finalTrack, ctx));
        if (entryRecordTrack) {
            finalTrack.swap(entryRecordTrack);
            ATH_MSG_VERBOSE(" track at muon entry record " << std::endl << m_printer->print(*finalTrack));
        }         
        m_candidateHandler->updateTrack(track, finalTrack);
    }

    std::unique_ptr<MuonSegment> MooTrackBuilder::combineToSegment(const EventContext& ctx, const MuonSegment& seg1, const MuonSegment& seg2, const PrepVec& externalPhiHits) const {
        // try to get track
        std::unique_ptr<Trk::Track> track = combine(ctx, seg1, seg2, externalPhiHits);

        if (!track) return nullptr;

        // create MuonSegment
        std::unique_ptr<MuonSegment> seg{m_trackToSegmentTool->convert(ctx, *track)};
        if (!seg) { ATH_MSG_WARNING(" conversion of track failed!! "); }

        return seg;
    }

    std::unique_ptr<Trk::Track> MooTrackBuilder::combine(const EventContext& ctx, const MuonSegment& seg1, const MuonSegment& seg2,
                                                         const PrepVec& externalPhiHits) const {
     
        // convert segments
        std::unique_ptr<MuPatSegment> segInfo1{m_candidateHandler->createSegInfo(ctx, seg1)};
        if (!segInfo1) {return nullptr; }
        std::unique_ptr<MuPatSegment> segInfo2{m_candidateHandler->createSegInfo(ctx, seg2)};
        if (!segInfo2) { return nullptr; }

        // call fit()
        return combine(ctx, *segInfo1, *segInfo2, externalPhiHits);
    }

    std::unique_ptr<MuonSegment> MooTrackBuilder::combineToSegment(const EventContext& ctx, const MuPatCandidateBase& firstCandidate, const MuPatCandidateBase& secondCandidate,
                                                   const PrepVec& externalPhiHits) const {
        // try to get track
        std::unique_ptr<Trk::Track> track = combine(ctx, firstCandidate, secondCandidate, externalPhiHits);

        if (!track) return nullptr;

        // create MuonSegment
        std::unique_ptr<MuonSegment> seg{m_trackToSegmentTool->convert(ctx, *track)};
        if (!seg) { ATH_MSG_WARNING(" conversion of track failed!! "); }

        return seg;
    }

    std::unique_ptr<Trk::Track> MooTrackBuilder::combine(const EventContext& ctx, const MuPatCandidateBase& firstCandidate,
                                                         const MuPatCandidateBase& secondCandidate, const PrepVec& externalPhiHits) const {
        ++m_ncalls;

        if (m_doTimeOutChecks && Athena::Timeout::instance(ctx).reached()) {
            ATH_MSG_DEBUG("Timeout reached. Aborting sequence.");
            ++m_nTimedOut;
            return nullptr;
        }

        std::set<MuonStationIndex::StIndex> stations = firstCandidate.stations();
        stations.insert(secondCandidate.stations().begin(), secondCandidate.stations().end());
        unsigned int nstations = stations.size();

        MagField::AtlasFieldCache fieldCache;
        // Get field cache object
        SG::ReadCondHandle<AtlasFieldCacheCondObj> readHandle{m_fieldCacheCondObjInputKey, ctx};
        const AtlasFieldCacheCondObj* fieldCondObj{*readHandle};

        if (!fieldCondObj) {
            ATH_MSG_ERROR("combine: Failed to retrieve AtlasFieldCacheCondObj with key " << m_fieldCacheCondObjInputKey.key());
            return nullptr;
        }
        fieldCondObj->getInitializedCache(fieldCache);

        bool slFit = !fieldCache.toroidOn() || nstations == 1 || (nstations == 2 && (stations.count(MuonStationIndex::EM) &&
                                                           (stations.count(MuonStationIndex::BO) || stations.count(MuonStationIndex::EO))));
        if (msgLvl(MSG::DEBUG)) {
            msg(MSG::DEBUG) << MSG::DEBUG << " combining entries: nstations " << nstations << " types:";
            for (std::set<MuonStationIndex::StIndex>::iterator it = stations.begin(); it != stations.end(); ++it) {
                msg(MSG::DEBUG) << MSG::DEBUG << "  " << MuonStationIndex::stName(*it);
            }
            if (slFit) {
                msg(MSG::DEBUG) << " doing SL fit ";
            } else {
                msg(MSG::DEBUG) << " doing curved fit ";
            }
            msg(MSG::DEBUG) << endmsg;
        }

        const MuPatTrack* trkCan1 = dynamic_cast<const MuPatTrack*>(&firstCandidate);
        const MuPatTrack* trkCan2 = dynamic_cast<const MuPatTrack*>(&secondCandidate);

        const MuPatSegment* segCan1 = dynamic_cast<const MuPatSegment*>(&firstCandidate);
        const MuPatSegment* segCan2 = dynamic_cast<const MuPatSegment*>(&secondCandidate);

        const MuPatTrack* candidate = nullptr;
        const MuPatSegment* segment = nullptr;
        if (trkCan1 && segCan2) {
            candidate = trkCan1;
            segment = segCan2;
        } else if (trkCan2 && segCan1) {
            candidate = trkCan2;
            segment = segCan1;
        }

        // check whether this combination was already tried, if yes reject the combination
        if (candidate && segment) {
            ATH_MSG_DEBUG(" Track/segment combination");
            const std::vector<MuPatSegment*>& excl = candidate->excludedSegments();
            if (std::find(excl.begin(),excl.end(), segment) != excl.end()){
                ATH_MSG_DEBUG(" Rejected segment based on exclusion list");
                return nullptr;                
            }
        }

        // the following bit of code checks whether the current combination of segments was already tested
        if (m_useTrackingHistory) {
            // create a set of all segments of the would-be candidate
            std::set<const MuPatSegment*> segments;
            if ((segCan1 && segCan2)) {
                segments.insert(segCan1);
                segments.insert(segCan2);
            }
            if (candidate && segment) {
                segments.insert(segment);
                segments.insert(candidate->segments().begin(), candidate->segments().end());                
            }
            // now loop over the segments and check if any of them is associated with a track that contains all of the segments
           
            for (const MuPatSegment* used : segments) {
                // loop over the tracks associated with the current segment
                for (const MuPatTrack* assoc_track : used->tracks()) {
                    // loop over the segments associated with the track
                    std::set<const MuPatSegment*> foundSegments;
                    for (const MuPatSegment* segOnTrack : assoc_track->segments()) {
                        if (segments.count(segOnTrack)) foundSegments.insert(segOnTrack);
                    }                   
                    // if all segments are already part of an existing track, don't perform the fit
                    if (foundSegments.size() == segments.size()) {
                        ATH_MSG_DEBUG("Combination already part of an existing track");
                        return nullptr;
                    }

                    // if all segments but one are already part of an existing track, check the exclusion list
                    if (candidate && !candidate->excludedSegments().empty() && foundSegments.size() == segments.size() - 1) {
                        // create destination vector for segments that are not found
                        std::vector<const MuPatSegment*> unassociatedSegments(segments.size(), nullptr);
                        std::vector<const MuPatSegment*>::iterator it = std::set_difference(
                            segments.begin(), segments.end(), foundSegments.begin(), foundSegments.end(), unassociatedSegments.begin());
                        const MuPatSegment* zero = nullptr;
                        unassociatedSegments.erase(std::find(unassociatedSegments.begin(), unassociatedSegments.end(), zero),
                                                   unassociatedSegments.end());

                        // check whether any pointers found
                        if (it != unassociatedSegments.begin()) {
                            // this should always be one as we required the difference to be one!
                            if (unassociatedSegments.size() != 1) {
                                ATH_MSG_DEBUG("Inconsistent result from set difference: size result "
                                              << unassociatedSegments.size() << " candidate " << segments.size() << " found "
                                              << foundSegments.size());
                                return nullptr;
                            }

                            // check that the result is indeed part of the original set
                            if (!segments.count(unassociatedSegments.front())) {
                                ATH_MSG_DEBUG("Segment point not part of the original set, aborting!");
                                return nullptr;
                            }

                            // now check whether the segment is part of the excluded segments
                            std::vector<MuPatSegment*>::const_iterator pos = std::find(
                                candidate->excludedSegments().begin(), candidate->excludedSegments().end(), unassociatedSegments.front());
                            if (pos != candidate->excludedSegments().end()) {
                                ATH_MSG_DEBUG("Segment found in exclusion list, not performing fit");
                                return nullptr;
                            }
                        }
                    }
                }
            }
        }

        // use slFitter for straight line fit, or toroid off, otherwise use normal Fitter
        if (slFit) return std::unique_ptr<Trk::Track>(m_slFitter->fit(ctx, firstCandidate, secondCandidate, externalPhiHits));

        return m_fitter->fit(ctx, firstCandidate, secondCandidate, externalPhiHits);
    }

    std::unique_ptr<Trk::Track> MooTrackBuilder::combine(const EventContext& ctx, const Trk::Track& track, const MuonSegment& seg,
                                                         const PrepVec& externalPhiHits) const {

        // convert segments
        std::unique_ptr<Trk::Track> inTrack = std::make_unique<Trk::Track>(track);
        std::unique_ptr<MuPatTrack> candidate(m_candidateHandler->createCandidate(inTrack));
        if (!candidate) return nullptr;
        std::unique_ptr<MuPatSegment> segInfo(m_candidateHandler->createSegInfo(ctx, seg));
        if (!segInfo) { return nullptr; }
       
        // call fit()
        return  combine(ctx, *candidate, *segInfo, externalPhiHits);
    }

    std::vector<std::unique_ptr<Trk::Track> > MooTrackBuilder::combineWithSegmentFinding(const EventContext& ctx, const Trk::Track& track, const MuonSegment& seg,
                                                                                         const PrepVec& externalPhiHits) const {
       
        // convert segments
        std::unique_ptr<Trk::Track> inTrack = std::make_unique<Trk::Track>(track);       
        std::unique_ptr<MuPatTrack> candidate = m_candidateHandler->createCandidate(inTrack);
        if (!candidate) return {};
        std::unique_ptr<MuPatSegment> segInfo(m_candidateHandler->createSegInfo(ctx, seg));
        if (!segInfo) return {};
        // call fit()
        return combineWithSegmentFinding(ctx, *candidate, *segInfo, externalPhiHits);
    }

    std::unique_ptr<Trk::TrackParameters> MooTrackBuilder::findClosestParameters(const Trk::Track& track, const Amg::Vector3D& pos) const {
        // are we in the endcap?
        bool isEndcap = m_edmHelperSvc->isEndcap(track);

        // position of segment
        double posSeg = isEndcap ? pos.z() : pos.perp();

        // position closest parameters
        double closest = 1e8;
        const Trk::TrackParameters* closestParameters = nullptr;
        bool closestIsMeasured = false;

        // loop over track and calculate residuals
        const Trk::TrackStates* states = track.trackStateOnSurfaces();
        if (!states) {
            ATH_MSG_DEBUG(" track without states! ");
            return nullptr;
        }

        // loop over TSOSs
        Trk::TrackStates::const_iterator tsit = states->begin();
        Trk::TrackStates::const_iterator tsit_end = states->end();
        for (; tsit != tsit_end; ++tsit) {
            // check whether state is a measurement
            const Trk::MeasurementBase* meas = (*tsit)->measurementOnTrack();
            if (!meas) { continue; }

            const Trk::TrackParameters* pars = (*tsit)->trackParameters();
            if (!pars) { continue; }

            // check whether measured parameters
            bool isMeasured = pars->covariance();

            // skip all none measured TrackParameters as soon as we found one with a measurement
            if (closestIsMeasured && !isMeasured) continue;

            // calculate position parameters and compare with position segment
            double posPars = isEndcap ? pars->position().z() : pars->position().perp();
            double diffPos = std::abs(posPars - posSeg);

            // accept if measured parameters or the current accepted parameters are not yet measured
            if ((isMeasured && !closestIsMeasured) || diffPos < closest) {
                closest = diffPos;
                closestParameters = pars;
                closestIsMeasured = isMeasured;

                // if we are within 100 mm take current
                if (closest < 100.) { break; }
            }
        }

        // return clone of parameters
        if (closestParameters) return closestParameters->uniqueClone();
        return nullptr;
    }

    std::unique_ptr<Trk::TrackParameters> MooTrackBuilder::getClosestParameters(const MuPatCandidateBase& candidate, const Trk::Surface& surf) const {
        // cast to segment, return segment parameters if cast success
        const MuPatSegment* segCandidate = dynamic_cast<const MuPatSegment*>(&candidate);
        if (segCandidate) return segCandidate->entryPars().uniqueClone();

        // for a track candidate, return the closest parameter on the track
        const MuPatTrack& trkCandidate = dynamic_cast<const MuPatTrack&>(candidate);
        return getClosestParameters(trkCandidate.track(), surf);
    }

    std::unique_ptr<Trk::TrackParameters> MooTrackBuilder::getClosestParameters(const Trk::Track& track, const Trk::Surface& surf) const {
        return MuonGetClosestParameters::closestParameters(track, surf);
    }

    std::vector<std::unique_ptr<Trk::Track> > MooTrackBuilder::combineWithSegmentFinding(const EventContext& ctx, const Trk::Track& track,
                                                                                         const Trk::TrackParameters& pars,
                                                                                         const std::set<Identifier>& chIds,
                                                                                         const PrepVec& patternPhiHits) const {
        // convert track
        std::unique_ptr<Trk::Track> inTrack = std::make_unique<Trk::Track>(track);
        std::unique_ptr<MuPatTrack> can = m_candidateHandler->createCandidate(inTrack);
        if (!can) { return {}; }
        return combineWithSegmentFinding(ctx, *can, pars, chIds, patternPhiHits);
    }

    std::vector<std::unique_ptr<Trk::Track> > MooTrackBuilder::combineWithSegmentFinding(const EventContext& ctx, const MuPatTrack& candidate,
                                                                                         const MuPatSegment& segInfo,
                                                                                         const PrepVec& externalPhiHits) const {
        /** second stage segment matching:
            - estimate segment parameters at segment position using fit of track + segment position
            - redo segment finding using the predicted parameters as seed
            - redo segment association
        */

        const MuonSegment& seg = *segInfo.segment;
        std::vector<std::unique_ptr<Trk::Track> > newTracks;

        // get chamber Id of segment
        std::set<Identifier> chIds = m_edmHelperSvc->chamberIds(seg);

        if (chIds.empty()) return newTracks;

        // for now do not redo segment making for CSCs
        if (m_idHelperSvc->isCsc(*chIds.begin())) {
            if (m_candidateMatchingTool->match(ctx, candidate, segInfo, true)) {
                std::unique_ptr<Trk::Track> newtrack(m_fitter->fit(ctx, candidate, segInfo, externalPhiHits));
                if (newtrack) newTracks.push_back(std::move(newtrack));
                return newTracks;
            } else {
                return newTracks;
            }
        }

        const Trk::Track& track = candidate.track();
        ATH_MSG_DEBUG(" in combineWithSegmentFinding ");
        ATH_MSG_VERBOSE(" segment " << m_printer->print(seg));

        // find track parameters on segment surface
        std::unique_ptr<Trk::TrackParameters> closestPars(findClosestParameters(track, seg.globalPosition()));

        if (!closestPars) {
            ATH_MSG_WARNING(" unable to find closest TrackParameters ");
            return newTracks;
        }

        ATH_MSG_VERBOSE(" closest parameter " << m_printer->print(*closestPars));

        // propagate to segment surface
        std::unique_ptr<Trk::TrackParameters> exPars(
            m_propagator->propagate(ctx,*closestPars, seg.associatedSurface(), Trk::anyDirection, false, m_magFieldProperties));

        if (!exPars) {
            ATH_MSG_WARNING(" Propagation failed!! ");
            return newTracks;
        }

        ATH_MSG_VERBOSE(" extrapolated parameter " << m_printer->print(*exPars));

        return combineWithSegmentFinding(ctx, candidate, *exPars, chIds, externalPhiHits);
    }

    void MooTrackBuilder::removeDuplicateWithReference(std::unique_ptr<Trk::SegmentCollection>& segments,
                                                       std::vector<const MuonSegment*>& referenceSegments) const {
        if (referenceSegments.empty()) return;

        ATH_MSG_DEBUG(" Removing duplicates from segment vector of size " << segments->size() << " reference size "
                                                                          << referenceSegments.size());

        CompareMuonSegmentKeys compareSegmentKeys{};

        // create a vector with pairs of MuonSegmentKey and a pointer to the corresponding segment to resolve ambiguities
        std::vector<std::pair<MuonSegmentKey, Trk::SegmentCollection::iterator> > segKeys;
        segKeys.reserve(segments->size());

        // loop over reference segments and make keys
        Trk::SegmentCollection::iterator sit = segments->begin();
        Trk::SegmentCollection::iterator sit_end = segments->end();
        for (; sit != sit_end; ++sit) {
            Trk::Segment* tseg = *sit;
            MuonSegment* mseg = dynamic_cast<MuonSegment*>(tseg);
            segKeys.push_back(std::make_pair(MuonSegmentKey(*mseg), sit));
        }

        // create a vector with pairs of MuonSegmentKey and a pointer to the corresponding segment to resolve ambiguities
        std::vector<MuonSegmentKey> referenceSegKeys;
        referenceSegKeys.reserve(referenceSegments.size());

        // loop over reference segments and make keys
        std::vector<const MuonSegment*>::iterator vit = referenceSegments.begin();
        std::vector<const MuonSegment*>::iterator vit_end = referenceSegments.end();
        for (; vit != vit_end; ++vit) { referenceSegKeys.push_back(MuonSegmentKey(**vit)); }

        // loop over segments and compare the current segment with the reference ones
        std::vector<std::pair<MuonSegmentKey, Trk::SegmentCollection::iterator> >::iterator skit = segKeys.begin();
        std::vector<std::pair<MuonSegmentKey, Trk::SegmentCollection::iterator> >::iterator skit_end = segKeys.end();
        for (; skit != skit_end; ++skit) {
            bool isDuplicate = false;

            std::vector<MuonSegmentKey>::iterator rskit = referenceSegKeys.begin();
            std::vector<MuonSegmentKey>::iterator rskit_end = referenceSegKeys.end();

            for (; rskit != rskit_end; ++rskit) {
                CompareMuonSegmentKeys::OverlapResult overlapResult = compareSegmentKeys(*rskit, skit->first);
                if (overlapResult == CompareMuonSegmentKeys::Identical) {
                    ATH_MSG_DEBUG(" discarding identical segment");
                    isDuplicate = true;
                    break;
                } else if (overlapResult == CompareMuonSegmentKeys::SuperSet) {
                    // reference segment superset of current: discard
                    ATH_MSG_DEBUG(" discarding (subset) ");
                    isDuplicate = true;
                    break;
                }
            }
            if (isDuplicate) segments->erase(skit->second);
        }
    }

    std::vector<std::unique_ptr<Trk::Track> > MooTrackBuilder::combineWithSegmentFinding(const EventContext& ctx, const MuPatTrack& candidate,
                                                                                         const Trk::TrackParameters& pars,
                                                                                         const std::set<Identifier>& chIds,
                                                                                         const PrepVec& externalPhiHits) const {
        std::vector<std::unique_ptr<Trk::Track> > newTracks;

        if (chIds.empty()) return newTracks;

        if (!m_idHelperSvc->isMdt(*chIds.begin())) {
            ATH_MSG_WARNING("combineWithSegmentFinding called with CSC hits!! retuning zero pointer");
            return newTracks;
        }

        // redo segment finding
        std::unique_ptr<Trk::SegmentCollection> segments = m_seededSegmentFinder->find(ctx, pars, chIds);

        // check whether we got segments
        if (!segments) {
            ATH_MSG_DEBUG(" failed to find new segments ");
            return newTracks;
        }
        if (segments->empty()) {
            ATH_MSG_DEBUG(" got empty vector!! ");
            return newTracks;
        }

        unsigned int nseg = segments->size();
        if (m_useExclusionList) {
            std::vector<const MuonSegment*> referenceSegments;
            for (std::vector<MuPatSegment*>::const_iterator esit = candidate.excludedSegments().begin();
                 esit != candidate.excludedSegments().end(); ++esit) {
                if ((*esit)->segment) referenceSegments.push_back((*esit)->segment);
            }
            removeDuplicateWithReference(segments, referenceSegments);
        }

        if (msgLvl(MSG::DEBUG) && segments->size() != nseg) {
            msg(MSG::DEBUG) << MSG::DEBUG
                            << " Rejected segments based on exclusion list, number of removed segments: " << nseg - segments->size()
                            << " total " << segments->size() << endmsg;
        }

        if (!segments->empty()) {
            // loop over segments
            for (Trk::Segment* tseg : *segments) {
                if (!tseg) continue;
                MuonSegment* mseg = dynamic_cast<MuonSegment*>(tseg);

                if (msgLvl(MSG::DEBUG)) {
                    msg(MSG::DEBUG) << MSG::DEBUG << " adding segment " << m_printer->print(*mseg);
                    if (msgLvl(MSG::VERBOSE)) {
                        msg(MSG::DEBUG) << std::endl << m_printer->print(mseg->containedMeasurements()) << endmsg;
                        if (msgLvl(MSG::VERBOSE) && candidate.track().measurementsOnTrack())
                            msg(MSG::DEBUG) << " track " << m_printer->print(candidate.track()) << std::endl
                                            << m_printer->print(candidate.track().measurementsOnTrack()->stdcont()) << endmsg;
                    } else {
                        msg(MSG::DEBUG) << endmsg;
                    }
                }
                std::unique_ptr<MuPatSegment> segInfo{m_candidateHandler->createSegInfo(ctx, *mseg)};
                
                if (!m_candidateMatchingTool->match(ctx, candidate, *segInfo, true)) { continue; }
               
                std::unique_ptr<Trk::Track> segTrack = m_fitter->fit(ctx, candidate, *segInfo, externalPhiHits);

                if (!segTrack) continue;

                ATH_MSG_DEBUG(" found new track " << m_printer->print(*segTrack));
                newTracks.push_back(std::move(segTrack));
            }
        }

        if (!newTracks.empty()) ATH_MSG_DEBUG(" found new tracks for segment " << newTracks.size());

        return newTracks;
    }

    std::unique_ptr<Trk::Track> MooTrackBuilder::recalibrateHitsOnTrack(const EventContext& ctx, const Trk::Track& track, bool doMdts,
                                                                        bool doCompetingClusters) const {
        // loop over track and calculate residuals
        const Trk::TrackStates* states = track.trackStateOnSurfaces();
        if (!states) {
            ATH_MSG_DEBUG(" track without states, discarding track ");
            return nullptr;
        }
        if (msgLvl(MSG::DEBUG)) {
            msg(MSG::DEBUG) << MSG::DEBUG << " recalibrating hits on track " << std::endl << m_printer->print(track);

            if (msgLvl(MSG::VERBOSE)) {
                if (track.measurementsOnTrack())
                    msg(MSG::DEBUG) << std::endl << m_printer->print(track.measurementsOnTrack()->stdcont()) << endmsg;
            } else {
                msg(MSG::DEBUG) << endmsg;
            }
        }
        // vector to store states, the boolean indicated whether the state was create in this routine (true) or belongs to the track (false)
        // If any new state is created, all states will be cloned and a new track will beformed from them.
        std::vector<std::unique_ptr<const Trk::TrackStateOnSurface>> newStates;
        newStates.reserve(states->size() + 5);

        // loop over TSOSs
        Trk::TrackStates::const_iterator state_itr = states->begin();
        Trk::TrackStates::const_iterator end_itr = states->end();        
        for (; state_itr != end_itr; ++state_itr) {
            const Trk::TrackStateOnSurface* tsit = (*state_itr);
            if (!tsit) continue;  // sanity check

            // check whether state is a measurement
            const Trk::TrackParameters* pars = tsit->trackParameters();
            if (!pars) {
                newStates.emplace_back(tsit->clone());
                continue;
            }

            // check whether state is a measurement
            const Trk::MeasurementBase* meas = tsit->measurementOnTrack();
            if (!meas) {
                newStates.emplace_back(tsit->clone());
                continue;
            }

            Identifier id = m_edmHelperSvc->getIdentifier(*meas);

            // Not a ROT, else it would have had an identifier. Keep the TSOS.
            if (!id.is_valid() || !m_idHelperSvc->isMuon(id)) {
                newStates.emplace_back(tsit->clone());
                continue;
            }

            ATH_MSG_VERBOSE(" new measurement " << m_idHelperSvc->toString(id));

            if (m_idHelperSvc->isMdt(id)) {
                if (doMdts) {
                    const MdtDriftCircleOnTrack* mdt = dynamic_cast<const MdtDriftCircleOnTrack*>(meas);
                    if (!mdt) {
                        ATH_MSG_WARNING(" Measurement with MDT identifier that is not a MdtDriftCircleOnTrack ");
                        continue;
                    }
                    std::unique_ptr<Trk::RIO_OnTrack> newMdt(m_mdtRotCreator->correct(*mdt->prepRawData(), *pars));
                    if (!newMdt) {
                        ATH_MSG_WARNING(" Failed to recalibrate MDT ");
                        continue;
                    }
                    std::unique_ptr<Trk::TrackStateOnSurface> tsos = MuonTSOSHelper::createMeasTSOSWithUpdate(
                        *tsit, std::move(newMdt), pars->uniqueClone(),
                         tsit->type(Trk::TrackStateOnSurface::Outlier) ? Trk::TrackStateOnSurface::Outlier
                                                                         : Trk::TrackStateOnSurface::Measurement);
                    newStates.push_back(std::move(tsos));

                } else {
                    newStates.emplace_back(tsit->clone());
                }

            } else if (m_idHelperSvc->isCsc(id)) {
                newStates.emplace_back(tsit->clone());

            } else if (m_idHelperSvc->isTrigger(id)) {
                if (doCompetingClusters) {
                    state_itr = insertClustersWithCompetingRotCreation(ctx, state_itr, end_itr, newStates);
                } else {
                    newStates.emplace_back(tsit->clone());
                }

            } else if (m_idHelperSvc->isMM(id) || m_idHelperSvc->issTgc(id)) {
                newStates.emplace_back(tsit->clone());
            } else {
                ATH_MSG_WARNING(" unknown Identifier ");
            }
        }

        ATH_MSG_DEBUG(" original track had " << states->size() << " TSOS, adding " << newStates.size() - states->size() << " new TSOS ");

        // states were added, create a new track
        auto trackStateOnSurfaces = std::make_unique<Trk::TrackStates>();
        trackStateOnSurfaces->reserve(newStates.size());
        for (std::unique_ptr<const Trk::TrackStateOnSurface>& new_state : newStates) {
            // add states. If nit->first is true we have a new state. If it is false the state is from the old track and has to be cloned
            trackStateOnSurfaces->push_back(std::move(new_state));
        }
        return std::make_unique<Trk::Track>(track.info(), std::move(trackStateOnSurfaces),
                                            track.fitQuality() ? track.fitQuality()->uniqueClone() : nullptr);
    }

    Trk::TrackStates::const_iterator MooTrackBuilder::insertClustersWithCompetingRotCreation(const EventContext& ctx,
        Trk::TrackStates::const_iterator tsit,
        Trk::TrackStates::const_iterator tsit_end,
        std::vector<std::unique_ptr<const Trk::TrackStateOnSurface> >& states) const {
        // iterator should point to a valid element
        if (tsit == tsit_end) {
            ATH_MSG_WARNING(" iterator pointing to end of vector, this should no happen ");
            return --tsit;
        }

        // check whether state is a measurement
        const Trk::MeasurementBase* meas = (*tsit)->measurementOnTrack();
        const Trk::TrackParameters* pars = (*tsit)->trackParameters();
        if (!meas || !pars) {
            ATH_MSG_WARNING(" iterator pointing to a TSOS without a measurement or TrackParameters ");
            if (tsit + 1 == tsit_end) --tsit;
            return tsit;
        }

        ATH_MSG_VERBOSE(" inserting with competing ROT creation ");

        // loop over states until we reached the last tgc hit in this detector element
        // keep trackof the identifiers and the states
        std::list<const Trk::PrepRawData*> etaPrds;
        std::list<const Trk::PrepRawData*> phiPrds;
        const Trk::TrkDetElementBase* currentDetEl = nullptr;
        std::vector<std::unique_ptr<const Trk::TrackStateOnSurface> > newStates;
        // keep track of outliers as we might have to drop them..
        std::vector<std::pair<bool, const Trk::TrackStateOnSurface*> > outlierStates;
        bool hasPhi {false}, hasEta{false};

        for (; tsit != tsit_end; ++tsit) {
            const Trk::TrackStateOnSurface* in_tsos = *tsit;
            if (!in_tsos) continue;

            // check whether state is a measurement, keep if not
            const Trk::MeasurementBase* meas = in_tsos->measurementOnTrack();
            if (!meas) {
                newStates.emplace_back(in_tsos->clone());
                continue;
            }

            // get identifier, keep state if it has no identifier.
            Identifier id = m_edmHelperSvc->getIdentifier(*meas);
            if (!id.is_valid()) {
                newStates.emplace_back(in_tsos->clone());
                continue;
            }

            // sanity check, this SHOULD be a RPC, TGC or CSC measurement
            if (!(m_idHelperSvc->isTrigger(id))) { break; }

            bool measuresPhi = m_idHelperSvc->measuresPhi(id);
            if (!hasPhi && measuresPhi) hasPhi = true;
            if (!hasEta && !measuresPhi) hasEta = true;

            // check whether state is a measurement
            if ((*tsit)->type(Trk::TrackStateOnSurface::Outlier)) {
                outlierStates.push_back(std::make_pair(measuresPhi, in_tsos));
                continue;
            }

            // check whether we are still in the same chamber, stop loop if not

            ATH_MSG_VERBOSE(" handling " << m_idHelperSvc->toString(id));

            std::list<const Trk::PrepRawData*>& prdList = measuresPhi ? phiPrds : etaPrds;
            const MuonClusterOnTrack* clus = dynamic_cast<const MuonClusterOnTrack*>(meas);
            if (clus) {
                const Trk::TrkDetElementBase* detEl = clus->detectorElement();
                if (!currentDetEl) currentDetEl = detEl;
                if (detEl != currentDetEl) {
                    ATH_MSG_VERBOSE(" new detector element stopping ");
                    break;
                }
                prdList.push_back(clus->prepRawData());
            } else {
                // split competing ROTs into constituents
                const CompetingMuonClustersOnTrack* comp = dynamic_cast<const CompetingMuonClustersOnTrack*>(meas);
                if (comp) {
                    const Trk::TrkDetElementBase* detEl = nullptr;
                    if (comp->containedROTs().empty()) {
                        ATH_MSG_WARNING(" CompetingROT without constituents ");
                        break;
                    }
                    detEl = comp->containedROTs().front()->detectorElement();
                    if (!currentDetEl) currentDetEl = detEl;
                    if (detEl != currentDetEl) {
                        ATH_MSG_VERBOSE(" new detector element stopping ");
                        break;
                    }
                    std::vector<const MuonClusterOnTrack*>::const_iterator clit = comp->containedROTs().begin();
                    std::vector<const MuonClusterOnTrack*>::const_iterator clit_end = comp->containedROTs().end();
                    for (; clit != clit_end; ++clit) { prdList.push_back((*clit)->prepRawData()); }

                } else {
                    ATH_MSG_WARNING(" Unknown trigger hit type! ");
                    continue;
                }
            }
        }

        // now that we have the lists of prds we can create the competing rots
        if (!etaPrds.empty()) {
            std::unique_ptr<CompetingMuonClustersOnTrack> etaCompRot = m_compRotCreator->createBroadCluster(etaPrds, 0.);
            if (!etaCompRot) {
                ATH_MSG_WARNING(" Failed to create CompetingMuonClustersOnTrack for eta hits! ");
            } else {
                std::unique_ptr<Trk::TrackParameters> etaPars;;
                // check whether original parameters are on surface, if so clone original parameters
                if (etaCompRot->associatedSurface() == pars->associatedSurface()) {
                    etaPars = pars->uniqueClone();
                } else {
                    // ownership relinquished, should be treated in createMeasTSOS
                    etaPars =
                        m_propagator->propagate(ctx,*pars, 
                          etaCompRot->associatedSurface(), 
                          Trk::anyDirection, false, m_magFieldProperties);
                }
                if (!etaPars) {
                    ATH_MSG_WARNING(" Failed to calculate TrackParameters for eta hits! ");
                } else {
                    std::unique_ptr<Trk::TrackStateOnSurface> tsos =
                        MuonTSOSHelper::createMeasTSOS(std::move(etaCompRot), std::move(etaPars), Trk::TrackStateOnSurface::Measurement);
                    newStates.push_back(std::move(tsos));
                }
            }
        }

        if (!phiPrds.empty()) {
            std::unique_ptr<CompetingMuonClustersOnTrack> phiCompRot = m_compRotCreator->createBroadCluster(phiPrds, 0.);
            if (!phiCompRot) {
                ATH_MSG_WARNING(" Failed to create CompetingMuonClustersOnTrack for phi hits! ");
            } else {
                std::unique_ptr<Trk::TrackParameters> phiPars;
                // check whether original parameters are on surface, if so clone original parameters
                if (phiCompRot->associatedSurface() == pars->associatedSurface()) {
                    phiPars = pars->uniqueClone();
                } else {
                    // ownership relinquished, handled in createMeasTSOS
                    phiPars =
                        m_propagator->propagate(ctx, *pars, phiCompRot->associatedSurface(), 
                        Trk::anyDirection, false, m_magFieldProperties);
                }
                if (!phiPars) {
                    ATH_MSG_WARNING(" Failed to calculate TrackParameters for phi hits! ");
                } else {
                    std::unique_ptr<Trk::TrackStateOnSurface> tsos =
                        MuonTSOSHelper::createMeasTSOS(std::move(phiCompRot), std::move(phiPars), Trk::TrackStateOnSurface::Measurement);
                    newStates.push_back(std::move(tsos));
                }
            }
        }

        // add outliers if there was no measurement on track in the same projection
        for (const auto& outlier : outlierStates) {
            if (hasPhi && outlier.first)
                newStates.emplace_back(outlier.second->clone());
            else if (hasEta && !outlier.first)
                newStates.emplace_back(outlier.second->clone());
            else if (msgLvl(MSG::DEBUG))
                msg(MSG::DEBUG) << " Dropping outlier " << endmsg;
        }

        // sort all states in this chamber
        std::stable_sort(newStates.begin(), newStates.end(), SortTSOSByDistanceToPars(pars));

        // insert the states into
        states.insert(states.end(), std::make_move_iterator(newStates.begin()), 
                                    std::make_move_iterator(newStates.end()));

        // iterator should point to the last TGC in this chamber
        return --tsit;
    }

    std::pair<std::unique_ptr<Trk::Track>, std::unique_ptr<Trk::Track> > MooTrackBuilder::splitTrack(const EventContext& ctx, const Trk::Track& track) const {
        // use slFitter for straight line fit, or toroid off, otherwise use normal Fitter

        if (m_edmHelperSvc->isSLTrack(track)) return m_slFitter->splitTrack(ctx, track);

        MagField::AtlasFieldCache fieldCache;
        // Get field cache object
        SG::ReadCondHandle<AtlasFieldCacheCondObj> readHandle{m_fieldCacheCondObjInputKey, ctx};
        const AtlasFieldCacheCondObj* fieldCondObj{*readHandle};

        if (!fieldCondObj) {
            ATH_MSG_ERROR("splitTrack: Failed to retrieve AtlasFieldCacheCondObj with key " << m_fieldCacheCondObjInputKey.key());
            return {};
        }
        fieldCondObj->getInitializedCache(fieldCache);

        if (!fieldCache.toroidOn()) return m_slFitter->splitTrack(ctx, track);

        return m_fitter->splitTrack(ctx, track);
    }

    std::vector<std::unique_ptr<MuPatTrack> > MooTrackBuilder::find(const EventContext& ctx, MuPatCandidateBase& candidate, 
                                                                    const std::vector<MuPatSegment*>& segVec) const {
        std::vector<std::unique_ptr<MuPatTrack> > candidates;
        // check whether we have segments
        if (segVec.empty()) return candidates;

        std::set<MuPatSegment*> usedSegments;
        std::map<MuPatSegment*, MuPatSegment*> slSegments;

        // int looseQualityLevel = 1; // Not used for the moment
        bool tightQualityCuts = false;
        ATH_MSG_DEBUG(" find: " << m_candidateHandler->print(candidate, 0) << std::endl << m_candidateHandler->print(segVec, 0));

        // store whether segment was added to at least one candidates

        // vector to store candidate extensions
        std::vector<std::pair<MuPatSegment*, std::unique_ptr<Trk::Track> > > extensions;
        extensions.reserve(segVec.size());

        // loop over segments
        for (MuPatSegment* seg : segVec) {
            if (usedSegments.count(seg)) continue;

            // check whether chamber is already included in candidate
            if (candidate.shareChambers(*seg)) {
                ATH_MSG_VERBOSE("addStationToSeed:: already on candidate " << std::endl << m_printer->print(*seg->segment));
                continue;
            }

            if (!m_candidateMatchingTool->match(ctx, candidate, *seg, tightQualityCuts)) {
                ATH_MSG_VERBOSE(" track/segment combination rejected based on angular matching " << std::endl
                                                                                                 << m_printer->print(*seg->segment));
                continue;
            }

            ATH_MSG_VERBOSE("combining: " << m_printer->print(*seg->segment));

            // try to combine track with segment
            std::unique_ptr<Trk::Track> track = combine(ctx, candidate, *seg, emptyPhiHits);

            // additional check in case the candidate is a MuPatTrack
            MuPatTrack* trkCan = dynamic_cast<MuPatTrack*>(&candidate);
            MuPatSegment* segCan = dynamic_cast<MuPatSegment*>(&candidate);
            if (trkCan) {
                if (!track) {
                    trkCan->addExcludedSegment(seg);
                    continue;
                }

                // is the new track better
                SortTracksByHitNumber sortTracks;
                if (!sortTracks(*track, trkCan->track())) {
                    ATH_MSG_VERBOSE(" rejecting track as new segment results in worse fit");
                    continue;
                }

                // check whether the track cleaner didn't remove one of the already added chamber layers
                // loop over hits
                std::set<MuonStationIndex::StIndex> stationLayersOnTrack;
                for ( const Trk::MeasurementBase* meas : *track->measurementsOnTrack()) {                 
                    Identifier id = m_edmHelperSvc->getIdentifier(*meas);
                    if (!id.is_valid() || m_idHelperSvc->isTrigger(id)) { continue; }
                    stationLayersOnTrack.insert(m_idHelperSvc->stationIndex(id));
                }

                bool hasAllLayers = true;
                for (const MuonStationIndex::StIndex& stIdx :candidate.stations()) {
                    if (!stationLayersOnTrack.count(stIdx)) {
                        ATH_MSG_VERBOSE(" missing layer " << MuonStationIndex::stName(stIdx));
                        hasAllLayers = false;
                    }
                }

                if (!hasAllLayers) {
                    ATH_MSG_VERBOSE(" rejecting track as one of the chamber layers of the candidate was removed ");
                    continue;
                }
            }

            if (!track) { continue; }

            usedSegments.insert(seg);

            // now loop over segments once more and try to add SL overlap if missed
            // first check that segment is not an overlap segment
            if (!seg->hasSLOverlap()) {
                std::unique_ptr<MuPatTrack> newCandidate;
                // loop over segments
                for (MuPatSegment* seg_1 : segVec) {
                    // select segments is different chamber
                    if (seg->chIndex == seg_1->chIndex) continue;
       
                    if (!newCandidate) {
                        std::unique_ptr<Trk::Track> trkTrkCan = std::make_unique<Trk::Track>(*track);
                        if (trkCan) {
                            // copy candidate and add segment
                            newCandidate = std::make_unique<MuPatTrack>(*trkCan);
                            m_candidateHandler->extendWithSegment(*newCandidate, *seg, trkTrkCan);
                        } else if (segCan) {
                            newCandidate = m_candidateHandler->createCandidate(*segCan, *seg, trkTrkCan);
                        }
                        if (!newCandidate) break;
                    }
                    if (!m_candidateMatchingTool->match(ctx, *newCandidate, *seg_1, tightQualityCuts)) {
                        ATH_MSG_VERBOSE("track/segment combination rejected based on angular matching "
                                        << std::endl
                                        << m_printer->print(*seg->segment));
                        continue;
                    }
       
                    ATH_MSG_VERBOSE("adding SL overlap " << m_printer->print(*seg_1->segment));
                    std::unique_ptr<Trk::Track> slOverlapTrack = combine(ctx, *track, *seg_1->segment, emptyPhiHits);
                    if (!slOverlapTrack) continue;

                    // is the new track better
                    SortTracksByHitNumber sortTracks;
                    if (!sortTracks(*slOverlapTrack, *track)) {
                        ATH_MSG_VERBOSE(__FILE__<<":"<<__LINE__<<" rejecting track as new segment results in worse fit");
                        continue;
                    }
                    ATH_MSG_VERBOSE("adding SL overlap ok, new track" << m_printer->print(*slOverlapTrack) << std::endl
                                                                      << m_printer->printStations(*slOverlapTrack));

                    track.swap(slOverlapTrack);
                    usedSegments.insert(seg_1);
                    slSegments[seg] = seg_1;
                    break;
                }
            }

            ATH_MSG_VERBOSE(" Track found " << m_printer->print(*track)<<std::endl<<m_printer->printMeasurements(*track));

            // add new solution
            extensions.push_back(std::make_pair(seg, std::move(track)));

        }  // for (sit)

        // loop over solutions and add them
        if (extensions.size() >= 1) {
            candidates.reserve(extensions.size());

            // additional check in case the candidate is a MuPatTrack
            MuPatTrack* trkCan = dynamic_cast<MuPatTrack*>(&candidate);
            MuPatSegment* segCan = dynamic_cast<MuPatSegment*>(&candidate);

            // if more than 1 extensions are found, first add the copies
            // start from the second one to make copies based on the existing candidates
            for (std::pair<MuPatSegment*, std::unique_ptr<Trk::Track> >& ext_itr : extensions) {
                std::unique_ptr<MuPatTrack> newCandidate;
                if (trkCan) {
                    // copy candidate and add segment
                    newCandidate = std::make_unique<MuPatTrack>(*trkCan);                    
                    m_candidateHandler->extendWithSegment(*newCandidate, *ext_itr.first, ext_itr.second);
                } else if (segCan) {
                    newCandidate = m_candidateHandler->createCandidate(*segCan, *ext_itr.first, ext_itr.second);
                }
                ATH_MSG_DEBUG(" " << m_printer->print(*ext_itr.first->segment));
                MuPatSegment* slOverlap = slSegments[ext_itr.first];

                if (slOverlap) {
                    ATH_MSG_DEBUG("SLOverlap " << m_printer->print(*slOverlap->segment));
                    // hack to allow me to add a second segment without changing the track
                    std::unique_ptr<Trk::Track> nullTrack;
                    newCandidate->addSegment(slOverlap, nullTrack);
                }
                candidates.push_back(std::move(newCandidate));

                ATH_MSG_DEBUG(" creating new candidate " << candidates.back().get() << std::endl
                                                         << m_printer->print(candidates.back()->track()) << std::endl
                                                         << m_printer->printStations(candidates.back()->track()));
            }
        }
        return candidates;
    }

    bool MooTrackBuilder::isSplitTrack(const EventContext& ctx, const Trk::Track& track1, const Trk::Track& track2) const {
        // some loose association cuts
        const DataVector<const Trk::TrackParameters>* parsVec1 = track1.trackParameters();
        if (!parsVec1 || parsVec1->empty()) {
            ATH_MSG_WARNING(" isSplitTrack::Track without parameters! ");
            return false;
        }
        const Trk::TrackParameters* pars1 = parsVec1->front();
        if (!pars1) {
            ATH_MSG_WARNING(" isSplitTrack::Track without NULL pointer in parameter vector! ");
            return false;
        }

        const DataVector<const Trk::TrackParameters>* parsVec2 = track2.trackParameters();
        if (!parsVec2 || parsVec2->empty()) {
            ATH_MSG_WARNING(" isSplitTrack::Track without parameters! ");
            return false;
        }
        const Trk::TrackParameters* pars2 = parsVec2->front();
        if (!pars2) {
            ATH_MSG_WARNING(" isSplitTrack::Track without NULL pointer in parameter vector! ");
            return false;
        }

        if (!m_candidateMatchingTool->sameSide(pars1->momentum().unit(), pars1->position(), pars2->position(), true)) {
            ATH_MSG_DEBUG(" tracks in opposite hemispheres ");
            return false;
        }

        double sinTheta1 = sin(pars1->momentum().theta());
        double sinTheta2 = sin(pars2->momentum().theta());
        double deltaSinTheta = sinTheta1 - sinTheta2;
        if (std::abs(deltaSinTheta) > 1.) {
            ATH_MSG_DEBUG(" too large opening angle in theta " << deltaSinTheta);
            // return false;
        }
        double sinPhi1 = sin(pars1->momentum().phi());
        double sinPhi2 = sin(pars2->momentum().phi());
        double deltaSinPhi = sinPhi1 - sinPhi2;
        if (std::abs(deltaSinPhi) > 1.) {
            ATH_MSG_DEBUG(" too large opening angle in phi " << deltaSinPhi);
            // return false;
        }

        const Trk::Track* referenceTrack = nullptr;
        const Trk::Track* otherTrack = nullptr;

        // first check whether the tracks have a momentum measurement
        bool isSL1 = m_edmHelperSvc->isSLTrack(track1);
        bool isSL2 = m_edmHelperSvc->isSLTrack(track2);

        // now decide which track to use as reference
        if (isSL1 && !isSL2) {
            referenceTrack = &track2;
            otherTrack = &track1;
        } else if (!isSL1 && isSL2) {
            referenceTrack = &track1;
            otherTrack = &track2;
        } else {
            SortTracksByHitNumber sortTracks;
            bool pickFirst = sortTracks(track1, track2);
            if (pickFirst) {
                referenceTrack = &track1;
                otherTrack = &track2;
            } else {
                referenceTrack = &track2;
                otherTrack = &track1;
            }
        }

        ATH_MSG_DEBUG(" close tracks " << std::endl << m_printer->print(*referenceTrack) << std::endl << m_printer->print(*otherTrack));

        // get iterators to TSOSs
        const Trk::TrackStates* statesRef = referenceTrack->trackStateOnSurfaces();
        if (!statesRef) {
            ATH_MSG_WARNING(" track without states, cannot perform cleaning ");
            return false;
        }
        Trk::TrackStates::const_iterator refTSOS = statesRef->begin();
        Trk::TrackStates::const_iterator refTSOS_end = statesRef->end();

        const Trk::TrackStates* statesOther = otherTrack->trackStateOnSurfaces();
        if (!statesOther) {
            ATH_MSG_WARNING(" track without states, cannot perform cleaning ");
            return false;
        }
        Trk::TrackStates::const_iterator otherTSOS = statesOther->begin();
        Trk::TrackStates::const_iterator otherTSOS_end = statesOther->end();

        DistanceAlongParameters distAlongPars;

        unsigned int nmatching(0);
        unsigned int noff(0);

        // keep track of previous distance and parameters as well
        double prevDist = 1e10;
        const Trk::TrackParameters* prevPars = nullptr;

        // now loop over the TSOSs of both tracks and compare hit by hit
        while (refTSOS != refTSOS_end && otherTSOS != otherTSOS_end) {
            const Trk::TrackParameters* parsRef = (*refTSOS)->trackParameters();
            if (!parsRef) {
                ++refTSOS;
                continue;
            }

            const Trk::TrackParameters* parsOther = (*otherTSOS)->trackParameters();
            if (!parsOther) {
                ++otherTSOS;
                continue;
            }

            double dist = distAlongPars(*parsRef, *parsOther);

            if (dist > 0.) {
                prevDist = dist;
                prevPars = parsRef;
                ++refTSOS;
                continue;
            } else {
                const Trk::TrackParameters* closestPars = nullptr;
                if (prevPars && std::abs(prevDist) < std::abs(dist)) {
                    closestPars = prevPars;
                } else {
                    closestPars = parsRef;
                }

                // check whether state is a measurement
                const Trk::MeasurementBase* meas = (*otherTSOS)->measurementOnTrack();
                if (meas && (*otherTSOS)->type(Trk::TrackStateOnSurface::Measurement)) {
                    Identifier id = m_edmHelperSvc->getIdentifier(*meas);
                    // skip pseudo measurements
                    if (!id.is_valid()) {
                        prevDist = dist;
                        prevPars = parsRef;
                        ++otherTSOS;
                        continue;
                    }
                    if (msgLvl(MSG::VERBOSE)) msg(MSG::VERBOSE) << m_idHelperSvc->toString(id);
                    // unique ptr ownership retained. Original code deleted impactPars
                    auto impactPars =
                        m_propagator->propagate(ctx, *closestPars, meas->associatedSurface(), 
                                                Trk::anyDirection, false, m_magFieldProperties);
                    if (impactPars) {
                        double residual = 1e10;
                        double pull = 1e10;
                        // pointer to resPull
                        std::unique_ptr<Trk::ResidualPull> resPull =
                            m_pullCalculator->residualPull(meas, impactPars.get(), Trk::ResidualPull::Unbiased);
                        if (resPull && resPull->pull().size() == 1) {
                            if (msgLvl(MSG::VERBOSE)) msg(MSG::VERBOSE) << "  residual " << m_printer->print(*resPull);
                            residual = resPull->residual().front();
                            pull = resPull->pull().front();
                        } else {
                            ATH_MSG_WARNING("failed to calculate residual and pull");
                        }

                        bool inBounds = false;
                        Amg::Vector2D LocVec2D;
                        bool ok = meas->associatedSurface().globalToLocal(impactPars->position(), impactPars->momentum(), LocVec2D);
                        // delete impactPars;
                        if (ok) {
                            if (msgLvl(MSG::VERBOSE))
                                msg(MSG::VERBOSE) << "  lpos (" << LocVec2D[Trk::locX] << "," << LocVec2D[Trk::locY] << ")";
                            double tol1 = 50.;
                            double tol2 = tol1;
                            Identifier id = m_edmHelperSvc->getIdentifier(*meas);
                            if (msgLvl(MSG::VERBOSE) && m_idHelperSvc->isMdt(id)) {
                                const MdtDriftCircleOnTrack* mdt = dynamic_cast<const MdtDriftCircleOnTrack*>(meas);
                                if (mdt) {
                                    int layer = m_idHelperSvc->mdtIdHelper().tubeLayer(id);
                                    int tube = m_idHelperSvc->mdtIdHelper().tube(id);
                                    double halfTubeLen = 0.5 * mdt->detectorElement()->getActiveTubeLength(layer, tube);
                                    if (msgLvl(MSG::VERBOSE)) msg(MSG::VERBOSE) << "  range " << halfTubeLen;
                                }
                            }

                            // for MM, perform the bound check from the detector element to take into account edge passivation
                            const MMClusterOnTrack* mmClusterOnTrack = dynamic_cast<const MMClusterOnTrack*>(meas);
                            if (mmClusterOnTrack) {
                                inBounds = mmClusterOnTrack->detectorElement()->insideActiveBounds(id, LocVec2D, tol1, tol2);
                            } else {
                                inBounds = meas->associatedSurface().insideBounds(LocVec2D, tol1, tol2);
                            }

                            if (msgLvl(MSG::VERBOSE)) {
                                if (inBounds)
                                    msg(MSG::VERBOSE) << " inBounds ";
                                else
                                    msg(MSG::VERBOSE) << " outBounds ";
                            }
                        } else {
                            ATH_MSG_WARNING("globalToLocal failed");
                        }

                        if (inBounds && (std::abs(residual) < 20. || std::abs(pull) < 10.)) {
                            ATH_MSG_VERBOSE(" --> matching ");
                            ++nmatching;
                        } else {
                            ATH_MSG_VERBOSE(" --> off ");
                            ++noff;
                        }

                    } else {
                        ATH_MSG_DEBUG("failed to extrapolate parameters to surface");
                    }
                }

                prevDist = dist;
                prevPars = parsRef;
                ++otherTSOS;
                continue;
            }
        }

        // if more hits are compatible with reference track than are not consider as split track
        if (nmatching > noff) return true;

        return false;
    }

    TrackCollection* MooTrackBuilder::mergeSplitTracks(const EventContext& ctx, const TrackCollection& tracks) const {
        // vector to store good track, boolean is used to identify whether the track was created in this routine or is from the collection
        std::vector<std::pair<bool, std::unique_ptr<Trk::Track> > > goodTracks;
        goodTracks.reserve(tracks.size());
        bool foundSplitTracks = false;

        ATH_MSG_DEBUG(" trying to merge split tracks, collection size " << tracks.size());

        // loop over tracks
        for (const Trk::Track* in_track : tracks) {
            // pointer to merged track
            std::unique_ptr<Trk::Track> mergedTrack;

            // compare them to all good tracks and look for split tracks
            for (std::pair<bool, std::unique_ptr<Trk::Track>>& good_trk : goodTracks) {
                // check whether track is split
                bool isSplit = isSplitTrack(ctx, *good_trk.second, *in_track);
                if (isSplit) {
                    // if we found a potential split track, try to combine them
                    std::unique_ptr<Trk::Track> track1 = std::make_unique<Trk::Track>(*good_trk.second);
                    std::unique_ptr<Trk::Track> track2 = std::make_unique<Trk::Track>(*in_track);
                    std::unique_ptr<MuPatTrack> can1 = m_candidateHandler->createCandidate(track1);
                    std::unique_ptr<MuPatTrack> can2 = m_candidateHandler->createCandidate(track2);
                    mergedTrack = combine(ctx, *can1, *can2, emptyPhiHits);

                    // we have found a split track and have successfully merged it
                    // replace the track in goodTracks with the new one
                    if (mergedTrack) {
                        ATH_MSG_DEBUG(" origninal tracks " << std::endl
                                                           << m_printer->print(*good_trk.second) << std::endl
                                                           << m_printer->printStations(*good_trk.second) << std::endl
                                                           << m_printer->print(*in_track) << std::endl
                                                           << m_printer->printStations(*in_track) << std::endl
                                                           << " merged track " << std::endl
                                                           << m_printer->print(*mergedTrack) << std::endl
                                                           << m_printer->printStations(*mergedTrack));
                        foundSplitTracks = true;
                        // check whether this is a new track, if so delete the old one before overwriting it
                        good_trk.first = true;
                        good_trk.second.swap(mergedTrack);
                        break;
                    } else {
                        ATH_MSG_VERBOSE(" failed to merge tracks " << std::endl
                                                                   << m_printer->print(*good_trk.second) << std::endl
                                                                   << m_printer->printStations(*good_trk.second) << std::endl
                                                                   << m_printer->print(*in_track) << std::endl
                                                                   << m_printer->printStations(*in_track));
                    }
                }
            }

            // if this track was not merged with another track insert it into goodTracks
            if (!mergedTrack) {
                std::unique_ptr<Trk::Track> newTrack = std::make_unique<Trk::Track>(*in_track);
                goodTracks.push_back(std::make_pair(false, std::move(newTrack)));
            }
        }

        // did we find any?
        if (!foundSplitTracks) return nullptr;
        // loop over the new track vector and create a new TrackCollection
        TrackCollection* newTracks = new TrackCollection();
        newTracks->reserve(goodTracks.size());
        for (std::pair<bool, std::unique_ptr<Trk::Track>>& good_trk: goodTracks) {
            // TrackCollection will take ownership
            newTracks->push_back(std::move(good_trk.second));
        }
        return newTracks;
    }

}  // namespace Muon
