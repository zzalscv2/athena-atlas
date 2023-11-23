/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonRefitTool.h"

#include "EventPrimitives/EventPrimitives.h"
#include "EventPrimitives/EventPrimitivesHelpers.h"
#include "MuonAlignErrorBase/AlignmentRotationDeviation.h"
#include "MuonAlignErrorBase/AlignmentTranslationDeviation.h"
#include "MuonRIO_OnTrack/MdtDriftCircleOnTrack.h"
#include "MuonReadoutGeometry/MdtReadoutElement.h"
#include "MuonSegmentMakerUtils/CompareMuonSegmentKeys.h"
#include "MuonSegmentMakerUtils/MuonSegmentKey.h"
#include "MuonTrackMakerUtils/MuonTSOSHelper.h"
#include "TrkEventPrimitives/LocalDirection.h"
#include "TrkMeasurementBase/MeasurementBase.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkPseudoMeasurementOnTrack/PseudoMeasurementOnTrack.h"
#include "TrkSurfaces/PlaneSurface.h"
#include "TrkSurfaces/Surface.h"
#include "TrkTrack/AlignmentEffectsOnTrack.h"
#include "TrkTrack/Track.h"
#include "TrkTrack/TrackStateOnSurface.h"
#include "TrkTrack/TrackCollection.h"
#include "TrkTrackSummary/TrackSummary.h"

namespace Muon {

    MuonRefitTool::MuonRefitTool(const std::string& ty, const std::string& na, const IInterface* pa) :
        AthAlgTool(ty, na, pa),
        m_errorStrategyBEE(MuonDriftCircleErrorStrategyInput()),
        m_errorStrategyEE(MuonDriftCircleErrorStrategyInput()),
        m_errorStrategyBIS78(MuonDriftCircleErrorStrategyInput()),
        m_errorStrategyBXE(MuonDriftCircleErrorStrategyInput()),
        m_errorStrategyEEL1C05(MuonDriftCircleErrorStrategyInput()),
        m_errorStrategyBarEnd(MuonDriftCircleErrorStrategyInput()),
        m_errorStrategySL(MuonDriftCircleErrorStrategyInput()),
        m_errorStrategyTwoStations(MuonDriftCircleErrorStrategyInput()),
        m_errorStrategy(MuonDriftCircleErrorStrategyInput()),
        m_muonErrorStrategy(MuonDriftCircleErrorStrategyInput()) {
        declareInterface<IMuonRefitTool>(this);
    }

    StatusCode MuonRefitTool::initialize() {
        ATH_MSG_INFO("Initializing MuonRefitTool " << name());

        ATH_CHECK(m_printer.retrieve());
        ATH_CHECK(m_edmHelperSvc.retrieve());
        ATH_CHECK(m_idHelperSvc.retrieve());
        m_BME_station = m_idHelperSvc->mdtIdHelper().stationNameIndex("BME");
        
        if (m_alignmentErrors) {
            if (!m_alignErrorTool.empty()) ATH_CHECK(m_alignErrorTool.retrieve());
        } else {
            m_alignErrorTool.disable();
        }
        ATH_CHECK(m_muonExtrapolator.retrieve());
        ATH_CHECK(m_trackFitter.retrieve());

        ATH_MSG_INFO("Retrieved " << m_trackFitter);

        ATH_CHECK(m_mdtRotCreator.retrieve());
        if (!m_compClusterCreator.empty()) ATH_CHECK(m_compClusterCreator.retrieve());

        if (!m_t0Fitter.empty()) {
            ATH_CHECK(m_t0Fitter.retrieve());
            ATH_MSG_INFO("Retrieved " << m_t0Fitter);
        }

        ATH_CHECK(m_muonEntryTrackExtrapolator.retrieve());

        MuonDriftCircleErrorStrategyInput bits;
        MuonDriftCircleErrorStrategy strategy(bits);
        strategy.setParameter(MuonDriftCircleErrorStrategy::BroadError, false);
        strategy.setParameter(MuonDriftCircleErrorStrategy::ScaledError, false);
        strategy.setParameter(MuonDriftCircleErrorStrategy::FixedError, false);
        strategy.setParameter(MuonDriftCircleErrorStrategy::ParameterisedErrors, false);
        strategy.setParameter(MuonDriftCircleErrorStrategy::StationError, false);
        strategy.setParameter(MuonDriftCircleErrorStrategy::ErrorAtPredictedPosition, false);
        strategy.setParameter(MuonDriftCircleErrorStrategy::T0Refit, false);
        strategy.setParameter(MuonDriftCircleErrorStrategy::WireSagGeomCorrection, false);
        strategy.setParameter(MuonDriftCircleErrorStrategy::TofCorrection, false);
        strategy.setParameter(MuonDriftCircleErrorStrategy::PropCorrection, false);
        strategy.setParameter(MuonDriftCircleErrorStrategy::TempCorrection, false);
        strategy.setParameter(MuonDriftCircleErrorStrategy::MagFieldCorrection, false);
        strategy.setParameter(MuonDriftCircleErrorStrategy::WireSagTimeCorrection, false);
        strategy.setParameter(MuonDriftCircleErrorStrategy::SlewCorrection, false);
        strategy.setParameter(MuonDriftCircleErrorStrategy::BackgroundCorrection, false);
        strategy.setParameter(MuonDriftCircleErrorStrategy::Segment, false);

        m_errorStrategyBEE = strategy;
        m_errorStrategyBEE.setParameter(MuonDriftCircleErrorStrategy::StationError, true);

        m_errorStrategyTwoStations = strategy;
        m_errorStrategyTwoStations.setParameter(MuonDriftCircleErrorStrategy::StationError, true);

        m_errorStrategyEE = strategy;
        m_errorStrategyEE.setParameter(MuonDriftCircleErrorStrategy::StationError, true);

        m_errorStrategyBIS78 = strategy;
        m_errorStrategyBIS78.setParameter(MuonDriftCircleErrorStrategy::StationError, true);

        m_errorStrategyBXE = strategy;
        m_errorStrategyBXE.setParameter(MuonDriftCircleErrorStrategy::StationError, true);

        m_errorStrategyEEL1C05 = strategy;
        m_errorStrategyEEL1C05.setParameter(MuonDriftCircleErrorStrategy::StationError, true);

        m_errorStrategySL = strategy;
        m_errorStrategySL.setParameter(MuonDriftCircleErrorStrategy::FixedError, true);
        m_errorStrategySL.setParameter(MuonDriftCircleErrorStrategy::BroadError, false);

        m_errorStrategyBarEnd = strategy;
        m_errorStrategyBarEnd.setParameter(MuonDriftCircleErrorStrategy::FixedError, true);
        m_errorStrategyBarEnd.setParameter(MuonDriftCircleErrorStrategy::BroadError, true);

        m_errorStrategy = strategy;
        m_errorStrategy.setParameter(MuonDriftCircleErrorStrategy::ScaledError, true);
        m_errorStrategy.setParameter(MuonDriftCircleErrorStrategy::BroadError, false);

        m_muonErrorStrategy.setStrategy(MuonDriftCircleErrorStrategy::Muon);
        m_muonErrorStrategy.setParameter(MuonDriftCircleErrorStrategy::ScaledError, true);
        m_muonErrorStrategy.setParameter(MuonDriftCircleErrorStrategy::BroadError, false);

        ATH_MSG_INFO("Options:");
        if (m_deweightBEE) ATH_MSG_INFO(" Deweight BEE");
        if (m_deweightEE) ATH_MSG_INFO(" Deweight EE");
        if (m_deweightBIS78) ATH_MSG_INFO(" Deweight BIS78");
        if (m_deweightBME) ATH_MSG_INFO(" Deweight BME");
        if (m_deweightBOE) ATH_MSG_INFO(" Deweight BOE");
        if (m_deweightEEL1C05) ATH_MSG_INFO(" Deweight EEL1C05");
        if (m_deweightTwoStationTracks) ATH_MSG_INFO(" Deweight Two stations");
        return StatusCode::SUCCESS;
    }

