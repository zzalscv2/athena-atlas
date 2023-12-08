/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonTrackSteering.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <set>
#include <sstream>
#include <string>
#include <utility>

#include "FourMomUtils/xAODP4Helpers.h"
#include "MuPatPrimitives/MuPatCandidateBase.h"
#include "MuPatPrimitives/MuPatSegment.h"
#include "MuPatPrimitives/MuPatTrack.h"
#include "MuonSegment/MuonSegment.h"
#include "MuonSegment/MuonSegmentCombination.h"
#include "MuonTrackMakerUtils/MuonTrackMakerStlTools.h"
#include "MuonTrackSteeringStrategy.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkSegment/SegmentCollection.h"
#include "TrkTrack/TrackCollection.h"
namespace Muon {

    std::string print(const MuPatSegment& /* seg */) { return ""; }

    std::string print(const std::vector<MuPatSegment*>& segVec) {
        std::ostringstream s;
        for (const MuPatSegment* sit : segVec) { s << " " << print(*sit); }
        return s.str();
    }

    std::string print(const MuPatTrack& track) {
        std::ostringstream s;
        s << "Track:" << print(track.segments());
        return s.str();
    }

    std::string print(const std::vector<std::unique_ptr<MuPatTrack> >& tracks) {
        std::ostringstream s;
        for (const std::unique_ptr<MuPatTrack>& tit : tracks) s << std::endl << print(*tit);

        return s.str();
    }
    //----------------------------------------------------------------------------------------------------------

    MuonTrackSteering::MuonTrackSteering(const std::string& t, const std::string& n, const IInterface* p) :
        AthAlgTool(t, n, p), m_combinedSLOverlaps(false) {
        declareInterface<IMuonTrackFinder>(this);

        declareProperty("StrategyList", m_stringStrategies, "List of strategies to be used by the track steering");
        declareProperty("SegSeedQCut", m_segQCut[0] = -2, "Required quality for segments to be a seed");
        declareProperty("Seg2ndQCut", m_segQCut[1] = -2, "Required quality for segments to be the second on a track");
        declareProperty("SegOtherQCut", m_segQCut[2] = -2, "Required quality for segments to be added to a track");
        declareProperty("OutputSingleStationTracks", m_outputSingleStationTracks = false);
        declareProperty("DoSummary", m_doSummary = false);
        declareProperty("UseTightSegmentMatching", m_useTightMatching = true);
        declareProperty("SegmentThreshold", m_segThreshold = 8);
        declareProperty("OnlyMdtSeeding", m_onlyMDTSeeding = true);
    }

    StatusCode MuonTrackSteering::initialize() {
        ATH_CHECK(m_edmHelperSvc.retrieve());
        ATH_CHECK(m_printer.retrieve());
        ATH_CHECK(m_candidateTool.retrieve());
        ATH_CHECK(m_candidateMatchingTool.retrieve());
        ATH_CHECK(m_trackBTool.retrieve());
        ATH_CHECK(m_ambiTool.retrieve());
        ATH_CHECK(m_mooBTool.retrieve());
        ATH_CHECK(m_trackRefineTool.retrieve());
        ATH_CHECK(m_trackSummaryTool.retrieve());
        ATH_CHECK(decodeStrategyVector(m_stringStrategies));
        if (m_outputSingleStationTracks) ATH_MSG_INFO("Single station track enabled ");
        ATH_CHECK(m_segmentFitter.retrieve(DisableTool{!m_outputSingleStationTracks}));
        ATH_CHECK(m_muonHoleRecoverTool.retrieve(DisableTool{!m_outputSingleStationTracks}));
        ATH_CHECK(m_trackSelector.retrieve(DisableTool{m_trackSelector.empty()}));
        if (!m_trackSelector.empty()) ATH_MSG_INFO("Track selection enabled: " << m_trackSelector);
        
        return StatusCode::SUCCESS;
    }

    std::unique_ptr<TrackCollection> MuonTrackSteering::find(const EventContext& ctx, const MuonSegmentCollection& coll) const {
        GarbageContainer trash_bin{};
        trash_bin.reserve(150);

        std::unique_ptr<TrackCollection> result = std::make_unique<TrackCollection>();

        SegColVec chamberSegments(MuonStationIndex::ChIndexMax);  // <! Segments sorted per Chamber
        SegColVec stationSegments(MuonStationIndex::StIndexMax);  // <! Segments sorted per station
        ChSet chambersWithSegments;
        StSet stationsWithSegments;
        // Extract segments into work arrays
        if (extractSegments(ctx, coll, chamberSegments, stationSegments, chambersWithSegments, stationsWithSegments, trash_bin)) {
            // Perform the actual track finding
            result = findTracks(ctx, chamberSegments, stationSegments);
        }
        return result;
    }

