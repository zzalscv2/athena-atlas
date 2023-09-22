/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonStauRecoTool.h"

#include <memory>

#include "EventPrimitives/EventPrimitivesHelpers.h"
#include "GaudiKernel/ConcurrencyFlags.h"
#include "MdtCalibData/IRtRelation.h"
#include "MdtCalibData/IRtResolution.h"
#include "MdtCalibData/MdtFullCalibData.h"
#include "MdtCalibData/TrRelation.h"
#include "MuonCompetingRIOsOnTrack/CompetingMuonClustersOnTrack.h"
#include "MuonRIO_OnTrack/CscClusterOnTrack.h"
#include "MuonRIO_OnTrack/MdtDriftCircleOnTrack.h"
#include "MuonRIO_OnTrack/MuonDriftCircleErrorStrategy.h"
#include "MuonRIO_OnTrack/RpcClusterOnTrack.h"
#include "MuonSegment/MuonSegment.h"
#include "TrkDriftCircleMath/DCSLFitter.h"
#include "TrkDriftCircleMath/Line.h"
#include "TrkDriftCircleMath/LocVec2D.h"
#include "TrkDriftCircleMath/Segment.h"
#include "TrkDriftCircleMath/SegmentFinder.h"
#include "TrkDriftCircleMath/TransformToLine.h"
#include "xAODTruth/TruthParticleContainer.h"

namespace {
    constexpr double inverseSpeedOfLight = 1 / Gaudi::Units::c_light;  // need 1/299.792458 inside calculateTof()/calculateBeta()

}

namespace MuonCombined {

    std::string printIntersectionToString(const Muon::MuonSystemExtension::Intersection& intersection) {
        std::ostringstream sout;
        sout << " sector " << intersection.layerSurface.sector << " "
             << Muon::MuonStationIndex::regionName(intersection.layerSurface.regionIndex) << " "
             << Muon::MuonStationIndex::layerName(intersection.layerSurface.layerIndex);
        return sout.str();
    }

    MuonStauRecoTool::MuonStauRecoTool(const std::string& type, const std::string& name, const IInterface* parent) :
        AthAlgTool(type, name, parent) {}

    StatusCode MuonStauRecoTool::initialize() {
        ATH_CHECK(m_idHelperSvc.retrieve());
        ATH_CHECK(m_printer.retrieve());
        ATH_CHECK(m_edmHelperSvc.retrieve());
        ATH_CHECK(m_segmentMaker.retrieve());
        ATH_CHECK(m_segmentMakerT0Fit.retrieve());
        ATH_CHECK(m_segmentMatchingTool.retrieve());
        ATH_CHECK(m_trackAmbibuityResolver.retrieve());
        ATH_CHECK(m_hitTimingTool.retrieve());
        ATH_CHECK(m_muonPRDSelectionTool.retrieve());
        ATH_CHECK(m_muonPRDSelectionToolStau.retrieve());
        ATH_CHECK(m_mdtCreator.retrieve());
        ATH_CHECK(m_mdtCreatorStau.retrieve());
        ATH_CHECK(m_insideOutRecoTool.retrieve());
        ATH_CHECK(m_updator.retrieve());
        ATH_CHECK(m_calibDbKey.initialize());
        ATH_CHECK(m_houghDataPerSectorVecKey.initialize(!m_houghDataPerSectorVecKey.empty()));

        if (m_doTruth) {
            // add pdgs from jobO to set
            for (auto pdg : m_pdgsToBeConsidered.value()) { m_selectedPdgs.insert(pdg); }
        }
        return StatusCode::SUCCESS;
    }

    void MuonStauRecoTool::extendWithPRDs(const InDetCandidateCollection& inDetCandidates, InDetCandidateToTagMap* tagMap,
                                          IMuonCombinedInDetExtensionTool::MuonPrdData prdData, TrackCollection* combTracks,
                                          TrackCollection* meTracks, Trk::SegmentCollection* segments, const EventContext& ctx) const {
        // Maybe we'll need this later, I wouldn't be surprised if the PRDs are retrieved somewhere down the chain
        // For now it's just a placeholder though
        if (!prdData.mdtPrds) ATH_MSG_DEBUG("empty PRDs passed");
        extend(inDetCandidates, tagMap, combTracks, meTracks, segments, ctx);
    }

    void MuonStauRecoTool::extend(const InDetCandidateCollection& inDetCandidates, InDetCandidateToTagMap* tagMap,
                                  TrackCollection* combTracks, TrackCollection* meTracks, Trk::SegmentCollection* segments,
                                  const EventContext& ctx) const {
        ATH_MSG_DEBUG(" extending " << inDetCandidates.size());

        if (meTracks) ATH_MSG_DEBUG("Not currently creating ME tracks for staus");
        for (const InDetCandidate* it : inDetCandidates) { handleCandidate(ctx, *it, tagMap, combTracks, segments); }
    }

    MuonStauRecoTool::TruthInfo* MuonStauRecoTool::getTruth(const xAOD::TrackParticle& indetTrackParticle) const {
        // in case we are using the truth, check if the truth link is set and create the TruthInfo object
        if (m_doTruth && indetTrackParticle.isAvailable<ElementLink<xAOD::TruthParticleContainer>>("truthParticleLink")) {
            const ElementLink<xAOD::TruthParticleContainer>& truthLink =
                indetTrackParticle.auxdata<ElementLink<xAOD::TruthParticleContainer>>("truthParticleLink");
            if (truthLink.isValid()) { return new TruthInfo((*truthLink)->pdgId(), (*truthLink)->m(), (*truthLink)->p4().Beta()); }
        }
        return nullptr;
    }

    void MuonStauRecoTool::handleCandidate(const EventContext& ctx, const InDetCandidate& indetCandidate, InDetCandidateToTagMap* tagMap,
                                           TrackCollection* combTracks, Trk::SegmentCollection* segments) const {
        if (m_ignoreSiAssocated && indetCandidate.isSiliconAssociated()) {
            ATH_MSG_DEBUG(" skip silicon associated track for extension ");
            return;
        }

        /** STAGE 0
            Preselection, preparation of truth related quantities, extrapolation in muon system
         */

        // get TrackParticle and apply the kinematic selection
        const xAOD::TrackParticle& indetTrackParticle = indetCandidate.indetTrackParticle();
        if (!indetTrackParticle.track() || indetTrackParticle.pt() < m_ptThreshold) return;

        // get truth info (will be zero pointer if running on data or m_doTruth == false)
        std::unique_ptr<TruthInfo> truthInfo(getTruth(indetTrackParticle));

        // if truth based reconstruction is enabled, check whether to accept the given pdgId
        if (!selectTruth(truthInfo.get())) {
            ATH_MSG_DEBUG("Truth reconstruction enabled: skipping ID track with pdgId: " << (truthInfo ? truthInfo->pdgId : 0));
            return;
        }

        // get intersections which precision layers in the muon system
        const Muon::MuonSystemExtension* muonSystemExtension = indetCandidate.getExtension();

        // summary for selected ID track
        if (m_doSummary || msgLvl(MSG::DEBUG)) {
            msg(MSG::INFO) << " ID track: pt " << indetTrackParticle.pt() << " eta " << indetTrackParticle.eta() << " phi "
                           << indetTrackParticle.phi();
            if (truthInfo) msg(MSG::INFO) << truthInfo->toString();
            if (!muonSystemExtension) msg(MSG::INFO) << " failed muonSystemExtension";
            msg(MSG::INFO) << endmsg;
        }

        // exit if no MuonSystemExtension was found
        if (!muonSystemExtension) { return; }

        // fill validation content
        if (!m_recoValidationTool.empty()) m_recoValidationTool->addTrackParticle(indetTrackParticle, *muonSystemExtension);
        /** STAGE 1
           process the muon system extension: loop over intersections, get associated data, time measurement, build beta seeds
        */

        AssociatedData associatedData;
        if (!extractTimeMeasurements(ctx, *muonSystemExtension, associatedData)) { return; }

        /** STAGE 2
            build candidates from seeds in the chamber layers
         */

        CandidateVec candidates;
        if (!createCandidates(associatedData, candidates)) { return; }

        if (!m_recoValidationTool.empty()) addCandidatesToNtuple(indetTrackParticle, candidates, 0);

        /** STAGE 3
            refine candidates: find segments using the beta seed of the candidate
        */

        if (!refineCandidates(ctx, candidates)) { return; }

        if (!m_recoValidationTool.empty()) addCandidatesToNtuple(indetTrackParticle, candidates, 1);

        /** STAGE 4
            combineCandidates: run the combined reconstruction
        */

        if (!combineCandidates(ctx, indetTrackParticle, candidates)) { return; }

        if (!m_recoValidationTool.empty()) addCandidatesToNtuple(indetTrackParticle, candidates, 2);
        /** STAGE 5
           resolve ambiguities
        */

        if (!resolveAmbiguities(candidates)) { return; }

        if (!m_recoValidationTool.empty()) addCandidatesToNtuple(indetTrackParticle, candidates, 3);
        /** STAGE 6
            create tag
        */
        addTag(indetCandidate, *candidates.front(), tagMap, combTracks, segments);
    }

