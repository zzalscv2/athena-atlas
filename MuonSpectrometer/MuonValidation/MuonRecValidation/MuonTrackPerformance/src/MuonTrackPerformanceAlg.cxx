/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonTrackPerformanceAlg.h"

#include <fstream>
#include <iostream>
#include <vector>

#include "AtlasHepMC/GenParticle.h"
#include "AtlasHepMC/GenVertex.h"
#include "MuonSegment/MuonSegment.h"
#include "MuonSegment/MuonSegmentCombination.h"
#include "TFile.h"
#include "TrkTrack/Track.h"

MuonTrackPerformanceAlg::MuonTrackPerformanceAlg(const std::string& name, ISvcLocator* pSvcLocator) :
    AthAlgorithm(name, pSvcLocator),
    m_eventInfo(nullptr),    
    m_nevents(0),
    m_ntracks(0),
    m_nmatchedTracks(0),
    m_nmatchedTracksSecondary(0),
    m_nmatchedFakeTracks(0),
    m_nfakeTracks(0),
    m_nfakeTracksHighPt(0),
    m_nfakeTracksLowPt(0),
    m_nfakeTracksSL(0),
    m_nmissedTracks(0),
    m_nmissedTracksOneStation(0),
    m_nmissedTracksSecondary(0),
    m_nmissedTracksOneStationSecondary(0),
    m_nmissingStationMomLoss(0),
    m_nmissingStationLayer(0),
    m_nmissingCscStationLayer(0),
    m_nmissingStationLayerTrigger(0),
    m_nmissingStation(0),
    m_nwrongStationLayer(0),
    m_nwrongStationLayerTrigger(0),
    m_nwrongStation(0),
    m_ntruthTracks(0),
    m_ntruthTracksSecondary(0) {
    declareProperty("DoHistos", m_doHistos = false);
    declareProperty("DoSummary", m_doSummary = 0);
    declareProperty("DoHitResiduals", m_doHitResiduals = 0);
    declareProperty("DoTrackDebug", m_doTrackDebug = 0);
    declareProperty("DoTruth", m_doTruth = true);
    declareProperty("writeToFile", m_writeToFile = true);
    declareProperty("FileName", m_fileName = "trkPerformance.txt");
    declareProperty("UsePtCut", m_usePtCut = true);
    declareProperty("IsCombined", m_isCombined = false);
    declareProperty("MuonMomentumCutSim", m_momentumCutSim = 2000.);
    declareProperty("LowMomentumThreshold", m_momentumCut = 2000.);
    declareProperty("DoSegments", m_doSegments = false);
    declareProperty("doStau", m_doStau = false);
    declareProperty("TrackType", m_trackType = 2);
    declareProperty("ProduceEventListMissedTracks", m_doEventListMissed = 0, "0: off, 1: two station, 2: + one station");
    declareProperty("ProduceEventListIncompleteTracks", m_doEventListIncomplete = 0,
                    "0: off, 1: missing precision layer, 2: +missing chamber");
    declareProperty("ProduceEventListFakeTracks", m_doEventListFake = 0, "0: off, 1: high pt, 2: +low pt, 3: +SL");
    declareProperty("MinMdtHits", m_minMdtHits = 3);
    declareProperty("MinCscEtaHits", m_minCscEtaHits = 3);
    declareProperty("MinCscPhiHits", m_minCscPhiHits = 3);
    declareProperty("MinRPCEtaHits", m_minRpcEtaHits = 1);
    declareProperty("MinRPCPhiits", m_minRpcPhiHits = 1);
    declareProperty("MinTGCEtaHits", m_minTgcEtaHits = 1);
    declareProperty("MinTGCPhiHits", m_minTgcPhiHits = 1);
    declareProperty("MinSTGCEtaHits", m_minsTgcEtaHits = 1);
    declareProperty("MinSTGCPhiHits", m_minsTgcPhiHits = 1);
    declareProperty("MinMMEtaHits", m_minMMEtaHits = 3);
    declareProperty("ConsideredPDGs", m_pdgsToBeConsidered);
}

MuonTrackPerformanceAlg::~MuonTrackPerformanceAlg() {}

StatusCode MuonTrackPerformanceAlg::initialize() {

    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_printer.retrieve());
    ATH_CHECK(m_edmHelperSvc.retrieve());
    ATH_CHECK(m_truthTool.retrieve(EnableTool{m_doTruth}));   

    ATH_CHECK(m_summaryHelperTool.retrieve());

    // add muons
    if (m_pdgsToBeConsidered.value().empty()) {
        m_selectedPdgs.insert(13);
        m_selectedPdgs.insert(-13);
    } else {
        // add pdgs
        for (auto pdg : m_pdgsToBeConsidered.value()) m_selectedPdgs.insert(pdg);
        msg(MSG::DEBUG) << " PDG codes used for matching";
        for (auto val : m_selectedPdgs) msg(MSG::DEBUG) << " " << val;
        msg(MSG::DEBUG) << endmsg;
    }

    ATH_CHECK(m_trackKey.initialize(!m_trackKey.key().empty())); 
    ATH_CHECK(m_muons.initialize(!m_muons.key().empty())); 
    
    ATH_CHECK(m_eventInfoKey.initialize());

    ATH_CHECK(m_mcEventColl.initialize(m_doTruth));
    if (!(m_idHelperSvc->hasSTGC() && m_idHelperSvc->hasMM())) m_muonSimData = {"MDT_SDO", "RPC_SDO", "TGC_SDO"};
    ATH_CHECK(m_muonSimData.initialize(m_doTruth));
    ATH_CHECK(m_cscSimData.initialize(m_doTruth && m_idHelperSvc->hasCSC()));
    ATH_CHECK(m_trackRecord.initialize(m_doTruth));

    return StatusCode::SUCCESS;
}

StatusCode MuonTrackPerformanceAlg::execute() {

    SG::ReadHandle<xAOD::EventInfo> evInfo(m_eventInfoKey);
    if (!evInfo.isValid()) {
        ATH_MSG_WARNING("failed to retrieve EventInfo");
        return StatusCode::FAILURE;
    }
    m_eventInfo = evInfo.cptr();
    handleTracks();

    ++m_nevents;

    return StatusCode::SUCCESS;
}

bool MuonTrackPerformanceAlg::handleSegmentCombi(const Muon::MuonSegmentCombination& combi) {
    /** This method loops over the segments in the combi and filles them into the internal structure of the MuonCombiTrackMaker */

    unsigned int nstations = combi.numberOfStations();

    // loop over chambers in combi and extract segments
    for (unsigned int i = 0; i < nstations; ++i) {
        // loop over segments in station
        const Muon::MuonSegmentCombination::SegmentVec* stationSegs = combi.stationSegments(i);

        // check if not empty
        if (!stationSegs || stationSegs->empty()) continue;

        Muon::MuonSegmentCombination::SegmentVec::const_iterator ipsg = stationSegs->begin();
        Muon::MuonSegmentCombination::SegmentVec::const_iterator ipsg_end = stationSegs->end();
        for (; ipsg != ipsg_end; ++ipsg) {
            const Muon::MuonSegment* seg = dynamic_cast<const Muon::MuonSegment*>((*ipsg).get());

            if (!seg) {
                ATH_MSG_WARNING("MuonSegmentCombination contains a segment that is not a MuonSegment!!");
                return false;
            }
        }
    }

    return true;
}