    StatusCode MuonRefitTool::finalize() {
        double scaleRefit = m_nrefits != 0 ? 1. / (double)m_nrefits : 1.;
        ATH_MSG_INFO("Number of refits                        "
                     << m_nrefits << std::endl
                     << "Good                                    " << scaleRefit * m_ngoodRefits << std::endl
                     << "Failed Outlier removal                  " << scaleRefit * m_failedOutlierRemoval << std::endl
                     << "Failed Error Update                     " << scaleRefit * m_failedErrorUpdate << std::endl
                     << "Failed Refit                            " << scaleRefit * m_failedRefit << std::endl
                     << "Failed Extrapolation to Muon Entry      " << scaleRefit * m_failedExtrapolationMuonEntry);
        return StatusCode::SUCCESS;
    }
    std::unique_ptr<Trk::Track> MuonRefitTool::refit(const Trk::Track& track, const EventContext& ctx,
                                                     const IMuonRefitTool::Settings* set) const {
        const IMuonRefitTool::Settings& settings = set ? *set : m_defaultSettings;

        // to keep track of the latest track
        std::unique_ptr<Trk::Track> newTrack;
        ++m_nrefits;
        if (settings.removeOutliers) {
            std::unique_ptr<Trk::Track> cleanedTrack = removeOutliers(track, settings);
            if (!cleanedTrack) {
                ATH_MSG_DEBUG("Track lost during outlier removal");
                ++m_failedOutlierRemoval;
                return std::make_unique<Trk::Track>(track);
            }
            if (cleanedTrack->perigeeParameters() != track.perigeeParameters()) {
                ATH_MSG_DEBUG("Outlier removal removed hits from track");
            }
            newTrack.swap(cleanedTrack);
        } else
            newTrack = std::make_unique<Trk::Track>(track);

        if (settings.updateErrors) {
            ATH_MSG_DEBUG("track hits before error updating: " << m_printer->printMeasurements(*newTrack));
            std::unique_ptr<Trk::Track> updateErrorTrack =
                m_alignmentErrors ? updateAlignmentErrors(*newTrack, ctx, settings) : updateErrors(*newTrack, ctx, settings);
            if (!updateErrorTrack) {
                ATH_MSG_WARNING("Failed to update errors");
                ++m_failedErrorUpdate;
                return newTrack;
            }
            newTrack.swap(updateErrorTrack);
        }

        if (settings.refit) {
            ATH_MSG_DEBUG("Original track" << m_printer->print(track));

            // do not put AEOTs on extremely bad chi2 tracks and do not refit them

            std::unique_ptr<Trk::Track> refittedTrack;
            if (track.fitQuality() && track.fitQuality()->chiSquared() < 10000 * track.fitQuality()->numberDoF())
                refittedTrack = std::unique_ptr<Trk::Track>(m_trackFitter->fit(ctx, *newTrack, false, Trk::muon));
            if (!refittedTrack) {
                ATH_MSG_DEBUG("Failed to refit track");
                ++m_failedRefit;
                // BUG fix Peter
                return std::make_unique<Trk::Track>(track);
            }
            ATH_MSG_DEBUG("Refitted track" << m_printer->print(*refittedTrack));
            ATH_MSG_DEBUG("Refitted track" << m_printer->printMeasurements(*refittedTrack));
            newTrack.swap(refittedTrack);
        }

        if (settings.extrapolateToMuonEntry) {
            std::unique_ptr<Trk::Track> extrapolatedTrack(m_muonEntryTrackExtrapolator->extrapolate(*newTrack, ctx));
            if (!extrapolatedTrack) {
                ATH_MSG_WARNING("Failed to back-extrapolate track");
                ++m_failedExtrapolationMuonEntry;
                return newTrack;
            }
            ATH_MSG_DEBUG("Entry track " << m_printer->print(*extrapolatedTrack));
            newTrack.swap(extrapolatedTrack);
        }
        ++m_ngoodRefits;

        return newTrack;
    }
    std::vector<std::unique_ptr<Trk::Track>> MuonRefitTool::refit(const std::vector<Trk::Track*>& tracks, const EventContext& ctx,
                                                                  const IMuonRefitTool::Settings* set) const {
        std::vector<std::unique_ptr<Trk::Track>> refittedTracks;
        refittedTracks.reserve(tracks.size());
        for (const Trk::Track* it : tracks) { refittedTracks.emplace_back(refit(*it, ctx, set)); }

        return refittedTracks;
    }

    std::unique_ptr<Trk::Track> MuonRefitTool::updateAlignmentErrors(const Trk::Track& track, const EventContext& ctx,
                                                                     const IMuonRefitTool::Settings& settings) const {
        // first scale the Mdt errors

        std::unique_ptr<Trk::Track> updatedTrack = updateMdtErrors(track, ctx, settings);

        std::unique_ptr<Trk::Track> updatedAEOTsTrack = m_simpleAEOTs ? makeSimpleAEOTs(*updatedTrack) : makeAEOTs(*updatedTrack);

        return updatedAEOTsTrack;
    }

    std::unique_ptr<Trk::Track> MuonRefitTool::makeAEOTs(const Trk::Track& track) const {
        //
        // use the new AlignmentEffectsOnTrack class and alignmentErrorTool
        //
        if (m_alignErrorTool.empty()) { return std::make_unique<Trk::Track>(track); }
        //
        // Use the alignmentErrorTool and store a list of hits with error on position and angle
        //
        std::map<std::vector<Identifier>, std::pair<double, double>> alignerrmap;

        std::vector<Trk::AlignmentDeviation*> align_deviations;
        m_alignErrorTool->makeAlignmentDeviations(track, align_deviations);

        int iok = 0;
        bool isSmallChamber = false;
        bool isLargeChamber = false;
        bool isEndcap = false;
        bool isBarrel = false;
        std::vector<int> usedRotations;

        // loop on deviations
        for (Trk::AlignmentDeviation* it : align_deviations) {
            double angleError = 0.;
            double translationError = 0.;
            bool differentChambers = false;
            int jdifferent = -1;
            isSmallChamber = false;
            isLargeChamber = false;
            isEndcap = false;
            isBarrel = false;

            if (dynamic_cast<MuonAlign::AlignmentTranslationDeviation*>(it)) {
                translationError = std::sqrt(it->getCovariance(0, 0));
                // vector to store hit id
                std::vector<Identifier> hitids;
                const auto& vec_riowithdev = it->getListOfHits();
                // bool to decide if deviation should be skipped (if it's for more than 1 station)
                for (const Trk::RIO_OnTrack* riowithdev : vec_riowithdev) {
                    const Identifier id_riowithdev = riowithdev->identify();
                    if (m_idHelperSvc->isEndcap(id_riowithdev)) {
                        isEndcap = true;
                    } else {
                        isBarrel = true;
                    }
                    if (m_idHelperSvc->isSmallChamber(id_riowithdev)) {
                        isSmallChamber = true;
                    } else {
                        isLargeChamber = true;
                    }
                    hitids.push_back(id_riowithdev);
                    if (hitids.size() > 1 && m_idHelperSvc->chamberId(id_riowithdev) != m_idHelperSvc->chamberId(hitids[0])) {
                        differentChambers = true;
                        jdifferent = hitids.size() - 1;
                    }
                }
                bool matchFound = false;
                if (!hitids.empty()) {
                    int iRot = -1;
                    for (Trk::AlignmentDeviation* itRot : align_deviations) {
                        ++iRot;
                        if (dynamic_cast<MuonAlign::AlignmentRotationDeviation*>(itRot)) {
                            if (itRot->hasValidHashOfHits() && it->hasValidHashOfHits()) {
                                if (itRot->getHashOfHits() == it->getHashOfHits()) {
                                    angleError = std::sqrt(itRot->getCovariance(0, 0));
                                    matchFound = true;
                                    usedRotations.push_back(iRot);
                                }
                            } else {
                                ATH_MSG_ERROR("One of the alignment deviations has an invalid hash created from the hits.");
                            }
                        }
                        if (matchFound) break;
                    }
                }
                // if deviation is accepted (i.e. only on one station) store the hit IDs associated with the deviation and the error

                // store (all) translationError with or without a matched angleError
                iok++;
                alignerrmap.insert(std::pair<std::vector<Identifier>, std::pair<double, double>>(
                    hitids, std::pair<double, double>(translationError, angleError)));

                if (matchFound)
                    ATH_MSG_DEBUG(" AlignmentMap entry " << iok << " filled with nr hitids " << hitids.size() << " "
                                                         << m_idHelperSvc->toString(hitids[0]) << " translationError " << translationError
                                                         << " angleError " << angleError);
                if (!matchFound)
                    ATH_MSG_DEBUG(" AlignmentMap entry No angleError" << iok << " filled with nr hitids " << hitids.size() << " "
                                                                      << m_idHelperSvc->toString(hitids[0]) << " translationError "
                                                                      << translationError << " angleError " << angleError);
                if (isEndcap) ATH_MSG_DEBUG(" AlignmentMap Endcap Chamber ");
                if (isBarrel) ATH_MSG_DEBUG(" AlignmentMap Barrel Chamber ");
                if (isSmallChamber) ATH_MSG_DEBUG(" AlignmentMap Small Chamber ");
                if (isLargeChamber) ATH_MSG_DEBUG(" AlignmentMap Large Chamber ");
                if (differentChambers)
                    ATH_MSG_DEBUG(" AlignmentMap entry " << iok << " for different Chamber "
                                                         << m_idHelperSvc->toString(hitids[jdifferent]));
            }
        }

        // now add the angleErrors that were NOT matched to a translationError

        int iRot = -1;
        for (Trk::AlignmentDeviation* itRot : align_deviations) {
            ++iRot;
            isSmallChamber = false;
            isLargeChamber = false;
            isEndcap = false;
            isBarrel = false;
            if (dynamic_cast<MuonAlign::AlignmentRotationDeviation*>(itRot)) {
                bool used = std::find(usedRotations.begin(), usedRotations.end(), iRot) != usedRotations.end();
                if (used) continue;
                ATH_MSG_ERROR("This following code should not be reached anymore!");
                const auto& vec_riowithdev = itRot->getListOfHits();

                std::vector<Identifier> hitids;
                // bool to decide if deviation should be skipped (if it's for more than 1 station)
                for (const Trk::RIO_OnTrack* riowithdev : vec_riowithdev) {
                    Identifier id_riowithdev = riowithdev->identify();
                    if (m_idHelperSvc->isEndcap(id_riowithdev)) {
                        isEndcap = true;
                    } else {
                        isBarrel = true;
                    }
                    if (m_idHelperSvc->isSmallChamber(id_riowithdev)) {
                        isSmallChamber = true;
                    } else {
                        isLargeChamber = true;
                    }
                    hitids.push_back(id_riowithdev);
                }

                double translationError = 0.;
                double angleError = std::sqrt(itRot->getCovariance(0, 0));

                iok++;
                alignerrmap.insert(std::pair<std::vector<Identifier>, std::pair<double, double>>(
                    hitids, std::pair<double, double>(translationError, angleError)));
                ATH_MSG_DEBUG(" AlignmentMap entry No Translation Error " << iok << " filled with nr hitids " << hitids.size() << " "
                                                                          << m_idHelperSvc->toString(hitids[0]) << " translationError "
                                                                          << translationError << " angleError " << angleError);
                if (isEndcap) ATH_MSG_DEBUG(" AlignmentMap Endcap Chamber");
                if (isBarrel) ATH_MSG_DEBUG(" AlignmentMap Barrel Chamber");
                if (isSmallChamber) ATH_MSG_DEBUG(" AlignmentMap Small Chamber ");
                if (isLargeChamber) ATH_MSG_DEBUG(" AlignmentMap Large Chamber ");
            }
        }

        // clean-up of alignment deviations
        for (auto* it : align_deviations) delete it;
        align_deviations.clear();

        const Trk::TrackStates* states = track.trackStateOnSurfaces();
        if (!states) {
            ATH_MSG_WARNING(" track without states, discarding track ");
            return nullptr;
        }

        std::bitset<Trk::TrackStateOnSurface::NumberOfTrackStateOnSurfaceTypes> typePattern(0);
        typePattern.set(Trk::TrackStateOnSurface::Alignment);

        std::vector<int> indexAEOTs;
        std::vector<std::unique_ptr<Trk::TrackStateOnSurface>> tsosAEOTs;

        ATH_MSG_DEBUG(" AlignmentMap size " << alignerrmap.size());

        std::set<MuonStationIndex::ChIndex> stationIds;

        for (const auto& itAli : alignerrmap) {
            unsigned int imiddle = (itAli.first.size()) / 2;
            Identifier idMiddle = itAli.first[imiddle];
            int index = -1;
            bool found = false;
            for (const Trk::TrackStateOnSurface* tsit : *states) {
                index++;
                const Trk::MeasurementBase* meas = tsit->measurementOnTrack();
                if (!meas) { continue; }
                Identifier id = m_edmHelperSvc->getIdentifier(*meas);
                if (!id.is_valid()) continue;

                if (m_idHelperSvc->isMdt(id)) stationIds.insert(m_idHelperSvc->chamberIndex(id));

                // make Alignment Effect using the surface of the TSOS

                if (idMiddle == id) {
                    /// Where do these magic numbers come from?
                    const double deltaError = std::max(itAli.second.first, 0.01);
                    const double angleError = std::max(itAli.second.second, 0.000001);
                    auto aEOT = std::make_unique<Trk::AlignmentEffectsOnTrack>(
                      0.,
                      deltaError,
                      0.,
                      angleError,
                      itAli.first,
                      tsit->measurementOnTrack()->associatedSurface());
                    std::unique_ptr<Trk::TrackStateOnSurface> tsosAEOT =
                      std::make_unique<Trk::TrackStateOnSurface>(
                        nullptr,
                        tsit->trackParameters()->uniqueClone(),
                        nullptr,
                        typePattern,
                        std::move(aEOT));
                    indexAEOTs.push_back(index);
                    tsosAEOTs.emplace_back(std::move(tsosAEOT));
                    found = true;
                    break;
                }
            }
            if (!found) ATH_MSG_WARNING(" This should not happen Identifier from AlignmentErrorTool is not found");
        }

        //
        // clone the TSOSs and add the tsosAEOTs
        //
        auto trackStateOnSurfaces = std::make_unique<Trk::TrackStates>();
        trackStateOnSurfaces->reserve(states->size() + indexAEOTs.size());
        int index = -1;
        for (const Trk::TrackStateOnSurface* tsit : *states) {
            index++;
            for (unsigned int i = 0; i < indexAEOTs.size(); i++) {
                if (index == indexAEOTs[i]) {
                    if (tsosAEOTs[i])
                        trackStateOnSurfaces->push_back(std::move(tsosAEOTs[i]));
                    else {
                        ATH_MSG_WARNING("There's a trial to push back the same AEOT twice to the track...");
                    }
                }
            }

            // Skip AEOTs that are already present, as they will be added above already
            if (tsit->alignmentEffectsOnTrack()) {
                ATH_MSG_DEBUG("makeAEOTs: Skipping insertion of old AEOT!");
                continue;
            }
            trackStateOnSurfaces->push_back(tsit->clone());
        }

        if (indexAEOTs.empty() && stationIds.size() > 1) ATH_MSG_WARNING(" Track without AEOT ");

        std::unique_ptr<Trk::Track> newTrack = std::make_unique<Trk::Track>(track.info(), std::move(trackStateOnSurfaces),
                                                                            track.fitQuality() ? track.fitQuality()->uniqueClone() : nullptr);

        ATH_MSG_DEBUG(m_printer->print(*newTrack));
        ATH_MSG_DEBUG(m_printer->printMeasurements(*newTrack));

        return newTrack;
    }