    bool MuonStauRecoTool::refineCandidates(const EventContext& ctx, MuonStauRecoTool::CandidateVec& candidates) const {
        // keep track of candidates for which segments are found
        CandidateVec refinedCandidates;

        // loop over candidates and redo segments using beta estimate from candidate
        ATH_MSG_DEBUG("Refining candidates " << candidates.size());
        for (auto& candidate : candidates) {
            ATH_MSG_DEBUG("   candidate: betaseed beta" << candidate->betaSeed.beta << ", error" << candidate->betaSeed.error
                                                        << " layerDataVec size" << candidate->layerDataVec.size() << " hits size"
                                                        << candidate->hits.size());

            // loop over layers and perform segment finding, collect segments per layer
            for (const auto& layerData : candidate->layerDataVec) {
                // store segments in layer
                std::vector<std::shared_ptr<const Muon::MuonSegment>> segments;

                // loop over maxima
                for (const auto& maximumData : layerData.maximumDataVec) {
                    // find segments for intersection
                    findSegments(layerData.intersection, *maximumData, segments, m_muonPRDSelectionToolStau, m_segmentMaker);
                }

                // skip if no segment were found
                if (segments.empty()) continue;

                // fill validation content
                if (!m_recoValidationTool.empty()) {
                    for (const auto& seg : segments) m_recoValidationTool->add(layerData.intersection, *seg, 2);
                }

                // match segments to intersection, store the ones that match
                std::vector<std::shared_ptr<const Muon::MuonSegment>> selectedSegments;
                m_segmentMatchingTool->select(ctx, layerData.intersection, segments, selectedSegments);
                // fill validation content
                if (!m_recoValidationTool.empty()) {
                    for (const auto& seg : selectedSegments) m_recoValidationTool->add(layerData.intersection, *seg, 3);
                }

                // add layer list
                candidate->allLayers.emplace_back(layerData.intersection, std::move(selectedSegments));
            }

            // keep candidate if any segments were found
            if (!candidate->allLayers.empty()) refinedCandidates.push_back(candidate);
        }

        // set candidates to the refinedCandidates
        candidates = refinedCandidates;

        // print results afer refineCandidate
        if (m_doSummary || msgLvl(MSG::DEBUG)) {
            msg(MSG::INFO) << " Summary::refineCandidates ";
            if (candidates.empty())
                msg(MSG::INFO) << " No candidated found ";
            else
                msg(MSG::INFO) << " candidates " << candidates.size();

            for (const auto& candidate : candidates) {
                msg(MSG::INFO) << std::endl
                               << "  candidate: beta fit result: beta " << candidate->betaFitResult.beta << " chi2/ndof "
                               << candidate->betaFitResult.chi2PerDOF() << " layers with segments" << candidate->allLayers.size();
                for (const auto& layer : candidate->allLayers)
                    msg(MSG::INFO) << std::endl
                                   << "     " << printIntersectionToString(layer.intersection) << " segments " << layer.segments.size();
            }
            msg(MSG::INFO) << endmsg;
        }

        return !candidates.empty();
    }