    bool MuonTrackSteering::extractSegments(const EventContext& ctx, const MuonSegmentCollection& coll, SegColVec& chamberSegments, SegColVec& stationSegments,
                                            ChSet& chambersWithSegments, StSet& stationsWithSegments, GarbageContainer& trash_bin) const {
        if (coll.empty()) return false;

        ATH_MSG_DEBUG("New collection " << coll.size());

        // Sort the input collection by chamber & station IDs
        for (const MuonSegment* segment : coll) {
            ATH_MSG_DEBUG("Adding segment ");
            std::unique_ptr<MuPatSegment> aSeg = m_candidateTool->createSegInfo(ctx, *segment);
            ATH_MSG_DEBUG(" -> MuPatSegment " << m_candidateTool->print(*aSeg));

            MuonStationIndex::ChIndex chIndex = aSeg->chIndex;
            MuonStationIndex::StIndex stIndex = aSeg->stIndex;
            if (chIndex < 0 || stIndex < 0) {
                ATH_MSG_WARNING("Chamber or station index invalid:" << m_candidateTool->print(*aSeg));
                continue;
            }
            chambersWithSegments.insert(chIndex);
            stationsWithSegments.insert(stIndex);

            std::vector<MuPatSegment*>& segments = chamberSegments[chIndex];
            segments.push_back(aSeg.get());
            if (!m_combinedSLOverlaps) {
                std::vector<MuPatSegment*>& segments2 = stationSegments[stIndex];
                segments2.push_back(aSeg.get());
            }
            trash_bin.push_back(std::move(aSeg));
        }

        if (m_combinedSLOverlaps) {
            combineOverlapSegments(ctx, chamberSegments[MuonStationIndex::BIS], chamberSegments[MuonStationIndex::BIL], stationSegments,
                                   stationsWithSegments, trash_bin);
            combineOverlapSegments(ctx, chamberSegments[MuonStationIndex::BMS], chamberSegments[MuonStationIndex::BML], stationSegments,
                                   stationsWithSegments, trash_bin);
            combineOverlapSegments(ctx, chamberSegments[MuonStationIndex::BOS], chamberSegments[MuonStationIndex::BOL], stationSegments,
                                   stationsWithSegments, trash_bin);
            combineOverlapSegments(ctx, chamberSegments[MuonStationIndex::EIS], chamberSegments[MuonStationIndex::EIL], stationSegments,
                                   stationsWithSegments, trash_bin);
            combineOverlapSegments(ctx, chamberSegments[MuonStationIndex::EMS], chamberSegments[MuonStationIndex::EML], stationSegments,
                                   stationsWithSegments, trash_bin);
            combineOverlapSegments(ctx, chamberSegments[MuonStationIndex::EOS], chamberSegments[MuonStationIndex::EOL], stationSegments,
                                   stationsWithSegments, trash_bin);
            combineOverlapSegments(ctx, chamberSegments[MuonStationIndex::EES], chamberSegments[MuonStationIndex::EEL], stationSegments,
                                   stationsWithSegments, trash_bin);
            combineOverlapSegments(ctx, chamberSegments[MuonStationIndex::CSS], chamberSegments[MuonStationIndex::CSL], stationSegments,
                                   stationsWithSegments, trash_bin);
            std::vector<MuPatSegment*>& segments = chamberSegments[MuonStationIndex::BEE];
            if (!segments.empty()) {
                chambersWithSegments.insert(MuonStationIndex::BEE);
                stationsWithSegments.insert(MuonStationIndex::BE);
                std::vector<MuPatSegment*>& segs = stationSegments[MuonStationIndex::BE];
                segs.insert(segs.end(), segments.begin(), segments.end());
            }
        }
        return true;
    }

