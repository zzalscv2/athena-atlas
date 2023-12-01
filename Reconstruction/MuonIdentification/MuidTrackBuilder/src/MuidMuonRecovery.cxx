/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

//////////////////////////////////////////////////////////////////////////////
// MuidMuonRecovery
//  AlgTool performing MS hit reallocation for a likely spectrometer-indet
//  match which has given combined fit problems.
//  Extrapolates indet track to MS.
//  Returns a combined track with full track fit.
//
//////////////////////////////////////////////////////////////////////////////

#include "MuidMuonRecovery.h"

#include <cmath>
#include <iomanip>
#include <vector>

#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "MuonRIO_OnTrack/CscClusterOnTrack.h"
#include "MuonRIO_OnTrack/MdtDriftCircleOnTrack.h"
#include "MuonReadoutGeometry/MuonReadoutElement.h"
#include "TrkEventPrimitives/LocalDirection.h"
#include "TrkEventPrimitives/ResidualPull.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkPseudoMeasurementOnTrack/PseudoMeasurementOnTrack.h"
#include "TrkSegment/SegmentCollection.h"
#include "TrkSurfaces/PlaneSurface.h"
#include "TrkTrack/Track.h"
#include "TrkTrackSummary/TrackSummary.h"

namespace Rec {

    MuidMuonRecovery::MuidMuonRecovery(const std::string& type, const std::string& name, const IInterface* parent) :
        AthAlgTool(type, name, parent){
        declareInterface<IMuidMuonRecovery>(this);
    }

    //<<<<<< PUBLIC MEMBER FUNCTION DEFINITIONS                             >>>>>>

    StatusCode MuidMuonRecovery::initialize() {
        ATH_MSG_INFO("Initializing MuidMuonRecovery");

        // get the Tools
        ATH_CHECK(m_extrapolator.retrieve());
        ATH_MSG_INFO("Retrieved tool " << m_extrapolator);
        ATH_CHECK(m_edmHelperSvc.retrieve());
        ATH_MSG_INFO("Retrieved tool " << m_edmHelperSvc);
        ATH_CHECK(m_idHelperSvc.retrieve());
        ATH_MSG_INFO("Retrieved tool " << m_idHelperSvc);
        ATH_CHECK(m_printer.retrieve());
        ATH_MSG_INFO("Retrieved tool " << m_printer);
        ATH_CHECK(m_residualCalculator.retrieve());
        ATH_MSG_INFO("Retrieved tool " << m_residualCalculator);

        if (!m_trackBuilder.empty()) {
            ATH_CHECK(m_trackBuilder.retrieve());
            ATH_MSG_INFO("Retrieved tool " << m_trackBuilder);
        }

        return StatusCode::SUCCESS;
    }