    void MuonStauRecoTool::extractTimeMeasurementsFromTrack(const EventContext& ctx, MuonStauRecoTool::Candidate& candidate) const {
        SG::ReadCondHandle<MuonCalib::MdtCalibDataContainer> mdtCalibConstants{m_calibDbKey, ctx};
        if (!mdtCalibConstants.isValid()) {
            ATH_MSG_FATAL("Failed to retrieve calibration constants "<<m_calibDbKey.fullKey());
            throw std::runtime_error("Failed to retrieve calibration constants");
        }
        ATH_MSG_VERBOSE("extractTimeMeasurementsFromTrack for candidate: beta seed " << candidate.betaSeed.beta);
        Trk::Track* combinedTrack = candidate.combinedTrack.get();
        if (!combinedTrack) return;

        // select around seed
        float betaSeed = candidate.betaFitResult.beta;

        // fitter + hits
        Muon::TimePointBetaFitter fitter;
        Muon::TimePointBetaFitter::HitVec hits;

        // loop over track and calculate residuals
        const Trk::TrackStates* states = combinedTrack->trackStateOnSurfaces();
        if (!states) {
            ATH_MSG_WARNING(" track without states, cannot extractTimeMeasurementsFromTrack ");
            return;
        }

        ATH_MSG_VERBOSE("Track : " << (*combinedTrack));

        // store RPC prds for clustering
        typedef std::vector<const Muon::MuonClusterOnTrack*> RpcClVec;
        using RpcClPerChMap = std::map<Identifier, std::tuple<const Trk::TrackParameters*, RpcClVec, RpcClVec>>;
        RpcClPerChMap rpcPrdsPerChamber;

        using MdtTubeData = std::pair<const Trk::TrackParameters*, const Muon::MdtDriftCircleOnTrack*>;
        using MdtTubeDataVec = std::vector<MdtTubeData>;
        using MdtChamberLayerData = std::map<int, MdtTubeDataVec>;
        MdtChamberLayerData mdtDataPerChamberLayer;

        // loop over TSOSs
        Trk::TrackStates::const_iterator tsit = states->begin();
        Trk::TrackStates::const_iterator tsit_end = states->end();
        for (; tsit != tsit_end; ++tsit) {
            const Trk::TrackParameters* pars = (*tsit)->trackParameters();
            if (!pars) continue;

            // check whether state is a measurement
            const Trk::MeasurementBase* meas = (*tsit)->measurementOnTrack();
            if (!meas || (*tsit)->type(Trk::TrackStateOnSurface::Outlier)) continue;

            // get Identifier and skip pseudo measurements, ID hits and all but MDT/RPC hits
            Identifier id = m_edmHelperSvc->getIdentifier(*meas);
            if (!id.is_valid() || !m_idHelperSvc->isMuon(id)) continue;

            // extract time measurements for RPCs
            if (m_idHelperSvc->isMdt(id)) {
                // MDTs
                const Muon::MdtDriftCircleOnTrack* mdt = dynamic_cast<const Muon::MdtDriftCircleOnTrack*>(meas);
                if (!mdt) continue;

                if (m_segmentMDTT) {
                    int chIndexWithBIR = m_idHelperSvc->chamberIndex(mdt->identify());
                    if (chIndexWithBIR == Muon::MuonStationIndex::BIL) {
                        std::string stName = m_idHelperSvc->chamberNameString(id);
                        if (stName[2] == 'R') { chIndexWithBIR += 1000; }
                    }
                    mdtDataPerChamberLayer[chIndexWithBIR].push_back(std::make_pair(pars, mdt));
                } else {
                    MuGirlNS::StauHitTechnology tech = MuGirlNS::MDTT_STAU_HIT;
                    float distance = pars->position().mag();
                    float time = 0.;

                    float ix = pars->position().x();
                    float iy = pars->position().y();
                    float iz = pars->position().z();
                    float ie = 0.;
                    float er = -1;
                    float sh = 0;
                    bool isEta = !m_idHelperSvc->measuresPhi(id);
                    float propTime = 0;
                    float tof = calculateTof(1, distance);

                    // use inverted RT relation together with track prediction to get estimate of drift time
                    float driftTime = mdt->driftTime();  // we need to add beta seed as it was subtracted when calibrating the hits
                    float locR = pars->parameters()[Trk::locR];
                    float errR = pars->covariance() ? Amg::error(*pars->covariance(), Trk::locR) : 0.3;
                    auto data = mdtCalibConstants->getCalibData(id, msgStream());
                    const auto& rtRelation = data->rtRelation;
                    bool out_of_bound_flag = false;
                    float drdt = rtRelation->rt()->driftvelocity(driftTime);
                    float rres = rtRelation->rtRes()->resolution(driftTime);
                    float tres = rres / drdt;
                    float TlocR = rtRelation->tr()->tFromR(std::abs(locR), out_of_bound_flag);
                    float trackTimeRes = errR / drdt;
                    float tofShiftFromBeta = calculateTof(betaSeed, distance) - tof;
                    er = std::sqrt(tres * tres + trackTimeRes * trackTimeRes);
                    mdtTimeCalibration(id, driftTime, er);
                    time = driftTime - TlocR + tofShiftFromBeta;
                    propTime = driftTime;
                    ie = trackTimeRes;
                    // try removal of hit from fit
                    if (!m_updator.empty()) {
                        std::unique_ptr<const Trk::TrackParameters> unbiasedPars(
                            m_updator->removeFromState(*pars, meas->localParameters(), meas->localCovariance()));
                        if (unbiasedPars) {
                            float locRu = unbiasedPars->parameters()[Trk::locR];
                            float TlocRu = rtRelation->tr()->tFromR(std::abs(locRu), out_of_bound_flag);
                            float errRu = unbiasedPars->covariance() ? Amg::error(*unbiasedPars->covariance(), Trk::locR) : 0.3;
                            float trackTimeResu = errRu / drdt;
                            sh = TlocR - TlocRu;
                            time = driftTime - TlocRu + tofShiftFromBeta;
                            er = std::sqrt(tres * tres + trackTimeResu * trackTimeResu);
                            ie = trackTimeResu;
                            ATH_MSG_VERBOSE(" Got unbiased parameters: r " << locR << " ur " << locRu << " err " << errR << " uerr "
                                                                           << errRu << " terr " << trackTimeRes << " terru "
                                                                           << trackTimeResu);
                        }
                    }

                    ATH_MSG_VERBOSE(" MDT  " << mdt->driftRadius() << " locR " << locR << " err " << errR << " drift time " << driftTime
                                             << " TlocR " << TlocR << " diff " << driftTime - TlocR << " tofShift " << tofShiftFromBeta
                                             << " time " << time << " err " << er << " intrinsic " << tres << " track " << trackTimeRes);

                    float beta = calculateBeta(time + tof, distance);
                    ATH_MSG_VERBOSE("  adding " << m_idHelperSvc->toString(id) << " distance " << distance << " time " << time << " beta"
                                                << beta << " diff " << std::abs(beta - betaSeed));
                    if (std::abs(beta - betaSeed) > m_mdttBetaAssociationCut) continue;

                    hits.push_back(Muon::TimePointBetaFitter::Hit(distance, time, er));
                    candidate.stauHits.push_back(MuGirlNS::StauHit(tech, time + tof, ix, iy, iz, id, ie, er, sh, isEta, propTime));
                }
            } else if (m_idHelperSvc->isRpc(id)) {
                // treat CompetingMuonClustersOnTrack differently than RpcClusterOnTrack
                std::vector<const Muon::MuonClusterOnTrack*> clusters;
                const Muon::CompetingMuonClustersOnTrack* crot = dynamic_cast<const Muon::CompetingMuonClustersOnTrack*>(meas);
                if (crot) {
                    clusters = crot->containedROTs();
                } else {
                    const Muon::RpcClusterOnTrack* rpc = dynamic_cast<const Muon::RpcClusterOnTrack*>(meas);
                    if (rpc) clusters.push_back(rpc);
                }
                Identifier chamberId = m_idHelperSvc->chamberId(id);
                bool measuresPhi = m_idHelperSvc->measuresPhi(id);
                auto pos = rpcPrdsPerChamber.find(chamberId);
                if (pos == rpcPrdsPerChamber.end()) {
                    if (measuresPhi)
                        rpcPrdsPerChamber[chamberId] = std::make_tuple(pars, clusters, RpcClVec());
                    else
                        rpcPrdsPerChamber[chamberId] = std::make_tuple(pars, RpcClVec(), clusters);
                } else {
                    RpcClVec& clVec = measuresPhi ? std::get<1>(pos->second) : std::get<2>(pos->second);
                    clVec.insert(clVec.end(), clusters.begin(), clusters.end());
                }
            } else if (m_idHelperSvc->isCsc(id)) {
                const Muon::CscClusterOnTrack* csc = dynamic_cast<const Muon::CscClusterOnTrack*>(meas);

                MuGirlNS::StauHitTechnology tech = MuGirlNS::CSC_STAU_HIT;
                float distance = pars->position().mag();
                float time = csc->prepRawData()->time();

                float ix = pars->position().x();
                float iy = pars->position().y();
                float iz = pars->position().z();
                float ie = 0.;
                float er = -1;
                float sh = 0;
                bool isEta = !m_idHelperSvc->measuresPhi(id);
                float propTime = 0;
                float tof = calculateTof(1, distance);
                candidate.stauHits.push_back(MuGirlNS::StauHit(tech, time + tof, ix, iy, iz, id, ie, er, sh, isEta, propTime));
            }
        }

        auto insertRpcs = [betaSeed, this](const Trk::TrackParameters& pars, const RpcClVec& clusters,
                                           MuonStauRecoTool::Candidate& candidate, Muon::TimePointBetaFitter::HitVec& hits) {
            if (clusters.empty()) return;

            std::vector<const Muon::MuonClusterOnTrack*> calibratedClusters;
            for (const auto* cluster : clusters) {
                const Muon::MuonClusterOnTrack* cl = m_muonPRDSelectionTool->calibrateAndSelect(pars, *cluster->prepRawData());
                if (cl) calibratedClusters.push_back(cl);
            }
            if (calibratedClusters.empty()) return;

            Muon::IMuonHitTimingTool::TimingResult result = m_hitTimingTool->calculateTimingResult(calibratedClusters);
            for (const auto* cl : calibratedClusters) delete cl;
            if (!result.valid) return;

            Identifier id = clusters.front()->identify();

            MuGirlNS::StauHitTechnology tech = MuGirlNS::RPC_STAU_HIT;
            float distance = pars.position().mag();
            float time = result.time;
            float ix = pars.position().x();
            float iy = pars.position().y();
            float iz = pars.position().z();
            float ie = 0.;
            float er = result.error;
            rpcTimeCalibration(id, time, er);
            float sh = 0;
            bool isEta = !m_idHelperSvc->measuresPhi(id);
            if (isEta) tech = MuGirlNS::RPCETA_STAU_HIT;
            float propTime = 0;
            float tof = calculateTof(1, distance);
            float beta = calculateBeta(time + tof, distance);
            ATH_MSG_VERBOSE("  adding " << m_idHelperSvc->toString(id) << " distance " << distance << " time " << time << " beta" << beta
                                        << " diff " << std::abs(beta - betaSeed));

            if (std::abs(beta - betaSeed) > m_mdttBetaAssociationCut) return;

            hits.push_back(Muon::TimePointBetaFitter::Hit(distance, time, er));
            candidate.stauHits.push_back(MuGirlNS::StauHit(tech, time + tof, ix, iy, iz, id, ie, er, sh, isEta, propTime));
        };

        // get RPC timing per chamber
        RpcClPerChMap::const_iterator chit = rpcPrdsPerChamber.begin();
        RpcClPerChMap::const_iterator chit_end = rpcPrdsPerChamber.end();
        ATH_MSG_VERBOSE("RPCs per chamber " << rpcPrdsPerChamber.size());

        for (; chit != chit_end; ++chit) {
            const Trk::TrackParameters* pars = std::get<0>(chit->second);
            const RpcClVec& phiClusters = std::get<1>(chit->second);
            const RpcClVec& etaClusters = std::get<2>(chit->second);
            insertRpcs(*pars, phiClusters, candidate, hits);
            insertRpcs(*pars, etaClusters, candidate, hits);
        }

        // get timing per MDT chamber, use segment error strategy (errors of the RT relation
        Muon::MuonDriftCircleErrorStrategyInput bits;
        Muon::MuonDriftCircleErrorStrategy calibrationStrategy(bits);
        calibrationStrategy.setStrategy(Muon::MuonDriftCircleErrorStrategy::Moore);
        calibrationStrategy.setParameter(Muon::MuonDriftCircleErrorStrategy::Segment, true);

        TrkDriftCircleMath::DCSLFitter mdtFitter;
        TrkDriftCircleMath::SegmentFinder segmentFinder;
        segmentFinder.setMaxDropDepth(2);
        segmentFinder.setChi2DropCut(5);
        segmentFinder.setDeltaCut(3);

        MdtChamberLayerData::const_iterator mit = mdtDataPerChamberLayer.begin();
        MdtChamberLayerData::const_iterator mit_end = mdtDataPerChamberLayer.end();
        for (; mit != mit_end; ++mit) {
            ATH_MSG_VERBOSE(" new station layer " << Muon::MuonStationIndex::chName((Muon::MuonStationIndex::ChIndex)(mit->first % 1000))
                                                  << " hits " << mit->second.size());
            if (mit->second.size() < 4) continue;

            // get RE element for first hit
            const MuonGM::MdtReadoutElement* detEl = mit->second.front().second->detectorElement();
            const Trk::PlaneSurface* surf = dynamic_cast<const Trk::PlaneSurface*>(&detEl->surface());
            if (!surf) {
                ATH_MSG_WARNING("MdtReadoutElement should always have a PlaneSurface as reference surface");
                continue;
            }
            Amg::Transform3D gToStation = detEl->GlobalToAmdbLRSTransform();

            // get TrackParameters and SL intersect the DetEl surface (above a few GeV SL intersection is accurate enough)
            const Trk::TrackParameters& firstPars = *mit->second.front().first;
            Trk::Intersection slIntersection = surf->straightLineIntersection(firstPars.position(), firstPars.momentum(), false, false);

            // calculate seed position and direction
            Trk::LocalDirection seedLocDir;
            surf->globalToLocalDirection(firstPars.momentum(), seedLocDir);
            Amg::Vector3D seedLocPos = gToStation * slIntersection.position;
            TrkDriftCircleMath::LocVec2D seedPos(seedLocPos.y(), seedLocPos.z());
            TrkDriftCircleMath::Line seedLine(seedPos, seedLocDir.angleYZ());
            TrkDriftCircleMath::DCOnTrackVec dcs;

            std::vector<std::pair<std::shared_ptr<const Muon::MdtDriftCircleOnTrack>, const Trk::TrackParameters*>> indexLookUp;
            unsigned index = 0;
            for (const auto& entry : mit->second) {
                const Trk::TrackParameters& pars = *entry.first;
                const Muon::MdtDriftCircleOnTrack& mdt = *entry.second;
                Identifier id = mdt.identify();
                // calibrate MDT
                std::shared_ptr<const Muon::MdtDriftCircleOnTrack> calibratedMdt(
                    m_mdtCreatorStau->correct(*mdt.prepRawData(), pars, &calibrationStrategy, betaSeed));
                if (!calibratedMdt.get()) {
                    ATH_MSG_WARNING("Failed to recalibrate existing MDT on track " << m_idHelperSvc->toString(id));
                    continue;
                }
                ATH_MSG_VERBOSE(" recalibrated MDT " << m_idHelperSvc->toString(id) << " r " << calibratedMdt->driftRadius() << " "
                                                     << Amg::error(calibratedMdt->localCovariance(), Trk::locR) << " old r "
                                                     << mdt.driftRadius() << " " << Amg::error(mdt.localCovariance(), Trk::locR)
                                                     << " r_track " << pars.parameters()[Trk::locR] << " residual "
                                                     << std::abs(mdt.driftRadius()) - std::abs(pars.parameters()[Trk::locR]));

                // calculate tube position taking into account the second coordinate
                Amg::Vector2D lp(0., pars.parameters()[Trk::locZ]);
                Amg::Vector3D gpos;
                mdt.associatedSurface().localToGlobal(lp, pars.momentum(), gpos);

                // calculate local AMDB position
                Amg::Vector3D locPos = gToStation * gpos;
                TrkDriftCircleMath::LocVec2D lpos(locPos.y(), locPos.z());

                double r = std::abs(calibratedMdt->driftRadius());
                double dr = Amg::error(calibratedMdt->localCovariance(), Trk::locR);

                // create identifier
                TrkDriftCircleMath::MdtId mdtid(m_idHelperSvc->mdtIdHelper().isBarrel(id), m_idHelperSvc->mdtIdHelper().multilayer(id) - 1,
                                                m_idHelperSvc->mdtIdHelper().tubeLayer(id) - 1, m_idHelperSvc->mdtIdHelper().tube(id) - 1);

                // create new DriftCircle
                TrkDriftCircleMath::DriftCircle dc(lpos, r, dr, TrkDriftCircleMath::DriftCircle::InTime, mdtid, index, &mdt);
                TrkDriftCircleMath::DCOnTrack dcOnTrack(dc, 1., 1.);

                dcs.push_back(dcOnTrack);
                indexLookUp.emplace_back(calibratedMdt, &pars);
                ++index;
            }

            // now loop over the hits and fit the segment taking out each of the hits individually
            for (unsigned int i = 0; i < dcs.size(); ++i) {
                TrkDriftCircleMath::HitSelection selection(dcs.size(), 0);
                selection[i] = 1;
                TrkDriftCircleMath::Segment result(TrkDriftCircleMath::Line(0., 0., 0.), TrkDriftCircleMath::DCOnTrackVec());
                if (!mdtFitter.fit(result, seedLine, dcs, selection)) {
                    ATH_MSG_DEBUG("Fit failed ");
                    continue;
                }
                TrkDriftCircleMath::Segment segment = result;
                segment.hitsOnTrack(dcs.size());
                unsigned int ndofFit = segment.ndof();
                if (ndofFit < 1) continue;
                double chi2NdofSegmentFit = segment.chi2() / ndofFit;
                bool hasDropHit = false;
                unsigned int dropDepth = 0;
                if (!segmentFinder.dropHits(segment, hasDropHit, dropDepth)) {
                    ATH_MSG_DEBUG("DropHits failed, fit chi2/ndof " << chi2NdofSegmentFit);
                    if (msgLvl(MSG::VERBOSE)) {
                        segmentFinder.debugLevel(20);
                        segment = result;
                        segmentFinder.dropHits(segment, hasDropHit, dropDepth);
                        segmentFinder.debugLevel(0);
                    }
                    continue;
                }
                if (i >= segment.dcs().size()) continue;
                TrkDriftCircleMath::TransformToLine toLine(segment.line());
                const TrkDriftCircleMath::DCOnTrack& dc = segment.dcs()[i];
                double res = dc.residual();
                double err = std::sqrt(dc.dr() * dc.dr() + dc.errorTrack() * dc.errorTrack());
                double pull = res / err;
                double rline = toLine.toLineY(dc.position());
                int index = dc.index();
                if (index < 0 || index >= (int)indexLookUp.size()) {
                    ATH_MSG_WARNING(" lookup of TrackParameters and MdtDriftCircleOnTrack failed " << index << " range: 0 - "
                                                                                                   << indexLookUp.size() - 1);
                    continue;
                }
                const Trk::TrackParameters* pars = indexLookUp[dc.index()].second;
                const Muon::MdtDriftCircleOnTrack* mdt = indexLookUp[dc.index()].first.get();
                Identifier id = mdt->identify();

                // calibrate MDT with nominal timing
                std::shared_ptr<const Muon::MdtDriftCircleOnTrack> calibratedMdt(
                    m_mdtCreator->correct(*mdt->prepRawData(), *pars, &calibrationStrategy, betaSeed));
                if (!calibratedMdt.get()) {
                    ATH_MSG_WARNING("Failed to recalibrate existing MDT on track " << m_idHelperSvc->toString(id));
                    continue;
                }
                float distance = pars->position().mag();
                float time = 0.;

                float ix = pars->position().x();
                float iy = pars->position().y();
                float iz = pars->position().z();
                float ie = 0.;
                float er = -1;
                float sh = 0;
                bool isEta = !m_idHelperSvc->measuresPhi(id);
                float propTime = 0;
                float tof = calculateTof(1, distance);

                // use inverted RT relation together with track prediction to get estimate of drift time
                float driftTime = calibratedMdt->driftTime();  // we need to add beta seed as it was subtracted when calibrating the hits
                float locR = rline;
                float errR = dc.errorTrack();
                auto data = mdtCalibConstants->getCalibData(id, msgStream());
                const auto& rtRelation = data->rtRelation;
                bool out_of_bound_flag = false;
                float drdt = rtRelation->rt()->driftvelocity(driftTime);
                float rres = rtRelation->rtRes()->resolution(driftTime);
                float tres = rres / drdt;
                float TlocR = rtRelation->tr()->tFromR(std::abs(locR), out_of_bound_flag);
                float trackTimeRes = errR / drdt;
                float tofShiftFromBeta = 0.;  // muonBetaCalculationUtils.calculateTof(betaSeed,distance)-tof;
                er = std::sqrt(tres * tres + trackTimeRes * trackTimeRes);
                mdtTimeCalibration(id, driftTime, er);
                time = driftTime - TlocR + tofShiftFromBeta;
                propTime = driftTime;
                ie = trackTimeRes;

                const float beta = calculateBeta(time + tof, distance);
                bool isSelected = std::abs(beta - betaSeed) < m_mdttBetaAssociationCut;

                if (msgLvl(MSG::DEBUG)) {
                    msg(MSG::DEBUG) << m_idHelperSvc->toString(id) << std::setprecision(2) << " segment: after fit " << std::setw(5)
                                    << chi2NdofSegmentFit << " ndof " << std::setw(2) << ndofFit;
                    if (segment.ndof() != ndofFit)
                        msg(MSG::DEBUG) << " after outlier " << std::setw(5) << chi2NdofSegmentFit << " ndof " << std::setw(2) << ndofFit;
                    msg(MSG::DEBUG) << " driftR " << std::setw(4) << dc.r() << " rline " << std::setw(5) << rline << " residual "
                                    << std::setw(5) << res << " pull " << std::setw(4) << pull << " time " << std::setw(3) << time
                                    << " beta" << std::setw(2) << beta << " err " << std::setw(3) << er << " intrinsic " << std::setw(3)
                                    << tres << " track " << std::setw(3) << trackTimeRes;
                    if (!isSelected) msg(MSG::DEBUG) << " outlier";
                    msg(MSG::DEBUG) << std::setprecision(5) << endmsg;
                }

                if (!isSelected) continue;

                hits.emplace_back(distance, time, er);
                candidate.stauHits.emplace_back(MuGirlNS::MDTT_STAU_HIT, time + tof, ix, iy, iz, id, ie, er, sh, isEta, propTime);
            }
        }
        // fit data
        Muon::TimePointBetaFitter::FitResult betaFitResult = fitter.fitWithOutlierLogic(hits);
        ATH_MSG_DEBUG(" extractTimeMeasurementsFromTrack: extracted " << candidate.stauHits.size() << " time measurements "
                                                                      << " status fit " << betaFitResult.status << " beta "
                                                                      << betaFitResult.beta << " chi2/ndof " << betaFitResult.chi2PerDOF());

        candidate.finalBetaFitResult = betaFitResult;
    }