    void MuonTrackSteering::combineOverlapSegments(const EventContext& ctx, std::vector<MuPatSegment*>& ch1, std::vector<MuPatSegment*>& ch2,
                                                   SegColVec& stationSegments, StSet& stationsWithSegments,
                                                   GarbageContainer& trash_bin) const {
        /** try to find small/large overlaps, insert segment into stationVec */

        // if both empty there is nothing to be done
        if (ch1.empty() && ch2.empty()) return;

        // get station index from the first segment in the first non empty vector
        MuonStationIndex::StIndex stIndex = !ch1.empty() ? ch1.front()->stIndex : ch2.front()->stIndex;

        SegCol& stationVec = stationSegments[stIndex];

        // vector to flag entries in the second station that were matched
        std::vector<bool> wasMatched2(ch2.size(), false);

        // loop over all possible combinations
        for (MuPatSegment* sit1 : ch1) {
            // do not combine poor quality segments
            int qualityLevel1 = ch1.size() > 5 ? 1 : 2;
            if (sit1->quality < qualityLevel1) {
                ATH_MSG_VERBOSE("resolveSLOverlaps::bad segment1 q: " << sit1->quality << " cut " << qualityLevel1 << std::endl
                                                                      << m_printer->print(*sit1->segment));
                stationVec.push_back(sit1);
                continue;
            }

            bool wasMatched1 = false;

            // apply looser cuts as we perform matching
            int qualityLevel2 = ch2.size() > 5 ? 1 : 2;
            /// Start with -1 as the first operation in the loop over the second set of chambers
            /// is the counter incrementation
            int idx_ch2 = -1;
            for (MuPatSegment* sit2 : ch2) {
                ++idx_ch2;
                // do not combine poor quality segments AND require at least one of the segments to have a quality beter than 1
                if (sit2->quality < qualityLevel2) {
                    ATH_MSG_VERBOSE("resolveSLOverlaps::bad segment2:  q " << sit2->quality << " cut " << qualityLevel2 << std::endl
                                                                           << m_printer->print(*sit2->segment));
                    continue;
                }
                if (sit1->quality < 2 && sit2->quality < 2) {
                    ATH_MSG_VERBOSE("resolveSLOverlaps:: combination of insufficient quality " << std::endl
                                                                                               << " q1 " << sit1->quality << " q2 "
                                                                                               << sit1->quality);
                    continue;
                }

                ATH_MSG_VERBOSE(" combining entries: " << std::endl
                                                       << m_printer->print(*sit1->segment) << std::endl
                                                       << m_printer->print(*sit2->segment));

                if (!m_candidateMatchingTool->match(ctx, *sit1, *sit2, false)) {
                    ATH_MSG_VERBOSE(" overlap combination rejected based on matching" << std::endl << m_printer->print(*sit2->segment));
                    continue;
                }

                // create MuonSegment
                static const Muon::IMuonSegmentTrackBuilder::PrepVec emptyPhiHits{};
                std::unique_ptr<MuonSegment> newseg{m_mooBTool->combineToSegment(ctx, *sit1, *sit2, emptyPhiHits)};
                if (!newseg) {
                    ATH_MSG_DEBUG(" Combination of segments failed ");
                    continue;
                }
                const Trk::FitQuality* fq = newseg->fitQuality();
                if (!fq || fq->numberDoF() == 0) {
                    ATH_MSG_WARNING(" no fit quality, dropping segment ");
                    continue;
                }
                if (fq->chiSquared() / fq->numberDoF() > 2.5) {
                    ATH_MSG_DEBUG("bad fit quality, dropping segment " << fq->chiSquared() / fq->numberDoF());
                    continue;
                }
                std::unique_ptr<MuPatSegment> segInfo = m_candidateTool->createSegInfo(ctx, *newseg);
                // check whether segment of good quality AND that its quality is equal or better than the input segments
                if (segInfo->quality < 2 || (segInfo->quality < sit1->quality || segInfo->quality < sit2->quality)) {
                    ATH_MSG_VERBOSE("resolveSLOverlaps::bad segment " << std::endl << m_printer->print(*segInfo->segment));
                    continue;
                }                
                int shared_eta = 0, shared_phi = 0;  // check for hits shared between segments

                const MuPatSegment* const_sit1 = sit1;
                const MuPatSegment* const_sit2 = sit2;
                for (const MuPatHitPtr& hit_ch1 : const_sit1->hitList()) {
                    for (const MuPatHitPtr& hit_ch2 : const_sit2->hitList()) {
                        if (hit_ch1->info().id == hit_ch2->info().id) {
                            if (hit_ch1->info().measuresPhi)
                                shared_phi++;
                            else
                                shared_eta++;
                        }
                    }
                }

                if (sit1->etaHits().size() + sit2->etaHits().size() - shared_eta - segInfo->etaHits().size() > 1) {
                    ATH_MSG_VERBOSE("resolveSLOverlaps::more than one eta measurement removed, dropping track "
                                    << std::endl
                                    << m_printer->print(*segInfo->segment));
                    continue;
                }

                int phiHitDiff = sit1->phiHits().size() + sit2->phiHits().size() - shared_phi - segInfo->phiHits().size();
                if (phiHitDiff > 1 || (sit1->phiHits().size() + sit2->phiHits().size() > 0 && segInfo->phiHits().empty())) {
                    ATH_MSG_VERBOSE("resolveSLOverlaps::more than one phi measurement removed, dropping track "
                                    << std::endl
                                    << m_printer->print(*segInfo->segment));
                    continue;
                }
                /// Actually this should be always 1. What should it conceptionally reject?
                /// The angle between the combined segment and the segment from the first collection?
                double cosPointingAngle = (newseg->globalPosition().x() * newseg->globalDirection().x() +
                                           newseg->globalPosition().y() * newseg->globalDirection().y()) /
                                          (newseg->globalPosition().perp() * newseg->globalDirection().perp());
                if (cosPointingAngle < 0.995) {
                    ATH_MSG_VERBOSE("resolveSLOverlaps: rejected due to too large pointing angle " << std::endl
                                                                                                   << m_printer->print(*segInfo->segment));
                    continue;
                }
                ATH_MSG_VERBOSE("created SL overlap segment: cos pointing " << cosPointingAngle << std::endl
                                                                            << m_printer->print(*segInfo->segment));

                // flag segments as matched
                wasMatched1 = true;
                wasMatched2[idx_ch2] = true;

                // add segment
                stationVec.push_back(segInfo.get());
                trash_bin.push_back(std::move(newseg));
                trash_bin.push_back(std::move(segInfo));
                
            }

            // if entry was not associated with entry in other station add it to entries
            if (!wasMatched1) { stationVec.push_back(sit1); }
        }

        // loop over entries in second station and add unassociated entries to candidate entries
        for (unsigned int i = 0; i < wasMatched2.size(); ++i) {
            if (!wasMatched2[i]) { stationVec.push_back(ch2[i]); }
        }

        // add station to list of stations with segments
        if (!stationVec.empty()) { stationsWithSegments.insert(stIndex); }

        // sort segment according to their quality
        std::stable_sort(stationVec.begin(), stationVec.end(), SortSegInfoByQuality());
    }