    std::unique_ptr<Trk::Track> MuonRefitTool::makeSimpleAEOTs(const Trk::Track& track) const {
        // use the new AlignmentEffectsOnTrack class

        const Trk::TrackStates* states = track.trackStateOnSurfaces();
        if (!states) {
            ATH_MSG_WARNING(" track without states, discarding track ");
            return nullptr;
        }

        //
        // first clone the TSOSs
        //
        auto trackStateOnSurfaces = std::make_unique<Trk::TrackStates>();
        trackStateOnSurfaces->reserve(states->size() + 1);
        for (const Trk::TrackStateOnSurface* tsit : *states) { trackStateOnSurfaces->push_back(tsit->clone()); }

        // loop over TSOSs and look for EM or BM chambers
        std::vector<const Trk::TrackStateOnSurface*> indicesOfAffectedTSOS;
        std::vector<const Trk::TrackStateOnSurface*> indicesOfAffectedTSOSInner;
        std::vector<Identifier> indicesOfAffectedIds;
        std::vector<Identifier> indicesOfAffectedIdsInner;
        int index {-1}, indexFirst {-1}, indexFirstInner {-1};
        for (const Trk::TrackStateOnSurface* tsit : *trackStateOnSurfaces) {
            ++index;
            if (!tsit) continue;  // sanity check

            const Trk::TrackParameters* pars = tsit->trackParameters();
            if (!pars) continue;

            // check whether state is a measurement
            const Trk::MeasurementBase* meas = tsit->measurementOnTrack();
            if (!meas) { continue; }

            // skip outliers
            if (tsit->type(Trk::TrackStateOnSurface::Outlier)) continue;
            if (tsit->alignmentEffectsOnTrack()) {
                ATH_MSG_WARNING(" AlignmentEffectOnTrack is already on track skip it");
                continue;
            }
            Identifier id = m_edmHelperSvc->getIdentifier(*meas);
            // Not a ROT, else it would have had an identifier. Keep the TSOS.
            if (!id.is_valid() || !m_idHelperSvc->isMuon(id)) continue;
            MuonStationIndex::StIndex stIndex = m_idHelperSvc->stationIndex(id);
            // skip phi measurements
            if ((m_idHelperSvc->isTrigger(id) && m_idHelperSvc->measuresPhi(id)) ||
                (m_idHelperSvc->isCsc(id) && m_idHelperSvc->measuresPhi(id)))
                continue;
            if (m_addAll) {
                // skip RPC and TGC eta (to avoid code crashes)
                if (m_idHelperSvc->isTrigger(id)) continue;
                if (indexFirst == -1) indexFirst = index;
                indicesOfAffectedTSOS.push_back(tsit);
                indicesOfAffectedIds.push_back(id);
            } else {
                // skip trigger hits and CSC phi measurements  and select precision hits
                if (m_idHelperSvc->isTrigger(id)) continue;
                if (stIndex == MuonStationIndex::BM || stIndex == MuonStationIndex::EM) {
                    if (indexFirst == -1) indexFirst = index;
                    indicesOfAffectedTSOS.push_back(tsit);
                    indicesOfAffectedIds.push_back(id);
                    //  two alignment discontinuities
                    if (m_addTwo) {
                        if (indexFirstInner == -1) indexFirstInner = index;
                        indicesOfAffectedTSOSInner.push_back(tsit);
                        indicesOfAffectedIdsInner.push_back(id);
                    }
                }
                if (stIndex == MuonStationIndex::BI || stIndex == MuonStationIndex::EI) {
                    if (indexFirstInner == -1) indexFirstInner = index;
                    indicesOfAffectedTSOSInner.push_back(tsit);
                    indicesOfAffectedIdsInner.push_back(id);
                }
            }
        }

        if (indicesOfAffectedTSOS.empty() && indicesOfAffectedTSOSInner.empty()) {
            std::unique_ptr<Trk::Track> newTrack = std::make_unique<Trk::Track>(track.info(), std::move(trackStateOnSurfaces),
                                                                                track.fitQuality() ? track.fitQuality()->uniqueClone() : nullptr);
            return newTrack;
        }

        std::bitset<Trk::TrackStateOnSurface::NumberOfTrackStateOnSurfaceTypes> typePattern(0);
        typePattern.set(Trk::TrackStateOnSurface::Alignment);

        std::unique_ptr<Trk::TrackStateOnSurface> tsosAEOT;
        if (!indicesOfAffectedTSOS.empty() && (m_addMiddle || m_addAll)) {
            int middle = indicesOfAffectedTSOS.size() / 2;
            const Trk::TrackStateOnSurface* tsos = indicesOfAffectedTSOS[middle];
            auto aEOT = std::make_unique<Trk::AlignmentEffectsOnTrack>(
              m_alignmentDelta,
              m_alignmentDeltaError,
              m_alignmentAngle,
              m_alignmentAngleError,
              indicesOfAffectedIds,
              tsos->measurementOnTrack()->associatedSurface());
            ATH_MSG_DEBUG(" AlignmentEffectsOnTrack on surface "
                          << aEOT->associatedSurface()
                          << " nr of tsos affected "
                          << indicesOfAffectedTSOS.size());
            tsosAEOT = std::make_unique<Trk::TrackStateOnSurface>(
              nullptr,
              tsos->trackParameters()->uniqueClone(),
              nullptr,
              typePattern,
              std::move(aEOT));
        }

        std::unique_ptr<Trk::TrackStateOnSurface> tsosAEOTInner;
        if (!indicesOfAffectedTSOSInner.empty() && (m_addInner || m_addTwo)) {
            int middle = indicesOfAffectedTSOSInner.size() / 2;
            const Trk::TrackStateOnSurface* tsosInner = indicesOfAffectedTSOSInner[middle];
            auto aEOTInner = std::make_unique<Trk::AlignmentEffectsOnTrack>(
              m_alignmentDelta,
              m_alignmentDeltaError,
              m_alignmentAngle,
              m_alignmentAngleError,
              indicesOfAffectedIdsInner,
              tsosInner->measurementOnTrack()->associatedSurface());
            tsosAEOTInner = std::make_unique<Trk::TrackStateOnSurface>(
              nullptr,
              tsosInner->trackParameters()->uniqueClone(),
              nullptr,
              typePattern,
              std::move(aEOTInner));
        }

        auto trackStateOnSurfacesAEOT = std::make_unique<Trk::TrackStates>();
        trackStateOnSurfacesAEOT->reserve(states->size() + 2);
        index = -1;
        for (const Trk::TrackStateOnSurface* tsit : *trackStateOnSurfaces) {
            index++;
            if (index == indexFirst && tsosAEOT) {
                trackStateOnSurfacesAEOT->push_back(std::move(tsosAEOT));
                if (!m_addAll) ATH_MSG_DEBUG(" AlignmentEffectsOnTrack for Middle added to trackStateOnSurfacesAEOT ");
                if (m_addAll) ATH_MSG_DEBUG(" AlignmentEffectsOnTrack for All stations added to trackStateOnSurfacesAEOT ");
            }
            if (index == indexFirstInner && tsosAEOTInner) {
                trackStateOnSurfacesAEOT->push_back(std::move(tsosAEOTInner));
                ATH_MSG_DEBUG(" AlignmentEffectsOnTrack for Inner added to trackStateOnSurfacesAEOT ");
                if (m_addTwo) ATH_MSG_DEBUG(" also AlignmentEffectsOnTrack for Middle added to trackStateOnSurfacesAEOT ");
            }
            trackStateOnSurfacesAEOT->push_back(tsit);
        }
        std::unique_ptr<Trk::Track> newTrack = std::make_unique<Trk::Track>(track.info(), std::move(trackStateOnSurfacesAEOT),
                                                                            track.fitQuality() ? track.fitQuality()->uniqueClone() : nullptr);
        ATH_MSG_DEBUG(m_printer->print(*newTrack));
        ATH_MSG_DEBUG(m_printer->printMeasurements(*newTrack));

        return newTrack;
    }