bool MuonTrackPerformanceAlg::handleTracks() {
    std::unique_ptr<TrackCollection> allTracks = std::make_unique<TrackCollection>();
    if (!m_trackKey.key().empty()) {  // MS tracks
        SG::ReadHandle<TrackCollection> trackCol(m_trackKey);
        if (!trackCol.isValid()) {
            ATH_MSG_WARNING(" Could not find tracks at " << m_trackKey.key());
            return false;
        }
        m_trackTypeString = m_trackKey.key();
        ATH_MSG_DEBUG(" Retrieved " << trackCol->size() << " tracks from " << trackCol.key());
        m_ntracks += trackCol->size();

        if (m_doTruth) { handleTrackTruth(*trackCol); }

        if ((msgLvl(MSG::DEBUG) || m_doSummary >= 2) && !m_doTruth) { doSummary(*trackCol); }
    } else {
        SG::ReadHandle<xAOD::MuonContainer> muons(m_muons);
        if (!muons.isValid()) {
            ATH_MSG_WARNING("could not find muons");
            return false;
        }
        if (m_doStau)
            m_trackTypeString = "MuGirlStauCombinedTracks";
        else {
            if (m_isCombined)
                m_trackTypeString = "CombinedMuonTracks";
            else {
                if (m_trackType == (int)xAOD::Muon::MSOnlyExtrapolatedMuonSpectrometerTrackParticle)
                    m_trackTypeString = "MSOnlyExtrapolatedMuonTracks";
                else
                    m_trackTypeString = "ExtrapolatedMuonTracks";
            }
        }
        for (auto muon : *muons) {
            if (!m_doStau) {
                // if combined and not stau, only take MuidCo and MuGirl
                if (m_isCombined && muon->author() != 1 && muon->author() != 6) continue;
                if (!m_isCombined) {
                    // only MuidCo, MuidSA, and STACO will have MSOnlyExtrapolated tracks
                    if (m_trackType == xAOD::Muon::MSOnlyExtrapolatedMuonSpectrometerTrackParticle && muon->author() != 1 &&
                        muon->author() != 5 && muon->author() != 2)
                        continue;
                    // MuGirl should also have ME tracks
                    if (m_trackType == xAOD::Muon::ExtrapolatedMuonSpectrometerTrackParticle && muon->author() != 1 &&
                        muon->author() != 5 && muon->author() != 2 && muon->author() != 6)
                        continue;
                }
            } else {
                // only staus
                if (muon->author() != 7) continue;
            }
            // TrackCollection is a DataVector so allTracks takes ownership of these copies
            const xAOD::TrackParticle* tp = muon->trackParticle((xAOD::Muon::TrackParticleType)m_trackType);
            if (!tp) {
                // possible that MS-only track doesn't exist for combined muon, if initial extrapolation fails but combined fit succeeds
                // otherwise, the track particle should be there, throw a warning
                if (m_trackType == xAOD::Muon::MSOnlyExtrapolatedMuonSpectrometerTrackParticle)
                    ATH_MSG_DEBUG("no track particle of type " << m_trackType << " for muon with author " << muon->author() << " and pT "
                                                               << muon->pt());
                else
                    ATH_MSG_WARNING("no track particle of type " << m_trackType << " for muon with author " << muon->author() << " and pT "
                                                                 << muon->pt());
                continue;
            }
            if (tp->track()) {
                m_ntracks++;
                allTracks->push_back(new Trk::Track(*tp->track()));
            } else
                ATH_MSG_WARNING("no track for this trackParticle, skipping");
        }
        ATH_MSG_DEBUG("got " << allTracks->size() << " tracks");

        if (m_doTruth) { handleTrackTruth(*allTracks.get()); }

        if ((msgLvl(MSG::DEBUG) || m_doSummary >= 2) && !m_doTruth) { doSummary(*allTracks.get()); }
    }

    return true;
}

bool MuonTrackPerformanceAlg::goodTruthTrack(const Muon::IMuonTrackTruthTool::TruthTreeEntry& entry) const {
    if ((!entry.cscHits.empty() || (!entry.mmHits.empty() && !entry.stgcHits.empty())) && entry.mdtHits.empty()) return false;
    TrackRecord* trackRecord = const_cast<TrackRecord*>(entry.truthTrack);
    if (!trackRecord) return false;
    if (m_usePtCut) {
        if (trackRecord->GetMomentum().perp() < m_momentumCutSim) return false;
    } else {
        if (trackRecord->GetMomentum().mag() < m_momentumCutSim) return false;
    }
    if (!selectPdg(trackRecord->GetPDGCode())) return false;
    if (m_isCombined && std::abs(trackRecord->GetMomentum().eta()) > 2.5) return false;
    int hits = entry.mdtHits.size();
    if (m_idHelperSvc->hasCSC()) hits += entry.cscHits.size();
    if (m_idHelperSvc->hasMM()) hits += entry.mmHits.size();
    return (hits > 4);
}