    //-----------------------------------------------------------------------------------------------------------

    std::unique_ptr<TrackCollection> MuonTrackSteering::findTracks(const EventContext& ctx, SegColVec& chamberSegments, SegColVec& stationSegments) const {
        // Very basic : output all of the segments we are starting with
        ATH_MSG_DEBUG("List of all strategies: " << m_strategies.size());
        for (unsigned int i = 0; i < m_strategies.size(); ++i) ATH_MSG_DEBUG((*(m_strategies[i])));

        std::vector<std::unique_ptr<MuPatTrack> > resultAll;

        // Outermost loop over strategies!
        for (unsigned int i = 0; i < m_strategies.size(); ++i) {
            if (!m_strategies[i]) continue;  // Check for empty strategy pointer

            const MuonTrackSteeringStrategy& strategy = *m_strategies[i];

            std::vector<std::unique_ptr<MuPatTrack> > result;

            // Segments that will be looped over...
            SegColVec mySegColVec(strategy.getAll().size());

            ATH_MSG_VERBOSE("Segments to be looped on: " << mySegColVec.size());

            std::set<MuonStationIndex::StIndex> stations;
            // Preprocessing : loop over layers
            for (unsigned int lit = 0; lit < strategy.getAll().size(); ++lit) {
                std::vector<MuonStationIndex::ChIndex> chambers = strategy.getCh(lit);

                // Optional : combine segments in the same station but different chambers
                if (strategy.option(MuonTrackSteeringStrategy::CombineSegInStation)) {
                    // Loop over stations in the layer
                    for (unsigned int chin = 0; chin < chambers.size(); ++chin) {
                        // get station index for the chamber
                        MuonStationIndex::StIndex stIndex = MuonStationIndex::toStationIndex(chambers[chin]);

                        // skip those that are already included
                        if (stations.count(stIndex)) continue;
                        SegCol& segments = stationSegments[stIndex];
                        // Add all of the MuPatSegments into the list for that layer
                        // db

                        if (strategy.option(MuonTrackSteeringStrategy::BarrelEndcapFilter)) {
                            // SegCol filteredSegments;
                            for (unsigned int iseg = 0; iseg < segments.size(); iseg++) {
                                double thetaSeg = std::abs((*segments[iseg]).segment->globalPosition().theta());

                                // only select segments in barrel/endcap overlap
                                if ((0.74159 > thetaSeg && thetaSeg > 0.51159) || (2.63 > thetaSeg && thetaSeg > 2.40))
                                    mySegColVec[lit].push_back(segments[iseg]);
                            }
                        } else {
                            mySegColVec[lit].insert(mySegColVec[lit].end(), segments.begin(), segments.end());
                        }

                        stations.insert(stIndex);
                    }  // End of loop over chambers

                } else {
                    // Loop over stations in the layer
                    for (unsigned int chin = 0; chin < chambers.size(); ++chin) {
                        SegCol& segments = chamberSegments[chambers[chin]];
                        // Throw all of the MuPatSegments into the list for that layer
                        mySegColVec[lit].insert(mySegColVec[lit].end(), segments.begin(), segments.end());
                    }  // End of loop over chambers
                }      // End of if combine segments in a layer

            }  // End of loop over layers

            // Preprocessing step two : sort all layers' segments by quality
            for (unsigned int lit = 0; lit < mySegColVec.size(); ++lit) {
                std::stable_sort(mySegColVec[lit].begin(), mySegColVec[lit].end(), SortSegInfoByQuality());
            }

            if (m_doSummary || msgLvl(MSG::DEBUG)) {
                bool hasSegments = false;
                for (unsigned int lit = 0; lit < mySegColVec.size(); ++lit) {
                    if (!mySegColVec[lit].empty()) {
                        hasSegments = true;
                        break;
                    }
                }
                if (hasSegments) {
                    msg(m_doSummary ? MSG::INFO : MSG::DEBUG) << "For strategy: " << strategy.getName() << " segments are: ";
                    for (unsigned int lit = 0; lit < mySegColVec.size(); ++lit)
                        for (unsigned int sit = 0; sit < mySegColVec[lit].size(); ++sit)
                            msg(m_doSummary ? MSG::INFO : MSG::DEBUG) << std::endl
                                                                      << "  " << m_candidateTool->print(*(mySegColVec[lit])[sit]);
                    msg(m_doSummary ? MSG::INFO : MSG::DEBUG) << endmsg;
                }
            }

            // Hang on to whether we want to cut seeds or not
            bool cutSeeds = strategy.option(MuonTrackSteeringStrategy::CutSeedsOnTracks);

            // Now assign priority for the layers
            std::vector<unsigned int> seeds;

            // Assign seeds dynamically according to
            if (strategy.option(MuonTrackSteeringStrategy::DynamicSeeding)) {
                // Loop through layers and do a little sort
                std::vector<std::pair<int, unsigned int> > occupancy;  // layer , occ
                for (unsigned int lit = 0; lit < mySegColVec.size(); ++lit) {
                    occupancy.emplace_back(mySegColVec[lit].size(), lit);
                }
                std::stable_sort(occupancy.begin(), occupancy.end());
                for (unsigned int lit = 0; lit < occupancy.size(); ++lit) { seeds.push_back(occupancy[lit].second); }
            } else {
                seeds = strategy.seeds();
                if (seeds.empty()) {
                    for (unsigned int j = 0; j < mySegColVec.size(); ++j) seeds.push_back(j);
                }
            }
            ATH_MSG_VERBOSE("Selected seed layers " << seeds.size());

            MuPatSegment* seedSeg = nullptr;
            // Loop over seed layers
            for (unsigned int lin = 0; lin < seeds.size(); ++lin) {
                // Loop over segments in that layer
                ATH_MSG_VERBOSE("New seed layer " << lin << " segments in layer " << mySegColVec[lin].size());

                for (unsigned int sin = 0; sin < mySegColVec[lin].size(); sin++) {
                    seedSeg = mySegColVec[lin].operator[](sin);
                    if (!seedSeg) continue;  // Check for empty poinnter

                    // Optionally, if the seed is on a track we skip it
                    if (cutSeeds && seedSeg->usedInFit) continue;

                    // See if the seed passes our quality cut
                    if (seedSeg->quality < m_segQCut[0] ||
                        (m_segQCut[0] == -99 && !(seedSeg->segQuality && seedSeg->segQuality->isStrict())))
                        continue;
                    if (m_onlyMDTSeeding && !seedSeg->isMdt) continue;

                    int segsInCone = 0;
                    double phiSeed = seedSeg->segment->globalPosition().phi();
                    double etaSeed = seedSeg->segment->globalPosition().eta();
                    for (unsigned int sin2 = 0; sin2 < mySegColVec[lin].size(); sin2++) {
                        if (sin == sin2) continue;
                        MuPatSegment* seg = mySegColVec[lin].operator[](sin2);

                        if (seg->quality < m_segQCut[0] || (m_segQCut[0] == -99 && !(seg->segQuality && seg->segQuality->isStrict())))
                            continue;

                        double phiSeg = seg->segment->globalPosition().phi();
                        double etaSeg = seg->segment->globalPosition().eta();

                        double deltaPhi = xAOD::P4Helpers::deltaPhi(phiSeed, phiSeg);
                        double deltaEta = std::abs(etaSeed - etaSeg);
                        double deltaR = std::hypot(deltaPhi, deltaEta);

                        if (deltaR < 0.35) segsInCone++;
                    }
                    ATH_MSG_VERBOSE("New seed " << sin << " segments in cone " << segsInCone);

                    if (segsInCone > m_segThreshold && seedSeg->quality < m_segQCut[0] + 1) continue;

                    std::vector<std::unique_ptr<MuPatTrack> > found =
                        findTrackFromSeed(ctx, *seedSeg, *(m_strategies[i]), seeds[lin], mySegColVec);

                    ATH_MSG_VERBOSE("  Tracks for seed: " << std::endl << " --- " << m_candidateTool->print(result));
                    if (!found.empty()) {
                        result.insert(result.end(), std::make_move_iterator(found.begin()), std::make_move_iterator(found.end()));
                    }
                }  // End of loop over segments in a layer
            }      // Done with loop over seed layers

            // Post-processing : refinement
            if (!result.empty() && strategy.option(MuonTrackSteeringStrategy::DoRefinement)) refineTracks(ctx, result);

            // Post-processing : ambiguity resolution
            if (msgLvl(MSG::DEBUG) && !result.empty()) {
                msg(MSG::DEBUG) << "Initial track collection for strategy: " << strategy.getName() << "  " << m_candidateTool->print(result)
                                << endmsg;
            }

            if (!result.empty() && strategy.option(MuonTrackSteeringStrategy::DoAmbiSolving)) solveAmbiguities(result);

            if (!result.empty())
                resultAll.insert(resultAll.end(), std::make_move_iterator(result.begin()), std::make_move_iterator(result.end()));

        }  // Done with loop over strategies

        if (!resultAll.empty()) { solveAmbiguities(resultAll); }

        if (m_outputSingleStationTracks) {
            SegCol& emSegments = stationSegments[MuonStationIndex::EM];
            // loop over segments in EM stations
            if (!emSegments.empty()) {
                for (MuPatSegment* sit : emSegments) {
                    // skip segments that are associated to a track
                    if (!sit->tracks().empty()) continue;

                    // only take highest quality segments
                    if (sit->quality < 2) continue;

                    // fit segment and add the track if fit ok
                    std::unique_ptr<Trk::Track> segmentTrack(m_segmentFitter->fit(*sit->segment));
                    if (segmentTrack) {
                        // Try to recover hits on the track
                        std::unique_ptr<Trk::Track> recoveredTrack(m_muonHoleRecoverTool->recover(*segmentTrack, ctx));
                        if (recoveredTrack) segmentTrack.swap(recoveredTrack);

                        // generate a track summary for this track
                        if (m_trackSummaryTool.isEnabled()) {
                            m_trackSummaryTool->computeAndReplaceTrackSummary(ctx, *segmentTrack, false);
                        }

                        std::unique_ptr<MuPatTrack> can = m_candidateTool->createCandidate(*sit, segmentTrack);
                        if (can)
                            resultAll.push_back(std::move(can));
                        else
                            ATH_MSG_WARNING("Failed to create MuPatTrack");
                    }
                }
            }
        }

        // Output all the tracks that we are ending with
        if (!resultAll.empty()) {
            if (m_doSummary)
                ATH_MSG_INFO("Final Output : " << m_candidateTool->print(resultAll) << endmsg);
            else
                ATH_MSG_DEBUG("Final Output : " << m_candidateTool->print(resultAll) << endmsg);
        }
        std::unique_ptr<TrackCollection> finalTrack = nullptr;
        if (!resultAll.empty()) { finalTrack = selectTracks(resultAll); }

        return finalTrack;
    }