    std::unique_ptr<Trk::Track> MuonRefitTool::updateErrors(const Trk::Track& track, const EventContext& ctx,
                                                            const IMuonRefitTool::Settings& settings) const {
        // loop over track and calculate residuals
        const Trk::TrackStates* states = track.trackStateOnSurfaces();
        if (!states) {
            ATH_MSG_WARNING(" track without states, discarding track ");
            return nullptr;
        }

        // vector to store states, the boolean indicated whether the state was create in this routine (true) or belongs to the track (false)
        // If any new state is created, all states will be cloned and a new track will beformed from them.
        std::vector<std::unique_ptr<Trk::TrackStateOnSurface>> newStates;
        newStates.reserve(states->size() + 5);

        const Trk::TrackParameters* startPars = nullptr;
        std::map<int, std::set<MuonStationIndex::StIndex>> stationsPerSector;

        // loop over TSOSs and find start parameters
        for (const Trk::TrackStateOnSurface* tsit : *states) {
            
            if (!tsit) continue;  // sanity check

            const Trk::TrackParameters* pars = tsit->trackParameters();
            if (!pars) continue;

            if (tsit->type(Trk::TrackStateOnSurface::Perigee)) {
                if (!dynamic_cast<const Trk::Perigee*>(pars)) {
                    if (!startPars) {
                        startPars = pars;
                    } else {
                        ATH_MSG_WARNING("Track with two fit starting parameters!!!");
                    }
                }
            }

            // check whether state is a measurement
            const Trk::MeasurementBase* meas = tsit->measurementOnTrack();
            if (!meas) { continue; }

            // skip outliers
            if (tsit->type(Trk::TrackStateOnSurface::Outlier)) continue;

            Identifier id = m_edmHelperSvc->getIdentifier(*meas);
            // Not a ROT, else it would have had an identifier. Keep the TSOS.
            if (!id.is_valid() || !m_idHelperSvc->isMuon(id)) continue;
            if (m_idHelperSvc->isTrigger(id) || (m_idHelperSvc->isCsc(id) && m_idHelperSvc->measuresPhi(id))) continue;
            MuonStationIndex::StIndex stIndex = m_idHelperSvc->stationIndex(id);
            int sector = m_idHelperSvc->sector(id);
            stationsPerSector[sector].insert(stIndex);
        }

        if (!startPars) {
            if (!track.trackParameters() || track.trackParameters()->empty()) {
                ATH_MSG_WARNING("Track without parameters, cannot update errors");
                return nullptr;
            }
            startPars = track.trackParameters()->front();
            ATH_MSG_VERBOSE("Did not find fit starting parameters, using first parameters " << m_printer->print(*startPars));
        }

        // loop over sectors and select the one with most layers
        std::vector<int> sectorsWithMostStations;
        unsigned int nmaxStations = 0;
        std::map<int, std::set<MuonStationIndex::StIndex>>::iterator stit = stationsPerSector.begin();
        std::map<int, std::set<MuonStationIndex::StIndex>>::iterator stit_end = stationsPerSector.end();
        for (; stit != stit_end; ++stit) {
            if (msgLvl(MSG::VERBOSE)) {
                ATH_MSG_VERBOSE(" sector " << stit->first);
                for (std::set<MuonStationIndex::StIndex>::iterator ssit = stit->second.begin(); ssit != stit->second.end(); ++ssit) {
                    ATH_MSG_VERBOSE(" " << MuonStationIndex::stName(*ssit));
                }
            }
            if (stit->second.size() > nmaxStations) {
                nmaxStations = stit->second.size();
                sectorsWithMostStations.clear();
                sectorsWithMostStations.push_back(stit->first);
            } else if (stit->second.size() == nmaxStations) {
                sectorsWithMostStations.push_back(stit->first);
            }
        }
        int selectedSector = -1;
        if (sectorsWithMostStations.empty()) {
            ATH_MSG_WARNING("No sector selected");
        } else if (sectorsWithMostStations.size() == 1) {
            selectedSector = sectorsWithMostStations.front();
        } else {
            ATH_MSG_DEBUG("Found track with special sector configuration " << sectorsWithMostStations.size() << " ch per sector "
                                                                           << nmaxStations << " using first sector");
            selectedSector = sectorsWithMostStations.front();
            if (selectedSector % 2 == 1 && sectorsWithMostStations.back() % 2 != 1) {
                ATH_MSG_DEBUG("Revising sector choice, picking small sector ");
                selectedSector = sectorsWithMostStations.back();
            }
        }

        // no check whether we have a barrel/endcap overlap
        
        static constexpr std::array<MuonStationIndex::StIndex, 3> barel_stations{MuonStationIndex::BI, MuonStationIndex::BM, MuonStationIndex::BO};
        static constexpr std::array<MuonStationIndex::StIndex, 5> endcap_stations{MuonStationIndex::EI,MuonStationIndex::EM, MuonStationIndex::EO, MuonStationIndex::EE, MuonStationIndex::BE};
        const std::set<MuonStationIndex::StIndex>& selected_set = stationsPerSector[selectedSector];
        const int nbarrel = std::accumulate(barel_stations.begin(),barel_stations.end(),0, [&selected_set](int n, const MuonStationIndex::StIndex& idx){
            return (selected_set.count(idx) > 0) + n;
        });
        const int  nendcap = std::accumulate(endcap_stations.begin(),endcap_stations.end(),0, [&selected_set](int n, const MuonStationIndex::StIndex& idx){
            return (selected_set.count(idx) > 0) + n;
        });
        bool barrelEndcap {false}, deweightBarrel{false}, deweightEndcap{false};
        if (nbarrel > 0 && nendcap > 0) {
            if (nbarrel < nendcap)
                deweightBarrel = true;
            else
                deweightEndcap = true;
            barrelEndcap = true;
        }
        if (msgLvl(MSG::DEBUG)) {
            ATH_MSG_DEBUG(" Selected sector " << selectedSector << " nstations " << nmaxStations << " barrel " << nbarrel << " endcap "
                                              << nendcap);
            if (barrelEndcap) {
                ATH_MSG_DEBUG(" barrel/endcap overlap ");
                if (deweightEndcap) ATH_MSG_DEBUG(" deweight endcap ");
                if (deweightBarrel) ATH_MSG_DEBUG(" deweight barrel ");
            }
        }

        unsigned int deweightHits = 0;
        unsigned int removedSectorHits = 0;
        bool addedPerigee = false;
        // loop over TSOSs
        for (const Trk::TrackStateOnSurface* tsos : * states) {
            if (!tsos) continue;  // sanity check

            // check whether state is a measurement, if not add it, except if we haven't added the perigee surface yet
            const Trk::TrackParameters* pars = tsos->trackParameters();
            if (settings.prepareForFit && !pars) {
                if (addedPerigee) {
                    newStates.emplace_back(tsos->clone());                  
                } else {
                    ATH_MSG_DEBUG("Dropping TSOS before perigee surface");                   
                }
                continue;
            }

            // if preparing for fit and not recreating the starting parameters, add the original perigee before back extrapolation to MS
            // entry
            if (settings.prepareForFit && !settings.recreateStartingParameters && tsos->type(Trk::TrackStateOnSurface::Perigee)) {
                if (pars == startPars) {
                    ATH_MSG_DEBUG("Found fit starting parameters " << m_printer->print(*pars));
                    std::unique_ptr<Trk::Perigee> perigee = createPerigee(*pars, ctx);
                    newStates.emplace_back(MuonTSOSHelper::createPerigeeTSOS(std::move(perigee)));
                    addedPerigee = true;
                    continue;
                } else {
                    ATH_MSG_DEBUG("Removing perigee");
                }
            }

            // check whether state is a measurement
            const Trk::MeasurementBase* meas = tsos->measurementOnTrack();
            if (!meas) {
                newStates.emplace_back(tsos->clone());
                continue;
            }

            if (settings.prepareForFit && settings.recreateStartingParameters && !addedPerigee) {
                // small shift towards the ip
                double sign = pars->position().dot(pars->momentum()) > 0 ? 1. : -1.;
                Amg::Vector3D perpos = pars->position() - 100. * sign * pars->momentum().unit();

                // create perigee
                double phi = pars->momentum().phi();
                double theta = pars->momentum().theta();
                double qoverp = pars->charge() / pars->momentum().mag();
                Trk::PerigeeSurface persurf(perpos);
                std::unique_ptr<Trk::Perigee> perigee = std::make_unique<Trk::Perigee>(0, 0, phi, theta, qoverp, persurf);
                newStates.emplace_back(MuonTSOSHelper::createPerigeeTSOS(std::move(perigee)));
                addedPerigee = true;
                ATH_MSG_DEBUG("Adding perigee in front of first measurement");
            }

            Identifier id = m_edmHelperSvc->getIdentifier(*meas);

            // Not a ROT, else it would have had an identifier. Keep the TSOS.
            if (!id.is_valid() || !m_idHelperSvc->isMuon(id)) {
                newStates.emplace_back(tsos->clone());
                continue;
            }

            if (!settings.updateErrors) {
                newStates.emplace_back(tsos->clone());
            } else {
                Identifier chId = m_idHelperSvc->chamberId(id);
                MuonStationIndex::StIndex stIndex = m_idHelperSvc->stationIndex(id);
                if (m_idHelperSvc->isMdt(id)) {
                    const MdtDriftCircleOnTrack* mdt = dynamic_cast<const MdtDriftCircleOnTrack*>(meas);
                    if (!mdt) {
                        ATH_MSG_WARNING(" Measurement with MDT identifier that is not a MdtDriftCircleOnTrack ");
                        continue;
                    }

                    bool hasT0Fit = false;
                    if (mdt->errorStrategy().creationParameter(Muon::MuonDriftCircleErrorStrategy::T0Refit)) hasT0Fit = true;

                    std::unique_ptr<MdtDriftCircleOnTrack> rot{};
                    int sector = m_idHelperSvc->sector(id);
                    Trk::TrackStateOnSurface::TrackStateOnSurfaceType type = tsos->type(Trk::TrackStateOnSurface::Outlier)
                                                                                 ? Trk::TrackStateOnSurface::Outlier
                                                                                 : Trk::TrackStateOnSurface::Measurement;

                    stIndex = m_idHelperSvc->stationIndex(id);

                    // error update for three stations with barrel-endcap and shared sectors
                    if (!m_deweightTwoStationTracks || nmaxStations > 2) {
                        if (m_deweightEEL1C05 && stIndex == MuonStationIndex::EE &&
                            m_idHelperSvc->chamberIndex(id) == MuonStationIndex::EEL && m_idHelperSvc->stationEta(id) < 0 &&
                            m_idHelperSvc->stationPhi(id) == 3) {
                            // for this chamber the errors are enormous (for a period of time)
                            rot.reset(m_mdtRotCreator->updateError(*mdt, pars, &m_errorStrategyEEL1C05));

                        } else if (deweightBarrel && 
                                   std::find(barel_stations.begin(),barel_stations.end(),stIndex) != barel_stations.end()) {
                            rot.reset(m_mdtRotCreator->updateError(*mdt, pars, &m_errorStrategyBarEnd));
                            if (settings.removeBarrelEndcapOverlap) type = Trk::TrackStateOnSurface::Outlier;

                        } else if (deweightEndcap &&
                                  std::find(endcap_stations.begin(), endcap_stations.end(), stIndex) != endcap_stations.end())  {  // BEE chambers enter the endcap alignment system!
                            rot.reset(m_mdtRotCreator->updateError(*mdt, pars, &m_errorStrategyBarEnd));
                            if (settings.removeBarrelEndcapOverlap) type = Trk::TrackStateOnSurface::Outlier;

                        } else if (settings.deweightOtherSectors && sector != selectedSector) {
                            ++deweightHits;
                            rot.reset(m_mdtRotCreator->updateError(*mdt, pars, &m_errorStrategySL));

                        } else if (m_deweightBEE && stIndex == MuonStationIndex::BE) {
                            rot.reset(m_mdtRotCreator->updateError(*mdt, pars, &m_errorStrategyBEE));
                            if (settings.removeBEE) type = Trk::TrackStateOnSurface::Outlier;

                        } else if (m_deweightEE && stIndex == MuonStationIndex::EE) {
                            rot.reset(m_mdtRotCreator->updateError(*mdt, pars, &m_errorStrategyEE));

                        } else if (m_deweightBIS78 && stIndex == MuonStationIndex::BI &&
                                   m_idHelperSvc->chamberIndex(id) == MuonStationIndex::BIS && abs(m_idHelperSvc->stationEta(id)) > 6) {
                            rot.reset(m_mdtRotCreator->updateError(*mdt, pars, &m_errorStrategyBIS78));

                        } else if (m_deweightBME && stIndex == MuonStationIndex::BM && m_idHelperSvc->stationPhi(id) == 7 &&
                                   (m_idHelperSvc->mdtIdHelper()).stationName(id) == m_BME_station) {
                            rot.reset(m_mdtRotCreator->updateError(*mdt, pars, &m_errorStrategyBXE));

                        } else if (m_deweightBOE && stIndex == MuonStationIndex::BO &&
                                   m_idHelperSvc->chamberIndex(id) == MuonStationIndex::BOL && abs(m_idHelperSvc->stationEta(id)) == 7 &&
                                   m_idHelperSvc->stationPhi(id) == 7) {
                            rot.reset(m_mdtRotCreator->updateError(*mdt, pars, &m_errorStrategyBXE));

                        } else {
                            /** default strategy */
                            MuonDriftCircleErrorStrategy strat(m_errorStrategy);
                            if (hasT0Fit) strat.setParameter(MuonDriftCircleErrorStrategy::T0Refit, true);
                            if (settings.broad) strat.setParameter(MuonDriftCircleErrorStrategy::BroadError, true);
                            rot.reset( m_mdtRotCreator->updateError(*mdt, pars, &strat));
                        }
                    } else {
                        rot.reset(m_mdtRotCreator->updateError(*mdt, pars, &m_errorStrategyTwoStations));
                    }

                   
                    
                    if (!rot) {
                        rot.reset(mdt->clone());
                        type = Trk::TrackStateOnSurface::Outlier;
                    }
                    if (settings.removeOtherSectors) {
                        if (sector != selectedSector) {
                            ++removedSectorHits;
                            type = Trk::TrackStateOnSurface::Outlier;
                        }
                    }
                    if (settings.chambersToBeremoved.count(chId) || settings.precisionLayersToBeremoved.count(stIndex)) {
                        type = Trk::TrackStateOnSurface::Outlier;
                    }

                    if (msgLvl(MSG::DEBUG)) {
                        ATH_MSG_DEBUG(m_idHelperSvc->toString(rot->identify())
                                      << " radius " << rot->driftRadius() << " new err "
                                      << Amg::error(rot->localCovariance(), Trk::locR) << " old err "
                                      << Amg::error(mdt->localCovariance(), Trk::locR));
                        if (hasT0Fit)
                            ATH_MSG_DEBUG(" HasT0");
                        else
                            ATH_MSG_DEBUG(" No T0");
                        if (type == Trk::TrackStateOnSurface::Outlier) ATH_MSG_DEBUG(" Outlier");
                        if (std::abs(rot->driftRadius() - mdt->driftRadius()) > 0.1)
                            ATH_MSG_DEBUG(" Bad recalibration: old r " << mdt->driftRadius());
                    }
                    //the following is a cop-out until can sort out the unique_ptr magic for rot, mdt
                    std::unique_ptr<Trk::TrackStateOnSurface> new_tsos = MuonTSOSHelper::createMeasTSOSWithUpdate(*tsos, std::move(rot), pars->uniqueClone(), type);
                    newStates.emplace_back(std::move(new_tsos));
                } else if (m_idHelperSvc->isCsc(id)) {
                    if (settings.chambersToBeremoved.count(chId) || settings.precisionLayersToBeremoved.count(stIndex)) {
                        std::unique_ptr<Trk::TrackStateOnSurface> new_tsos = MuonTSOSHelper::cloneTSOS(*tsos, Trk::TrackStateOnSurface::Outlier);
                        newStates.emplace_back(std::move(new_tsos));

                    } else {
                        newStates.emplace_back(tsos->clone());
                    }
                } else if (m_idHelperSvc->isTrigger(id)) {
                    if (m_idHelperSvc->measuresPhi(id)) {
                        MuonStationIndex::PhiIndex phiIndex = m_idHelperSvc->phiIndex(id);

                        if (settings.chambersToBeremoved.count(chId) || settings.phiLayersToBeremoved.count(phiIndex)) {
                            std::unique_ptr<Trk::TrackStateOnSurface> new_tsos = MuonTSOSHelper::cloneTSOS(*tsos, Trk::TrackStateOnSurface::Outlier);
                            newStates.emplace_back(std::move(new_tsos));

                        } else {
                            newStates.emplace_back(tsos->clone());
                        }

                    } else {
                        if (settings.updateTriggerErrors) {
                            newStates.emplace_back(tsos->clone());

                        } else {
                            newStates.emplace_back(tsos->clone());
                        }
                    }
                } else if (m_idHelperSvc->isMM(id) || m_idHelperSvc->issTgc(id)) {
                    newStates.emplace_back(tsos->clone());

                } else {
                    ATH_MSG_WARNING(" unknown Identifier " << m_idHelperSvc->mdtIdHelper().print_to_string(id));
                }
            }
        }

        if (deweightHits > 0) ATH_MSG_DEBUG(" de-weighted " << deweightHits << " MDT hits from neighbouring sectors");
        if (removedSectorHits > 0) ATH_MSG_DEBUG(" removed " << removedSectorHits << " MDT hits from neighbouring sectors");

        ATH_MSG_VERBOSE(" original track had " << states->size() << " TSOS, adding " << newStates.size() - states->size() << " new TSOS ");

        // states were added, create a new track
        auto trackStateOnSurfaces = std::make_unique<Trk::TrackStates>();
        trackStateOnSurfaces->reserve(newStates.size());
        for (std::unique_ptr<Trk::TrackStateOnSurface>& new_state : newStates) {
            trackStateOnSurfaces->push_back(std::move(new_state));
        }
        std::unique_ptr<Trk::Track> newTrack = std::make_unique<Trk::Track>(track.info(), std::move(trackStateOnSurfaces),
                                                                            track.fitQuality() ? track.fitQuality()->uniqueClone() : nullptr);
        ATH_MSG_DEBUG("new track measurements: " << m_printer->printMeasurements(*newTrack));

        return newTrack;
    }