bool MuonTrackPerformanceAlg::handleTrackTruth(const TrackCollection& trackCollection) {
    bool didOutput = false;

    unsigned int ntruthTracks(0);
    unsigned int ntruthTracksSecondary(0);

    SG::ReadHandle<TrackRecordCollection> truthTrackCol(m_trackRecord);
    SG::ReadHandle<McEventCollection> mcEventCollection(m_mcEventColl);
    std::vector<const MuonSimDataCollection*> muonSimData;
    for (SG::ReadHandle<MuonSimDataCollection>& simDataMap : m_muonSimData.makeHandles()) {
        if (!simDataMap.isValid()) {
            ATH_MSG_WARNING(simDataMap.key() << " not valid");
            continue;
        }
        if (!simDataMap.isPresent()) continue;
        muonSimData.push_back(simDataMap.cptr());
    }
    const CscSimDataCollection* cscSimData = nullptr;
    if (m_idHelperSvc->hasCSC()) {
        SG::ReadHandle<CscSimDataCollection> cscSimDataMap(m_cscSimData);
        if (!cscSimDataMap.isValid()) {
            ATH_MSG_WARNING(cscSimDataMap.key() << " not valid");
        } else {
            cscSimData = cscSimDataMap.cptr();
        }
    }
    const Muon::IMuonTrackTruthTool::TruthTree truthTree =
        m_truthTool->createTruthTree(truthTrackCol.cptr(), mcEventCollection.cptr(), muonSimData, cscSimData);

    // map TrackRecord onto entry for printing
    std::map<const TrackRecord*, Muon::IMuonTrackTruthTool::TruthTreeEntry> truthTrackEntryMap;
    Muon::IMuonTrackTruthTool::TruthTree::const_iterator tit = truthTree.begin();
    Muon::IMuonTrackTruthTool::TruthTree::const_iterator tit_end = truthTree.end();
    for (; tit != tit_end; ++tit) {
        if (!tit->second.truthTrack) continue;

        truthTrackEntryMap[tit->second.truthTrack] = tit->second;

        if (!goodTruthTrack(tit->second)) continue;

        bool isSec = isSecondary(tit->second);

        if (isSec)
            ++ntruthTracksSecondary;
        else
            ++ntruthTracks;
    }

    m_ntruthTracks += ntruthTracks;
    m_ntruthTracksSecondary += ntruthTracksSecondary;

    std::set<const Trk::Track*> matchedTracks;
    std::set<const Trk::Track*> matchedFakeTracks;
    std::map<const TrackRecord*, Muon::IMuonTrackTruthTool::MatchResult> bestMatched;

    unsigned int nmatched(0);
    unsigned int nmatchedSecondary(0);

    Muon::IMuonTrackTruthTool::ResultVec result = m_truthTool->match(truthTree, trackCollection);
    Muon::IMuonTrackTruthTool::ResultVec::iterator rit = result.begin();
    Muon::IMuonTrackTruthTool::ResultVec::iterator rit_end = result.end();
    for (; rit != rit_end; ++rit) {
        Muon::MuonTrackTruth& trackTruth = rit->second;

        // skip track if not matched to any muon
        if (!trackTruth.truthTrack) continue;

        // skip match if zero overlap with truth track
        if (trackTruth.numberOfMatchedHits() == 0) continue;

        std::map<const TrackRecord*, Muon::IMuonTrackTruthTool::TruthTreeEntry>::iterator pos =
            truthTrackEntryMap.find(trackTruth.truthTrack);
        if (pos == truthTrackEntryMap.end()) {
            ATH_MSG_WARNING("Truth track not found in map, this should not happen!!");
            continue;
        }

        if (!goodTruthTrack(pos->second)) continue;

        // see whether we already had an track matched to this truth track
        if (!bestMatched.count(trackTruth.truthTrack)) {
            // no track found yet, this is the best match
            bestMatched[trackTruth.truthTrack] = *rit;

            matchedTracks.insert(rit->first);

            bool isSec = isSecondary(pos->second);

            if (isSec)
                ++nmatchedSecondary;
            else
                ++nmatched;

        } else {
            ++m_nmatchedFakeTracks;
            matchedFakeTracks.insert(rit->first);
        }
    }

    m_nmatchedTracks += nmatched;
    m_nmatchedTracksSecondary += nmatchedSecondary;
    m_nfakeTracks += trackCollection.size() - nmatched;

    // now we have gathered all links between the tracks and truth.
    // Let's generate some diagnostics

    EventData eventData;
    eventData.eventNumber = eventNumber();
    eventData.eventPosition = m_nevents;

    if (m_doTrackDebug >= 5 || msgLvl(MSG::DEBUG)) {
        ATH_MSG_INFO("Event " << eventData.eventNumber << " truth tracks " << ntruthTracks);
    }

    // first loop over truth tracks that were found by reco
    std::map<const TrackRecord*, Muon::IMuonTrackTruthTool::MatchResult>::iterator mit = bestMatched.begin();
    std::map<const TrackRecord*, Muon::IMuonTrackTruthTool::MatchResult>::iterator mit_end = bestMatched.end();
    for (; mit != mit_end; ++mit) {
        Muon::MuonTrackTruth& truthTrack = mit->second.second;
        std::map<const TrackRecord*, Muon::IMuonTrackTruthTool::TruthTreeEntry>::iterator pos =
            truthTrackEntryMap.find(truthTrack.truthTrack);
        if (pos == truthTrackEntryMap.end()) {
            ATH_MSG_WARNING("Truth track not found in map, this should not happen!!");
            continue;
        }

        // create track summary
        TrackData* trackData = evaluateTrackTruthOverlap(truthTrack);
        if (!trackData) continue;
        addTrackToTrackData(*mit->second.first, *trackData);
        trackData->truthTrack = new TrackRecord(*mit->first);
        if (truthTrack.truthTrajectory) {
            trackData->truthTrajectory = new TruthTrajectory(*truthTrack.truthTrajectory);
            HepMC::ConstGenParticlePtr mother = getMother(*truthTrack.truthTrajectory);
            if (mother) {
                trackData->motherPdg = mother->pdg_id();
                if (mother->end_vertex())
                    trackData->productionVertex = new Amg::Vector3D(
                        mother->end_vertex()->position().x(), mother->end_vertex()->position().y(), mother->end_vertex()->position().z());
            }
            HepMC::ConstGenParticlePtr original = getInitialState(*truthTrack.truthTrajectory);
            if (original) {
                trackData->momentumAtProduction =
                    new Amg::Vector3D(original->momentum().x(), original->momentum().y(), original->momentum().z());
            }
        }
        // check whether track is all ok
        if (trackData->allOk()) {
            if (m_doTrackDebug >= 5 || msgLvl(MSG::DEBUG)) {
                ATH_MSG_INFO(print(*trackData));
                didOutput = true;
            }
            delete trackData;
            continue;
        }

        // if we get here something was wrong, let's evaluate what
        if (trackData->hasMissingChambers()) {
            if (trackData->isEndcapSLTrack() && trackData->isMissingInner()) {
                ++m_nmissingStationMomLoss;
                eventData.missingStationMomLoss.push_back(trackData);
            } else if (trackData->hasMissingLayers()) {
                ++m_nmissingStationLayer;
                eventData.missingStationLayer.push_back(trackData);
            } else if (trackData->hasMissingLayersTrigger()) {
                ++m_nmissingStationLayerTrigger;
                eventData.missingStationLayerTrigger.push_back(trackData);
            } else {
                ++m_nmissingStation;
                eventData.missingStation.push_back(trackData);
            }
        } else if (trackData->hasWrongChambers()) {
            if (trackData->hasWrongLayers()) {
                eventData.wrongStationLayer.push_back(trackData);
                ++m_nwrongStationLayer;
            } else if (trackData->hasWrongLayersTrigger()) {
                eventData.wrongStationLayer.push_back(trackData);
                ++m_nwrongStationLayerTrigger;
            } else {
                ++m_nwrongStation;
                eventData.wrongStation.push_back(trackData);
            }
        } else {
            ATH_MSG_WARNING("Unknown problem with matched track " << std::endl << print(*trackData));
            delete trackData;
            continue;
        }

        if (msgLvl(MSG::DEBUG) || m_doTrackDebug >= 4) {
            ATH_MSG_INFO("Incomplete track in event " << eventInfo() << " nevents " << m_nevents << std::endl
                         << print(*trackData));
            didOutput = true;
        }
    }

    // now the truth tracks that were not found
    unsigned int nmissed(0);
    tit = truthTree.begin();
    tit_end = truthTree.end();
    for (; tit != tit_end; ++tit) {
        // skip bad truth tracks
        if (!goodTruthTrack(tit->second)) continue;

        // check whether truth track was matched
        if (bestMatched.count(tit->second.truthTrack)) continue;

        TrackData* trackData = createTrackData(tit->second);
        if (!trackData) {
            ATH_MSG_WARNING("Failed to create TrackData for truth track");
            continue;
        }
        trackData->truthTrack = new TrackRecord(*tit->second.truthTrack);
        if (tit->second.truthTrajectory) {
            trackData->truthTrajectory = new TruthTrajectory(*tit->second.truthTrajectory);
            HepMC::ConstGenParticlePtr mother = getMother(*tit->second.truthTrajectory);
            if (mother) {
                trackData->motherPdg = mother->pdg_id();
                if (mother->end_vertex())
                    trackData->productionVertex = new Amg::Vector3D(
                        mother->end_vertex()->position().x(), mother->end_vertex()->position().y(), mother->end_vertex()->position().z());
            }
            HepMC::ConstGenParticlePtr original = getInitialState(*tit->second.truthTrajectory);
            if (original) {
                trackData->momentumAtProduction =
                    new Amg::Vector3D(original->momentum().x(), original->momentum().y(), original->momentum().z());
            }
        }
        if (m_doTrackDebug >= 3 || msgLvl(MSG::DEBUG)) {
            ATH_MSG_INFO("Truth track not found: " << std::endl << print(*trackData));
            didOutput = true;
        }
        bool isSec = isSecondary(tit->second);

        if (trackData->missingLayers.size() == 1) {
            if (isSec)
                ++m_nmissedTracksOneStationSecondary;
            else
                ++m_nmissedTracksOneStation;
            eventData.missingTruthTracksOneStation.push_back(trackData);
        } else {
            if (isSec)
                ++m_nmissedTracksSecondary;
            else
                ++m_nmissedTracks;
            eventData.missingTruthTracks.push_back(trackData);
        }
        ++nmissed;
    }

    // sanity check
    if (ntruthTracks < nmatched)
        ATH_MSG_WARNING("found more matched tracks than truth tracks: truth  " << ntruthTracks << "  matched " << nmatched);

    if (nmissed != ntruthTracks - nmatched)
        ATH_MSG_WARNING("inconsisted number of missed tracks: truth  " << ntruthTracks << "  matched " << nmatched << " missed " << nmissed);

    // finally print fake tracks
    TrackCollection::const_iterator trit = trackCollection.begin();
    TrackCollection::const_iterator trit_end = trackCollection.end();
    for (; trit != trit_end; ++trit) {
        if (matchedTracks.count(*trit)) continue;

        double pt = 1e9;
        if ((**trit).perigeeParameters()) {
            if (m_usePtCut)
                pt = (**trit).perigeeParameters()->momentum().perp();
            else
                pt = (**trit).perigeeParameters()->momentum().mag();
        }
        bool isSL = false;
        if (m_edmHelperSvc->isSLTrack(**trit)) {
            pt = 0.;
            isSL = true;
        }
        bool isHighPt = pt > m_momentumCut;
        bool matchedFake = matchedFakeTracks.count(*trit);
        std::string fakeType = isHighPt ? "HighPt" : "LowPt";
        if (isSL) fakeType = "SL";
        if (matchedFake) fakeType += " (matched)";

        TrackSummary summary;
        if (matchedFake) summary.trackPars = "(matched) ";
        summary.trackPars += m_printer->print(**trit);
        summary.chambers = m_printer->printStations(**trit);

        TrackData* trackData = new TrackData();
        addTrackToTrackData(**trit, *trackData);
        if (isHighPt) {
            eventData.fakeTracks.push_back(trackData);
            ++m_nfakeTracksHighPt;
        } else if (isSL) {
            ++m_nfakeTracksSL;
            eventData.fakeTracksSL.push_back(trackData);
        } else {
            ++m_nfakeTracksLowPt;
            eventData.fakeTracksLowPt.push_back(trackData);
        }

        if (m_doTrackDebug >= 3 || msgLvl(MSG::DEBUG)) {
            msg() << MSG::INFO << "Fake track " << fakeType << ": " << std::endl
                  << summary.trackPars << "  " << summary.chambers << std::endl;
            didOutput = true;
        }
    }
    if (didOutput) { msg() << MSG::INFO << endmsg; }

    if (!eventData.goodEvent()) m_badEvents.push_back(eventData);

    return true;
}

StatusCode MuonTrackPerformanceAlg::finalize() {
    ATH_MSG_INFO(std::endl << MuonTrackPerformanceAlg::printTrackCounters());

    // write to file
    if (m_writeToFile) {
        std::string outfile = "trkPerformance_";
        outfile.append(m_trackTypeString);
        outfile.append(".txt");
        m_fileOutput.open(outfile.c_str(), std::ios::trunc);

        m_fileOutput << MuonTrackPerformanceAlg::printTrackCounters(false) << std::endl;

        m_fileOutput.close();
    }

    if (m_doTruth && m_doTrackDebug >= 1) {
        printMissingTracks();
        printMissingChambers();
        printFakeTracks();
    }

    std::vector<EventData>::iterator eit = m_badEvents.begin();
    std::vector<EventData>::iterator eit_end = m_badEvents.end();
    for (; eit != eit_end; ++eit) { clearEvent(*eit); }

    return StatusCode::SUCCESS;
}

int MuonTrackPerformanceAlg::eventNumber() const {
    if (!m_eventInfo) return -1;
    return m_eventInfo->eventNumber();
}

std::string MuonTrackPerformanceAlg::eventInfo() const { return std::to_string(eventNumber()); }