    void MuonStauRecoTool::addTag(const InDetCandidate& indetCandidate, MuonStauRecoTool::Candidate& candidate,
                                  InDetCandidateToTagMap* tagMap, TrackCollection* combTracks, Trk::SegmentCollection* segments) const {
        // get combined track and the segments
        combTracks->push_back(candidate.combinedTrack.release());
        ElementLink<TrackCollection> comblink(*combTracks, combTracks->size() - 1);
        std::vector<ElementLink<Trk::SegmentCollection>> segmentLinks;
        for (const auto& layer : candidate.allLayers) {
            for (const auto& segment : layer.segments) {
                segments->push_back(segment->clone());
                ElementLink<Trk::SegmentCollection> segLink(*segments, segments->size() - 1);
                segmentLinks.push_back(segLink);
            }
        }

        // create tag
        MuonCombined::MuGirlLowBetaTag* tag = new MuonCombined::MuGirlLowBetaTag(comblink, segmentLinks);

        // add additional info
        tag->setMuBeta(candidate.betaFitResult.beta);

        // add StauExtras
        MuGirlNS::StauExtras* stauExtras = new MuGirlNS::StauExtras();
        stauExtras->betaAll = candidate.betaFitResult.beta;
        stauExtras->betaAllt = candidate.finalBetaFitResult.beta;
        stauExtras->numRpcHitsInSeg = 0;
        stauExtras->numCaloCells = 0;
        stauExtras->rpcBetaAvg = 0;
        stauExtras->rpcBetaRms = 0;
        stauExtras->rpcBetaChi2 = 0;
        stauExtras->rpcBetaDof = 0;
        stauExtras->mdtBetaAvg = 0;
        stauExtras->mdtBetaRms = 0;
        stauExtras->mdtBetaChi2 = 0;
        stauExtras->mdtBetaDof = 0;
        stauExtras->caloBetaAvg = 0;
        stauExtras->caloBetaRms = 0;
        stauExtras->caloBetaChi2 = 0;
        stauExtras->caloBetaDof = 0;
        stauExtras->hits = candidate.stauHits;
        tag->setStauExtras(stauExtras);

        // print results afer refineCandidate
        if (m_doSummary || msgLvl(MSG::DEBUG)) {
            msg(MSG::INFO) << " Summary::addTag ";
            msg(MSG::INFO) << std::endl
                           << "  candidate: beta fit result: beta " << candidate.betaFitResult.beta << " chi2/ndof "
                           << candidate.betaFitResult.chi2PerDOF() << "  segments" << segmentLinks.size();
            for (const auto& segment : segmentLinks) msg(MSG::INFO) << std::endl << "     " << m_printer->print(**segment);
            if (*comblink)
                msg(MSG::INFO) << std::endl
                               << "   track " << m_printer->print(**comblink) << std::endl
                               << m_printer->printStations(**comblink);
            msg(MSG::INFO) << endmsg;
        }

        // add tag to IndetCandidate
        tagMap->addEntry(&indetCandidate, tag);
    }