    std::unique_ptr<Trk::Track> MuonRefitTool::updateMdtErrors(const Trk::Track& track, const EventContext& ctx,
                                                               const IMuonRefitTool::Settings& settings) const {
        // uses the muonErrorStrategy

        // loop over track and calculate residuals
        const Trk::TrackStates* states = track.trackStateOnSurfaces();
        if (!states) {
            ATH_MSG_WARNING(" track without states, discarding track ");
            return nullptr;
        }

        // vector to store states, the boolean indicated whether the state was create in this routine (true) or belongs to the track (false)
        // If any new state is created, all states will be cloned and a new track will beformed from them.
        std::vector<std::unique_ptr<Trk::TrackStateOnSurface>> newStates;
        newStates.reserve(states->size() + 5);

        const Trk::TrackParameters* startPars = nullptr;

        // loop over TSOSs and find start parameters
        for (const Trk::TrackStateOnSurface* tsos : *states) {
            if (!tsos) continue;  // sanity check

            const Trk::TrackParameters* pars = tsos->trackParameters();
            if (!pars) continue;

            if (tsos->type(Trk::TrackStateOnSurface::Perigee)) {
                if (!dynamic_cast<const Trk::Perigee*>(pars)) {
                    if (!startPars) {
                        startPars = pars;
                    } else {
                        ATH_MSG_WARNING("Track with two fit starting parameters!!!");
                    }
                }
            }

            // check whether state is a measurement
            const Trk::MeasurementBase* meas = tsos->measurementOnTrack();
            if (!meas) { continue; }

            // skip outliers
            if (tsos->type(Trk::TrackStateOnSurface::Outlier)) continue;

            Identifier id = m_edmHelperSvc->getIdentifier(*meas);
            // Not a ROT, else it would have had an identifier. Keep the TSOS.
            if (!id.is_valid() || !m_idHelperSvc->isMuon(id)) continue;
            if (m_idHelperSvc->isTrigger(id) || (m_idHelperSvc->isCsc(id) && m_idHelperSvc->measuresPhi(id))) continue;
        }

        if (!startPars) {
            if (!track.trackParameters() || track.trackParameters()->empty()) {
                ATH_MSG_WARNING("Track without parameters, cannot update errors");
                return nullptr;
            }
            startPars = track.trackParameters()->front();
            ATH_MSG_VERBOSE("Did not find fit starting parameters, using first parameters " << m_printer->print(*startPars));
        }

        bool addedPerigee = false;
        // loop over TSOSs
        for (const Trk::TrackStateOnSurface* tsos : *states) {
            if (!tsos) continue;  // sanity check

            // check whether state is a measurement, if not add it, except if we haven't added the perigee surface yet
            const Trk::TrackParameters* pars = tsos->trackParameters();
            if (settings.prepareForFit && !pars) {
                if (addedPerigee) {
                    newStates.emplace_back(tsos->clone());
                    continue;
                } else {
                    ATH_MSG_DEBUG("Dropping TSOS before perigee surface");
                    continue;
                }
            }

            // if preparing for fit and not recreating the starting parameters, add the original perigee before back extrapolation to MS
            // entry
            if (settings.prepareForFit && !settings.recreateStartingParameters && tsos->type(Trk::TrackStateOnSurface::Perigee)) {
                if (pars == startPars) {
                    ATH_MSG_DEBUG("Found fit starting parameters " << m_printer->print(*pars));
                    std::unique_ptr<Trk::Perigee> perigee = createPerigee(*pars, ctx);
                    newStates.emplace_back(MuonTSOSHelper::createPerigeeTSOS(std::move(perigee)));
                    addedPerigee = true;
                    continue;
                } else {
                    ATH_MSG_DEBUG("Removing perigee");
                }
            }

            // check whether state is a measurement
            const Trk::MeasurementBase* meas = tsos->measurementOnTrack();
            if (!meas) {
                newStates.emplace_back(tsos->clone());
                continue;
            }

            if (settings.prepareForFit && settings.recreateStartingParameters && !addedPerigee) {
                // small shift towards the ip
                double sign = pars->position().dot(pars->momentum()) > 0 ? 1. : -1.;
                Amg::Vector3D perpos = pars->position() - 100. * sign * pars->momentum().unit();

                // create perigee
                double phi = pars->momentum().phi();
                double theta = pars->momentum().theta();
                double qoverp = pars->charge() / pars->momentum().mag();
                Trk::PerigeeSurface persurf(perpos);
                std::unique_ptr<Trk::Perigee> perigee = std::make_unique<Trk::Perigee>(0, 0, phi, theta, qoverp, persurf);
                newStates.emplace_back(MuonTSOSHelper::createPerigeeTSOS(std::move(perigee)));
                addedPerigee = true;
                ATH_MSG_DEBUG("Adding perigee in front of first measurement");
            }

            Identifier id = m_edmHelperSvc->getIdentifier(*meas);

            // Not a ROT, else it would have had an identifier. Keep the TSOS.
            if (!id.is_valid() || !m_idHelperSvc->isMuon(id)) {
                newStates.emplace_back(tsos->clone());
                continue;
            }

            if (!settings.updateErrors) {
                newStates.emplace_back(tsos->clone());
            } else {
                Identifier chId = m_idHelperSvc->chamberId(id);
                MuonStationIndex::StIndex stIndex = m_idHelperSvc->stationIndex(id);
                if (m_idHelperSvc->isMdt(id)) {
                    const MdtDriftCircleOnTrack* mdt = dynamic_cast<const MdtDriftCircleOnTrack*>(meas);
                    if (!mdt) {
                        ATH_MSG_WARNING(" Measurement with MDT identifier that is not a MdtDriftCircleOnTrack ");
                        continue;
                    }

                    bool hasT0Fit = false;
                    if (mdt->errorStrategy().creationParameter(Muon::MuonDriftCircleErrorStrategy::T0Refit)) hasT0Fit = true;

                    Trk::RIO_OnTrack* rot = nullptr;
                    Trk::TrackStateOnSurface::TrackStateOnSurfaceType type = tsos->type(Trk::TrackStateOnSurface::Outlier)
                                                                                 ? Trk::TrackStateOnSurface::Outlier
                                                                                 : Trk::TrackStateOnSurface::Measurement;

                    stIndex = m_idHelperSvc->stationIndex(id);

                    // use the muonErrorStrategy
                    MuonDriftCircleErrorStrategy strat(m_muonErrorStrategy);
                    if (hasT0Fit) strat.setParameter(MuonDriftCircleErrorStrategy::T0Refit, true);
                    if (settings.broad) strat.setParameter(MuonDriftCircleErrorStrategy::BroadError, true);
                    rot = m_mdtRotCreator->updateError(*mdt, pars, &strat);

                    MdtDriftCircleOnTrack* newMdt = rot ? dynamic_cast<MdtDriftCircleOnTrack*>(rot) : nullptr;
                    if (!newMdt) {
                        newMdt = mdt->clone();
                        type = Trk::TrackStateOnSurface::Outlier;
                    }
                    if (settings.chambersToBeremoved.count(chId) || settings.precisionLayersToBeremoved.count(stIndex)) {
                        type = Trk::TrackStateOnSurface::Outlier;
                    }

                    if (msgLvl(MSG::DEBUG)) {
                        ATH_MSG_DEBUG(" updateMdtErrors " << m_idHelperSvc->toString(newMdt->identify()) << " radius "
                                                          << newMdt->driftRadius() << " new err "
                                                          << Amg::error(newMdt->localCovariance(), Trk::locR) << " old err "
                                                          << Amg::error(mdt->localCovariance(), Trk::locR));
                        if (hasT0Fit)
                            ATH_MSG_DEBUG(" HasT0");
                        else
                            ATH_MSG_DEBUG(" No T0");
                        if (type == Trk::TrackStateOnSurface::Outlier) ATH_MSG_DEBUG(" Outlier");
                        if (std::abs(newMdt->driftRadius() - mdt->driftRadius()) > 0.1)
                            ATH_MSG_DEBUG(" Bad recalibration: old r " << mdt->driftRadius());
                    }
                    std::unique_ptr<MdtDriftCircleOnTrack> newUniqueMdt {newMdt};
                    std::unique_ptr<Trk::TrackStateOnSurface> new_tsos = MuonTSOSHelper::createMeasTSOSWithUpdate(*tsos, std::move(newUniqueMdt), pars->uniqueClone(), type);
                    newStates.emplace_back(std::move(new_tsos));
                } else if (m_idHelperSvc->isCsc(id)) {
                    if (settings.chambersToBeremoved.count(chId) || settings.precisionLayersToBeremoved.count(stIndex)) {
                        std::unique_ptr<Trk::TrackStateOnSurface> new_tsos = MuonTSOSHelper::cloneTSOS(*tsos, Trk::TrackStateOnSurface::Outlier);
                        newStates.emplace_back(std::move(new_tsos));

                    } else {
                        newStates.emplace_back(tsos->clone());
                    }
                } else if (m_idHelperSvc->isTrigger(id)) {
                    if (m_idHelperSvc->measuresPhi(id)) {
                        MuonStationIndex::PhiIndex phiIndex = m_idHelperSvc->phiIndex(id);

                        if (settings.chambersToBeremoved.count(chId) || settings.phiLayersToBeremoved.count(phiIndex)) {
                            std::unique_ptr<Trk::TrackStateOnSurface> new_tsos = MuonTSOSHelper::cloneTSOS(*tsos, Trk::TrackStateOnSurface::Outlier);
                            newStates.emplace_back(std::move(new_tsos));

                        } else {
                            newStates.emplace_back(tsos->clone());
                        }

                    } else {
                        if (settings.updateTriggerErrors) {
                            newStates.emplace_back(tsos->clone());

                        } else {
                            newStates.emplace_back(tsos->clone());
                        }
                    }
                } else if (m_idHelperSvc->isMM(id) || m_idHelperSvc->issTgc(id)) {
                    newStates.emplace_back(tsos->clone());

                } else {
                    ATH_MSG_WARNING(" unknown Identifier " << m_idHelperSvc->mdtIdHelper().print_to_string(id));
                }
            }
        }

        ATH_MSG_VERBOSE(" original track had " << states->size() << " TSOS, adding " << newStates.size() - states->size() << " new TSOS ");

        // states were added, create a new track
        auto trackStateOnSurfaces = std::make_unique<Trk::TrackStates>();
        trackStateOnSurfaces->reserve(newStates.size());
        for ( std::unique_ptr<Trk::TrackStateOnSurface>& state : newStates) {
            // add states. If nit->first is true we have a new state. If it is false the state is from the old track and has to be cloned
            trackStateOnSurfaces->push_back(std::move(state));
        }
        std::unique_ptr<Trk::Track> newTrack = std::make_unique<Trk::Track>(track.info(), std::move(trackStateOnSurfaces),
                                                                            track.fitQuality() ? track.fitQuality()->uniqueClone() : nullptr);
        return newTrack;
    }