void MuonTrackPerformanceAlg::doSummary(const TrackCollection& trackCollection) const {
    msg() << " Summarizing tracks in event " << eventInfo() << " nevents " << m_nevents;
    if (trackCollection.empty())
        msg() << " : no tracks found" << endmsg;
    else {
        msg() << " : " << trackCollection.size() << " tracks found " << std::endl;
        TrackCollection::const_iterator tit = trackCollection.begin();
        TrackCollection::const_iterator tit_end = trackCollection.end();
        for (; tit != tit_end; ++tit) {
            if (m_doHitResiduals > 0) {
                msg() << m_printer->print(**tit) << "  " << m_printer->printStations(**tit) << std::endl;
                msg() << m_printer->printMeasurements(**tit);
            } else {
                msg() << m_printer->print(**tit) << "  " << m_printer->printStations(**tit);
            }
            // should finish with an 'endmsg' else the buffer does not get flushed.
            if (tit == tit_end - 1)
                msg() << endmsg;
            else
                msg() << std::endl;
        }
    }
}

void MuonTrackPerformanceAlg::doSummary(const Muon::IMuonTrackTruthTool::TruthTree& truthTracks) const {
    msg() << " Summarizing tracks in event " << eventInfo() << " nevents " << m_nevents;
    if (truthTracks.empty())
        msg() << " : no truth tracks" << endmsg;
    else {
        msg() << " : " << truthTracks.size() << " truth tracks found " << std::endl;
        Muon::IMuonTrackTruthTool::TruthTree::const_iterator tit = truthTracks.begin();
        Muon::IMuonTrackTruthTool::TruthTree::const_iterator tit_end = truthTracks.end();
        for (; tit != tit_end; ++tit) {
            msg() << print(tit->second);
            // should finish with an 'endmsg' else the buffer does not get flushed.
            Muon::IMuonTrackTruthTool::TruthTree::const_iterator tit_temp = tit;
            if (++tit_temp == tit_end)
                msg() << endmsg;
            else
                msg() << std::endl;
        }
    }
}

std::string MuonTrackPerformanceAlg::print(const Muon::IMuonTrackTruthTool::TruthTreeEntry& trackTruth) const {
    std::ostringstream sout;
    if (!trackTruth.truthTrack) {
        sout << " No TrackRecord found! ";
        return sout.str();
    }

    TrackRecord& trackRecord = const_cast<TrackRecord&>(*trackTruth.truthTrack);
    double charge = trackRecord.GetPDGCode() < 0 ? +1 : -1;

    // keep track of which chambers are present
    std::map<Identifier, std::pair<int, int> > precisionIds;
    std::map<Identifier, std::pair<int, int> > triggerIds;
    std::map<Identifier, std::pair<int, int> > nhitsCompIds;

    unsigned int neta = 0;
    unsigned int netaTrig = 0;
    unsigned int nphi = 0;

    std::set<Identifier> allIds;
    MuonSimDataCollection::const_iterator mit = trackTruth.mdtHits.begin();
    MuonSimDataCollection::const_iterator mit_end = trackTruth.mdtHits.end();
    for (; mit != mit_end; ++mit) allIds.insert(mit->first);

    if (m_idHelperSvc->hasCSC()) {
        CscSimDataCollection::const_iterator cit = trackTruth.cscHits.begin();
        CscSimDataCollection::const_iterator cit_end = trackTruth.cscHits.end();
        for (; cit != cit_end; ++cit) allIds.insert(m_idHelperSvc->layerId(cit->first));
    }
    if (m_idHelperSvc->hasSTGC()) {
        mit = trackTruth.stgcHits.begin();
        mit_end = trackTruth.stgcHits.end();
        for (; mit != mit_end; ++mit) allIds.insert(m_idHelperSvc->layerId(mit->first));
    }
    if (m_idHelperSvc->hasMM()) {
        mit = trackTruth.mmHits.begin();
        mit_end = trackTruth.mmHits.end();
        for (; mit != mit_end; ++mit) allIds.insert(m_idHelperSvc->layerId(mit->first));
    }

    mit = trackTruth.rpcHits.begin();
    mit_end = trackTruth.rpcHits.end();
    for (; mit != mit_end; ++mit) allIds.insert(m_idHelperSvc->layerId(mit->first));

    mit = trackTruth.tgcHits.begin();
    mit_end = trackTruth.tgcHits.end();
    for (; mit != mit_end; ++mit) allIds.insert(m_idHelperSvc->layerId(mit->first));

    std::set<Identifier>::iterator it = allIds.begin();
    std::set<Identifier>::iterator it_end = allIds.end();
    for (; it != it_end; ++it) {
        Identifier id = *it;
        Identifier chId = m_idHelperSvc->chamberId(id);
        bool measuresPhi = m_idHelperSvc->measuresPhi(id);
        if (measuresPhi) {
            ++nhitsCompIds[chId].first;
        } else {
            ++nhitsCompIds[chId].second;
        }

        if (m_idHelperSvc->isTrigger(chId)) {
            if (measuresPhi) {
                ++triggerIds[chId].first;
                ++nphi;
            } else {
                ++triggerIds[chId].second;
                ++netaTrig;
            }
            continue;
        }
        if (measuresPhi) {
            ++precisionIds[chId].first;
            ++nphi;
        } else {
            ++precisionIds[chId].second;
            ++neta;
        }
    }
    sout << "Truth:        hits " << std::setw(5) << neta + nphi + netaTrig << " r " << (int)trackRecord.GetPosition().perp() << " z "
         << (int)trackRecord.GetPosition().z() << std::setprecision(5) << " phi " << trackRecord.GetMomentum().phi() << " theta "
         << trackRecord.GetMomentum().theta() << std::setw(6) << " q*mom " << (int)trackRecord.GetMomentum().mag() * charge << " pt "
         << std::setw(5) << (int)trackRecord.GetMomentum().perp();
    if (std::abs(trackRecord.GetPDGCode()) != 13) sout << " pdg " << trackRecord.GetPDGCode();

    if (trackTruth.truthTrajectory) {
        HepMC::ConstGenParticlePtr mother = getMother(*trackTruth.truthTrajectory);
        if (mother) { sout << " mother " << mother->pdg_id(); }
    }

    sout << "   Eta hits " << neta << "  phi " << nphi << "  eta trig " << netaTrig << std::endl;
    std::map<Identifier, std::pair<int, int> >::iterator iit = precisionIds.begin();
    std::map<Identifier, std::pair<int, int> >::iterator iit_end = precisionIds.end();
    for (; iit != iit_end; ++iit) {
        sout.setf(std::ios::left);
        sout << "  " << std::setw(32) << m_idHelperSvc->toStringChamber(iit->first) << " hits: eta " << std::setw(3) << iit->second.second
             << " phi " << std::setw(3) << iit->second.first << std::endl;
    }
    iit = triggerIds.begin();
    iit_end = triggerIds.end();
    for (; iit != iit_end; ++iit) {
        sout << "  " << std::setw(32) << m_idHelperSvc->toStringChamber(iit->first) << " hits: eta " << std::setw(3)
             << nhitsCompIds[iit->first].second << " phi " << std::setw(3) << nhitsCompIds[iit->first].first << "   stations: Eta "
             << iit->second.second << " Phi " << iit->second.first << std::endl;
    }

    return sout.str();
}

std::string MuonTrackPerformanceAlg::print(const MuonTrackPerformanceAlg::EventData& event,
                                           const std::vector<MuonTrackPerformanceAlg::TrackData*>& tracks, const std::string& message) const {
    std::ostringstream sout;
    if (!tracks.empty()) {
        sout << "  Event " << event.eventNumber << " position in file " << event.eventPosition << " has " << tracks.size() << " " << message
             << std::endl;
        std::vector<TrackData*>::const_iterator it = tracks.begin();
        std::vector<TrackData*>::const_iterator it_end = tracks.end();
        for (; it != it_end; ++it) { sout << print(**it) << std::endl; }
    }
    return sout.str();
}

std::string MuonTrackPerformanceAlg::print(const MuonTrackPerformanceAlg::EventData& event, const std::vector<const Trk::Track*>& tracks,
                                           const std::string& message) const {
    std::ostringstream sout;
    if (!tracks.empty()) {
        sout << "  Event " << event.eventNumber << " position in file " << event.eventPosition << " has " << tracks.size() << " " << message
             << std::endl;
        std::vector<const Trk::Track*>::const_iterator it = tracks.begin();
        std::vector<const Trk::Track*>::const_iterator it_end = tracks.end();
        for (; it != it_end; ++it) { sout << m_printer->print(**it) << std::endl << m_printer->printStations(**it) << std::endl; }
    }
    return sout.str();
}