    bool MuonStauRecoTool::resolveAmbiguities(MuonStauRecoTool::CandidateVec& candidates) const {
        ATH_MSG_DEBUG("Resolving ambiguities: candidates " << candidates.size());

        // push tracks into a collection and run ambi-solver
        TrackCollection tracks(SG::VIEW_ELEMENTS);
        std::map<const Trk::Track*, std::shared_ptr<Candidate>> trackCandidateLookup;
        for (const auto& candidate : candidates) {
            Trk::Track* track = candidate->combinedTrack.get();
            if (track) {
                tracks.push_back(track);
                trackCandidateLookup[track] = candidate;
            }
        }

        // first handle easy cases of zero or one track
        if (tracks.empty()) return false;
        if (tracks.size() == 1) return true;

        // more than 1 track call ambiguity solver and select first track
        std::unique_ptr<const TrackCollection> resolvedTracks(m_trackAmbibuityResolver->process(&tracks));
        if (!resolvedTracks || resolvedTracks->empty()) {
            ATH_MSG_WARNING("No track survived the ambiguity solving");
            return false;
        }
        const Trk::Track* selectedTrack = resolvedTracks->front();

        // get candidate
        auto pos = trackCandidateLookup.find(selectedTrack);
        if (pos == trackCandidateLookup.end()) {
            ATH_MSG_WARNING("candidate lookup failed, this should not happen");
            return false;
        }

        // remove all candidates that were not combined
        std::shared_ptr<Candidate> candidate = pos->second;
        candidates.clear();
        candidates.push_back(candidate);

        // print results afer resolveAmbiguities
        if (m_doSummary || msgLvl(MSG::DEBUG)) {
            msg(MSG::INFO) << " Summary::resolveAmbiguities ";
            msg(MSG::INFO) << std::endl
                           << "  candidate: beta fit result: beta " << candidate->betaFitResult.beta << " chi2/ndof "
                           << candidate->betaFitResult.chi2PerDOF() << " layers with segments" << candidate->allLayers.size() << std::endl
                           << "   track " << m_printer->print(*candidate->combinedTrack) << std::endl
                           << m_printer->printStations(*candidate->combinedTrack);
            msg(MSG::INFO) << endmsg;
        }

        return true;
    }

    bool MuonStauRecoTool::combineCandidates(const EventContext& ctx, const xAOD::TrackParticle& indetTrackParticle,
                                             MuonStauRecoTool::CandidateVec& candidates) const {
        // keep track of candidates that have a successfull fit
        CandidateVec combinedCandidates;

        // loop over candidates and redo segments using beta estimate from candidate
        ATH_MSG_DEBUG("Combining candidates " << candidates.size());
        for (auto& candidate : candidates) {
            // find best matching track
            std::pair<std::unique_ptr<const Muon::MuonCandidate>, std::unique_ptr<Trk::Track>> result =
                m_insideOutRecoTool->findBestCandidate(ctx, indetTrackParticle, candidate->allLayers);

            if (result.first && result.second) {
                ATH_MSG_DEBUG("   combined track found " << std::endl
                                                         << m_printer->print(*result.second) << std::endl
                                                         << m_printer->printStations(*result.second));
                // add segments and track pointer to the candidate
                candidate->muonCandidate = std::move(result.first);
                candidate->combinedTrack = std::move(result.second);

                // extract times form track
                extractTimeMeasurementsFromTrack(ctx, *candidate);
                combinedCandidates.push_back(candidate);
                if (!m_recoValidationTool.empty()) m_recoValidationTool->addTimeMeasurements(indetTrackParticle, candidate->stauHits);
            }
        }

        // remove all candidates that were not combined
        candidates = combinedCandidates;

        // print results afer combineCandidate
        if (m_doSummary || msgLvl(MSG::DEBUG)) {
            msg(MSG::INFO) << " Summary::combineCandidates ";
            if (candidates.empty())
                msg(MSG::INFO) << " No candidated found ";
            else
                msg(MSG::INFO) << " candidates " << candidates.size();

            for (const auto& candidate : candidates) {
                msg(MSG::INFO) << std::endl
                               << "  candidate: beta fit result: " << candidate->betaFitResult.beta << " chi2/ndof "
                               << candidate->betaFitResult.chi2PerDOF();
                if (candidate->finalBetaFitResult.status != 0)
                    msg(MSG::INFO) << " MDTT beta fit result: " << candidate->finalBetaFitResult.beta << " chi2/ndof "
                                   << candidate->finalBetaFitResult.chi2PerDOF();
                msg(MSG::INFO) << " layers with segments" << candidate->allLayers.size() << std::endl
                               << "   track " << m_printer->print(*candidate->combinedTrack) << std::endl
                               << m_printer->printStations(*candidate->combinedTrack);
            }
            msg(MSG::INFO) << endmsg;
        }

        return !candidates.empty();
    }

    bool MuonStauRecoTool::createCandidates(const MuonStauRecoTool::AssociatedData& associatedData,
                                            MuonStauRecoTool::CandidateVec& candidates) const {
        // loop over layers and select seed maxima
        MaximumDataVec seedMaximumDataVec;
        LayerDataVec::const_iterator it = associatedData.layerData.begin();
        LayerDataVec::const_iterator it_end = associatedData.layerData.end();
        for (; it != it_end; ++it) {
            // loop over maximumDataVec
            for (const auto& maximumData : it->maximumDataVec) {
                // add all maximumData that have a time measurement
                if (!maximumData->betaSeeds.empty()) seedMaximumDataVec.push_back(maximumData);
            }
        }
        ATH_MSG_DEBUG("Creating candidates from seeds  " << seedMaximumDataVec.size());

        if (seedMaximumDataVec.empty()) {
            if (m_doSummary || msgLvl(MSG::DEBUG)) msg(MSG::INFO) << " Summary::createCandidates, no seeds found " << endmsg;
            return false;
        }

        // sorting lambda for MaximumData seeds
        auto SortMaximumDataVec = [](const std::shared_ptr<MaximumData>& max1, const std::shared_ptr<MaximumData>& max2) {
            return max1->maximum->max < max2->maximum->max;
        };
        std::stable_sort(seedMaximumDataVec.begin(), seedMaximumDataVec.end(), SortMaximumDataVec);

        // loop over seeds and create candidates
        Muon::TimePointBetaFitter fitter;
        std::set<const MaximumData*> usedMaximumData;
        MaximumDataVec::iterator sit = seedMaximumDataVec.begin();
        MaximumDataVec::iterator sit_end = seedMaximumDataVec.end();
        for (; sit != sit_end; ++sit) {
            // only use once
            if (usedMaximumData.count(sit->get())) continue;
            usedMaximumData.insert(sit->get());

            // create new candidates from the beta seeds of the maximum
            CandidateVec newCandidates;
            for (const auto& betaSeed : (*sit)->betaSeeds) { newCandidates.push_back(std::make_shared<Candidate>(betaSeed)); }
            // extend the candidates
            extendCandidates(newCandidates, usedMaximumData, associatedData.layerData.begin(), associatedData.layerData.end());

            // loop over the candidates and fit them
            for (auto& newCandidate : newCandidates) {
                // fit data
                newCandidate->betaFitResult = fitter.fitWithOutlierLogic(newCandidate->hits);
                ATH_MSG_DEBUG(" New candidate: time measurements "
                              << newCandidate->hits.size() << " status " << newCandidate->betaFitResult.status << " beta "
                              << newCandidate->betaFitResult.beta << " chi2/ndof " << newCandidate->betaFitResult.chi2PerDOF());
                // if the fit was successfull add the candidate to the candidate vector
                if (newCandidate->betaFitResult.status != 0) {
                    newCandidate->combinedTrack = nullptr;  // no track exists at this stage
                    candidates.push_back(newCandidate);
                }
            }
        }

        // print results afer createCandidate
        if (m_doSummary || msgLvl(MSG::DEBUG)) {
            msg(MSG::INFO) << " Summary::createCandidates ";
            if (candidates.empty())
                msg(MSG::INFO) << " No candidated found ";
            else
                msg(MSG::INFO) << " candidates " << candidates.size();

            for (const auto& candidate : candidates) {
                msg(MSG::INFO) << std::endl
                               << "  candidate: beta seed " << candidate->betaSeed.beta << " beta fit result: beta "
                               << candidate->betaFitResult.beta << " chi2/ndof " << candidate->betaFitResult.chi2PerDOF() << " layers "
                               << candidate->layerDataVec.size();
                for (const auto& layerData : candidate->layerDataVec)
                    msg(MSG::INFO) << std::endl
                                   << "     " << printIntersectionToString(layerData.intersection) << " maximumDataVec "
                                   << layerData.maximumDataVec.size();
            }
            msg(MSG::INFO) << endmsg;
        }

        return !candidates.empty();
    }