    std::unique_ptr<Trk::Track> MuonRefitTool::removeOutliers(const Trk::Track& track, const IMuonRefitTool::Settings& settings) const {
        // loop over track and calculate residuals
        const Trk::TrackStates* states = track.trackStateOnSurfaces();
        if (!states) {
            ATH_MSG_WARNING(" track without states, discarding track ");
            return nullptr;
        }

        Identifier currentMdtChId;
        std::set<Identifier> removedIdentifiers;
        std::vector<const MdtDriftCircleOnTrack*> mdts;
        const Trk::TrackParameters* chamberPars = nullptr;

        // loop over TSOSs and find start parameters
        Trk::TrackStates::const_iterator tsit = states->begin();
        Trk::TrackStates::const_iterator tsit_end = states->end();
        for (; tsit != tsit_end; ++tsit) {
            if (!*tsit) continue;  // sanity check

            // check whether state is a measurement
            const Trk::TrackParameters* pars = (*tsit)->trackParameters();
            if (!pars) { continue; }

            if (!(*tsit)->type(Trk::TrackStateOnSurface::Measurement)) { continue; }

            // check whether state is a measurement
            const Trk::MeasurementBase* meas = (*tsit)->measurementOnTrack();
            if (!meas) { continue; }

            Identifier id = m_edmHelperSvc->getIdentifier(*meas);

            // Not a ROT, else it would have had an identifier. Keep the TSOS.
            if (!id.is_valid()) { continue; }

            if (m_idHelperSvc->isMdt(id)) {
                const MdtDriftCircleOnTrack* mdt = dynamic_cast<const MdtDriftCircleOnTrack*>(meas);
                if (!mdt) {
                    ATH_MSG_WARNING(" Measurement with MDT identifier that is not a MdtDriftCircleOnTrack ");
                    continue;
                }
                // get ch ID
                Identifier chId = m_idHelperSvc->chamberId(id);

                // if we have a new chambers
                if (chId != currentMdtChId) {
                    // check that there are pars (not the case for the first mdt), if so we collected all hits for this chamber so call
                    // cleaning
                    if (chamberPars) {
                        if (!removeMdtOutliers(*chamberPars, mdts, removedIdentifiers, settings)) {
                            if (mdts.size() > 4)
                                ATH_MSG_WARNING("Problem removing outliers in chamber " << m_idHelperSvc->toStringChamber(currentMdtChId)
                                                                                        << " hits " << mdts.size());
                            if (settings.discardNotCleanedTracks) return nullptr;
                        }
                    }
                    // update to new chamber
                    chamberPars = pars;
                    mdts.clear();
                    currentMdtChId = chId;
                }

                mdts.push_back(mdt);
            }
        }

        // clean the last chamber on the track
        if (chamberPars) {
            if (!removeMdtOutliers(*chamberPars, mdts, removedIdentifiers, settings)) {
                if (mdts.size() > 4)
                    ATH_MSG_WARNING("Problem removing outliers in chamber " << m_idHelperSvc->toStringChamber(currentMdtChId) << " hits "
                                                                            << mdts.size());
                if (settings.discardNotCleanedTracks) return nullptr;
            }
        }

        if (removedIdentifiers.empty()) {
            ATH_MSG_DEBUG("No hits remove, returning original track");
            return std::make_unique<Trk::Track>(track);
        }

        // states were added, create a new track
        auto  trackStateOnSurfaces = std::make_unique<Trk::TrackStates>();
        trackStateOnSurfaces->reserve(states->size());

        ATH_MSG_DEBUG("Removing nhits: " << removedIdentifiers.size());

        for (const Trk::TrackStateOnSurface* tsos : *states) {
            if (!*tsit) continue;  // sanity check

            // check whether state is a measurement
            const Trk::MeasurementBase* meas = tsos->measurementOnTrack();
            if (meas) {
                Identifier id = m_edmHelperSvc->getIdentifier(*meas);

                if (removedIdentifiers.count(id)) {
                    std::unique_ptr<Trk::TrackStateOnSurface> new_tsos = MuonTSOSHelper::cloneTSOS(*tsos, Trk::TrackStateOnSurface::Outlier);
                    trackStateOnSurfaces->push_back(std::move(new_tsos));
                    continue;
                }
            }
            trackStateOnSurfaces->push_back(tsos->clone());
        }

        std::unique_ptr<Trk::Track> newTrack = std::make_unique<Trk::Track>(track.info(), std::move(trackStateOnSurfaces),
                                                                            track.fitQuality() ? track.fitQuality()->uniqueClone() : nullptr);
        return newTrack;
    }