void MuonTrackPerformanceAlg::printMissingChambers() const {
    std::fstream outputFile;
    if (m_doEventListIncomplete > 0) {
        std::string filename = name();
        filename += "EventListIncompleteTracks.txt";
        outputFile.open(filename.c_str(), std::ios_base::out | std::ios_base::trunc);
    }

    msg() << MSG::INFO << "Summarizing events with endcap track without EI layer resulting in momentum loss, total "
           << m_nmissingStationMomLoss << std::endl;
    std::vector<EventData>::const_iterator eit = m_badEvents.begin();
    std::vector<EventData>::const_iterator eit_end = m_badEvents.end();
    for (; eit != eit_end; ++eit) {
        if (eit->missingStationMomLoss.empty()) continue;
        msg() << print(*eit, eit->missingStationMomLoss, "  tracks without inner layer in endcap") << std::endl;
        if (m_doEventListIncomplete > 0) outputFile << eit->eventNumber << std::endl;
    }
    msg() << endmsg;

    msg() << MSG::INFO << "Summarizing events with track with missing layer, total " << m_nmissingStationLayer << std::endl;
    eit = m_badEvents.begin();
    eit_end = m_badEvents.end();
    for (; eit != eit_end; ++eit) {
        if (eit->missingStationLayer.empty()) continue;
        msg() << print(*eit, eit->missingStationLayer, "  tracks with a missing layer") << std::endl;
        if (m_doEventListIncomplete > 0) outputFile << eit->eventNumber << std::endl;
    }
    msg() << endmsg;

    msg() << MSG::INFO << "Summarizing events with track with missing chamber, total " << m_nmissingStation << std::endl;
    eit = m_badEvents.begin();
    eit_end = m_badEvents.end();
    for (; eit != eit_end; ++eit) {
        if (eit->missingStation.empty()) continue;
        msg() << print(*eit, eit->missingStation, "  tracks with a missing chamber") << std::endl;
        if (m_doEventListIncomplete > 1) outputFile << eit->eventNumber << std::endl;
    }
    msg() << endmsg;

    msg() << MSG::INFO << "Summarizing events with track with wrong layer, total " << m_nwrongStationLayer << std::endl;
    eit = m_badEvents.begin();
    eit_end = m_badEvents.end();
    for (; eit != eit_end; ++eit) {
        if (eit->wrongStation.empty()) continue;
        msg() << print(*eit, eit->wrongStation, "  tracks with a wrong layer") << std::endl;
        if (m_doEventListIncomplete > 0) outputFile << eit->eventNumber << std::endl;
    }
    msg() << endmsg;

    msg() << MSG::INFO << "Summarizing events with track with wrong chamber, total " << m_nwrongStation << std::endl;
    eit = m_badEvents.begin();
    eit_end = m_badEvents.end();
    for (; eit != eit_end; ++eit) {
        if (eit->wrongStation.empty()) continue;
        msg() << print(*eit, eit->wrongStation, "  tracks with a wrong chamber") << std::endl;
        if (m_doEventListIncomplete > 0) outputFile << eit->eventNumber << std::endl;
    }
    msg() << endmsg;

    msg() << MSG::INFO << "Summarizing events with track with missing trigger layer, total " << m_nmissingStationLayerTrigger << std::endl;
    eit = m_badEvents.begin();
    eit_end = m_badEvents.end();
    for (; eit != eit_end; ++eit) {
        if (eit->missingStationLayerTrigger.empty()) continue;
        msg() << print(*eit, eit->missingStationLayerTrigger, "  tracks with a missing trigger layer") << std::endl;
        if (m_doEventListIncomplete > 0) outputFile << eit->eventNumber << std::endl;
    }
    msg() << endmsg;

    msg() << MSG::INFO << "Summarizing events with track with wrong trigger layer, total " << m_nwrongStationLayerTrigger << std::endl;
    eit = m_badEvents.begin();
    eit_end = m_badEvents.end();
    for (; eit != eit_end; ++eit) {
        if (eit->wrongStationLayerTrigger.empty()) continue;
        msg() << print(*eit, eit->wrongStationLayerTrigger, "  tracks with a wrong trigger layer") << std::endl;
        if (m_doEventListIncomplete > 0) outputFile << eit->eventNumber << std::endl;
    }
    msg() << endmsg;
}

void MuonTrackPerformanceAlg::printMissingTracks() const {
    std::fstream outputFile;
    if (m_doEventListMissed > 0) {
        std::string filename = name();
        filename += "EventListMissedTracks.txt";
        outputFile.open(filename.c_str(), std::ios_base::out | std::ios_base::trunc);
    }

    msg() << MSG::INFO << "Summarizing events with missing track, total " << m_nmissedTracks << std::endl;
    std::vector<EventData>::const_iterator eit = m_badEvents.begin();
    std::vector<EventData>::const_iterator eit_end = m_badEvents.end();
    for (; eit != eit_end; ++eit) {
        if (eit->missingTruthTracks.empty()) continue;
        msg() << print(*eit, eit->missingTruthTracks, "  missing tracks") << std::endl;
        if (m_doEventListMissed > 0) outputFile << eit->eventNumber << std::endl;
    }
    msg() << endmsg;

    msg() << MSG::INFO << "Summarizing events with missing single station track, total " << m_nmissedTracksOneStation << std::endl;
    eit = m_badEvents.begin();
    eit_end = m_badEvents.end();
    for (; eit != eit_end; ++eit) {
        if (eit->missingTruthTracksOneStation.empty()) continue;
        msg() << print(*eit, eit->missingTruthTracksOneStation, "  missing single station tracks") << std::endl;
        if (m_doEventListMissed > 1) outputFile << eit->eventNumber << std::endl;
    }
    msg() << endmsg;
}

void MuonTrackPerformanceAlg::printFakeTracks() const {
    std::fstream outputFile;
    if (m_doEventListFake > 0) {
        std::string filename = name();
        filename += "EventListFakeTracks.txt";
        outputFile.open(filename.c_str(), std::ios_base::out | std::ios_base::trunc);
    }

    msg() << MSG::INFO << "Summarizing events with fake tracks, high pt, total " << m_nfakeTracksHighPt << std::endl;
    std::vector<EventData>::const_iterator eit = m_badEvents.begin();
    std::vector<EventData>::const_iterator eit_end = m_badEvents.end();
    for (; eit != eit_end; ++eit) {
        if (eit->fakeTracks.empty()) continue;
        msg() << print(*eit, eit->fakeTracks, "  high pt fake tracks") << std::endl;
        if (m_doEventListFake > 0) outputFile << eit->eventNumber << std::endl;
    }
    msg() << endmsg;

    msg() << MSG::INFO << "Summarizing events with fake tracks, low pt, total " << m_nfakeTracksLowPt << std::endl;
    eit = m_badEvents.begin();
    eit_end = m_badEvents.end();
    for (; eit != eit_end; ++eit) {
        if (eit->fakeTracksLowPt.empty()) continue;
        msg() << print(*eit, eit->fakeTracksLowPt, "  low pt fake tracks") << std::endl;
        if (m_doEventListFake > 1) outputFile << eit->eventNumber << std::endl;
    }
    msg() << endmsg;

    msg() << MSG::INFO << "Summarizing events with fake tracks, SL, total " << m_nfakeTracksSL << std::endl;
    eit = m_badEvents.begin();
    eit_end = m_badEvents.end();
    for (; eit != eit_end; ++eit) {
        if (eit->fakeTracksSL.empty()) continue;
        msg() << print(*eit, eit->fakeTracksSL, "  SL fake tracks") << std::endl;
        if (m_doEventListFake > 2) outputFile << eit->eventNumber << std::endl;
    }
    msg() << endmsg;
}