    void MuonStauRecoTool::extendCandidates(MuonStauRecoTool::CandidateVec& candidates, std::set<const MaximumData*>& usedMaximumData,
                                            MuonStauRecoTool::LayerDataVec::const_iterator it,
                                            MuonStauRecoTool::LayerDataVec::const_iterator it_end) const {
        // get current layer and move forward the
        const LayerData& layerData = *it;
        ATH_MSG_DEBUG(" extendCandidates: " << printIntersectionToString(layerData.intersection) << " maxima "
                                            << layerData.maximumDataVec.size());

        CandidateVec newCandidates;  // store new candidates
        for (auto& candidate : candidates) {
            // keep track of how often we extend this candidate
            unsigned int nextensions = 0;

            // copy content of the candidate for reference
            LayerDataVec layerDataVec = candidate->layerDataVec;
            Muon::TimePointBetaFitter::HitVec hits = candidate->hits;

            // loop over maximumDataVec of the layer
            for (const auto& maximumData : layerData.maximumDataVec) {
                // create new hit vector
                Muon::TimePointBetaFitter::HitVec newhits;  // create new hits vector and add the ones from the maximum
                if (extractTimeHits(*maximumData, newhits, &candidate->betaSeed)) {
                    // decide which candidate to update, create a new candidate if a maximum was already selected in the layer
                    Candidate* theCandidate = nullptr;
                    if (nextensions == 0)
                        theCandidate = candidate.get();
                    else {
                        std::shared_ptr<Candidate> newCandidate(new Candidate(candidate->betaSeed));
                        newCandidate->layerDataVec = layerDataVec;
                        newCandidate->hits = hits;
                        newCandidates.push_back(newCandidate);
                        theCandidate = newCandidate.get();
                    }

                    // create a LayerData object to add to the selected candidate
                    LayerData newLayerData(layerData.intersection);
                    newLayerData.maximumDataVec.push_back(maximumData);

                    // update the candidate
                    theCandidate->hits.insert(theCandidate->hits.end(), newhits.begin(), newhits.end());
                    theCandidate->layerDataVec.push_back(newLayerData);
                    usedMaximumData.insert(maximumData.get());

                    ATH_MSG_DEBUG(" adding maximumData: candidate hits " << theCandidate->hits.size() << " LayerDataVec "
                                                                         << theCandidate->layerDataVec.size() << " nextensions "
                                                                         << nextensions);

                    ++nextensions;
                }
            }
        }
        ATH_MSG_DEBUG("   extendCandidates done, new candidates " << newCandidates.size());

        // add the new candidates
        candidates.insert(candidates.end(), newCandidates.begin(), newCandidates.end());

        // move to the next layer, if we haven't reached the last layer, continue recursion
        ++it;
        if (it != it_end) extendCandidates(candidates, usedMaximumData, it, it_end);
    }

    bool MuonStauRecoTool::extractTimeMeasurements(const EventContext& ctx, const Muon::MuonSystemExtension& muonSystemExtension,
                                                   MuonStauRecoTool::AssociatedData& associatedData) const {
        // get layer intersections
        const std::vector<Muon::MuonSystemExtension::Intersection>& layerIntersections = muonSystemExtension.layerIntersections();

        // find RPC time measurements and segments to seed the beta fit using t0 fitting
        for (std::vector<Muon::MuonSystemExtension::Intersection>::const_iterator it = layerIntersections.begin();
             it != layerIntersections.end(); ++it) {
            // create layer data object and add maxima
            LayerData layerData(*it);
            associateHoughMaxima(layerData);

            // skip layer of not maxima are associated
            if (layerData.maximumDataVec.empty()) continue;

            associatedData.layerData.push_back(layerData);

            // loop over associated maxima
            for (auto& maximum : layerData.maximumDataVec) {
                // extract RPC timing
                extractRpcTimingFromMaximum(*it, *maximum);

                // find segments for intersection
                std::vector<std::shared_ptr<const Muon::MuonSegment>> t0fittedSegments;
                findSegments(*it, *maximum, t0fittedSegments, m_muonPRDSelectionTool, m_segmentMakerT0Fit);
                if (t0fittedSegments.empty()) continue;

                // match segments to intersection, store the ones that match
                m_segmentMatchingTool->select(ctx, *it, t0fittedSegments, maximum->t0fittedSegments);

                // get beta seeds for Maximum
                getBetaSeeds(*maximum);
            }
        }

        // print results afer extractTimeMeasurements
        if (m_doSummary || msgLvl(MSG::DEBUG)) {
            msg(MSG::INFO) << " Summary::extractTimeMeasurements ";
            if (associatedData.layerData.empty())
                msg(MSG::INFO) << " No layers associated ";
            else
                msg(MSG::INFO) << " Associated layers " << associatedData.layerData.size();

            for (const auto& layerData : associatedData.layerData) {
                unsigned int nmaxWithBeta = 0;
                for (const auto& maximumData : layerData.maximumDataVec) {
                    if (!maximumData->betaSeeds.empty()) ++nmaxWithBeta;
                }
                msg(MSG::INFO) << std::endl
                               << " layer " << printIntersectionToString(layerData.intersection) << " associated maxima "
                               << layerData.maximumDataVec.size() << " maxima with beta seeds " << nmaxWithBeta;
            }
            msg(MSG::INFO) << endmsg;
        }

        // return false if no layers were associated
        return !associatedData.layerData.empty();
    }

    bool MuonStauRecoTool::extractTimeHits(const MaximumData& maximumData, Muon::TimePointBetaFitter::HitVec& hits,
                                           const BetaSeed* seed) const {
        unsigned int nstart = hits.size();

        auto addHit = [&](float distance, float time, float error, float cut) {
            if (seed) {
                float beta = calculateBeta(time + calculateTof(1, distance), distance);
                ATH_MSG_VERBOSE("  matching hit: distance " << distance << " time " << time << " beta" << beta << " diff "
                                                            << std::abs(beta - seed->beta));
                if (std::abs(beta - seed->beta) > cut) return;
            } else {
                ATH_MSG_VERBOSE("  addHit: distance " << distance << " time " << time << " beta"
                                                      << calculateBeta(time + calculateTof(1, distance), distance));
            }
            if (error != 0.) hits.emplace_back(distance, time, error);
        };

        // add rpc measurements
        for (const auto& rpc : maximumData.rpcTimeMeasurements) {
            float time = rpc.time;
            float error = rpc.error;
            rpcTimeCalibration(rpc.rpcClusters.front()->identify(), time, error);
            addHit(rpc.rpcClusters.front()->globalPosition().mag(), time, error, m_rpcBetaAssociationCut);
        }

        // add segment t0 fits
        // if not seeded take all segments
        if (!seed) {
            for (const auto& seg : maximumData.t0fittedSegments) {
                if (!seg->hasFittedT0()) continue;
                float time = seg->time();
                float error = seg->errorTime();
                Identifier id = m_edmHelperSvc->chamberId(*seg);
                segmentTimeCalibration(id, time, error);
                addHit(seg->globalPosition().mag(), time, error, m_segmentBetaAssociationCut);
            }
        } else {
            // pick the best match
            const Muon::MuonSegment* bestSegment = nullptr;
            float smallestResidual = FLT_MAX;
            for (const auto& seg : maximumData.t0fittedSegments) {
                if (!seg->hasFittedT0()) continue;
                float distance = seg->globalPosition().mag();
                float time = seg->time();
                float beta = calculateBeta(time + calculateTof(1, distance), distance);
                float residual = std::abs(beta - seed->beta);

                if (residual < smallestResidual) {
                    smallestResidual = residual;
                    bestSegment = seg.get();
                }
            }
            if (bestSegment) {
                addHit(bestSegment->globalPosition().mag(), bestSegment->time(), bestSegment->errorTime(), m_segmentBetaAssociationCut);
                ATH_MSG_VERBOSE("  adding best segment:  " << m_printer->print(*bestSegment));
            }
        }
        ATH_MSG_VERBOSE("  extractTimeHits done: added " << hits.size() - nstart << " hits");

        return nstart != hits.size();
    }

    void MuonStauRecoTool::getBetaSeeds(MaximumData& maximumData) const {
        // skip maximumData if no timing information is available
        if (maximumData.rpcTimeMeasurements.empty() && maximumData.t0fittedSegments.empty()) return;

        // fitter + hits
        Muon::TimePointBetaFitter fitter;
        Muon::TimePointBetaFitter::HitVec hits;
        extractTimeHits(maximumData, hits);

        Muon::TimePointBetaFitter::FitResult result = fitter.fitWithOutlierLogic(hits);
        float chi2ndof = result.chi2PerDOF();

        ATH_MSG_DEBUG(" fitting beta for maximum: time measurements " << hits.size() << " status " << result.status << " beta "
                                                                      << result.beta << " chi2/ndof " << chi2ndof);
        if (result.status != 0) maximumData.betaSeeds.emplace_back(result.beta, 1.);
    }