    std::vector<std::unique_ptr<MuPatTrack> > MuonTrackSteering::findTrackFromSeed(const EventContext& ctx, MuPatSegment& seedSeg,
                                                                                   const MuonTrackSteeringStrategy& strat,
                                                                                   const unsigned int layer, const SegColVec& segs) const {
        // the resulting vector of tracks to be returned
        std::vector<std::unique_ptr<MuPatTrack> > result;
        ATH_MSG_DEBUG("Working on seed: " << std::endl << " --- " << m_candidateTool->print(seedSeg));
        const unsigned int endLayer = strat.getAll().size();
        ///  Loop over layers following the seed layer
        for (unsigned int ilayer = 0; ilayer < strat.getAll().size(); ++ilayer) {
            if (ilayer == layer) continue;  // don't include the layer of the seed

            if (segs[ilayer].empty()) continue;

            std::vector<MuPatSegment*> matchedSegs;
            bool tightCuts = false;
            //
            if (m_useTightMatching) {
                double phiSeed = (seedSeg.segment)->globalPosition().phi();
                double etaSeed = (seedSeg.segment)->globalPosition().eta();

                int segsInCone = 0;
                for (unsigned int j = 0; j < segs[ilayer].size(); j++) {
                    double phiSeg = (*segs[ilayer][j]).segment->globalPosition().phi();
                    double etaSeg = (*segs[ilayer][j]).segment->globalPosition().eta();

                    double deltaPhi = xAOD::P4Helpers::deltaPhi(phiSeed, phiSeg);
                    double deltaEta = std::abs(etaSeed - etaSeg);
                    double deltaR = std::hypot(deltaPhi, deltaEta);

                    if (deltaR < 0.35) segsInCone++;
                }

                if (segsInCone > m_segThreshold) {
                    for (unsigned int j = 0; j < segs[ilayer].size(); ++j) {
                        bool isMatched = m_candidateMatchingTool->match(ctx, seedSeg, *segs[ilayer][j], true);

                        if (isMatched) matchedSegs.push_back(segs[ilayer][j]);
                    }
                    if (matchedSegs.empty()) continue;
                    tightCuts = true;
                }
            }

            std::vector<std::unique_ptr<MuPatTrack> > tracks;

            if (!matchedSegs.empty() && m_useTightMatching)
                tracks = m_trackBTool->find(ctx, seedSeg, matchedSegs);
            else
                tracks = m_trackBTool->find(ctx, seedSeg, segs[ilayer]);
            if (!tracks.empty()) {
                // if we reached the end of the sequence, we should save what we have else continue to next layer
                if (ilayer + 1 == strat.getAll().size()) {
                    result.insert(result.end(), std::make_move_iterator(tracks.begin()), std::make_move_iterator(tracks.end()));
                    break;
                }

                // loop on found tracks
                for (std::unique_ptr<MuPatTrack>& cit : tracks) {
                    unsigned int nextLayer = ilayer + 1;
                    if (nextLayer < strat.getAll().size()) {
                        int cutLevel = tightCuts ? 1 : 0;
                        std::vector<std::unique_ptr<MuPatTrack> > nextTracks =
                            extendWithLayer(ctx, *cit, segs, nextLayer, endLayer, cutLevel);
                        if (!nextTracks.empty()) {
                            result.insert(result.end(), std::make_move_iterator(nextTracks.begin()),
                                          std::make_move_iterator(nextTracks.end()));
                        } else {
                            result.push_back(std::move(cit));
                        }
                    }
                }
            }
        }

        ATH_MSG_DEBUG("Constructed " << result.size() << " tracks with strategy " << strat.getName());
        return result;
    }