    bool MuonRefitTool::removeMdtOutliers(const Trk::TrackParameters& pars, const std::vector<const MdtDriftCircleOnTrack*>& hits,
                                          std::set<Identifier>& removedIdentifiers, const IMuonRefitTool::Settings& settings) const {
        if (hits.size() < 3) {
            ATH_MSG_VERBOSE("Too few hits, cannot perform cleaning");
            return false;
        }
        ATH_MSG_VERBOSE("Performing cleaning, nhits " << hits.size());

        TrkDriftCircleMath::DCOnTrackVec dcsOnTrack;
        TrkDriftCircleMath::DCVec dcs;
        /* ********  Mdt hits  ******** */

        const MuonGM::MdtReadoutElement* detEl = nullptr;

        Amg::Transform3D gToStation;

        // set to get Identifiers of chambers with hits
        std::vector<std::pair<Identifier, bool>> indexIdMap;
        indexIdMap.reserve(hits.size());

        TrkDriftCircleMath::DCSLFitter dcslFitter;
        TrkDriftCircleMath::SegmentFinder segFinder(5., 3., false);
        if (!m_t0Fitter.empty()) {
            std::shared_ptr<const TrkDriftCircleMath::DCSLFitter> fitter(m_t0Fitter->getFitter(), Muon::IDCSLFitProvider::Unowned{});
            segFinder.setFitter(fitter);
        }
        segFinder.debugLevel(m_finderDebugLevel);
        segFinder.setRecoverMDT(false);

        unsigned index = 0;
        for (const MdtDriftCircleOnTrack* mdt : hits) {
            if (!mdt) { continue; }
            Identifier id = mdt->identify();

            if (!detEl) {
                detEl = mdt->prepRawData()->detectorElement();
                if (!detEl) {
                    ATH_MSG_WARNING(" error aborting not detEl found ");
                    break;
                }
                gToStation = detEl->GlobalToAmdbLRSTransform();
            }
            // calculate local AMDB position
            Amg::Vector3D locPos = gToStation * mdt->prepRawData()->globalPosition();
            TrkDriftCircleMath::LocVec2D lpos(locPos.y(), locPos.z());

            double r = std::abs(mdt->localParameters()[Trk::locR]);
            double dr = Amg::error(mdt->localCovariance(), Trk::locR);
            ATH_MSG_VERBOSE("New MDT " << m_idHelperSvc->toString(id) << "  r " << mdt->localParameters()[Trk::locR] << " dr  " << dr
                                       << "  (original) " << Amg::error(mdt->localCovariance(), Trk::locR));

            // create identifier
            TrkDriftCircleMath::MdtId mdtid(m_idHelperSvc->mdtIdHelper().isBarrel(id), m_idHelperSvc->mdtIdHelper().multilayer(id) - 1,
                                            m_idHelperSvc->mdtIdHelper().tubeLayer(id) - 1, m_idHelperSvc->mdtIdHelper().tube(id) - 1);

            // create new DriftCircle
            TrkDriftCircleMath::DriftCircle dc(lpos, r, 1., dr, TrkDriftCircleMath::DriftCircle::InTime, mdtid, index, mdt);
            dcsOnTrack.emplace_back(dc, 1., 1.);
            dcs.emplace_back(std::move(dc));
            indexIdMap.emplace_back(id, false);

            ++index;
        }

        if (!detEl) return false;
        // define axis of chamber in global coordinates
        Amg::Transform3D amdbToGlobal = detEl->AmdbLRSToGlobalTransform();

        // create new surface
        Amg::Transform3D surfaceTransform(amdbToGlobal.rotation());
        surfaceTransform.pretranslate(pars.position());
        double surfDim = 500.;
        const std::unique_ptr<Trk::PlaneSurface> surf = std::make_unique<Trk::PlaneSurface>(surfaceTransform, surfDim, surfDim);

        Amg::Vector3D dir = pars.momentum().unit();
        if (dir.y() * pars.position().y() < 0.) { dir *= -1.; }
        Trk::LocalDirection locDir;
        surf->globalToLocalDirection(dir, locDir);

        Amg::Vector3D locDirTrack(gToStation.linear() * dir);
        double track_angleYZ = std::atan2(locDirTrack.z(), locDirTrack.y());

        // transform nominal pointing chamber position into surface frame
        Amg::Vector3D dirCh(gToStation.linear() * detEl->center());
        double chamber_angleYZ = std::atan2(dirCh.z(), dirCh.y());
        double angleYZ = locDir.angleYZ();

        const Amg::Vector3D lpos = gToStation * pars.position();

        TrkDriftCircleMath::LocVec2D segPos(lpos.y(), lpos.z());
        TrkDriftCircleMath::Line segPars(segPos, angleYZ);

        ATH_MSG_DEBUG("Seeding angles " << track_angleYZ << " from surf " << angleYZ << " ch angle " << chamber_angleYZ << " pos "
                                        << segPos);
        segFinder.setPhiRoad(track_angleYZ, chamber_angleYZ, 0.14);

        if (msgLvl(MSG::VERBOSE)) {
            TrkDriftCircleMath::Segment segment(TrkDriftCircleMath::Line(0., 0., 0.), TrkDriftCircleMath::DCOnTrackVec());
            if (dcslFitter.fit(segment, segPars, dcsOnTrack)) {
                segment.hitsOnTrack(dcsOnTrack.size());
                ATH_MSG_DEBUG(" segment after fit " << segment.chi2() << " ndof " << segment.ndof() << " local parameters "
                                                    << segment.line().x0() << " " << segment.line().y0() << "  phi "
                                                    << segment.line().phi());
            } else {
                ATH_MSG_DEBUG("Fit failed: hits" << dcsOnTrack.size());
            }
        }

        TrkDriftCircleMath::SegVec segments = segFinder.findSegments(dcs);
        if (!segments.empty()) { ATH_MSG_DEBUG("Found segments " << segments.size()); }

        if (segments.size() != 1) {
            if (hits.size() > 3)
                ATH_MSG_WARNING(" Found two solutions ");
            else
                ATH_MSG_DEBUG(" Found two solutions ");
            double dthetaBest = 10000.;
            int index = 0;
            int indexBest = -1;
            TrkDriftCircleMath::SegIt sit = segments.begin();
            TrkDriftCircleMath::SegIt sit_end = segments.end();
            for (; sit != sit_end; ++sit, ++index) {
                double dtheta = std::abs(sit->line().phi() - track_angleYZ);
                if (dtheta < dthetaBest) {
                    dthetaBest = dtheta;
                    indexBest = index;
                }
                if (sit->hitsOnTrack() > 4) { ATH_MSG_DEBUG("Recoverable segment " << *sit); }
            }
            if (indexBest != -1) {
                TrkDriftCircleMath::SegVec selectedSegments;
                selectedSegments.push_back(segments[indexBest]);
                segments = selectedSegments;
                ATH_MSG_DEBUG("Selected segment " << segments.front());

            } else {
                return false;
            }
        }

        TrkDriftCircleMath::Segment& segment = segments.front();
        if (settings.discardNotCleanedTracks && !segment.hasT0Shift()) return false;

        if (segment.hasT0Shift() || segment.hitsOnTrack() > 5) { ATH_MSG_DEBUG("Segment with t0 shift " << segment.t0Shift()); }

        if (dcs.size() == segment.hitsOnTrack()) {
            ATH_MSG_DEBUG(" No hits removed ");
            return true;
        } else if (dcs.size() > segment.hitsOnTrack() + 1) {
            ATH_MSG_DEBUG(" more than one hit removed ");
            if (segment.hitsOnTrack() < 4) return false;
        }

        ATH_MSG_DEBUG(" removed hits: " << dcs.size() - segment.hitsOnTrack());

        float tubeRadius = detEl->innerTubeRadius();

        TrkDriftCircleMath::MatchDCWithLine matchDC(segment.line(), 3., TrkDriftCircleMath::MatchDCWithLine::Pull, tubeRadius);
        const TrkDriftCircleMath::DCOnTrackVec& matchedDCs = matchDC.match(segment.dcs());

        for (TrkDriftCircleMath::DCOnTrackCit dcit = matchedDCs.begin(); dcit != matchedDCs.end(); ++dcit) {
            if (dcit->state() == TrkDriftCircleMath::DCOnTrack::OnTrack) {
                if (std::abs(dcit->r()) - std::abs(dcit->rot()->driftRadius()) > 0.1) {
                    ATH_MSG_DEBUG("Large change in drift radius: r_old " << dcit->rot()->driftRadius() << "  r_new " << dcit->r());
                }
                continue;
            }
            indexIdMap[dcit->index()].second = true;
        }

        std::vector<std::pair<Identifier, bool>>::iterator iit = indexIdMap.begin();
        std::vector<std::pair<Identifier, bool>>::iterator iit_end = indexIdMap.end();
        for (; iit != iit_end; ++iit) {
            if (iit->second) {
                ATH_MSG_VERBOSE(" removing hit " << m_idHelperSvc->toString(iit->first));
                removedIdentifiers.insert(iit->first);
            }
        }
        return true;
    }

    std::unique_ptr<Trk::Perigee> 
    MuonRefitTool::createPerigee(const Trk::TrackParameters& pars, const EventContext& ctx) const {
        std::unique_ptr<Trk::Perigee> perigee;
        if (m_muonExtrapolator.empty()) { return perigee; }
        Trk::PerigeeSurface persurf(pars.position());
        std::unique_ptr<Trk::TrackParameters> exPars{m_muonExtrapolator->extrapolateDirectly(ctx, pars, persurf)};
        perigee.reset (dynamic_cast<Trk::Perigee*>(exPars.release()));
        if (!perigee) {
            ATH_MSG_WARNING(" Extrapolation to Perigee surface did not return a perigee!! ");
            return perigee;
        }
        return perigee;
    }

}  // namespace Muon