    void MuonStauRecoTool::findSegments(const Muon::MuonSystemExtension::Intersection& intersection, MaximumData& maximumData,
                                        std::vector<std::shared_ptr<const Muon::MuonSegment>>& segments,
                                        const ToolHandle<Muon::IMuonPRDSelectionTool>& muonPRDSelectionTool,
                                        const ToolHandle<Muon::IMuonSegmentMaker>& segmentMaker) const {
        const MuonHough::MuonLayerHough::Maximum& maximum = *maximumData.maximum;
        const std::vector<std::shared_ptr<const Muon::MuonClusterOnTrack>>& phiClusterOnTracks = maximumData.phiClusterOnTracks;

        // lambda to handle calibration and selection of MDTs
        auto handleMdt = [intersection, muonPRDSelectionTool](const Muon::MdtPrepData& prd,
                                                              std::vector<const Muon::MdtDriftCircleOnTrack*>& mdts) {
            const Muon::MdtDriftCircleOnTrack* mdt = muonPRDSelectionTool->calibrateAndSelect(intersection, prd);
            if (mdt) mdts.push_back(mdt);
        };

        // lambda to handle calibration and selection of clusters
        auto handleCluster = [intersection, muonPRDSelectionTool](const Muon::MuonCluster& prd,
                                                                  std::vector<const Muon::MuonClusterOnTrack*>& clusters) {
            const Muon::MuonClusterOnTrack* cluster = muonPRDSelectionTool->calibrateAndSelect(intersection, prd);
            if (cluster) clusters.push_back(cluster);
        };

        // loop over hits in maximum and add them to the hit list
        std::vector<const Muon::MdtDriftCircleOnTrack*> mdts;
        std::vector<const Muon::MuonClusterOnTrack*> clusters;

        // insert phi hits, clone them
        clusters.reserve(phiClusterOnTracks.size());

        for (const auto& phiClusterOnTrack : phiClusterOnTracks) { clusters.push_back(phiClusterOnTrack->clone()); }

        ATH_MSG_DEBUG("About to loop over Hough::Hits");

        MuonHough::HitVec::const_iterator hit = maximum.hits.begin();
        MuonHough::HitVec::const_iterator hit_end = maximum.hits.end();
        for (; hit != hit_end; ++hit) {
            ATH_MSG_DEBUG("hit x,y_min,y_max,w = " << (*hit)->x << "," << (*hit)->ymin << "," << (*hit)->ymax << "," << (*hit)->w);
            // treat the case that the hit is a composite TGC hit
            if ((*hit)->tgc) {
                for (const auto& prd : (*hit)->tgc->etaCluster.hitList) handleCluster(*prd, clusters);
            } else if ((*hit)->prd) {
                Identifier id = (*hit)->prd->identify();
                if (m_idHelperSvc->isMdt(id))
                    handleMdt(static_cast<const Muon::MdtPrepData&>(*(*hit)->prd), mdts);
                else
                    handleCluster(static_cast<const Muon::MuonCluster&>(*(*hit)->prd), clusters);
            }
        }

        ATH_MSG_DEBUG("About to loop over calibrated hits");

        ATH_MSG_DEBUG("Dumping MDTs");
        for (const auto* it : mdts) ATH_MSG_DEBUG(*it);

        ATH_MSG_DEBUG("Dumping clusters");
        for (const auto* it : clusters) ATH_MSG_DEBUG(*it);

        // require at least 2 MDT hits
        if (mdts.size() > 2) {
            // run segment finder
            std::unique_ptr<Trk::SegmentCollection> segColl(new Trk::SegmentCollection(SG::VIEW_ELEMENTS));
            segmentMaker->find(intersection.trackParameters->position(), intersection.trackParameters->momentum(), mdts, clusters,
                               !clusters.empty(), segColl.get(), intersection.trackParameters->momentum().mag());
            if (segColl) {
                Trk::SegmentCollection::iterator sit = segColl->begin();
                Trk::SegmentCollection::iterator sit_end = segColl->end();
                for (; sit != sit_end; ++sit) {
                    Trk::Segment* tseg = *sit;
                    Muon::MuonSegment* mseg = dynamic_cast<Muon::MuonSegment*>(tseg);
                    ATH_MSG_DEBUG("Segment:  " << m_printer->print(*mseg));
                    segments.push_back(std::shared_ptr<const Muon::MuonSegment>(mseg));
                }
            }
        }
        // clean-up memory
        for (const auto* hit : mdts) delete hit;
        for (const auto* hit : clusters) delete hit;
    }

    void MuonStauRecoTool::extractRpcTimingFromMaximum(const Muon::MuonSystemExtension::Intersection& intersection,
                                                       MaximumData& maximumData) const {
        // extract trigger hits per chamber
        const MuonHough::MuonLayerHough::Maximum& maximum = *maximumData.maximum;
        std::map<Identifier, std::vector<const Muon::RpcPrepData*>> rpcPrdsPerChamber;

        // lambda to add the PRD
        auto addRpc = [&rpcPrdsPerChamber, this](const Trk::PrepRawData* prd) {
            const Muon::RpcPrepData* rpcPrd = dynamic_cast<const Muon::RpcPrepData*>(prd);
            if (rpcPrd) {
                Identifier chamberId = m_idHelperSvc->chamberId(rpcPrd->identify());
                rpcPrdsPerChamber[chamberId].push_back(rpcPrd);
            }
        };

        // extract eta hits
        MuonHough::HitVec::const_iterator hit = maximum.hits.begin();
        MuonHough::HitVec::const_iterator hit_end = maximum.hits.end();
        for (; hit != hit_end; ++hit) {
            if ((*hit)->tgc || !(*hit)->prd || !m_idHelperSvc->isRpc((*hit)->prd->identify())) continue;
            addRpc((*hit)->prd);
        }

        // extract phi hits
        for (const auto& rot : maximumData.phiClusterOnTracks) { addRpc(rot->prepRawData()); }

        // exit if no hits are found
        if (rpcPrdsPerChamber.empty()) return;

        std::map<Identifier, std::vector<const Muon::RpcPrepData*>>::iterator chit = rpcPrdsPerChamber.begin();
        std::map<Identifier, std::vector<const Muon::RpcPrepData*>>::iterator chit_end = rpcPrdsPerChamber.end();
        for (; chit != chit_end; ++chit) {
            // cluster hits
            Muon::RpcHitClusteringObj clustering(&m_idHelperSvc->rpcIdHelper());
            if (!clustering.cluster(chit->second)) {
                ATH_MSG_WARNING("Clustering failed");
                return;
            }

            ATH_MSG_DEBUG(" " << m_idHelperSvc->toStringChamber(chit->first) << " clustered RPCs: " << chit->second.size()
                              << " eta clusters " << clustering.clustersEta.size() << " phi clusters " << clustering.clustersPhi.size());
            createRpcTimeMeasurementsFromClusters(intersection, clustering.clustersEta, maximumData.rpcTimeMeasurements);
            createRpcTimeMeasurementsFromClusters(intersection, clustering.clustersPhi, maximumData.rpcTimeMeasurements);
        }
    }

    void MuonStauRecoTool::createRpcTimeMeasurementsFromClusters(const Muon::MuonSystemExtension::Intersection& intersection,
                                                                 const std::vector<Muon::RpcClusterObj>& clusterObjects,
                                                                 MuonStauRecoTool::RpcTimeMeasurementVec& rpcTimeMeasurements) const {
        // loop over the clusters
        for (const auto& cluster : clusterObjects) {
            if (cluster.hitList.empty() || !cluster.hitList.front()) {
                ATH_MSG_WARNING("Cluster without hits: " << cluster.hitList.size());
                continue;
            }
            ATH_MSG_DEBUG("  new cluster: " << m_idHelperSvc->toString(cluster.hitList.front()->identify()) << " size "
                                            << cluster.hitList.size());

            // create the ROTs
            std::vector<const Muon::MuonClusterOnTrack*> clusters;
            for (const auto* rpc : cluster.hitList) {
                const Muon::MuonClusterOnTrack* rot(m_muonPRDSelectionTool->calibrateAndSelect(intersection, *rpc));
                if (rot) {
                    clusters.push_back(rot);
                    ATH_MSG_DEBUG("   strip " << m_idHelperSvc->toString(rot->identify()) << " time "
                                              << static_cast<const Muon::RpcClusterOnTrack*>(rot)->time());
                }
            }
            // get the timing result for the cluster
            Muon::IMuonHitTimingTool::TimingResult result = m_hitTimingTool->calculateTimingResult(clusters);
            if (result.valid) {
                // add the result
                RpcTimeMeasurement rpcTimeMeasurement;
                rpcTimeMeasurement.time = result.time;
                rpcTimeMeasurement.error = result.error;
                for (const auto* cl : clusters) {
                    const Muon::RpcClusterOnTrack* rcl = dynamic_cast<const Muon::RpcClusterOnTrack*>(cl);
                    if (rcl) rpcTimeMeasurement.rpcClusters.push_back(std::shared_ptr<const Muon::RpcClusterOnTrack>(rcl));
                }
                rpcTimeMeasurements.push_back(rpcTimeMeasurement);
            } else {
                // if no time measurement was created we need to clean up the memory
                for (const auto* cl : clusters) delete cl;
            }
        }
    }