    StatusCode MuidMuonRecovery::finalize() {
        ATH_MSG_INFO("Recovery attempts " << m_recoveryAttempts << ", failedFit " << m_recoveryFitFailure << ", success "
                                          << m_recoverySuccess);

        return StatusCode::SUCCESS;
    }
    std::unique_ptr<Trk::Track> MuidMuonRecovery::recoverableMatch(const Trk::Track& indetTrack, const Trk::Track& spectrometerTrack,
                                                                   const EventContext& ctx) const {
        // skip low pt ID tracks
        if (!indetTrack.perigeeParameters() || indetTrack.perigeeParameters()->momentum().mag() < m_minP ||
            indetTrack.perigeeParameters()->momentum().perp() < m_minPt) {
            return nullptr;
        }

        ++m_recoveryAttempts;

        ATH_MSG_DEBUG("Entering new recovery" << std::endl
                                              << " ID track " << m_printer->print(indetTrack) << std::endl
                                              << " MS track " << m_printer->print(spectrometerTrack) << std::endl
                                              << m_printer->printStations(spectrometerTrack));

        const Trk::TrackParameters* lastIndetPars = nullptr;
        int index = static_cast<int>(indetTrack.trackParameters()->size());

        while (!lastIndetPars && index > 0) {
            --index;
            lastIndetPars = (*indetTrack.trackParameters())[index] ? (*indetTrack.trackParameters())[index] : nullptr;
        }

        if (!lastIndetPars) {
            ATH_MSG_WARNING("ID track parameters don't have error matrix!");
            return nullptr;
        }

        // track builder prefers estimate of inner, middle and outer spectrometer track parameters
        std::unique_ptr<Trk::TrackParameters> innerParameters, middleParameters, outerParameters;
        std::unique_ptr<Trk::TrackParameters> lastPars = lastIndetPars->uniqueClone();
        bool innerParsSet{false};

        std::vector<const Trk::TrackStateOnSurface*> stations;
        std::set<Muon::MuonStationIndex::StIndex> etaIndices, phiIndices, badEtaIndices, badPhiIndices;

        unsigned int nmeas = 0;

        for (const Trk::TrackStateOnSurface* tsosit : *spectrometerTrack.trackStateOnSurfaces()) {
            const Trk::MeasurementBase* meas = tsosit->measurementOnTrack();
            if (!meas) continue;

            if (tsosit->type(Trk::TrackStateOnSurface::Outlier)) continue;

            Identifier id = m_edmHelperSvc->getIdentifier(*meas);
            if (!id.is_valid()) continue;

            Muon::MuonStationIndex::StIndex index = m_idHelperSvc->stationIndex(id);
            bool measuresPhi = m_idHelperSvc->measuresPhi(id);
            ++nmeas;

            if (measuresPhi) {
                if (phiIndices.count(index)) continue;
                ATH_MSG_DEBUG("Adding phi station " << m_idHelperSvc->toString(id));
                phiIndices.insert(index);
            } else if (m_idHelperSvc->isMdt(id) || (m_idHelperSvc->isCsc(id))) {
                if (etaIndices.count(index)) continue;

                ATH_MSG_DEBUG("Adding eta station " << m_idHelperSvc->toString(id));
                etaIndices.insert(index);
            } else {
                continue;
            }

            std::unique_ptr<Trk::TrackParameters> exPars{};
            if (lastPars->associatedSurface() == meas->associatedSurface()) {
                ATH_MSG_DEBUG("Using existing pars");
                exPars = std::move(lastPars);
            } else {
                exPars = m_extrapolator->extrapolate(ctx, *lastPars, meas->associatedSurface(), Trk::alongMomentum, false, Trk::muon);
            }

            if (!exPars) {
                ATH_MSG_DEBUG("Failed to extrapolate to station" << m_idHelperSvc->toStringChamber(id));
                continue;
            }

            std::optional<Trk::ResidualPull> res {m_residualCalculator->residualPull(meas, exPars.get(), Trk::ResidualPull::Unbiased)};

            ATH_MSG_DEBUG(" " << m_idHelperSvc->toStringChamber(id) << "  residual " << m_printer->print(*res));

            if (std::abs(res->pull().front()) > m_pullCut) {
                if (measuresPhi) {
                    badPhiIndices.insert(index);
                } else {
                    badEtaIndices.insert(index);
                }
            }


            if (msgLvl(MSG::DEBUG)) {
                if (!m_idHelperSvc->measuresPhi(id)) {
                    const MuonGM::MuonReadoutElement* detEl = nullptr;
                    if (m_idHelperSvc->isMdt(id)) {
                        const Muon::MdtDriftCircleOnTrack* mdt = dynamic_cast<const Muon::MdtDriftCircleOnTrack*>(meas);
                        if (mdt) { detEl = mdt->detectorElement(); }
                    } else if (m_idHelperSvc->isCsc(id)) {
                        const Muon::CscClusterOnTrack* csc = dynamic_cast<const Muon::CscClusterOnTrack*>(meas);
                        if (csc) { detEl = csc->detectorElement(); }
                    }

                    if (detEl) {
                        const Trk::PlaneSurface* detSurf = dynamic_cast<const Trk::PlaneSurface*>(&detEl->surface());
                        if (detSurf) {
                            Trk::LocalDirection idDir{};
                            detSurf->globalToLocalDirection(exPars->momentum(), idDir);

                            const Trk::TrackParameters* pars = tsosit->trackParameters();
                            Trk::LocalDirection msDir{};
                            detSurf->globalToLocalDirection(pars->momentum(), msDir);
                            ATH_MSG_DEBUG(" local Angles: id (" << idDir.angleXZ() << "," << idDir.angleYZ() << ")  ms ("
                                                                << msDir.angleXZ() << "," << msDir.angleYZ() << ")");
                        }
                    }
                }
            }  // end DEBUG toggle

            if (!innerParsSet && !innerParameters && exPars && lastPars) {
               innerParameters = std::move(exPars);
            } else if (exPars && innerParameters && !middleParameters ) {
                middleParameters = std::move(exPars);
            }
            lastPars = std::move(exPars);
            innerParsSet = true;
        }

        if (middleParameters) {
            outerParameters = std::move(lastPars);
        } else {
            middleParameters = std::move(innerParameters);
            if (!middleParameters) {
                ATH_MSG_DEBUG("parameter extrapolation failed");
                return nullptr;
            }
        }

        bool cleanEta = badEtaIndices.size() == 1 && etaIndices.size() > 1;
        bool cleanPhi = badPhiIndices.size() == 1;

        if (!cleanPhi && !cleanEta) {
            ATH_MSG_DEBUG("No layers removed");
            return nullptr;
        }

        if (badEtaIndices.size() == etaIndices.size()) {
            ATH_MSG_DEBUG("All layers removed");
            return nullptr;
        }

        Trk::MeasurementSet spectrometerMeasurements;
        for (const Trk::TrackStateOnSurface*  tsosit : *spectrometerTrack.trackStateOnSurfaces()) {
            const Trk::MeasurementBase* meas = tsosit->measurementOnTrack();
            if (!meas) continue;
            if (tsosit->type(Trk::TrackStateOnSurface::Outlier)) continue;

            Identifier id = m_edmHelperSvc->getIdentifier(*meas);
            if (!id.is_valid()) continue;

            Muon::MuonStationIndex::StIndex index = m_idHelperSvc->stationIndex(id);
            bool measuresPhi = m_idHelperSvc->measuresPhi(id);
            if (cleanEta && !measuresPhi && badEtaIndices.count(index)) continue;
            if (cleanPhi && measuresPhi && badPhiIndices.count(index)) continue;
            spectrometerMeasurements.push_back(meas);
        }

        ATH_MSG_DEBUG("Number of measurements before cleaning " << nmeas << " after cleaning " << spectrometerMeasurements.size());

        if (spectrometerMeasurements.size() < 6) {
            ATH_MSG_DEBUG("Too few hits left - discarding fit");
            return nullptr;
        }

        // fit the combined track
        std::unique_ptr<Trk::Track> combinedTrack;
        if (!m_trackBuilder.empty()) {
            combinedTrack = m_trackBuilder->indetExtension(ctx, indetTrack, spectrometerMeasurements, std::move(innerParameters), std::move(middleParameters),
                                                           std::move(outerParameters));
        }
        if (combinedTrack) {
            ++m_recoverySuccess;
            combinedTrack->info().setPatternRecognitionInfo(Trk::TrackInfo::MuidMuonRecoveryTool);

            ATH_MSG_DEBUG("Recovered track " << std::endl
                                             << m_printer->print(*combinedTrack) << std::endl
                                             << m_printer->printStations(*combinedTrack));
        } else {
            ++m_recoveryFitFailure;
            ATH_MSG_DEBUG("track fit failure ");
        }
        return combinedTrack;
    }

}  // namespace Rec