    std::vector<std::unique_ptr<MuPatTrack> > MuonTrackSteering::extendWithLayer(const EventContext& ctx, MuPatTrack& candidate, const SegColVec& segs,
                                                                                 unsigned int nextlayer, const unsigned int endlayer, 
                                                                                 int cutLevel) const {
        std::vector<std::unique_ptr<MuPatTrack> > result;
        if (nextlayer < endlayer) {
            for (; nextlayer != endlayer; nextlayer++) {
                if (segs[nextlayer].empty()) continue;

                std::vector<std::unique_ptr<MuPatTrack> > nextTracks = m_trackBTool->find(ctx, candidate, segs[nextlayer]);
                if (!nextTracks.empty()) {
                    for (std::unique_ptr<MuPatTrack>& cit : nextTracks) {
                        std::vector<std::unique_ptr<MuPatTrack> > nextTracks2 =
                            extendWithLayer(ctx, *cit, segs, nextlayer + 1, endlayer, cutLevel);
                        if (!nextTracks2.empty()) {
                            result.insert(result.end(), std::make_move_iterator(nextTracks2.begin()),
                                          std::make_move_iterator(nextTracks2.end()));
                        } else {
                            result.push_back(std::move(cit));
                        }
                    }
                }
            }
        }

        return result;
    }