    void MuonStauRecoTool::associateHoughMaxima(MuonStauRecoTool::LayerData& layerData) const {
        
        if (m_houghDataPerSectorVecKey.empty()) return;
        // get intersection and layer identifiers
        const Muon::MuonSystemExtension::Intersection& intersection = layerData.intersection;
        int sector = intersection.layerSurface.sector;
        Muon::MuonStationIndex::DetectorRegionIndex regionIndex = intersection.layerSurface.regionIndex;
        Muon::MuonStationIndex::LayerIndex layerIndex = intersection.layerSurface.layerIndex;

        // get hough data
        SG::ReadHandle<Muon::MuonLayerHoughTool::HoughDataPerSectorVec> houghDataPerSectorVec{m_houghDataPerSectorVecKey};
        if (!houghDataPerSectorVec.isValid()) {
            ATH_MSG_ERROR("Hough data per sector vector not found");
            return;
        }

        // sanity check
        if (static_cast<int>(houghDataPerSectorVec->vec.size()) <= sector - 1) {
            ATH_MSG_WARNING(" sector " << sector
                                       << " larger than the available sectors in the Hough tool: " << houghDataPerSectorVec->vec.size());
            return;
        }

        // get hough maxima in the layer
        unsigned int sectorLayerHash = Muon::MuonStationIndex::sectorLayerHash(regionIndex, layerIndex);
        const Muon::MuonLayerHoughTool::HoughDataPerSector& houghDataPerSector = houghDataPerSectorVec->vec[sector - 1];

        // sanity check
        if (houghDataPerSector.maxVec.size() <= sectorLayerHash) {
            ATH_MSG_WARNING(" houghDataPerSector.maxVec.size() smaller than hash " << houghDataPerSector.maxVec.size() << " hash "
                                                                                   << sectorLayerHash);
            return;
        }
        const Muon::MuonLayerHoughTool::MaximumVec& maxVec = houghDataPerSector.maxVec[sectorLayerHash];
        if (maxVec.empty()) return;

        // get local coordinates in the layer frame
        bool barrelLike = intersection.layerSurface.regionIndex == Muon::MuonStationIndex::Barrel;

        // in the endcaps take the r in the sector frame from the local position of the extrapolation
        float phi = intersection.trackParameters->position().phi();
        float r = barrelLike ? m_muonSectorMapping.transformRToSector(intersection.trackParameters->position().perp(), phi,
                                                                      intersection.layerSurface.sector, true)
                             : intersection.trackParameters->parameters()[Trk::locX];

        float z = intersection.trackParameters->position().z();
        float errx = Amg::error(*intersection.trackParameters->covariance(), Trk::locX);
        float x = barrelLike ? z : r;
        float y = barrelLike ? r : z;
        float theta = std::atan2(y, x);

        // get phi hits
        const Muon::MuonLayerHoughTool::PhiMaximumVec& phiMaxVec = houghDataPerSector.phiMaxVec[regionIndex];
        ATH_MSG_DEBUG("   Got Phi Hough maxima " << phiMaxVec.size() << " phi " << phi);

        // lambda to handle calibration and selection of clusters
        auto handleCluster = [intersection, this](const Muon::MuonCluster& prd,
                                                  std::vector<std::shared_ptr<const Muon::MuonClusterOnTrack>>& clusters) {
            const Muon::MuonClusterOnTrack* cluster = m_muonPRDSelectionTool->calibrateAndSelect(intersection, prd);
            if (cluster) clusters.push_back(std::shared_ptr<const Muon::MuonClusterOnTrack>(cluster));
        };

        // loop over maxima and associate phi hits with the extrapolation, should optimize this but calculating the residual with the phi
        // maximum
        std::vector<std::shared_ptr<const Muon::MuonClusterOnTrack>> phiClusterOnTracks;
        Muon::MuonLayerHoughTool::PhiMaximumVec::const_iterator pit = phiMaxVec.begin();
        Muon::MuonLayerHoughTool::PhiMaximumVec::const_iterator pit_end = phiMaxVec.end();
        for (; pit != pit_end; ++pit) {
            const MuonHough::MuonPhiLayerHough::Maximum& maximum = **pit;
            for (const std::shared_ptr<MuonHough::PhiHit>& hit : maximum.hits) {
                // treat the case that the hit is a composite TGC hit
                if (hit->tgc && !hit->tgc->phiCluster.hitList.empty()) {
                    Identifier id = hit->tgc->phiCluster.hitList.front()->identify();
                    if (m_idHelperSvc->layerIndex(id) != intersection.layerSurface.layerIndex) continue;
                    for (const Muon::MuonCluster* prd : hit->tgc->phiCluster.hitList) handleCluster(*prd, phiClusterOnTracks);
                } else if (hit->prd && !(hit->prd->type(Trk::PrepRawDataType::sTgcPrepData) || hit->prd->type(Trk::PrepRawDataType::MMPrepData))) {
                    const Identifier id = hit->prd->identify();
                    if (m_idHelperSvc->layerIndex(id) != intersection.layerSurface.layerIndex) continue;
                    handleCluster(static_cast<const Muon::MuonCluster&>(*hit->prd), phiClusterOnTracks);
                }
            }
        }

        ATH_MSG_DEBUG(" associateHoughMaxima: " << printIntersectionToString(intersection) << " maxima " << maxVec.size() << " x,y=(" << x
                                                << "," << y << ") errorx " << errx << " "
                                                << " angle " << theta);

        // loop over maxima and associate them to the extrapolation
        Muon::MuonLayerHoughTool::MaximumVec::const_iterator mit = maxVec.begin();
        Muon::MuonLayerHoughTool::MaximumVec::const_iterator mit_end = maxVec.end();
        for (; mit != mit_end; ++mit) {
            const MuonHough::MuonLayerHough::Maximum& maximum = **mit;
            if (std::find_if(maximum.hits.begin(),maximum.hits.end(),[](const std::shared_ptr<MuonHough::Hit>& hit){
                return hit->prd && (hit->prd->type(Trk::PrepRawDataType::sTgcPrepData) || hit->prd->type(Trk::PrepRawDataType::MMPrepData));
            }) != maximum.hits.end()) continue;
            float residual = maximum.pos - x;
            float residualTheta = maximum.theta - theta;
            float refPos = (maximum.hough != nullptr) ? maximum.hough->m_descriptor.referencePosition : 0;
            float maxwidth = (maximum.binposmax - maximum.binposmin);
            if (maximum.hough) maxwidth *= maximum.hough->m_binsize;

            float pull = residual / std::sqrt(errx * errx + maxwidth * maxwidth / 12.);

            ATH_MSG_DEBUG("   Hough maximum " << maximum.max << " position (" << refPos << "," << maximum.pos << ") residual " << residual
                                              << " pull " << pull << " angle " << maximum.theta << " residual " << residualTheta);

            // fill validation content

            // select maximum and add it to LayerData
            if (std::abs(pull) > 5) continue;
            layerData.maximumDataVec.emplace_back(std::make_shared<MaximumData>(intersection, &maximum, phiClusterOnTracks));
        }
    }
    void MuonStauRecoTool::mdtTimeCalibration(const Identifier& /*id*/, float& time, float& error) const {
        time -= 1.5;
        error *= 1.;
    }
    void MuonStauRecoTool::rpcTimeCalibration(const Identifier& /*id*/, float& time, float& error) const {
        time -= 0;
        error *= 0.5;
    }
    void MuonStauRecoTool::segmentTimeCalibration(const Identifier& /*id*/, float& time, float& error) const {
        time -= 1.5;
        error *= 1.;
    }
    /// Calcualte for zero betas
    float MuonStauRecoTool::calculateTof(const float beta, const float dist) const {
        return std::abs(beta) > 0 ? dist * inverseSpeedOfLight / beta : 1.e12;
    }
    /// In cases of invalid times just return an phyisical value of 20 times the speed of light
    /// The subsequent checks remove the attempt by default then
    float MuonStauRecoTool::calculateBeta(const float time, const float dist) const {
        return time != 0. ? dist * inverseSpeedOfLight / time : 20.;
    }
    void MuonStauRecoTool::addCandidatesToNtuple(const xAOD::TrackParticle& indetTrackParticle,
                                                 const MuonStauRecoTool::CandidateVec& candidates, int stage) const {
        if (Gaudi::Concurrency::ConcurrencyFlags::numThreads() > 1) {
            ATH_MSG_WARNING("You are calling the non thread-safe MuonRecoValidationTool with multiple threads, will most likely crash");
        }
        if (m_recoValidationTool.empty()) return;

        ATH_MSG_DEBUG("add candidates to ntuple, stage " << stage);
        for (const auto& candidate : candidates) {
            int ntimes = 0;
            float beta = -1.;
            float chi2ndof = -1.;
            if (candidate->finalBetaFitResult.status != 0) {
                ntimes = candidate->stauHits.size();
                beta = candidate->finalBetaFitResult.beta;
                chi2ndof = candidate->finalBetaFitResult.chi2PerDOF();
            } else if (candidate->betaFitResult.status != 0) {
                ntimes = candidate->hits.size();
                beta = candidate->betaFitResult.beta;
                chi2ndof = candidate->betaFitResult.chi2PerDOF();
            } else {
                ntimes = 1;
                beta = candidate->betaSeed.beta;
                chi2ndof = 0;
            }
            if (candidate->combinedTrack) ATH_MSG_DEBUG("candidate has combined track");
            m_recoValidationTool->addMuonCandidate(indetTrackParticle, candidate->muonCandidate.get(), candidate->combinedTrack.get(),
                                                   ntimes, beta, chi2ndof, stage);
        }
    }

}  // namespace MuonCombined