std::string MuonTrackPerformanceAlg::printTrackCounters(bool doSecondaries) const {
    std::ostringstream sout;
    sout.precision(4);

    double evScale = m_nevents != 0 ? 1. / m_nevents : 1.;

    sout << std::endl;
    sout << "Summarizing results for track collection " << m_trackTypeString << "  " << m_nevents << " events: " << std::endl
         << "  number of tracks                           " << std::setw(12) << m_ntracks << "   average per event " << m_ntracks * evScale;

    if (m_doTruth) {
        unsigned int nmatchedTracks = !doSecondaries ? m_nmatchedTracks : m_nmatchedTracks + m_nmatchedTracksSecondary;
        unsigned int nmissedTracks = !doSecondaries ? m_nmissedTracks : m_nmissedTracks + m_nmissedTracksSecondary;
        unsigned int nmissedTracksOneStation =
            !doSecondaries ? m_nmissedTracksOneStation : m_nmissedTracksOneStation + m_nmissedTracksOneStationSecondary;
        unsigned int ntruthTracks = !doSecondaries ? m_ntruthTracks : m_ntruthTracks + m_ntruthTracksSecondary;

        double trTrkScale = ntruthTracks != 0 ? 1. / ntruthTracks : 1.;
        double trTrkScaleSec = m_ntruthTracksSecondary != 0 ? 1. / m_ntruthTracksSecondary : 1.;

        sout << std::endl
             << "  number of truth tracks                     " << std::setw(12) << ntruthTracks << "   average per event "
             << ntruthTracks * evScale << std::endl
             << "  number of good tracks                      " << std::setw(12) << nmatchedTracks << "   average per event "
             << nmatchedTracks * evScale << std::endl
             << "  number of missed multi station tracks      " << std::setw(12) << nmissedTracks << "   average per event "
             << nmissedTracks * evScale << std::endl
             << "  number of missed one station tracks        " << std::setw(12) << nmissedTracksOneStation << "   average per event "
             << nmissedTracksOneStation * evScale << std::endl;
        if (doSecondaries) {
            sout << "  number of secondary truth tracks           " << std::setw(12) << m_ntruthTracksSecondary << "   average per event "
                 << m_ntruthTracksSecondary * evScale << std::endl
                 << "  number of secondary good tracks            " << std::setw(12) << m_nmatchedTracksSecondary << "   average per event "
                 << m_nmatchedTracksSecondary * evScale << std::endl
                 << "  number of secondary missed multi st tracks " << std::setw(12) << m_nmissedTracksSecondary << "   average per event "
                 << m_nmissedTracksSecondary * evScale << std::endl
                 << "  number of secondary missed one st tracks   " << std::setw(12) << m_nmissedTracksOneStationSecondary
                 << "   average per event " << m_nmissedTracksOneStationSecondary * evScale << std::endl;
        }
        sout << "  number of fake tracks                      " << std::setw(12) << m_nfakeTracks << "   average per event "
             << m_nfakeTracks * evScale << std::endl
             << "  number of high pt fake tracks              " << std::setw(12) << m_nfakeTracksHighPt << "   average per event "
             << m_nfakeTracksHighPt * evScale << std::endl
             << "  number of low pt fake tracks               " << std::setw(12) << m_nfakeTracksLowPt << "   average per event "
             << m_nfakeTracksLowPt * evScale << std::endl
             << "  number of SL fake tracks                   " << std::setw(12) << m_nfakeTracksSL << "   average per event "
             << m_nfakeTracksSL * evScale << std::endl
             << "  number of matched fake tracks              " << std::setw(12) << m_nmatchedFakeTracks << "   average per event "
             << m_nmatchedFakeTracks * evScale << std::endl
             << "  number of tracks with lost momentum        " << std::setw(12) << m_nmissingStationMomLoss
             << "   average per truth track " << m_nmissingStationMomLoss * trTrkScale << std::endl
             << "  number of tracks missing precision layer   " << std::setw(12) << m_nmissingStationLayer << "   average per truth track "
             << m_nmissingStationLayer * trTrkScale << std::endl
             << "  number of tracks missing trigger layer     " << std::setw(12) << m_nmissingStationLayerTrigger
             << "   average per truth track " << m_nmissingStationLayerTrigger * trTrkScale << std::endl
             << "  number of tracks missing chamber           " << std::setw(12) << m_nmissingStation << "   average per truth track "
             << m_nmissingStation * trTrkScale << std::endl
             << "  number of tracks wrong precision layer     " << std::setw(12) << m_nwrongStationLayer << "   average per truth track "
             << m_nwrongStationLayer * trTrkScale << std::endl
             << "  number of tracks wrong trigger layer       " << std::setw(12) << m_nwrongStationLayerTrigger
             << "   average per truth track " << m_nwrongStationLayerTrigger * trTrkScale << std::endl
             << "  number of tracks wrong chamber             " << std::setw(12) << m_nwrongStation << "   average per truth track "
             << m_nwrongStation * trTrkScale << std::endl
             << "  efficiency: " << nmatchedTracks * trTrkScale << "  fake rate " << m_nfakeTracks * evScale << "  high pt "
             << m_nfakeTracksHighPt * evScale << "  low pt " << m_nfakeTracksLowPt * evScale << "  missed track rate (per truth track) "
             << nmissedTracks * trTrkScale;
        if (doSecondaries) sout << " secondary efficiency " << m_nmatchedTracksSecondary * trTrkScaleSec;
    }
    sout << std::endl;

    return sout.str();
}

std::pair<int, int> MuonTrackPerformanceAlg::countHitsInChamber(const Identifier& chId, const std::set<Identifier>& hitIds) const {
    // loop over hits in set, calculate their chID and compare it with the input chamber
    int nhitsPhi = m_idHelperSvc->isMdt(chId) ? -1 : 0;
    int nhitsEta = 0;
    std::set<Identifier>::const_iterator it = hitIds.begin();
    std::set<Identifier>::const_iterator it_end = hitIds.end();
    for (; it != it_end; ++it) {
        Identifier ch = m_idHelperSvc->chamberId(*it);
        if (ch == chId) {
            if (m_idHelperSvc->measuresPhi(*it))
                ++nhitsPhi;
            else
                ++nhitsEta;
        }
    }
    return std::make_pair(nhitsEta, nhitsPhi);
}

bool MuonTrackPerformanceAlg::insertChamber(const Identifier& chId, const std::set<Identifier>& hits, int minEtaHits, int minPhiHits,
                                            MuonTrackPerformanceAlg::ChamberData& chamberData) const {
    // flag whether chamber passes selection
    bool passesThreshold = false;

    // count eta and phi hits in chamber
    std::pair<int, int> missingHits = countHitsInChamber(chId, hits);
    if (missingHits.first >= minEtaHits) passesThreshold = true;
    if (missingHits.second >= minPhiHits) passesThreshold = true;


    // if inside cuts, copy the hits into summary
    if (passesThreshold) {
        chamberData.chId = chId;
        std::set<Identifier>::const_iterator it = hits.begin();
        std::set<Identifier>::const_iterator it_end = hits.end();
        for (; it != it_end; ++it) {
            Identifier ch = m_idHelperSvc->chamberId(*it);
            if (ch == chId) {
                chamberData.hits.insert(*it);
            }
        }
    }
    return passesThreshold;
}

bool MuonTrackPerformanceAlg::insertTechnology(const std::set<Identifier>& chIds, const std::set<Identifier>& hits, int minEtaHits,
                                               int minPhiHits, std::vector<MuonTrackPerformanceAlg::ChamberData>& chambers) const {
    // loop over chambers
    std::set<Identifier>::const_iterator it = chIds.begin();
    std::set<Identifier>::const_iterator it_end = chIds.end();
    for (; it != it_end; ++it) {
        ChamberData chData;
        if (insertChamber(*it, hits, minEtaHits, minPhiHits, chData)) chambers.push_back(chData);
    }
    return !chambers.empty();
}

bool MuonTrackPerformanceAlg::insertStationLayers(const std::set<Identifier>& chIds,
                                                  const std::set<Muon::MuonStationIndex::StIndex>& exclusionList,
                                                  std::set<Muon::MuonStationIndex::StIndex>& layers) const {
    unsigned int inputSize = layers.size();

    std::set<Identifier>::const_iterator chit = chIds.begin();
    std::set<Identifier>::const_iterator chit_end = chIds.end();
    for (; chit != chit_end; ++chit) {
        Muon::MuonStationIndex::StIndex stIndex = m_idHelperSvc->stationIndex(*chit);
        if (!exclusionList.count(stIndex)) layers.insert(stIndex);
    }

    return inputSize != layers.size();
}

bool MuonTrackPerformanceAlg::insertStationLayers(const std::vector<MuonTrackPerformanceAlg::ChamberData>& chambers,
                                                  const std::set<Muon::MuonStationIndex::StIndex>& exclusionList,
                                                  std::set<Muon::MuonStationIndex::StIndex>& layers, bool usePrecision) const {
    std::set<Identifier> chIds;
    std::vector<ChamberData>::const_iterator chit = chambers.begin();
    std::vector<ChamberData>::const_iterator chit_end = chambers.end();
    for (; chit != chit_end; ++chit) {
        bool isTrigger = m_idHelperSvc->isTrigger(chit->chId);
        if ((usePrecision && isTrigger) || (!usePrecision && !isTrigger)) continue;
        chIds.insert(m_idHelperSvc->chamberId(chit->chId));
    }
    return insertStationLayers(chIds, exclusionList, layers);
}