    //-----------------------------------------------------------------------------------------------------------
    std::unique_ptr<TrackCollection> MuonTrackSteering::selectTracks(std::vector<std::unique_ptr<MuPatTrack> >& candidates, bool takeOwnership) const {
        std::unique_ptr<TrackCollection> result = takeOwnership ?std::make_unique<TrackCollection>() : std::make_unique<TrackCollection>(SG::VIEW_ELEMENTS);
        result->reserve(candidates.size());
        for (std::unique_ptr<MuPatTrack>& cit : candidates) {
            auto & thisTrack =  cit->track();
            // if track selector is configured, use it and remove bad tracks
            if (!m_trackSelector.empty() && !m_trackSelector->decision(thisTrack)) continue;

            Trk::Track* track{nullptr};
            if (takeOwnership)
                track = new Trk::Track(thisTrack);
            else
                track = &thisTrack;
            // add track summary to this track
            if (m_trackSummaryTool.isEnabled()) { m_trackSummaryTool->computeAndReplaceTrackSummary(*track, false); }
            result->push_back(track);
        }
        return result;
    }

    void MuonTrackSteering::refineTracks(const EventContext& ctx, std::vector<std::unique_ptr<MuPatTrack> >& candidates) const {
        for (std::unique_ptr<MuPatTrack>& cit : candidates) { m_trackRefineTool->refine(ctx, *cit); }
    }

    //-----------------------------------------------------------------------------------------------------------

    void MuonTrackSteering::solveAmbiguities(std::vector<std::unique_ptr<MuPatTrack> >& tracks,
                                             const MuonTrackSteeringStrategy* /*strat*/) const {
        // the resulting vector of tracks to be returned
        std::unique_ptr<TrackCollection> trkColl(selectTracks(tracks, false));
        if (!trkColl || trkColl->empty()) { return; }

        std::unique_ptr<const TrackCollection> resolvedTracks(m_ambiTool->process(trkColl.get()));
        if (!resolvedTracks) { return; }

        ATH_MSG_DEBUG("   resolved track candidates: old size " << trkColl->size() << " new size " << resolvedTracks->size());

        std::vector<std::unique_ptr<MuPatTrack> >::iterator pat = tracks.begin();
        for (; pat != tracks.end();) {
            bool found = false;
            for (const Trk::Track* rtrk : *resolvedTracks) {
                if (&(*pat)->track() == rtrk) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                pat = tracks.erase(pat);
            } else {
                ++pat;
            }
        }

    }

    //-----------------------------------------------------------------------------------------------------------