MuonTrackPerformanceAlg::TrackData* MuonTrackPerformanceAlg::evaluateTrackTruthOverlap(const Muon::MuonTrackTruth& truthTrack) const {
    TrackData* trackData = new TrackData();

    // handle missing chambers
    insertTechnology(truthTrack.mdts.missedChambers, truthTrack.mdts.missedHits, m_minMdtHits, 0, trackData->missingChambers);
    if (m_idHelperSvc->hasCSC())
        insertTechnology(truthTrack.cscs.missedChambers, truthTrack.cscs.missedHits, m_minCscEtaHits, m_minCscPhiHits,
                         trackData->missingChambers);
    if (m_idHelperSvc->hasSTGC())
        insertTechnology(truthTrack.stgcs.missedChambers, truthTrack.stgcs.missedHits, m_minsTgcEtaHits, m_minsTgcPhiHits,
                         trackData->missingChambers);
    if (m_idHelperSvc->hasMM())
        insertTechnology(truthTrack.mms.missedChambers, truthTrack.mms.missedHits, m_minMMEtaHits, 0, trackData->missingChambers);
    insertTechnology(truthTrack.rpcs.missedChambers, truthTrack.rpcs.missedHits, m_minRpcEtaHits, m_minRpcPhiHits,
                     trackData->missingChambers);
    insertTechnology(truthTrack.tgcs.missedChambers, truthTrack.tgcs.missedHits, m_minTgcEtaHits, m_minTgcPhiHits,
                     trackData->missingChambers);

    // handle wrong chambers
    insertTechnology(truthTrack.mdts.wrongChambers, truthTrack.mdts.wrongHits, m_minMdtHits, 0, trackData->wrongChambers);
    if (m_idHelperSvc->hasCSC())
        insertTechnology(truthTrack.cscs.wrongChambers, truthTrack.cscs.wrongHits, m_minCscEtaHits, m_minCscPhiHits,
                         trackData->wrongChambers);
    if (m_idHelperSvc->hasSTGC())
        insertTechnology(truthTrack.stgcs.wrongChambers, truthTrack.stgcs.wrongHits, m_minsTgcEtaHits, m_minsTgcPhiHits,
                         trackData->wrongChambers);
    if (m_idHelperSvc->hasMM())
        insertTechnology(truthTrack.mms.wrongChambers, truthTrack.mms.wrongHits, m_minMMEtaHits, 0, trackData->wrongChambers);
    insertTechnology(truthTrack.rpcs.wrongChambers, truthTrack.rpcs.wrongHits, m_minRpcEtaHits, m_minRpcPhiHits, trackData->wrongChambers);
    insertTechnology(truthTrack.tgcs.wrongChambers, truthTrack.tgcs.wrongHits, m_minTgcEtaHits, m_minTgcPhiHits, trackData->wrongChambers);

    // handle layer information for precision chambers
    std::set<Muon::MuonStationIndex::StIndex> dummyList;
    insertStationLayers(truthTrack.mdts.matchedChambers, dummyList, trackData->layers);
    if (m_idHelperSvc->hasCSC()) insertStationLayers(truthTrack.cscs.matchedChambers, dummyList, trackData->layers);
    if (m_idHelperSvc->hasMM()) insertStationLayers(truthTrack.mms.matchedChambers, dummyList, trackData->layers);

    insertStationLayers(trackData->missingChambers, trackData->layers, trackData->missingLayers, true);
    insertStationLayers(trackData->wrongChambers, trackData->layers, trackData->wrongLayers, true);

    // handle layer information for precision chambers
    insertStationLayers(truthTrack.rpcs.matchedChambers, dummyList, trackData->layersTrigger);
    insertStationLayers(truthTrack.tgcs.matchedChambers, dummyList, trackData->layersTrigger);
    if (m_idHelperSvc->hasSTGC()) insertStationLayers(truthTrack.stgcs.matchedChambers, dummyList, trackData->layersTrigger);

    insertStationLayers(trackData->missingChambers, trackData->layersTrigger, trackData->missingLayersTrigger, false);
    insertStationLayers(trackData->wrongChambers, trackData->layersTrigger, trackData->wrongLayersTrigger, false);

    return trackData;
}

std::string MuonTrackPerformanceAlg::print(const MuonTrackPerformanceAlg::TrackData& trackData) const {
    std::ostringstream sout;

    //   bool atIP = false;
    //   if( trackData.trackPars && trackData.trackPars->associatedSurface().center().mag() < 1000. ) atIP = true;

    if (trackData.truthTrack) {
        double charge = trackData.truthTrack->GetPDGCode() < 0 ? +1 : -1;
        Trk::Perigee perigee(Amg::Vector3D(trackData.truthTrack->GetPosition().x(), trackData.truthTrack->GetPosition().y(),
                                           trackData.truthTrack->GetPosition().z()),
                             Amg::Vector3D(trackData.truthTrack->GetMomentum().x(), trackData.truthTrack->GetMomentum().y(),
                                           trackData.truthTrack->GetMomentum().z()),
                             charge, Trk::PerigeeSurface(Amg::Vector3D(0., 0., 0.)));
        sout << "Truth: " << m_printer->print(perigee);  // << " barcode " << trackData.truthTrack->GetBarCode();
        if (std::abs(trackData.truthTrack->GetPDGCode()) == 13) {
            if (trackData.motherPdg != -1) sout << " mother " << trackData.motherPdg;
        } else {
            sout << " pdg " << trackData.truthTrack->GetPDGCode();
        }
        if (trackData.momentumAtProduction) sout << " production p:  " << trackData.momentumAtProduction->mag();

        // if( trackData.productionVertex ) sout << " production vertex: r  " << trackData.productionVertex->perp()
        //     				  << " z " << trackData.productionVertex->z();
        sout << std::endl;
    }

    if (trackData.trackPars) {
        sout << "Track: " << m_printer->print(*trackData.trackPars) << " chi2/ndof " << trackData.chi2Ndof << std::endl;
        if (trackData.trackPars->covariance()) {
            double qOverP = std::abs(trackData.trackPars->parameters()[Trk::qOverP]);
            double dpp = 0.;
            double cov00 = (*trackData.trackPars->covariance())(0, 0);
            double cov11 = (*trackData.trackPars->covariance())(1, 1);
            double cov22 = (*trackData.trackPars->covariance())(2, 2);
            double cov33 = (*trackData.trackPars->covariance())(3, 3);
            double cov44 = (*trackData.trackPars->covariance())(4, 4);
            if (qOverP > 0) dpp = std::sqrt(cov44) / qOverP;
            sout << " error d0 " << std::sqrt(cov00) << " z0 " << std::sqrt(cov11) << " phi (mrad) " << 1000 * std::sqrt(cov22)
                 << " theta (mrad) " << 1000 * std::sqrt(cov33) << " dp/p " << dpp << std::endl;
        }

        if (trackData.trackSummary && trackData.trackSummary->muonTrackSummary())
            sout << m_printer->print(*trackData.trackSummary->muonTrackSummary()) << std::endl;
    }

    if (!trackData.missingChambers.empty()) {
        sout << " Missing Chambers: " << std::endl;
        std::vector<ChamberData>::const_iterator chIt = trackData.missingChambers.begin();
        std::vector<ChamberData>::const_iterator chIt_end = trackData.missingChambers.end();
        for (; chIt != chIt_end; ++chIt) {
            sout << "    " << m_idHelperSvc->toStringChamber(chIt->chId) << " hits " << chIt->hits.size() << std::endl;
            if (m_doTrackDebug >= 6) {
                std::set<Identifier>::const_iterator hit = chIt->hits.begin();
                std::set<Identifier>::const_iterator hit_end = chIt->hits.end();
                for (; hit != hit_end; ++hit) { sout << "          " << m_idHelperSvc->toString(*hit) << std::endl; }
            }
        }
    }

    if (!trackData.wrongChambers.empty()) {
        sout << " Wrong Chambers: " << std::endl;
        std::vector<ChamberData>::const_iterator chIt = trackData.wrongChambers.begin();
        std::vector<ChamberData>::const_iterator chIt_end = trackData.wrongChambers.end();
        for (; chIt != chIt_end; ++chIt) {
            sout << "    " << m_idHelperSvc->toStringChamber(chIt->chId) << " hits " << chIt->hits.size() << std::endl;
            if (m_doTrackDebug >= 6) {
                std::set<Identifier>::const_iterator hit = chIt->hits.begin();
                std::set<Identifier>::const_iterator hit_end = chIt->hits.end();
                for (; hit != hit_end; ++hit) { sout << "          " << m_idHelperSvc->toString(*hit) << std::endl; }
            }
        }
    }

    if (!trackData.missingLayers.empty() || !trackData.missingLayersTrigger.empty()) {
        sout << " Missing Layers: ";
        if (!trackData.missingLayers.empty()) {
            sout << " Precision: ";
            std::set<Muon::MuonStationIndex::StIndex>::const_iterator it = trackData.missingLayers.begin();
            std::set<Muon::MuonStationIndex::StIndex>::const_iterator it_end = trackData.missingLayers.end();
            for (; it != it_end; ++it) sout << " " << Muon::MuonStationIndex::stName(*it);
        }

        if (!trackData.missingLayersTrigger.empty()) {
            sout << " Trigger: ";
            std::set<Muon::MuonStationIndex::StIndex>::const_iterator it = trackData.missingLayersTrigger.begin();
            std::set<Muon::MuonStationIndex::StIndex>::const_iterator it_end = trackData.missingLayersTrigger.end();
            for (; it != it_end; ++it) sout << " " << Muon::MuonStationIndex::stName(*it);
        }
        sout << std::endl;
    }

    if (!trackData.wrongLayers.empty() || !trackData.wrongLayersTrigger.empty()) {
        sout << " TrackData.Wrong Layers: ";
        if (!trackData.wrongLayers.empty()) {
            sout << " Precision: ";
            std::set<Muon::MuonStationIndex::StIndex>::const_iterator it = trackData.wrongLayers.begin();
            std::set<Muon::MuonStationIndex::StIndex>::const_iterator it_end = trackData.wrongLayers.end();
            for (; it != it_end; ++it) sout << " " << Muon::MuonStationIndex::stName(*it);
        }

        if (!trackData.wrongLayersTrigger.empty()) {
            sout << " Trigger: ";
            std::set<Muon::MuonStationIndex::StIndex>::const_iterator it = trackData.wrongLayersTrigger.begin();
            std::set<Muon::MuonStationIndex::StIndex>::const_iterator it_end = trackData.wrongLayersTrigger.end();
            for (; it != it_end; ++it) sout << " " << Muon::MuonStationIndex::stName(*it);
        }
        sout << std::endl;
    }
    return sout.str();
}