    StatusCode MuonTrackSteering::decodeStrategyVector(const std::vector<std::string>& strategy) {
        for (unsigned int i = 0; i < strategy.size(); ++i) {
            std::unique_ptr<const MuonTrackSteeringStrategy> holder = decodeStrategy(strategy[i]);
            if (!holder) {
                // complain
                ATH_MSG_DEBUG("failed to decode strategy");
            } else {
                // flag whether segments should be combined
                if (holder->option(MuonTrackSteeringStrategy::CombineSegInStation)) m_combinedSLOverlaps = true;
                m_strategies.emplace_back(std::move(holder));
            }
        }
        return StatusCode::SUCCESS;
    }

    //-----------------------------------------------------------------------------------------------------------

    std::unique_ptr<const MuonTrackSteeringStrategy> MuonTrackSteering::decodeStrategy(const std::string& strategy) const {
        const std::string delims(" \t[],;");

        // The strategy name
        std::string name;

        // The strategy options (which should be a vector of enums, but I'll use strings now to check that I'm
        // decoding the stragegy correctly)
        std::vector<std::string> options;

        // The strategy sequence (which should be a vector of vector of station enumbs, but again I'll use
        // strings instead of enums to test that I've got the decoding correct)
        typedef std::vector<std::string> ChamberGroup;
        std::vector<ChamberGroup> sequence;
        std::string seqStr;

        bool success = false;
        std::unique_ptr<const MuonTrackSteeringStrategy> result;

        std::string::size_type length = strategy.length();

        // Extract the strategy name and options
        std::string::size_type begIdx, endIdx;
        begIdx = strategy.find_first_not_of(delims);
        if (std::string::npos != begIdx) {
            endIdx = strategy.find(':', begIdx);
            if (std::string::npos != endIdx) {
                seqStr = strategy.substr(endIdx + 1, length - endIdx - 1);
                std::string nameopt = strategy.substr(begIdx, endIdx - begIdx);
                std::string::size_type bi = nameopt.find('[');
                if (std::string::npos != bi) {
                    name = nameopt.substr(0, bi);

                    // Decode options
                    std::string::size_type ei = nameopt.find(']', bi);
                    if (std::string::npos == ei) { ei = nameopt.length(); }
                    std::string inputOpt = nameopt.substr(bi + 1, ei - bi - 1);
                    success = decodeList(inputOpt, options);
                } else {
                    name = nameopt;
                }
            }
        }
        if (msgLvl(MSG::DEBUG)) {
            ATH_MSG_DEBUG("From strat: " << strategy << " with success " << success << " end " << endIdx << " beg " << begIdx
                                         << " Name: " << name << " options: ");
            for (std::vector<std::string>::iterator oit = options.begin(); oit != options.end(); ++oit)
                msg(MSG::DEBUG) << "  " << *oit << endmsg;
        }
        // Name and options successfully decoded, now decode the sequence and groups
        if (success) {
            begIdx = endIdx + 1;
            do {
                endIdx = strategy.find(';', begIdx);
                std::string::size_type lstIdx = endIdx;
                if (std::string::npos == endIdx) { lstIdx = strategy.length(); }
                std::string grpString = strategy.substr(begIdx, lstIdx - begIdx);
                ChamberGroup group;
                success = success && decodeList(grpString, group);
                sequence.push_back(group);
                begIdx = lstIdx + 1;
            } while (std::string::npos != endIdx && success);
        }

        if (success) {
            std::vector<std::vector<MuonStationIndex::ChIndex> > path;
            for (unsigned int i = 0; i < sequence.size(); ++i) {
                std::vector<MuonStationIndex::ChIndex> idxGrp;
                for (unsigned int j = 0; j < sequence[i].size(); ++j) {
                    MuonStationIndex::ChIndex idx = MuonStationIndex::chIndex(sequence[i][j]);
                    if (MuonStationIndex::ChUnknown == idx) {
                        if (sequence[i][j] != "all" && sequence[i][j] != "ALL" && sequence[i][j] != "All") {
                            // Complain
                            ATH_MSG_WARNING("I am complaining: Bad station index.");
                        } else {  // asked for all chambers
                            idxGrp.clear();
                            for (int all = MuonStationIndex::BIS; all != MuonStationIndex::ChIndexMax; ++all)
                                idxGrp.push_back(MuonStationIndex::ChIndex(all));
                        }
                    } else {
                        idxGrp.push_back(idx);
                    }
                }
                path.push_back(idxGrp);
            }
            result = std::make_unique<MuonTrackSteeringStrategy>(name, options, path);
        }

        return result;
    }

    //-----------------------------------------------------------------------------------------------------------

    bool MuonTrackSteering::decodeList(const std::string& input, std::vector<std::string>& list) {
        bool result = true;
        std::string::size_type begIdx = 0;
        std::string::size_type endIdx = 0;
        do {
            endIdx = input.find(',', begIdx);
            std::string::size_type lstIdx = endIdx;
            if (std::string::npos == endIdx) { lstIdx = input.length(); }
            std::string item = input.substr(begIdx, lstIdx - begIdx);
            list.push_back(item);
            begIdx = lstIdx + 1;
        } while (std::string::npos != endIdx);
        return result;
    }

}  // namespace Muon