void MuonTrackPerformanceAlg::addTrackToTrackData(const Trk::Track& track, MuonTrackPerformanceAlg::TrackData& trackData) const {
    trackData.trackPars = track.perigeeParameters() ? new Trk::Perigee(*track.perigeeParameters()) : nullptr;
    trackData.chi2Ndof = 0.;

    if (track.fitQuality() && track.fitQuality()->numberDoF())
        trackData.chi2Ndof = track.fitQuality()->chiSquared() / track.fitQuality()->numberDoF();

    trackData.trackSummary = track.trackSummary() ? new Trk::TrackSummary(*track.trackSummary()) : nullptr;
    if (trackData.trackSummary && !trackData.trackSummary->muonTrackSummary()) {
        m_summaryHelperTool->addDetailedTrackSummary(track, *trackData.trackSummary);
    }
}

MuonTrackPerformanceAlg::TrackData* MuonTrackPerformanceAlg::createTrackData(
    const Muon::IMuonTrackTruthTool::TruthTreeEntry& trackTruth) const {
    if (!trackTruth.truthTrack) { return nullptr; }

    TrackData* trackData = new TrackData();

    std::map<Identifier, std::set<Identifier> > chambers;

    MuonSimDataCollection::const_iterator mit = trackTruth.mdtHits.begin();
    MuonSimDataCollection::const_iterator mit_end = trackTruth.mdtHits.end();
    for (; mit != mit_end; ++mit) {
        Identifier chId = m_idHelperSvc->chamberId(mit->first);
        chambers[chId].insert(mit->first);
    }

    if (m_idHelperSvc->hasCSC()) {
        CscSimDataCollection::const_iterator cit = trackTruth.cscHits.begin();
        CscSimDataCollection::const_iterator cit_end = trackTruth.cscHits.end();
        for (; cit != cit_end; ++cit) {
            Identifier chId = m_idHelperSvc->chamberId(cit->first);
            chambers[chId].insert(m_idHelperSvc->layerId(cit->first));
        }
    }
    if (m_idHelperSvc->hasSTGC()) {
        mit = trackTruth.stgcHits.begin();
        mit_end = trackTruth.stgcHits.end();
        for (; mit != mit_end; ++mit) {
            Identifier chId = m_idHelperSvc->chamberId(mit->first);
            chambers[chId].insert(mit->first);
        }
    }
    if (m_idHelperSvc->hasMM()) {
        mit = trackTruth.mmHits.begin();
        mit_end = trackTruth.mmHits.end();
        for (; mit != mit_end; ++mit) {
            Identifier chId = m_idHelperSvc->chamberId(mit->first);
            chambers[chId].insert(mit->first);
        }
    }

    mit = trackTruth.rpcHits.begin();
    mit_end = trackTruth.rpcHits.end();
    for (; mit != mit_end; ++mit) {
        Identifier chId = m_idHelperSvc->chamberId(mit->first);
        chambers[chId].insert(mit->first);
    }

    mit = trackTruth.tgcHits.begin();
    mit_end = trackTruth.tgcHits.end();
    for (; mit != mit_end; ++mit) {
        Identifier chId = m_idHelperSvc->chamberId(mit->first);
        chambers[chId].insert(mit->first);
    }

    std::vector<ChamberData> missingChambers;
    std::map<Identifier, std::set<Identifier> >::const_iterator chIt = chambers.begin();
    std::map<Identifier, std::set<Identifier> >::const_iterator chIt_end = chambers.end();
    for (; chIt != chIt_end; ++chIt) {
        ChamberData chamberData;
        unsigned int minEtaHits = 0;
        unsigned int minPhiHits = 0;
        if (m_idHelperSvc->isMdt(chIt->first)) {
            minEtaHits = m_minMdtHits;
            minPhiHits = m_minMdtHits;
        } else if (m_idHelperSvc->isRpc(chIt->first)) {
            minEtaHits = m_minRpcEtaHits;
            minPhiHits = m_minRpcPhiHits;
        } else if (m_idHelperSvc->isTgc(chIt->first)) {
            minEtaHits = m_minTgcEtaHits;
            minPhiHits = m_minTgcPhiHits;
        } else if (m_idHelperSvc->issTgc(chIt->first)) {
            minEtaHits = m_minsTgcEtaHits;
            minPhiHits = m_minsTgcPhiHits;
        } else if (m_idHelperSvc->isMM(chIt->first)) {
            minEtaHits = m_minMMEtaHits;
            minPhiHits = m_minMMEtaHits;
        } else if (m_idHelperSvc->isCsc(chIt->first)) {
            minEtaHits = m_minCscEtaHits;
            minPhiHits = m_minCscPhiHits;
        } else {
            ATH_MSG_WARNING("unexpected identifier");
            continue;
        }
        if (insertChamber(chIt->first, chIt->second, minEtaHits, minPhiHits, chamberData)) {
            trackData->missingChambers.push_back(chamberData);
            Muon::MuonStationIndex::StIndex stIndex = m_idHelperSvc->stationIndex(chIt->first);
            if (m_idHelperSvc->isTrigger(chIt->first))
                trackData->missingLayersTrigger.insert(stIndex);
            else
                trackData->missingLayers.insert(stIndex);
        }
    }
    return trackData;
}

void MuonTrackPerformanceAlg::clearTracks(std::vector<const Trk::Track*> tracks) {
    std::vector<const Trk::Track*>::const_iterator it = tracks.begin();
    std::vector<const Trk::Track*>::const_iterator it_end = tracks.end();
    for (; it != it_end; ++it) delete *it;
    tracks.clear();
}

void MuonTrackPerformanceAlg::clearTracks(std::vector<MuonTrackPerformanceAlg::TrackData*> tracks) {
    std::vector<TrackData*>::const_iterator it = tracks.begin();
    std::vector<TrackData*>::const_iterator it_end = tracks.end();
    for (; it != it_end; ++it) { delete *it; }
    tracks.clear();
}

void MuonTrackPerformanceAlg::clearEvent(MuonTrackPerformanceAlg::EventData& event) {
    clearTracks(event.missingTruthTracks);
    clearTracks(event.missingTruthTracksOneStation);

    clearTracks(event.missingStationMomLoss);
    clearTracks(event.missingStationLayer);
    clearTracks(event.missingStation);

    clearTracks(event.wrongStationLayer);
    clearTracks(event.wrongStation);

    clearTracks(event.fakeTracks);
    clearTracks(event.fakeTracksLowPt);
    clearTracks(event.fakeTracksSL);
}

HepMC::ConstGenParticlePtr MuonTrackPerformanceAlg::getMother(const TruthTrajectory& traj) const {
    std::vector<HepMcParticleLink>::const_reverse_iterator pit = traj.rbegin();
    std::vector<HepMcParticleLink>::const_reverse_iterator pit_end = traj.rend();
    for (; pit != pit_end; ++pit) {
        if (std::abs((*pit)->pdg_id()) != 13) return *pit;
    }
    return nullptr;
}

HepMC::ConstGenParticlePtr MuonTrackPerformanceAlg::getInitialState(const TruthTrajectory& traj) const {
    std::vector<HepMcParticleLink>::const_reverse_iterator pit = traj.rbegin();
    std::vector<HepMcParticleLink>::const_reverse_iterator pit_end = traj.rend();
    for (; pit != pit_end; ++pit) {
        if (std::abs((*pit)->pdg_id()) != 13) {
            if (pit != traj.rbegin())
                --pit;
            else
                return nullptr;
            return *pit;
        }
    }
    return nullptr;
}

bool MuonTrackPerformanceAlg::isSecondary(const Muon::MuonTrackTruth& truthTrack) const {
    if (!truthTrack.truthTrajectory) return false;
    return isSecondary(*truthTrack.truthTrajectory);
}

bool MuonTrackPerformanceAlg::isSecondary(const Muon::IMuonTrackTruthTool::TruthTreeEntry& entry) const {
    if (!entry.truthTrajectory) return false;
    return isSecondary(*entry.truthTrajectory);
}

bool MuonTrackPerformanceAlg::isSecondary(const TruthTrajectory& truthTrajectory) const {
    HepMC::ConstGenParticlePtr mother = getMother(truthTrajectory);
    if (mother && mother->end_vertex() && mother->end_vertex()->position().perp() > 100.) return true;
    return false;
}
