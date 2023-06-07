/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MooTrackFitter.h"

#include "EventPrimitives/EventPrimitivesHelpers.h"
#include "GeoPrimitives/GeoPrimitivesHelpers.h"
#include "GeoPrimitives/GeoPrimitivesToStringConverter.h"
#include "MuPatPrimitives/MuPatCandidateBase.h"
#include "MuPatPrimitives/MuPatSegment.h"
#include "MuPatPrimitives/MuPatTrack.h"
#include "MuPatPrimitives/SortMuPatHits.h"
#include "MuonRIO_OnTrack/MdtDriftCircleOnTrack.h"
#include "MuonReadoutGeometry/CscReadoutElement.h"
#include "MuonReadoutGeometry/MMReadoutElement.h"
#include "MuonReadoutGeometry/MdtReadoutElement.h"
#include "MuonReadoutGeometry/RpcReadoutElement.h"
#include "MuonReadoutGeometry/TgcReadoutElement.h"
#include "MuonReadoutGeometry/sTgcReadoutElement.h"
#include "MuonSegment/MuonSegment.h"
#include "MuonTrackMakerUtils/MuonTSOSHelper.h"
#include "MuonTrackMakerUtils/MuonTrackMakerStlTools.h"
#include "MuonTrackMakerUtils/SortMeasurementsByPosition.h"
#include "TrkDetDescrUtils/Intersection.h"
#include "TrkDriftCircleMath/Line.h"
#include "TrkDriftCircleMath/LocVec2D.h"
#include "TrkDriftCircleMath/MatchDCWithLine.h"
#include "TrkDriftCircleMath/Segment.h"
#include "TrkEventPrimitives/LocalDirection.h"
#include "TrkExUtils/IntersectionSolution.h"
#include "TrkExUtils/TrackSurfaceIntersection.h"
#include "TrkGeometry/Layer.h"
#include "TrkGeometry/MagneticFieldProperties.h"
#include "TrkGeometry/MaterialProperties.h"
#include "TrkMeasurementBase/MeasurementBase.h"
#include "TrkPseudoMeasurementOnTrack/PseudoMeasurementOnTrack.h"
#include "TrkSurfaces/StraightLineSurface.h"
#include "TrkTrack/Track.h"
#include "TrkTrack/TrackInfo.h"
#include "TrkTrackSummary/MuonTrackSummary.h"
#include "TrkTrackSummary/TrackSummary.h"

namespace {
    const double FiftyOverSqrt12 =  50. / std::sqrt(12);

    struct Unowned{
        void operator()(const Muon::MuonSegment*) const {};
    };
}
namespace Muon {

    MooTrackFitter::MooTrackFitter(const std::string& t, const std::string& n, const IInterface* p) : AthAlgTool(t, n, p) {
        declareInterface<MooTrackFitter>(this);
    }

    StatusCode MooTrackFitter::initialize() {
        ATH_CHECK(m_propagator.retrieve());
        ATH_CHECK(m_trackSummaryTool.retrieve());
        ATH_CHECK(m_cleaner.retrieve());
        ATH_CHECK(m_overlapResolver.retrieve());
        ATH_CHECK(m_trackFitter.retrieve());
        ATH_CHECK(m_momentumEstimator.retrieve());
        ATH_CHECK(m_idHelperSvc.retrieve());
        ATH_CHECK(m_edmHelperSvc.retrieve());
        ATH_CHECK(m_hitHandler.retrieve());
        ATH_CHECK(m_printer.retrieve());

        // Configuration of the material effects
        m_ParticleHypothesis = Trk::ParticleSwitcher::particle[m_matEffects];
        m_magFieldProperties = m_slProp ? Trk::NoField : Trk::FullField;

        ATH_CHECK(m_trackToSegmentTool.retrieve());
        ATH_CHECK(m_phiHitSelector.retrieve());

        return StatusCode::SUCCESS;
    }

    StatusCode MooTrackFitter::finalize() {
        double nfits = m_nfits.load() > 0 ? m_nfits.load() : 1.;
        double nfailedExtractInital = m_nfailedExtractInital / nfits;
        double nfailedMinMaxPhi = m_nfailedMinMaxPhi / nfits;
        double nfailedParsInital = m_nfailedParsInital / nfits;
        double nfailedExtractCleaning = m_nfailedExtractCleaning / nfits;
        double nfailedFakeInitial = m_nfailedFakeInitial / nfits;
        double nfailedTubeFit = m_nfailedTubeFit / nfits;
        double noPerigee = m_noPerigee / nfits;
        double nlowMomentum = m_nlowMomentum / nfits;
        double nfailedExtractPrecise = m_nfailedExtractPrecise / nfits;
        double nfailedFakePrecise = m_nfailedFakePrecise / nfits;
        double nfailedFitPrecise = m_nfailedFitPrecise / nfits;
        double nsuccess = m_nsuccess / nfits;
        ATH_MSG_INFO(" Summarizing statistics: nfits "
                     << m_nfits << std::endl
                     << "|  extract  | phi range | startPars | clean phi |  add fake |    fit    | no perigee |  low mom  |  extract  |  "
                        "add fake | final fit |   passed  |"
                     << std::endl
                     << std::setprecision(2) << std::setw(12) << nfailedExtractInital << std::setw(12) << nfailedMinMaxPhi << std::setw(12)
                     << nfailedParsInital << std::setw(12) << nfailedExtractCleaning << std::setw(12) << nfailedFakeInitial << std::setw(12)
                     << nfailedTubeFit << std::setw(13) << noPerigee << std::setw(12) << nlowMomentum << std::setw(12)
                     << nfailedExtractPrecise << std::setw(12) << nfailedFakePrecise << std::setw(12) << nfailedFitPrecise << std::setw(12)
                     << nsuccess);

        return StatusCode::SUCCESS;
    }

    bool MooTrackFitter::corruptEntry(const MuPatCandidateBase& entry) const {
        if (entry.hits().size() < entry.etaHits().size()) {
            ATH_MSG_WARNING(" Corrupt Entry: more eta hits than hits! ");
            return true;
        }
        if (entry.etaHits().size() < 3) { return (entry.ncscHitsEta < 2); }
        return false;
    }

    std::unique_ptr<Trk::Track> MooTrackFitter::refit(const EventContext& ctx, const MuPatTrack& trkCan) const {
       
        // internal representation of the track in fitter
        FitterData fitterData;

        // copy hit list
        fitterData.hitList = trkCan.hitList();

        // extract hits from hit list and add them to fitterData
        if (!extractData(fitterData, true)) return nullptr;

        // create start parameters
        const Trk::Perigee* pp = trkCan.track().perigeeParameters();
        if (!pp) return nullptr;

        // fit track
        std::unique_ptr<Trk::Track> track = fit(ctx, *pp, fitterData.measurements,  Trk::muon, false);

        if (track) {
            // clean and evaluate track
            std::set<Identifier> excludedChambers;
            std::unique_ptr<Trk::Track> cleanTrack = cleanAndEvaluateTrack(ctx, *track, excludedChambers);
            if (cleanTrack && !(*cleanTrack->perigeeParameters() == *track->perigeeParameters())) track.swap(cleanTrack);
        } else {
            ATH_MSG_DEBUG(" Fit failed ");
        }
        return track;
    }

    std::unique_ptr<Trk::Track> MooTrackFitter::refit(const EventContext& ctx, const Trk::Track& track) const {
        if (msgLvl(MSG::DEBUG)) {
            const Trk::TrackStates* states = track.trackStateOnSurfaces();
            int nStates = 0;
            if (states) nStates = states->size();
            msg(MSG::DEBUG) << MSG::DEBUG << "refit: fitting track with hits: " << nStates;
            if (msgLvl(MSG::VERBOSE)) { msg(MSG::VERBOSE) << std::endl << m_printer->printMeasurements(track); }
            msg(MSG::DEBUG) << endmsg;
        }
        // fit track
        std::unique_ptr<Trk::Track> newTrack = m_trackFitter->fit(ctx,track, false, m_ParticleHypothesis);

        if (newTrack) {
            // clean and evaluate track
            std::set<Identifier> excludedChambers;
            std::unique_ptr<Trk::Track> cleanTrack = cleanAndEvaluateTrack(ctx, *newTrack, excludedChambers);

            if (cleanTrack) {
                // check whether cleaner returned same track, if not delete old track
                if (!(*cleanTrack->perigeeParameters() == *newTrack->perigeeParameters())) newTrack.swap(cleanTrack);
            } else {
                ATH_MSG_DEBUG(" Refit failed, rejected by cleaner ");
                newTrack.reset();
            }
        } else {
            ATH_MSG_DEBUG(" Refit failed ");
        }

        return newTrack;
    }

    std::unique_ptr<Trk::Track> MooTrackFitter::fit(const EventContext& ctx, const MuPatCandidateBase& entry1, const MuPatCandidateBase& entry2,
                                                    const PrepVec& externalPhiHits) const {
        ++m_nfits;
        // internal representation of the track in fitter
        FitterData fitterData;

        // extract hits and geometrical information
        if (!extractData(entry1, entry2, fitterData)) {
            ATH_MSG_DEBUG(" Failed to extract data for initial fit");
            return nullptr;
        }
        ++m_nfailedExtractInital;

        // get the minimum and maximum phi compatible with all eta hits, don't use for cosmics
        if (!m_cosmics && !getMinMaxPhi(fitterData)) {
            ATH_MSG_DEBUG(" Phi range check failed, candidate stations not pointing. Will not fit track");
            return nullptr;
        }

        ++m_nfailedMinMaxPhi;

        // create start parameters
        createStartParameters(ctx, fitterData);
        std::unique_ptr<Trk::Perigee>& startPars = fitterData.startPars;
        if (!startPars) {
            ATH_MSG_DEBUG(" Creation of start parameters failed ");
            return nullptr;
        }
        ATH_MSG_VERBOSE("Extracted start parameters: "<<Amg::toString(startPars->momentum().unit())<< " "<<Amg::toString(startPars->position()));
        ++m_nfailedParsInital;

        // clean phi hits and reevaluate hits. Do not run for cosmics
        bool hasCleaned = m_cleanPhiHits && cleanPhiHits(ctx, startPars->momentum().mag(), fitterData, externalPhiHits);
        if (hasCleaned) {
            ATH_MSG_DEBUG(" Cleaned phi hits, re-extracting hits");
            bool usePrecise = m_usePreciseHits || (fitterData.firstHasMomentum || fitterData.secondHasMomentum);
            ATH_MSG_VERBOSE("Call extract data with usePrecise "<<usePrecise);
            if (!extractData(fitterData, usePrecise)) {
                ATH_MSG_DEBUG(" Failed to extract data after phi hit cleaning");
                return nullptr;
            }
        }
        ++m_nfailedExtractCleaning;

        // check whether there are enough phi constraints, if not add fake phi hits
        if (!addFakePhiHits(ctx, fitterData, *startPars)) {
            ATH_MSG_DEBUG(" Failed to add fake phi hits for precise fit");
            return nullptr;
        }
        ++m_nfailedFakeInitial;

        // fit track with broad errors, no material
        bool doPreFit = m_usePrefit;
        Trk::ParticleHypothesis particleType = Trk::ParticleSwitcher::particle[0];
        if (fitterData.firstHasMomentum || fitterData.secondHasMomentum) doPreFit = false;
        if (!doPreFit) particleType = Trk::muon;

        if (m_cosmics) {
            Muon::CosmicMuPatHitSorter sorter{*startPars};
            std::stable_sort(fitterData.measurements.begin(),fitterData.measurements.end(),sorter);
        }
        std::unique_ptr<Trk::Track> track = fit(ctx, *startPars, fitterData.measurements, particleType, doPreFit);

        if (!track) {
            ATH_MSG_DEBUG(" Fit failed ");
            return nullptr;
        }
        ++m_nfailedTubeFit;

        // create start parameters
        const Trk::Perigee* pp = track->perigeeParameters();
        if (!pp) {
            ATH_MSG_DEBUG(" Track without perigee parameters, exit ");
            return nullptr;
        }
        ++m_noPerigee;

        if (!m_slFit && !validMomentum(*pp)) {
            ATH_MSG_VERBOSE(" Low momentum, rejected ");
            return nullptr;
        }
        ++m_nlowMomentum;

        if (!fitterData.firstHasMomentum && !fitterData.secondHasMomentum && doPreFit) {
            ATH_MSG_DEBUG(" Performing second fit ");

            // refit with precise errors
            FitterData fitterDataRefit{};
            fitterDataRefit.startPars.reset(pp->clone());
            fitterDataRefit.firstIsTrack = fitterData.firstIsTrack;
            fitterDataRefit.secondIsTrack = fitterData.secondIsTrack;
            fitterDataRefit.firstHasMomentum = fitterData.firstHasMomentum;
            fitterDataRefit.secondHasMomentum = fitterData.secondHasMomentum;
            fitterDataRefit.avePhi = fitterData.avePhi;
            fitterDataRefit.phiMin = fitterData.phiMin;
            fitterDataRefit.phiMax = fitterData.phiMax;
            fitterDataRefit.firstEntry = fitterData.firstEntry;
            fitterDataRefit.secondEntry = fitterData.secondEntry;
            fitterDataRefit.hitList = fitterData.hitList;

            // extract hits from hit list and add them to fitterData
            if (!extractData(fitterDataRefit, true)) {
                ATH_MSG_DEBUG(" Failed to extract data for precise fit");
                return nullptr;
            }
            ++m_nfailedExtractPrecise;

            // check whether there are enough phi constraints, if not add fake phi hits
            if (!addFakePhiHits(ctx, fitterDataRefit, *startPars)) {
                ATH_MSG_DEBUG(" Failed to add fake phi hits for precise fit");
                return nullptr;
            }
            ++m_nfailedFakePrecise;

            // fit track
            std::unique_ptr<Trk::Track> newTrack = fit(ctx, *pp, fitterDataRefit.measurements,  Trk::muon, false);
            if (newTrack)
                track.swap(newTrack);
            else if (!m_allowFirstFit) {
                ATH_MSG_DEBUG(" Precise fit failed ");
                return nullptr;
            } else {
                ATH_MSG_DEBUG(" Precise fit failed, keep fit with broad errors");
            }
            ++m_nfailedFitPrecise;

            fitterData.garbage.insert(fitterData.garbage.end(), std::make_move_iterator(fitterDataRefit.garbage.begin()),
                                                                std::make_move_iterator(fitterDataRefit.garbage.end()));
            fitterDataRefit.garbage.clear();
        }

        if (track) {
            // clean and evaluate track
            std::set<Identifier> excludedChambers;
            if (fitterData.firstIsTrack && !fitterData.secondIsTrack) {
                excludedChambers = fitterData.secondEntry->chamberIds();
                ATH_MSG_VERBOSE(" Using exclusion list of second entry for cleaning");
            } else if (!fitterData.firstIsTrack && fitterData.secondIsTrack) {
                excludedChambers = fitterData.firstEntry->chamberIds();
                ATH_MSG_VERBOSE(" Using exclusion list of first entry for cleaning");
            }
            if (!excludedChambers.empty()) { ATH_MSG_DEBUG(" Using exclusion list for cleaning"); }
            std::unique_ptr<Trk::Track> cleanTrack = cleanAndEvaluateTrack(ctx, *track, excludedChambers);
            if (cleanTrack) {
                if (!(*cleanTrack->perigeeParameters() == *track->perigeeParameters())) track.swap(cleanTrack);
            } else
                track.reset();
        }

        if (track) ++m_nsuccess;

        if (msgLvl(MSG::DEBUG) && track) msg(MSG::DEBUG) << MSG::DEBUG << " Track found " << endmsg;
        return track;
    }

    bool MooTrackFitter::extractData(const MuPatCandidateBase& entry1, const MuPatCandidateBase& entry2,
                                     MooTrackFitter::FitterData& fitterData) const {
        // sanity checks on the entries
        if (corruptEntry(entry1)) {
            ATH_MSG_DEBUG(" corrupt first entry,  cannot perform fit: eta hits " << entry1.etaHits().size());
            return false;
        }
        if (corruptEntry(entry2)) {
            ATH_MSG_DEBUG(" corrupt second entry, cannot perform fit: eta hits " << entry2.etaHits().size());
            return false;
        }
        // are we in the endcap region?
        bool isEndcap = entry1.hasEndcap() || entry2.hasEndcap();

        // measurement sorting function
        SortMeasurementsByPosition sortMeasurements(isEndcap);

        bool entry1IsFirst = sortMeasurements(entry1.hits().front(), entry2.hits().front());
        if (m_cosmics) {
            DistanceToPars distToPars(&entry1.entryPars());
            double distToSecond = distToPars(entry2.entryPars().position());
            if (distToSecond < 0) entry1IsFirst = false;
            ATH_MSG_DEBUG(" first entry dir " << Amg::toString(entry1.entryPars().momentum()) 
                                               << " pos " << Amg::toString(entry1.entryPars().position()) << " second "
                                               <<  Amg::toString(entry2.entryPars().position()) << "  dist " << distToSecond);
        }
        const MuPatCandidateBase& firstEntry = entry1IsFirst ? entry1 : entry2;
        const MuPatCandidateBase& secondEntry = entry1IsFirst ? entry2 : entry1;

        fitterData.firstEntry = &firstEntry;
        fitterData.secondEntry = &secondEntry;

        // check whether we are dealing with a track or a segment
        if (dynamic_cast<const MuPatTrack*>(fitterData.firstEntry)) {
            fitterData.firstIsTrack = true;
            fitterData.firstHasMomentum = fitterData.firstEntry->hasMomentum();
        }
        if (dynamic_cast<const MuPatTrack*>(fitterData.secondEntry)) {
            fitterData.secondIsTrack = true;
            fitterData.secondHasMomentum = fitterData.secondEntry->hasMomentum();
        }
        // merge hitLists and add them to the fitterData
        fitterData.hitList = m_hitHandler->merge(entry1.hitList(), entry2.hitList());

        bool usePrecise = m_usePreciseHits || (fitterData.firstHasMomentum || fitterData.secondHasMomentum);
        if (msgLvl(MSG::DEBUG)) {
            msg(MSG::DEBUG) << MSG::DEBUG << " entering fitter: etaHits, first entry: ";
            if (fitterData.firstIsTrack)
                msg(MSG::DEBUG) << " track ";
            else
                msg(MSG::DEBUG) << " segment ";
            msg(MSG::DEBUG) << fitterData.firstEntry->etaHits().size() << std::endl
                            << m_hitHandler->print(fitterData.firstEntry->hitList()) << std::endl
                            << "  second entry: ";
            if (fitterData.secondIsTrack)
                msg(MSG::DEBUG) << " track ";
            else
                msg(MSG::DEBUG) << " segment ";
            msg(MSG::DEBUG) << fitterData.secondEntry->etaHits().size() << std::endl
                            << m_hitHandler->print(fitterData.secondEntry->hitList()) << endmsg;
        }

        if (msgLvl(MSG::DEBUG)) {
            msg(MSG::DEBUG) << MSG::DEBUG << " merged hit lists, new list size:  " << fitterData.hitList.size();
            if (usePrecise) msg(MSG::DEBUG) << " using precise errors" << endmsg;
            if (msgLvl(MSG::VERBOSE)) msg(MSG::DEBUG) << std::endl << m_hitHandler->print(fitterData.hitList);
            msg(MSG::DEBUG) << endmsg;
        }

        return extractData(fitterData, usePrecise);
    }

    bool MooTrackFitter::extractData(MooTrackFitter::FitterData& fitterData, bool usePreciseHits) const {
        ATH_MSG_DEBUG(" extracting hits from hit list, using " << (usePreciseHits ? "precise measurements" : "broad measurements"));

        MuPatHitList& hitList = fitterData.hitList;
        // make sure the vector is sufficiently large
        unsigned int nhits = hitList.size();

        fitterData.measurements.clear();
        fitterData.firstLastMeasurements.clear();
        fitterData.etaHits.clear();
        fitterData.phiHits.clear();
        fitterData.measurements.reserve(nhits);
        fitterData.firstLastMeasurements.reserve(nhits);

        if (usePreciseHits && fitterData.startPars) removeSegmentOutliers(fitterData);

        MuonStationIndex::StIndex firstStation = MuonStationIndex::StUnknown;
        MuonStationIndex::ChIndex currentChIndex = MuonStationIndex::ChUnknown;
        bool currentMeasPhi = false;
        MuPatHitPtr previousHit{nullptr};

        // loop over hit list
        for (const MuPatHitPtr& hit : hitList) {
            const Identifier& id = hit->info().id;
            if (hit->info().status != MuPatHit::OnTrack || !id.is_valid()) {
                ATH_MSG_VERBOSE("Discard outlier "<<m_idHelperSvc->toString(id));
                continue;
            }
            // in theory, there are only MuPatHit objects for MS hits
            if (!m_idHelperSvc->isMuon(id)) {
                ATH_MSG_WARNING("given Identifier " << id.get_compact() << " (" << m_idHelperSvc->toString(id)
                                                    << ") is not a muon Identifier, continuing");
                continue;
            }

            MuonStationIndex::ChIndex chIndex = m_idHelperSvc->chamberIndex(id);
            MuonStationIndex::StIndex stIndex = MuonStationIndex::toStationIndex(chIndex);
            fitterData.stations.insert(stIndex);

            if (firstStation == MuonStationIndex::StUnknown) {
                firstStation = stIndex;
            }

            const bool measuresPhi = hit->info().measuresPhi;

            if (!m_idHelperSvc->isTgc(id) && !measuresPhi) {
                const bool isSmall = m_idHelperSvc->isSmallChamber(id);
                SmallLargeChambers& stCount = fitterData.smallLargeChambersPerStation[stIndex];
                stCount.first += isSmall;
                stCount.second += !isSmall;
            }

            bool isEndcap = m_idHelperSvc->isEndcap(id);
            fitterData.hasEndcap |= isEndcap;
            fitterData.hasBarrel |= !isEndcap;

            const Trk::MeasurementBase* meas = usePreciseHits ? &hit->preciseMeasurement() : &hit->broadMeasurement();

            // special treatment of hits in first stations on the track to stabalise the track fit
            if (!usePreciseHits && m_preciseFirstStation) { meas = &hit->preciseMeasurement(); }

            if (measuresPhi)
                fitterData.phiHits.push_back(meas);
            else
                fitterData.etaHits.push_back(meas);

            if (msgLvl(MSG::DEBUG)) {
                double rDrift = meas->localParameters()[Trk::locR];
                double rError = Amg::error(meas->localCovariance(), Trk::locR);
                if (usePreciseHits && m_idHelperSvc->isMdt(id) && std::abs(rDrift) < 0.01 && rError > 4.) {
                    ATH_MSG_WARNING(" MDT hit error broad but expected precise error ");
                }
            }

            fitterData.measurements.push_back(meas);

            if (!measuresPhi && m_idHelperSvc->isTrigger(id)) continue;

            if (!previousHit) {
                currentChIndex = chIndex;
                currentMeasPhi = measuresPhi;
                fitterData.firstLastMeasurements.push_back(&hit->broadMeasurement());
            } else if (currentChIndex == chIndex && currentMeasPhi == measuresPhi) {
                previousHit = hit;
            } else {
                  currentChIndex = chIndex;
                  currentMeasPhi = measuresPhi;
                  previousHit = hit;
                  fitterData.firstLastMeasurements.push_back(&hit->broadMeasurement());
            }            
        }

        // add last hit if not already inserted
        if (previousHit && &previousHit->broadMeasurement() != fitterData.firstLastMeasurements.back())
            fitterData.firstLastMeasurements.push_back(&previousHit->broadMeasurement());

        // require at least 6 measurements on a track
        if (fitterData.measurements.size() < 7) {
            ATH_MSG_VERBOSE(" Too few measurements, cannot perform fit  " << fitterData.measurements.size());
            return false;
        }

        // require at least 6 measurements on a track
        if (fitterData.etaHits.size() < 7) {
            ATH_MSG_VERBOSE(" Too few eta measurements, cannot perform fit  " << fitterData.etaHits.size());
            return false;
        }

        ATH_MSG_VERBOSE(" Extracted measurements: total " << fitterData.measurements.size() << "  eta "
                     << fitterData.etaHits.size() << "  phi " << fitterData.phiHits.size()<< std::endl << m_printer->print(fitterData.measurements));
        return true;
    }

    bool MooTrackFitter::addFakePhiHits(const EventContext& ctx, MooTrackFitter::FitterData& fitterData, const Trk::TrackParameters& startpar) const {

        // check whether we have enough phi constraints
        unsigned nphiConstraints = hasPhiConstrain(fitterData);

        // do we have enough constraints to fit the track
        if (nphiConstraints >= 2) {
            if (fitterData.firstEntry->stations().size() == 1 && fitterData.firstEntry->containsStation(MuonStationIndex::EI) &&
                (fitterData.firstEntry->phiHits().empty() || (fitterData.firstEntry->containsChamber(MuonStationIndex::CSS) ||
                                                              fitterData.firstEntry->containsChamber(MuonStationIndex::CSL)))) {
                ATH_MSG_VERBOSE( " Special treatment of the forward region: adding fake at ip ");
            }
            return true;
        }

        // in case of a single station overlap fit, calculate a global position that is more or less located at
        // the overlap. It is used to decide on which side of the tube the fake should be produced
        std::unique_ptr<Amg::Vector3D> overlapPos;
         std::unique_ptr<const Amg::Vector3D> phiPos;
        if (fitterData.numberOfSLOverlaps() > 0 || (fitterData.numberOfSmallChambers() > 0 && fitterData.numberOfLargeChambers() > 0)) {
            // in case of SL overlaps, pass average position of the two segments
            //       overlapPos = new Amg::Vector3D( 0.5*(fitterData.firstEntry->etaHits().front()->globalPosition() +
            //                                               fitterData.secondEntry->etaHits().front()->globalPosition()) );
            overlapPos = std::make_unique<Amg::Vector3D>(1., 1., 1.);
            double phi = fitterData.avePhi;
            double theta = fitterData.secondEntry->etaHits().front()->globalPosition().theta();
            if (m_cosmics) {
                if (fitterData.firstEntry->hasSLOverlap()) {
                    theta = fitterData.firstEntry->entryPars().momentum().theta();
                    phi = fitterData.firstEntry->entryPars().momentum().phi();
                } else {
                    theta = fitterData.secondEntry->entryPars().momentum().theta();
                    phi = fitterData.secondEntry->entryPars().momentum().phi();
                }
            }
            Amg::setThetaPhi(*overlapPos, theta, phi);

            if (fitterData.startPars) {
                phiPos = std::make_unique<Amg::Vector3D>(fitterData.startPars->momentum());
            } else {
                phiPos = std::make_unique<Amg::Vector3D>(*overlapPos);
            }

            if (msgLvl(MSG::VERBOSE)) {
                if (fitterData.numberOfSLOverlaps() > 0) {
                    if (fitterData.stations.size() == 1)
                        msg(MSG::VERBOSE) << " one station fit with SL overlap, using overlapPos " << *overlapPos << endmsg;
                    else
                        msg(MSG::VERBOSE) << " multi station fit with SL overlap, using overlapPos " << *overlapPos << endmsg;
                } else if (fitterData.numberOfSmallChambers() > 0 && fitterData.numberOfLargeChambers() > 0) {
                    msg(MSG::VERBOSE) << " multi station fit, SL overlap not in same station, using overlapPos " << *overlapPos << endmsg;
                } else
                    ATH_MSG_WARNING(" Unknown overlap type ");
            }
        }
        // add fake phi hit on first and last measurement
        if (fitterData.phiHits.empty()) {
            const MuPatSegment* segInfo1 = dynamic_cast<const MuPatSegment*>(fitterData.firstEntry);
            const MuPatSegment* segInfo2 = dynamic_cast<const MuPatSegment*>(fitterData.secondEntry);
            if (segInfo1 && segInfo2 && fitterData.stations.size() == 1 && fitterData.numberOfSLOverlaps() == 1) {              
                // only perform SL overlap fit for MDT segments
                Identifier chId = m_edmHelperSvc->chamberId(*segInfo1->segment);
                if (!m_idHelperSvc->isMdt(chId)) return false;

                ATH_MSG_VERBOSE(" Special treatment for tracks with one station, a SL overlap and no phi hits ");

                IMuonSegmentInOverlapResolvingTool::SegmentMatchResult result =
                    m_overlapResolver->matchResult(ctx, *segInfo1->segment, *segInfo2->segment);

                if (!result.goodMatch()) {
                    ATH_MSG_VERBOSE(" Match failed ");
                    return false;
                }
                // create a track parameter for the segment
                double locx1 = result.segmentResult1.positionInTube1;
                double locy1 =
                    segInfo1->segment->localParameters().contains(Trk::locY) ? segInfo1->segment->localParameters()[Trk::locY] : 0.;
                Trk::AtaPlane segPars1(locx1, locy1, result.phiResult.segmentDirection1.phi(), result.phiResult.segmentDirection1.theta(),
                                       0., segInfo1->segment->associatedSurface());
                // ownership retained, original code deleted exPars1
                std::unique_ptr<Trk::TrackParameters> exPars1 = m_propagator->propagate(ctx,
                                                       segPars1, fitterData.measurements.front()->associatedSurface(), Trk::anyDirection,
                                                       false, m_magFieldProperties);
                if (exPars1) {
                    Amg::Vector3D position = exPars1->position();
                    std::unique_ptr<Trk::MeasurementBase> fake =
                        createFakePhiForMeasurement(*fitterData.measurements.front(), &position, nullptr, 10.);
                    if (fake) {
                        fitterData.phiHits.push_back(fake.get());
                        fitterData.measurements.insert(fitterData.measurements.begin(), fake.get());
                        fitterData.firstLastMeasurements.insert(fitterData.firstLastMeasurements.begin(), fake.get());
                        fitterData.garbage.push_back(std::move(fake));
                    }
                } else {
                    ATH_MSG_WARNING(" failed to create fake for first segment ");
                    return false;
                }
                double locx2 = result.segmentResult2.positionInTube1;
                double locy2 =
                    segInfo2->segment->localParameters().contains(Trk::locY) ? segInfo2->segment->localParameters()[Trk::locY] : 0.;
                Trk::AtaPlane segPars2(locx2, locy2, result.phiResult.segmentDirection2.phi(), result.phiResult.segmentDirection2.theta(),
                                       0., segInfo2->segment->associatedSurface());
                // ownership retained
                auto exPars2 = m_propagator->propagate(ctx,
                                                       segPars2, fitterData.measurements.back()->associatedSurface(), Trk::anyDirection,
                                                       false, m_magFieldProperties);
                if (exPars2) {
                    Amg::Vector3D position = exPars2->position();
                    std::unique_ptr<Trk::MeasurementBase> fake =
                        createFakePhiForMeasurement(*fitterData.measurements.back(), &position, nullptr, 10.);
                    if (fake) {
                        fitterData.phiHits.push_back(fake.get());
                        fitterData.measurements.push_back(fake.get());
                        fitterData.firstLastMeasurements.push_back(fake.get());
                        fitterData.garbage.push_back(std::move(fake));
                    }
                } else {
                    ATH_MSG_WARNING(" failed to create fake for second segment ");
                    return false;
                }
            } else if (nphiConstraints == 0 || (fitterData.stations.size() == 1) ||
                       (nphiConstraints == 1 && fitterData.numberOfSLOverlaps() == 0 && fitterData.numberOfSmallChambers() > 0 &&
                        fitterData.numberOfLargeChambers() > 0)) {
                {
                    std::unique_ptr<Trk::MeasurementBase> fake = createFakePhiForMeasurement(*fitterData.measurements.front(), 
                                                                                             overlapPos.get(), phiPos.get(), 100.);
                    if (fake) {
                        fitterData.phiHits.push_back(fake.get());
                        fitterData.measurements.insert(fitterData.measurements.begin(), fake.get());
                        fitterData.firstLastMeasurements.insert(fitterData.firstLastMeasurements.begin(), fake.get());
                        fitterData.garbage.push_back(std::move(fake));
                    }
                
                }
                {
                    std::unique_ptr<Trk::MeasurementBase> fake = createFakePhiForMeasurement(*fitterData.measurements.back(), 
                                                                                              overlapPos.get(), phiPos.get(), 100.);
                    if (fake) {
                        fitterData.phiHits.push_back(fake.get());
                        fitterData.measurements.push_back(fake.get());
                        fitterData.firstLastMeasurements.push_back(fake.get());
                        fitterData.garbage.push_back(std::move(fake));
                    }
                }
                
            } else if (fitterData.numberOfSLOverlaps() == 1) {
                // there is one overlap, add the fake in the station without the overlap
                ATH_MSG_VERBOSE(" Special treatment for tracks with one SL overlap and no phi hits ");

                MuonStationIndex::StIndex overlapStation = MuonStationIndex::StUnknown;
                SLStationMap::iterator it = fitterData.smallLargeChambersPerStation.begin();
                SLStationMap::iterator it_end = fitterData.smallLargeChambersPerStation.end();
                for (; it != it_end; ++it) {
                    if (it->second.first && it->second.second) {
                        overlapStation = it->first;
                        break;
                    }
                }
                if (overlapStation == MuonStationIndex::StUnknown) {
                    ATH_MSG_WARNING(" unexpected condition, unknown station type ");                
                    return false;
                }

                Identifier firstId = m_edmHelperSvc->getIdentifier(*fitterData.measurements.front());
                if (!firstId.is_valid()) {
                    ATH_MSG_WARNING(" unexpected condition, first measurement has no identifier ");                  
                    return false;
                }
                MuonStationIndex::StIndex firstSt = m_idHelperSvc->stationIndex(firstId);
                if (overlapStation == firstSt) {
                    ATH_MSG_VERBOSE(" Adding fake in same station as overlap ");

                    // create pseudo at end of track
                    std::unique_ptr<Trk::MeasurementBase> fake = createFakePhiForMeasurement(*fitterData.measurements.back(), 
                                                                                        overlapPos.get(), phiPos.get(), 100.);
                    if (fake) {
                        fitterData.phiHits.push_back(fake.get());
                        fitterData.measurements.push_back(fake.get());
                        fitterData.firstLastMeasurements.push_back(fake.get());
                        fitterData.garbage.push_back(std::move(fake));
                    }
                } else {
                    ATH_MSG_VERBOSE(" Adding fake in other station as overlap ");
                    // create pseudo at begin of track
                    std::unique_ptr<Trk::MeasurementBase> fake = createFakePhiForMeasurement(*fitterData.measurements.front(), 
                                                                                            overlapPos.get(), phiPos.get(), 100.);
                    if (fake) {
                        fitterData.phiHits.push_back(fake.get());
                        fitterData.measurements.insert(fitterData.measurements.begin(), fake.get());
                        fitterData.firstLastMeasurements.insert(fitterData.firstLastMeasurements.begin(), fake.get());
                        fitterData.garbage.push_back(std::move(fake));
                    }
                }

            } else {
                ATH_MSG_WARNING(" unexpected condition, cannot create fakes ");           
                return false;
            }
        } else {
            // calculate distance between first phi/eta hit and last phi/eta hit
            double distFirstEtaPhi =
                (fitterData.measurements.front()->globalPosition() - fitterData.phiHits.front()->globalPosition()).mag();
            double distLastEtaPhi = (fitterData.measurements.back()->globalPosition() - fitterData.phiHits.back()->globalPosition()).mag();

            // if there is one phi, use its phi + IP constraint to calculate position of fake
            if (!overlapPos && m_seedWithAvePhi) {
                phiPos = std::make_unique<Amg::Vector3D>(fitterData.phiHits.back()->globalPosition());
                ATH_MSG_VERBOSE(" using pointing constraint to calculate fake phi hit ");
            }

            // create a fake on the first and/or last MDT measurement if not near phi hits
            const Trk::Surface *firstmdtsurf = nullptr, *lastmdtsurf = nullptr;
            const Trk::TrackParameters *lastmdtpar = nullptr;
            int indexfirst = 0, indexlast = (int)fitterData.measurements.size();
            MuPatHitCit hitit = fitterData.hitList.begin();
            for (; hitit != fitterData.hitList.end(); hitit++) {              
                if ((**hitit).info().measuresPhi) break;
                if ((**hitit).info().type == MuPatHit::MDT || (**hitit).info().type == MuPatHit::sTGC) {
                    firstmdtsurf = &(**hitit).measurement().associatedSurface();
                    break;
                }
                indexfirst++;
            }
            hitit = fitterData.hitList.end();
            hitit--;
            for (; hitit != fitterData.hitList.begin(); hitit--) {              
                if ((**hitit).info().measuresPhi) break;
                if ((**hitit).info().type == MuPatHit::MDT) {
                    lastmdtsurf = &(**hitit).measurement().associatedSurface();
                    lastmdtpar = &(**hitit).parameters();
                    break;
                }
                indexlast--;
            }
            bool phifromextrapolation = false;
            if (!fitterData.secondEntry->phiHits().empty() || !fitterData.firstEntry->phiHits().empty()) phifromextrapolation = true;

            const Trk::Surface *firstphisurf = nullptr, *lastphisurf = nullptr;
            Amg::Vector3D firstphinormal{Amg::Vector3D::Zero()}, lastphinormal{Amg::Vector3D::Zero()};
            if (!fitterData.phiHits.empty()) {
                firstphisurf = &fitterData.phiHits.front()->associatedSurface();
                lastphisurf = &fitterData.phiHits.back()->associatedSurface();
                firstphinormal = firstphisurf->normal();
                lastphinormal = lastphisurf->normal();
                if (firstphinormal.dot(startpar.momentum()) < 0) firstphinormal = -firstphinormal;
                if (lastphinormal.dot(startpar.momentum()) < 0) lastphinormal = -lastphinormal;
            }
            if (lastmdtsurf && (!firstphisurf || (lastmdtsurf->center() - lastphisurf->center()).dot(lastphinormal) > 1000)) {
                ATH_MSG_VERBOSE(" Adding fake at last hit: dist first phi/first eta " << distFirstEtaPhi << " dist last phi/last eta "
                                                                                      << distLastEtaPhi);
                std::unique_ptr<Trk::MeasurementBase> fake{};
                if (fitterData.secondEntry->hasSLOverlap() || phifromextrapolation) {
                    // this scope manages lifetime of mdtpar
                    std::unique_ptr<Trk::TrackParameters> mdtpar{};
                    if (fitterData.secondEntry->hasSLOverlap()){
                        mdtpar = lastmdtpar->uniqueClone();
                    } else {
                        mdtpar = m_propagator->propagateParameters(ctx, startpar, *lastmdtsurf, 
                                                        Trk::alongMomentum, false, m_magFieldProperties);
                    }
                    if (mdtpar) {
                        Amg::MatrixX cov(1, 1);
                        cov(0, 0) = 100.;
                        fake = std::make_unique<Trk::PseudoMeasurementOnTrack>(
                            Trk::LocalParameters(Trk::DefinedParameter(mdtpar->parameters()[Trk::locY], Trk::locY)), cov,
                            mdtpar->associatedSurface());
                       
                    }
                } else {
                    fake = createFakePhiForMeasurement(*fitterData.measurements.back(), overlapPos.get(), phiPos.get(), 10.);                
                }                    
                if (fake) {
                    fitterData.phiHits.push_back(fake.get());
                    fitterData.measurements.insert(fitterData.measurements.begin() + indexlast, fake.get());
                    fitterData.firstLastMeasurements.push_back(fake.get());
                    fitterData.garbage.push_back(std::move(fake));
                }
            }

            if (firstmdtsurf && (!firstphisurf || (firstmdtsurf->center() - firstphisurf->center()).dot(firstphinormal) < -1000)) {
                ATH_MSG_VERBOSE(" Adding fake at first hit: dist first phi/first eta " << distFirstEtaPhi << " dist last phi/last eta "
                                                                                       << distLastEtaPhi);
                std::unique_ptr<Trk::MeasurementBase> fake{};
                if (phifromextrapolation) {
                    // lifetime of mdtpar managed here
                    auto mdtpar = m_propagator->propagateParameters(ctx, startpar, 
                                                *firstmdtsurf, Trk::oppositeMomentum, false, m_magFieldProperties);
                    if (mdtpar) {
                        Amg::MatrixX cov(1, 1);
                        cov(0, 0) = 100.;
                        fake = std::make_unique<Trk::PseudoMeasurementOnTrack>(
                            Trk::LocalParameters(Trk::DefinedParameter(mdtpar->parameters()[Trk::locY], Trk::locY)), cov,
                            mdtpar->associatedSurface());                        
                    }
                } else{
                    fake = createFakePhiForMeasurement(*fitterData.measurements.front(), overlapPos.get(), phiPos.get(), 100.);
                }
                if (fake) {
                    fitterData.phiHits.insert(fitterData.phiHits.begin(), fake.get());
                    fitterData.measurements.insert(fitterData.measurements.begin() + indexfirst, fake.get());
                    fitterData.firstLastMeasurements.insert(fitterData.firstLastMeasurements.begin(), fake.get());
                    fitterData.garbage.push_back(std::move(fake));
                }
            }           
        }             
        return true;
    }

    std::unique_ptr<Trk::MeasurementBase> MooTrackFitter::createFakePhiForMeasurement(const Trk::MeasurementBase& meas,
                                                                            const Amg::Vector3D* overlapPos, const Amg::Vector3D* phiPos,
                                                                            double errPos) const {
        // check whether measuring phi
        Identifier id = m_edmHelperSvc->getIdentifier(meas);
        if (id == Identifier()) {
            ATH_MSG_WARNING(" Cannot create fake phi hit from a measurement without Identifier ");
            return nullptr;
        }
        // check whether phi measurement, exit if true
        if (m_idHelperSvc->measuresPhi(id)) {
            ATH_MSG_WARNING(" Measurement is a phi measurement! " << m_idHelperSvc->toString(id));
            return nullptr;
        }

       
        // the MDT and CSC hits are ROTs
        const Trk::RIO_OnTrack* rot = dynamic_cast<const Trk::RIO_OnTrack*>(&meas);
        if (!rot) {
            const CompetingMuonClustersOnTrack* crot = dynamic_cast<const CompetingMuonClustersOnTrack*>(&meas);
            // if also to a crot we cannot create a fake phi hit
            if (!crot || crot->containedROTs().empty()) return nullptr;
            rot = &crot->rioOnTrack(0);
        }

        

        // get channel length and detector element
       const Trk::TrkDetElementBase* detEl = rot->detectorElement();      
        if (!detEl) {
            return nullptr;
        }
        
        auto [halfLengthL, halfLengthR] = getElementHalfLengths(id, detEl);
        
        // error
        std::optional<Amg::Vector2D> lpos = std::nullopt;
        if (phiPos) {
            Amg::Vector3D dir(1., 0., 0.);
            Amg::setThetaPhi(dir, rot->globalPosition().theta(), phiPos->phi());
            Amg::Vector3D ip{Amg::Vector3D::Zero()};
            Trk::Perigee perigee(ip, dir, 0., ip);

            // special treatment of MDTs, intersect with detector element surface instead of wire surface to avoid problem with
            // shift of phi angle after projection into wire frame
            const Trk::Surface& measSurf = m_idHelperSvc->isMdt(id) ? detEl->surface() : meas.associatedSurface();

            Trk::Intersection intersect = measSurf.straightLineIntersection(ip, dir);
            if (!intersect.valid) {
                ATH_MSG_WARNING(" Intersect with surface failed for measurement "
                                << m_printer->print(meas) << " position " << Amg::toString(ip) << " direction: phi " << dir.phi()
                                << " theta " << dir.theta() << " seed: phi " << phiPos->phi() << " theta "
                                << rot->globalPosition().theta());
            } else {
                // now get central phi position on surface of segment
                lpos = meas.associatedSurface().globalToLocal(intersect.position, 3000.);
                
            }
        } else {
            // now get central phi position on surface of segment            
            lpos = meas.associatedSurface().globalToLocal(detEl->center(), 3000.);
            if (!lpos){
                lpos = meas.associatedSurface().globalToLocal(meas.associatedSurface().center(), 3000.); 
            }
          
        }

        if (!lpos) {
            ATH_MSG_WARNING(" No local position, cannot make fake phi hit "<<m_idHelperSvc->toString(id)<<" "<<typeid(meas).name());
            return nullptr;
        }

        double ly = (*lpos)[Trk::locY];
        double halfLength = ly < 0 ? halfLengthL : halfLengthR;
        bool shiftedPos = false;

        if (std::abs(ly) > halfLength) {
            double lyold = ly;
            ly = ly < 0 ? -halfLength : halfLength;
            ATH_MSG_DEBUG(" extrapolated position outside detector, shifting it back:  " << lyold << "  size " << halfLength << "  new pos "
                                                                                         << ly);
            if (phiPos && std::abs(lyold) - halfLength > 1000.) {
                ATH_MSG_DEBUG(" rejecting track ");
                return nullptr;
            }
            shiftedPos = true;
        }

        ATH_MSG_VERBOSE("Creating fake measurement using measurement " << m_printer->print(meas));

        const Trk::Surface& surf = meas.associatedSurface();

        // no overlap, put fake phi hit at center of the chamber
        if (overlapPos) {
            // overlap, fake hit at 100. mm from the edge, error 100./sqrt(12.)

            // transform estimate of overlap position into surface frame
            std::optional<Amg::Vector2D> loverlapPos = surf.globalToLocal(*overlapPos, 3000.);
            if (!loverlapPos) {
                ATH_MSG_DEBUG(" globalToLocal failed for overlap position: " << surf.transform() * (*overlapPos));
                // calculate position fake
                double lyfake = halfLength - 50.;
                if (ly < 0.) lyfake *= -1.;
                ly = lyfake;
                errPos = FiftyOverSqrt12;
                
                if (msgLvl(MSG::VERBOSE)){
                    Amg::Vector2D LocVec2D_plus(0., lyfake);
                    const Amg::Vector3D fakePos_plus = meas.associatedSurface().localToGlobal(LocVec2D_plus);
                    Amg::Vector2D LocVec2D_min(0., -lyfake);
                    const Amg::Vector3D fakePos_min = meas.associatedSurface().localToGlobal(LocVec2D_min);

                    double phi_min = fakePos_min.phi();
                    double phi_plus = fakePos_plus.phi();
                    double phi_overlap = phiPos->phi();                
                    ATH_MSG_VERBOSE(" fake lpos " << lyfake << " ch half length " << halfLength << " phi+ " << phi_plus << " phi- " << phi_min
                                                << " phi overlap " << phi_overlap << " err " << errPos);
                }
                
            } else {
                ly = (*loverlapPos)[Trk::locY];
                halfLength = ly < 0 ? halfLengthL : halfLengthR;  // safer to redo since ly changed
                if (std::abs(ly) > halfLength) { ly = ly < 0 ? -halfLength : halfLength; }
                ATH_MSG_VERBOSE(" fake from overlap: lpos " << ly << " ch half length " << halfLength << " overlapPos " << *overlapPos);
            }
        }

        Trk::LocalParameters locPars(Trk::DefinedParameter(ly, Trk::locY));
        // Error matrix
        Amg::MatrixX cov(1, 1);
        cov(0, 0) = errPos * errPos;
        std::unique_ptr<Trk::PseudoMeasurementOnTrack> fake = std::make_unique<Trk::PseudoMeasurementOnTrack>(locPars, cov, surf);

        if (msgLvl(MSG::DEBUG)) {
            Amg::Vector2D LocVec2D(0., fake->localParameters().get(Trk::locY));
            const Amg::Vector3D fakePos = meas.associatedSurface().localToGlobal(LocVec2D);

            msg(MSG::DEBUG) << MSG::DEBUG << " createFakePhiForMeasurement for:  " << m_idHelperSvc->toStringChamber(id) << "   locY " << ly
                            << "  errpr " << errPos << " phi " << fakePos.phi() << endmsg;

            if (!shiftedPos && !overlapPos && phiPos && std::abs(phiPos->phi() - fakePos.phi()) > 0.01) {
                Amg::Transform3D gToLocal = meas.associatedSurface().transform().inverse();
                Amg::Vector3D locMeas = gToLocal * fake->globalPosition();
                ATH_MSG_WARNING(" Problem calculating fake from IP seed: phi fake " << fakePos.phi() << "  IP phi " << phiPos->phi()
                                                                                    << " local meas pos " << locMeas);
            }
        }
        return fake;
    }

    unsigned int MooTrackFitter::hasPhiConstrain(MooTrackFitter::FitterData& fitterData) const {
        // check distance between first and last hit to determine whether we need additional phi constrainst

        if ((fitterData.firstEntry->containsChamber(MuonStationIndex::CSS) ||
             fitterData.firstEntry->containsChamber(MuonStationIndex::CSL)) &&
            !fitterData.secondEntry->phiHits().empty())
            return 2;

        unsigned int nphiConstraints = fitterData.phiHits.size();
        // count SL overlap as one phi constraint
        //     if( fitterData.numberOfSLOverlaps() > 0 ) {
        //       nphiConstraints += fitterData.numberOfSLOverlaps();
        //       ATH_MSG_VERBOSE(" track has small/large overlaps " << fitterData.numberOfSLOverlaps() );
        //     }else if( (fitterData.numberOfSmallChambers() > 0 && fitterData.numberOfLargeChambers() > 0 ) ){
        //       nphiConstraints += 1;
        //       ATH_MSG_VERBOSE(" track has small and large chambers " );
        //     }

        double distanceMin = 400.;
        double distance = 0.;
        // we need at least two phis
        if (fitterData.phiHits.size() > 1) {
            // assume the hits to be sorted
            const Amg::Vector3D& gposFirstPhi = fitterData.phiHits.front()->globalPosition();
            const Amg::Vector3D& gposLastPhi = fitterData.phiHits.back()->globalPosition();
            double distFirstEtaPhi =
                (fitterData.measurements.front()->globalPosition() - fitterData.phiHits.front()->globalPosition()).mag();
            double distLastEtaPhi = (fitterData.measurements.back()->globalPosition() - fitterData.phiHits.back()->globalPosition()).mag();

            // calculate difference between hits
            Amg::Vector3D globalDistance = gposLastPhi - gposFirstPhi;

            // calculate 'projective' distance
            distance = fitterData.hasEndcap ? std::abs(globalDistance.z()) : globalDistance.perp();

            // if the distance between the first and last phi hit is smaller than 1000. count as 1 phi hit
            if (distance < distanceMin || distFirstEtaPhi > 1000 || distLastEtaPhi > 1000) {
                nphiConstraints -= fitterData.phiHits.size();  // subtract phi hits as they should only count as one phi hit
                nphiConstraints += 1;                          // add one phi hit
                ATH_MSG_VERBOSE(" distance between phi hits too small, updating phi constraints ");
            } else {
                ATH_MSG_VERBOSE(" distance between phi hits sufficient, no fake hits needed ");
            }
        }
        if (msgLvl(MSG::DEBUG)) {
            msg(MSG::DEBUG) << MSG::DEBUG << " Check phi: " << std::endl
                            << " | nphi hits | distance | SL station overlaps | small ch | large ch | nphiConstraints " << std::endl
                            << std::setw(12) << fitterData.phiHits.size() << std::setw(11) << (int)distance << std::setw(22)
                            << fitterData.numberOfSLOverlaps() << std::setw(11) << fitterData.numberOfSmallChambers() << std::setw(11)
                            << fitterData.numberOfLargeChambers() << std::setw(18) << nphiConstraints << endmsg;
        }

        return nphiConstraints;
    }

    unsigned int MooTrackFitter::hasPhiConstrain(Trk::Track* track) const {
        std::map<MuonStationIndex::StIndex, StationPhiData> stationDataMap;

        // Get a MuonTrackSummary.
        const Trk::TrackSummary* summary = track->trackSummary();
        Trk::MuonTrackSummary muonSummary;
        if (summary) {
            if (summary->muonTrackSummary()) {
                muonSummary = *summary->muonTrackSummary();
            } else {
                Trk::TrackSummary tmpSum(*summary);
                m_trackSummaryTool->addDetailedTrackSummary(*track, tmpSum);
                if (tmpSum.muonTrackSummary()) muonSummary = *(tmpSum.muonTrackSummary());
            }
        } else {
            Trk::TrackSummary tmpSummary;
            m_trackSummaryTool->addDetailedTrackSummary(*track, tmpSummary);
            if (tmpSummary.muonTrackSummary()) muonSummary = *(tmpSummary.muonTrackSummary());
        }

        std::vector<Trk::MuonTrackSummary::ChamberHitSummary>::const_iterator chit = muonSummary.chamberHitSummary().begin();
        std::vector<Trk::MuonTrackSummary::ChamberHitSummary>::const_iterator chit_end = muonSummary.chamberHitSummary().end();
        for (; chit != chit_end; ++chit) {
            // get station data
            const Identifier& chId = chit->chamberId();
            MuonStationIndex::StIndex stIndex = m_idHelperSvc->stationIndex(chId);
            StationPhiData& stData = stationDataMap[stIndex];
            if (chit->isMdt()) {
                if (m_idHelperSvc->isSmallChamber(chId))
                    ++stData.nSmallChambers;
                else
                    ++stData.nLargeChambers;
            } else {
                if (chit->phiProjection().nhits) ++stData.nphiHits;
            }
        }

        unsigned int phiConstraints = 0;
        unsigned int stationsWithSmall = 0;
        unsigned int stationsWithLarge = 0;

        std::map<MuonStationIndex::StIndex, StationPhiData>::iterator sit = stationDataMap.begin();
        std::map<MuonStationIndex::StIndex, StationPhiData>::iterator sit_end = stationDataMap.end();
        for (; sit != sit_end; ++sit) {
            StationPhiData& stData = sit->second;

            // count all stations with phi hits as phi constraint
            phiConstraints += stData.nphiHits;

            if (stData.nSmallChambers > 0 && stData.nLargeChambers > 0)
                ++phiConstraints;
            else if (stData.nSmallChambers > 0)
                ++stationsWithSmall;
            else if (stData.nLargeChambers > 0)
                ++stationsWithLarge;
        }

        if (stationsWithSmall > 0 && stationsWithLarge > 0) { ++phiConstraints; }

        return phiConstraints;
    }
    std::pair<double, double> MooTrackFitter::getElementHalfLengths(const Identifier& id, const Trk::TrkDetElementBase*  ele ) const{
        if (m_idHelperSvc->isMdt(id)) {
            const MuonGM::MdtReadoutElement* mdtDetEl = dynamic_cast<const MuonGM::MdtReadoutElement*>(ele);
            if (!mdtDetEl){ return std::make_pair(0., 0.); }
            const double halfLength = 0.5 * mdtDetEl->getActiveTubeLength(m_idHelperSvc->mdtIdHelper().tubeLayer(id), 
                                                                          m_idHelperSvc->mdtIdHelper().tube(id));
            return std::make_pair(halfLength, halfLength);
                
        } else if (m_idHelperSvc->isCsc(id)) {
            const MuonGM::CscReadoutElement* cscDetEl = dynamic_cast<const MuonGM::CscReadoutElement*>(ele);
            if (!cscDetEl) { return std::make_pair(0., 0.);}
            const double halfLenghth =  0.5 * cscDetEl->stripLength(id);
            return std::make_pair(halfLenghth, halfLenghth);           
        } else if (m_idHelperSvc->isRpc(id)) {
            const MuonGM::RpcReadoutElement* rpcDetEl = dynamic_cast<const MuonGM::RpcReadoutElement*>(ele);
            if (!rpcDetEl) { return std::make_pair(0, 0);}
            const double halfLength = 0.5 * rpcDetEl->StripLength(false); // eta-strip
            return std::make_pair(halfLength, halfLength);            
        } else if (m_idHelperSvc->isTgc(id)) {
            const MuonGM::TgcReadoutElement* tgcDetEl = dynamic_cast<const MuonGM::TgcReadoutElement*>(ele);
            if (!tgcDetEl) {return std::make_pair(0.,0.);}
            const double halfLength = 0.5 * tgcDetEl->WireLength(m_idHelperSvc->tgcIdHelper().gasGap(id), m_idHelperSvc->tgcIdHelper().channel(id));
            return std::make_pair(halfLength, halfLength);
        } else if (m_idHelperSvc->issTgc(id)) {
            const MuonGM::sTgcReadoutElement* stgcDetEl = dynamic_cast<const MuonGM::sTgcReadoutElement*>(ele);
            if (!stgcDetEl || !stgcDetEl->getDesign(id)) {return std::make_pair(0.,0.);}               
            const double halfLength =  0.5 * stgcDetEl->getDesign(id)->channelLength(m_idHelperSvc->stgcIdHelper().channel(id)); 
            return std::make_pair(halfLength, halfLength);
                
        } else if (m_idHelperSvc->isMM(id)) {
            const MuonGM::MMReadoutElement* mmDetEl = dynamic_cast<const MuonGM::MMReadoutElement*>(ele);
            if (mmDetEl) {
                return std::make_pair(mmDetEl->stripActiveLengthLeft(id) ,
                                      mmDetEl->stripActiveLengthRight(id)); // components along the local Y axis
            }
        }
        return std::make_pair(0.,0.);
    }
    bool MooTrackFitter::getMinMaxPhi(FitterData& fitterData) const {
        double phiStart = fitterData.etaHits.front()->globalPosition().phi();
        double phiOffset = 0.;
        double phiRange = 0.75 * M_PI;
        double phiRange2 = 0.25 * M_PI;
        if (phiStart > phiRange || phiStart < -phiRange)
            phiOffset = 2 * M_PI;
        else if (phiStart > -phiRange2 && phiStart < phiRange2)
            phiOffset = M_PI;

        double phiMin = -999.;
        double phiMax = 999.;

        for (const Trk::MeasurementBase* meas :fitterData.etaHits) {
       
            const Trk::RIO_OnTrack* rot = dynamic_cast<const Trk::RIO_OnTrack*>(meas);
            if (!rot) {
                const CompetingMuonClustersOnTrack* crot = dynamic_cast<const CompetingMuonClustersOnTrack*>(meas);
                // if also to a crot we cannot create a fake phi hit
                if (!crot || crot->containedROTs().empty()) continue;
                rot = crot->containedROTs().front();
            }

            if (!rot) {
                ATH_MSG_WARNING(" Measurement not a ROT ");
                continue;
            }

            Identifier id = rot->identify();
            const Trk::Surface& surf = meas->associatedSurface();
            double x = rot->localParameters()[Trk::locX];

            // distinguishing left-right just to treat the asymmetry of the active region in MM chambers
            // getElementHalfLengths
            auto [halfLengthL, halfLengthR] = getElementHalfLengths(id, rot->detectorElement());
          
            Amg::Vector2D lpLeft(x, -halfLengthL);
            const Amg::Vector3D gposLeft = surf.localToGlobal(lpLeft);
            double phiLeft = gposLeft.phi();

            Amg::Vector2D lpRight(x, halfLengthR);
            const Amg::Vector3D gposRight = surf.localToGlobal(lpRight);
            double phiRight = gposRight.phi();

            if (phiOffset > 1.5 * M_PI) {
                if (phiLeft < 0) phiLeft = phiOffset + phiLeft;
                if (phiRight < 0) phiRight = phiOffset + phiRight;
            } else if (phiOffset > 0.) {
                phiLeft = phiLeft + phiOffset;
                phiRight = phiRight + phiOffset;
            }

            bool leftSmaller = phiLeft < phiRight;

            double phiMinMeas = leftSmaller ? phiLeft : phiRight;
            double phiMaxMeas = leftSmaller ? phiRight : phiLeft;
            double orgPhiMin = phiMinMeas;
            double orgPhiMax = phiMaxMeas;

            if (phiOffset > 1.5 * M_PI) {
                if (orgPhiMin > M_PI) orgPhiMin = orgPhiMin - phiOffset;
                if (orgPhiMax > M_PI) orgPhiMax = orgPhiMax - phiOffset;
            } else if (phiOffset > 0.) {
                orgPhiMin = orgPhiMin - phiOffset;
                orgPhiMax = orgPhiMax - phiOffset;
            }

            if (phiMinMeas > phiMin) { phiMin = phiMinMeas; }
            if (phiMaxMeas < phiMax) { phiMax = phiMaxMeas; }
        }
        if (phiMin < -998 || phiMax > 998) {
            ATH_MSG_WARNING(" Could not determine minimum and maximum phi ");
            return false;
        }

        double diffPhi = phiMax - phiMin;
        double avePhi = phiMin + 0.5 * diffPhi;

        ATH_MSG_VERBOSE("Phi ranges: min " << phiMin << "  max " << phiMax << " average phi " << avePhi);

        if (diffPhi < 0) {
            if (std::abs(diffPhi) > m_openingAngleCut) {
                ATH_MSG_VERBOSE(" Inconsistent min/max, rejecting track ");
                return false;
            }
        }

        if (!fitterData.phiHits.empty()) {
            for (const Trk::MeasurementBase* meas : fitterData.phiHits) {
                const double minPhi = std::min(phiMin, phiMax);
                const double maxPhi = std::max(phiMin, phiMax);                
                double phiMeas = meas->globalPosition().phi();
                if (phiOffset > 1.5 * M_PI) {
                    if (phiMeas < 0.) phiMeas = phiOffset + phiMeas;
                } else if (phiOffset > 0.) {
                    phiMeas = phiMeas + phiOffset;
                }
                double diffMin = phiMeas - minPhi;
                double diffMax = phiMeas - maxPhi;
                if (diffMin < 0. || diffMax > 0.) {
                    if (diffMin < -m_openingAngleCut || diffMax > m_openingAngleCut) {
                        ATH_MSG_VERBOSE(" Phi hits inconsistent with min/max, rejecting track: phi meas " << phiMeas);
                        return false;
                    }
                }
            }
        }

        if (phiOffset > 1.5 * M_PI) {
            if (phiMax > M_PI) phiMax = phiMax - phiOffset;
            if (phiMin > M_PI) phiMin = phiMin - phiOffset;
            if (avePhi > M_PI) avePhi = avePhi - phiOffset;
        } else if (phiOffset > 0.) {
            phiMax = phiMax - phiOffset;
            phiMin = phiMin - phiOffset;
            avePhi = avePhi - phiOffset;
        }

        fitterData.avePhi = avePhi;
        fitterData.phiMin = phiMin;
        fitterData.phiMax = phiMax;

        return true;
    }

    std::shared_ptr<const MuonSegment> MooTrackFitter::segmentFromEntry(const EventContext& ctx, const MuPatCandidateBase& entry) const {
        // if track entry use first segment
        const MuPatTrack* trkEntry = dynamic_cast<const MuPatTrack*>(&entry);
        if (trkEntry) {
            std::shared_ptr<MuonSegment> seg{m_trackToSegmentTool->convert(ctx, trkEntry->track())};
            return seg;
        }

        // if segment entry use segment directly
        const MuPatSegment* segEntry = dynamic_cast<const MuPatSegment*>(&entry);
        if (segEntry) { 
            return std::shared_ptr<const MuonSegment> {segEntry->segment, Unowned()};          
        }

        return nullptr;
    }

    double MooTrackFitter::qOverPFromEntry(const MuPatCandidateBase& entry) const {
        return entry.entryPars().charge() / entry.entryPars().momentum().mag();
    }

    double MooTrackFitter::qOverPFromEntries(const EventContext& ctx, const MuPatCandidateBase& firstEntry, const MuPatCandidateBase& secondEntry) const {
        if (m_slFit) return 0;

        double qOverP = 0;
        // if available take momentum from entries
        if (firstEntry.hasMomentum())
            return qOverPFromEntry(firstEntry);
        else if (secondEntry.hasMomentum())
            return qOverPFromEntry(secondEntry);

        // no momentum yet, estimate from segments
        std::shared_ptr<const MuonSegment> segFirst = segmentFromEntry(ctx, firstEntry);
        if (!segFirst) {
            ATH_MSG_WARNING(" failed to get segment for first entry, this should not happen ");
            return qOverP;
        }

        std::shared_ptr<const MuonSegment> segSecond = segmentFromEntry(ctx, secondEntry);
        if (!segSecond) {
            ATH_MSG_WARNING(" failed to get segment for second entry, this should not happen ");
            return qOverP;
        }

        std::vector<const MuonSegment*> segments{segFirst.get(), segSecond.get()};
   
        double momentum{1.};
        m_momentumEstimator->fitMomentumVectorSegments(ctx, segments, momentum);
        // momentum = restrictedMomentum(momentum);

        if (momentum == 0.) return 0.;

        qOverP = 1. / momentum;
        return qOverP;
    }

    double MooTrackFitter::phiSeeding(const EventContext& ctx, MooTrackFitter::FitterData& fitterData) const {
        if (m_cosmics) {
            // check whether the first entry is a track, if not consider the second entry for phi seeding
            if (!dynamic_cast<const MuPatTrack*>(fitterData.firstEntry)) {
                // if second entry is a track, use its phi value
                if (dynamic_cast<const MuPatTrack*>(fitterData.secondEntry)) {
                    ATH_MSG_DEBUG("Using phi of second entry " << fitterData.secondEntry->entryPars().momentum().phi());
                    return fitterData.secondEntry->entryPars().momentum().phi();
                }
            }

            // add fake phi hit on first and last measurement
            const MuPatSegment* segInfo1 = dynamic_cast<const MuPatSegment*>(fitterData.firstEntry);
            const MuPatSegment* segInfo2 = dynamic_cast<const MuPatSegment*>(fitterData.secondEntry);
            if (segInfo1 && segInfo2 && fitterData.stations.size() == 1 && fitterData.numberOfSLOverlaps() == 1) {
                // only perform SL overlap fit for MDT segments
                Identifier chId = m_edmHelperSvc->chamberId(*segInfo1->segment);
                if (m_idHelperSvc->isMdt(chId)) {
                    IMuonSegmentInOverlapResolvingTool::SegmentMatchResult result =
                        m_overlapResolver->matchResult(ctx, *segInfo1->segment, *segInfo2->segment);

                    if (!result.goodMatch()) {
                        ATH_MSG_VERBOSE(" Match failed ");
                        return false;
                    }
                    double overlapPhi = result.phiResult.segmentDirection1.phi();
                    ATH_MSG_DEBUG(" using overlap phi " << overlapPhi);
                    return overlapPhi;
                }
            }

            // calculate the difference between the segment positions to estimate the direction of the muon
            Amg::Vector3D difPos = fitterData.firstEntry->entryPars().position() - fitterData.secondEntry->entryPars().position();
            // check whether the new direction is pointing down
            if (difPos.y() > 0) difPos *= -1.;

            // calculate difference in angle between phi from segment and the phi of the two new segment positions
            const double dphi = fitterData.firstEntry->entryPars().momentum().deltaPhi(difPos);
            if (std::abs(dphi) > 0.2) {
                ATH_MSG_DEBUG("Large diff between phi of segment direction and of position "
                              << fitterData.firstEntry->entryPars().momentum().phi() << "  from pos " << difPos.phi() << " dphi " << dphi);
                return difPos.phi();
            }

            ATH_MSG_DEBUG("Using phi of first entry " << fitterData.firstEntry->entryPars().momentum().phi() << "  phi from position "
                                                      << dphi);

            return fitterData.firstEntry->entryPars().momentum().phi();
        }

        double phi(0.);
        // update parameters if seeding purely with positions
        if (m_seedPhiWithEtaHits) {
            // use eta of vector connecting first/last hit
            MooTrackFitter::MeasVec& etaHits = fitterData.etaHits;
            Amg::Vector3D difPos = etaHits.back()->globalPosition() - etaHits.front()->globalPosition();
            if (difPos.mag() > 3000) {
                ATH_MSG_DEBUG("Seeding phi using average phi of eta hits ");
                phi = difPos.phi();
                return phi;
            }
        }

        // should have at least one phi hit
        MooTrackFitter::MeasVec& phiHits = fitterData.phiHits;

        if (phiHits.empty()) {
            ATH_MSG_DEBUG("No phi hits, using average phi ");
            // use average phi value to seed fit
            return fitterData.avePhi;
        }

        // in this case can't do much...
        if (phiHits.size() == 1) { return phiHits.front()->globalPosition().phi(); }

        // in endcap, if first segment in EI use its direction
        const MuPatSegment* segInfo1 = dynamic_cast<const MuPatSegment*>(fitterData.firstEntry);
        if (segInfo1 && segInfo1->containsStation(MuonStationIndex::EI) && !segInfo1->phiHits().empty()) {
            return segInfo1->segment->globalDirection().phi();
        }

        // in endcap, if first segment in EM use its direction
        if (segInfo1 && segInfo1->containsStation(MuonStationIndex::EM) && !segInfo1->phiHits().empty()) {
            return segInfo1->segment->globalDirection().phi();
        }

        if (m_seedWithAvePhi) {
            // use average phi value to seed fit
            MeasCit hit = phiHits.begin();
            MeasCit hit_end = phiHits.end();
            Amg::Vector3D avePos(0., 0., 0.);
            for (; hit != hit_end; ++hit) avePos += (*hit)->globalPosition();
            avePos /= phiHits.size();
            phi = avePos.phi();
        } else {
            // use phi of vector connecting first/last hit
            Amg::Vector3D difPos = phiHits.back()->globalPosition() - phiHits.front()->globalPosition();
            phi = difPos.phi();
        }

        return phi;
    }

    double MooTrackFitter::thetaSeeding(const MuPatCandidateBase& entry, MooTrackFitter::MeasVec& etaHits) const {
        // should have at least one eta hit
        if (etaHits.empty()) {
            ATH_MSG_WARNING(" cannot calculate eta seed from empty vector ");
            return 0;
        }

        double theta(0.);
        if (m_seedWithSegmentTheta) {
            theta = entry.entryPars().momentum().theta();
        } else {
            // in this case can't do much...
            if (etaHits.size() == 1) {
                theta = etaHits.front()->globalPosition().theta();
            } else {
                // use eta of vector connecting first/last hit
                Amg::Vector3D difPos = etaHits.back()->globalPosition() - etaHits.front()->globalPosition();
                theta = difPos.theta();
            }
        }

        return theta;
    }

   void MooTrackFitter::createStartParameters(const EventContext& ctx, MooTrackFitter::FitterData& fitterData) const {
        // get momentum + charge from entry if available, else use MuonSegmentMomentum to estimate the momentum
        double qOverP = qOverPFromEntries(ctx, *fitterData.firstEntry, *fitterData.secondEntry);
        std::unique_ptr<Trk::Perigee> startPars{nullptr};
        const MuPatCandidateBase *entry1 = fitterData.firstEntry, *entry2 = fitterData.secondEntry;

        const MuPatTrack* trkEntry1 = dynamic_cast<const MuPatTrack*>(entry1);
        const MuPatTrack* trkEntry2 = dynamic_cast<const MuPatTrack*>(entry2);
        const MuPatSegment* seg1 = dynamic_cast<const MuPatSegment*>(entry1);
        const MuPatSegment* seg2 = dynamic_cast<const MuPatSegment*>(entry2);

        Amg::Vector3D dir1{Amg::Vector3D::Zero()}, dir2{Amg::Vector3D::Zero()};
        Amg::Vector3D point1{Amg::Vector3D::Zero()}, point2{Amg::Vector3D::Zero()};
        if (seg1) {
            dir1 = seg1->segment->globalDirection();
            point1 = seg1->segment->globalPosition();
        } else {
            if (trkEntry1->track().perigeeParameters()) {
                dir1 = trkEntry1->track().perigeeParameters()->momentum().unit();
                point1 = trkEntry1->track().perigeeParameters()->position();
            } else {
                return;
            }
        }
        if (seg2) {
            dir2 = seg2->segment->globalDirection();
            point2 = seg2->segment->globalPosition();
        } else {
            if (trkEntry2->track().perigeeParameters()) {
                dir2 = trkEntry2->track().perigeeParameters()->momentum().unit();
                point2 = trkEntry2->track().perigeeParameters()->position();
            } else {
                return;
            }
        }
        if (dir1.dot(point2 - point1) < 0) {
            std::swap(point1, point2);
            std::swap(dir1, dir2);          
            entry1 = fitterData.secondEntry;
            entry2 = fitterData.firstEntry;
            trkEntry1 = dynamic_cast<const MuPatTrack*>(entry1);
            trkEntry2 = dynamic_cast<const MuPatTrack*>(entry2);
            seg1 = dynamic_cast<const MuPatSegment*>(entry1);
            seg2 = dynamic_cast<const MuPatSegment*>(entry2);
        }

        const MuPatCandidateBase* bestentry = entry1;
        double dist1 = -1, dist2 = -1;
        MuPatHitPtr firstphi1{nullptr}, lastphi1{nullptr}, firstphi2{nullptr}, lastphi2{nullptr};

        for (const MuPatHitPtr& hit : entry1->hitList()) {
            if (hit->info().type != MuPatHit::Pseudo && hit->info().measuresPhi) {
                if (!firstphi1) firstphi1 = hit;
                lastphi1 = hit;
            }
        }
        for (const MuPatHitPtr& hit : entry2->hitList()) {
            if (hit->info().type != MuPatHit::Pseudo && hit->info().measuresPhi) {
                if (!firstphi2) firstphi2 = hit;
                lastphi2 = hit;
            }
        }
        if (firstphi1) dist1 = std::abs((firstphi1->measurement().globalPosition() - lastphi1->measurement().globalPosition()).dot(dir1));
        if (firstphi2) dist2 = std::abs((firstphi2->measurement().globalPosition() - lastphi2->measurement().globalPosition()).dot(dir2));
        if (dist2 > dist1) { bestentry = entry2; }
        const MuPatTrack* besttrkEntry = dynamic_cast<const MuPatTrack*>(bestentry);
        const MuPatSegment* bestseg = dynamic_cast<const MuPatSegment*>(bestentry);
        if (besttrkEntry) {
            const Trk::TrackParameters* mdtpar = nullptr;
            for (DataVector<const Trk::TrackParameters>::const_iterator parit = besttrkEntry->track().trackParameters()->begin();
                 parit != besttrkEntry->track().trackParameters()->end(); ++parit) {
                mdtpar = *parit;
                if (mdtpar) break;
            }
            if (!mdtpar) {
                ATH_MSG_WARNING("Failed to find valid Trackparameters on track ");
                return;
            }
            Amg::VectorX newpar = mdtpar->parameters();  // besttrkEntry->track().perigeeParameters()->parameters();
            newpar[Trk::qOverP] = qOverP;
            Trk::PerigeeSurface persurf(mdtpar->position());
            startPars = std::make_unique<Trk::Perigee>(0, 0, newpar[Trk::phi], newpar[Trk::theta], qOverP,
                                         persurf);  // besttrkEntry->track().perigeeParameters()->cloneToNew(newpar);
        }

        else {
            Trk::PerigeeSurface persurf(bestseg->segment->globalPosition());
            double phi = bestseg->segment->globalDirection().phi();
            if ((fitterData.firstEntry->containsChamber(MuonStationIndex::CSS) ||
                 fitterData.firstEntry->containsChamber(MuonStationIndex::CSL)) &&
                fitterData.secondEntry->hasSLOverlap())
                phi = (fitterData.hitList.back()->parameters().position() - bestseg->segment->globalPosition()).phi();

            double theta = bestseg->segment->globalDirection().theta();
            // create start parameter
            startPars = std::make_unique<Trk::Perigee>(0, 0, phi, theta, qOverP, persurf);
           
        }
        if (!startPars) {
            ATH_MSG_DEBUG(" failed to create perigee ");
            return;
        }
        fitterData.startPars = std::move(startPars);
    }

    std::unique_ptr<Trk::Perigee> MooTrackFitter::createPerigee(const EventContext& ctx, const Trk::TrackParameters& firstPars, const Trk::MeasurementBase& firstMeas) const {
        // const Amg::Vector3D& firstPos = firstMeas.globalPosition();

        Amg::Vector3D perpos{Amg::Vector3D::Zero()};
        // propagate segment parameters to first measurement
        const Trk::TrackParameters* exPars = &firstPars;
        std::unique_ptr<Trk::TrackParameters> garbage;
        double shift = 1.;
        if (m_seedAtStartOfTrack) {
            // not sure whats going on with ownership here, so let this code take care of it
            garbage =
                m_propagator->propagate(ctx, firstPars, firstMeas.associatedSurface(), Trk::anyDirection, false, m_magFieldProperties);
            if (!exPars) {
                ATH_MSG_DEBUG(" Propagation failed in createPerigee!! ");
                return nullptr;
            }
            exPars = garbage.get();
            shift = 100.;
        }

        // small shift towards the ip
        double sign = exPars->position().dot(exPars->momentum()) > 0 ? 1. : -1.;
        perpos = exPars->position() - shift * sign * exPars->momentum().unit();

        // create perigee
        double phi = exPars->momentum().phi();
        double theta = exPars->momentum().theta();
        double qoverp = exPars->charge() / exPars->momentum().mag();
        if (m_slFit)
            qoverp = 0;
        else {
            if (!validMomentum(*exPars)) {
                return nullptr;
            }
        }

        Trk::PerigeeSurface persurf(perpos);
        std::unique_ptr<Trk::Perigee> perigee = std::make_unique<Trk::Perigee>(0, 0, phi, theta, qoverp, persurf);

        ATH_MSG_DEBUG( std::setprecision(5) << " creating perigee: phi " << phi << " theta " << theta << " q*mom "
                            << perigee->charge() * perigee->momentum().mag() << " r " << perigee->position().perp() << " z "
                            << perigee->position().z() << " input q*mom " << firstPars.charge() * firstPars.momentum().mag());
        return perigee;
    }

    double MooTrackFitter::restrictedMomentum(double momentum) const {
        // restrict range of momenta to 2 GeV / 10 GeV
        if (momentum > 0) {
            if (momentum > 20000.)
                momentum = 20000.;
            else if (momentum < 2000)
                momentum = 2000;
        } else {
            if (momentum < -20000.)
                momentum = -20000.;
            else if (momentum > -2000.)
                momentum = -2000.;
        }

        return momentum;
    }

    std::unique_ptr<Trk::Track> MooTrackFitter::fit(const EventContext& ctx, const Trk::Perigee& startPars, MooTrackFitter::MeasVec& hits,
                                                    Trk::ParticleHypothesis partHypo, bool prefit) const {
        if (hits.empty()) return nullptr;
        std::unique_ptr<Trk::Perigee> perigee{};
        ATH_MSG_VERBOSE(std::setprecision(5) << " track start parameter: phi " << startPars.momentum().phi() << " theta "
                                             << startPars.momentum().theta() << " q*mom " << startPars.charge() * startPars.momentum().mag()
                                             << " r " << startPars.position().perp() << " z " << startPars.position().z() << std::endl
                                             << " start par is a perigee "
                                             << " partHypo " << partHypo << std::endl
                                             << m_printer->print(hits));

        const Trk::TrackParameters* pars = &startPars;
        if (m_seedAtStartOfTrack) {
            DistanceAlongParameters distAlongPars;
            double dist = distAlongPars(startPars, *hits.front());

            if (dist < 0.) {
                ATH_MSG_DEBUG(" start parameters after first hit, shifting them.... ");
               perigee = createPerigee(ctx, startPars, *hits.front());
                if (perigee) {
                    pars = perigee.get();                               
                } else {
                    ATH_MSG_DEBUG(" failed to move start pars, failing fit ");
                    return nullptr;
                }
            }
        }

        // fit track

        ATH_MSG_VERBOSE("fit: " << (prefit ? "prefit" : "fit") << "track with hits: " << hits.size() << std::endl
                                << m_printer->print(hits));
        //The 'pars' is  from perigee.get(), and the perigee object still exists in the 'garbage' vector
        //cppcheck-suppress invalidLifetime
        std::unique_ptr<Trk::Track> track{m_trackFitter->fit(ctx, hits, *pars, m_runOutlier, partHypo)};

        // 'sign' track
        if (track) {
            ATH_MSG_VERBOSE("Got back this track:"<<*track);
            track->info().setPatternRecognitionInfo(m_patRecInfo);
        }

        return track;
    }

    std::unique_ptr<Trk::Track> MooTrackFitter::fitWithRefit(const EventContext& ctx, const Trk::Perigee& startPars, MooTrackFitter::MeasVec& hits) const {
        
        std::unique_ptr<Trk::Track> track = fit(ctx, startPars, hits,  Trk::muon, false);

        // exceptions that are not refitted
        if (m_slFit) return track;

        // refit track to ensure correct handling of material effects
        if (track) {
            const Trk::Perigee* pp = track->perigeeParameters();
            if (pp) {
                ATH_MSG_VERBOSE(" found track: " << m_printer->print(*track));

                // refit if absolute difference of the initial momentum and the final momentum is larger than 5 GeV
                double difMom = startPars.momentum().mag() - pp->momentum().mag();
                if (std::abs(difMom) > 5000.) {
                    ATH_MSG_DEBUG(" absolute difference in momentum too large, refitting track. Dif momentum= " << difMom);
                    ATH_MSG_DEBUG("fitWithRefit: refitting track with hits: " << hits.size() << std::endl << m_printer->print(hits));
                    std::unique_ptr<Trk::Track> refittedTrack(m_trackFitter->fit(ctx, hits, *pp, false, m_ParticleHypothesis));
                    if (refittedTrack) {
                        track.swap(refittedTrack);

                        if (msgLvl(MSG::DEBUG)) {
                            const Trk::Perigee* pp = track->perigeeParameters();
                            if (pp) {
                                ATH_MSG_DEBUG(" refitted track fit perigee: r " << pp->position().perp() << " z " << pp->position().z());
                            }
                        }
                    }
                }
            }
        }

        return track;
    }

    bool MooTrackFitter::cleanPhiHits(const EventContext& ctx, double momentum, MooTrackFitter::FitterData& fitterData,
                                      const std::vector<const Trk::PrepRawData*>& patternPhiHits) const {
        ATH_MSG_VERBOSE(" cleaning phi hits ");

        MeasVec& phiHits = fitterData.phiHits;

        // copy phi ROTs into vector, split up competing ROTs
        std::vector<const Trk::RIO_OnTrack*> rots;
        std::vector<std::unique_ptr<const Trk::RIO_OnTrack>> rotsNSW;  // hack as the phi hit cleaning does not work for NSW hits
        std::set<Identifier> ids;
        std::set<MuonStationIndex::StIndex> stations;
        rots.reserve(phiHits.size() + 5);

        for (const Trk::MeasurementBase* hit : phiHits) {
            const Trk::RIO_OnTrack* rot = dynamic_cast<const Trk::RIO_OnTrack*>(hit);
            if (rot) {
                if (m_idHelperSvc->issTgc(rot->identify())) {
                    rotsNSW.push_back(rot->uniqueClone());
                    continue;
                }
                rots.push_back(rot);
                ids.insert(rot->identify());
                continue;
            }
            const CompetingMuonClustersOnTrack* crot = dynamic_cast<const CompetingMuonClustersOnTrack*>(hit);
            if (crot) {
                for (const MuonClusterOnTrack* mit : crot->containedROTs()) {
                    rots.push_back(mit);
                    ids.insert(mit->identify());
                    MuonStationIndex::StIndex stIndex = m_idHelperSvc->stationIndex(mit->identify());
                    stations.insert(stIndex);
                }
            } else {
                ATH_MSG_WARNING(" phi hits should be ROTs or competing ROTs! Dropping hit ");
            }
        }

        if (rots.empty()) return false;

        if (rots.size() > m_phiHitsMax) {
            ATH_MSG_DEBUG(" too many phi hits, not running cleaning " << rots.size());
            return true;
        }

        if (msgLvl(MSG::VERBOSE)) {
            msg(MSG::VERBOSE) << " contained phi hits ";
            for (const Trk::RIO_OnTrack* rit : rots) { msg(MSG::DEBUG) << std::endl << "   " << m_printer->print(*rit); }
            msg(MSG::DEBUG) << endmsg;
        }

        // if available, extract additional phi hits from the road that were not on the entries
        std::vector<const Trk::PrepRawData*> roadPhiHits;
        std::copy_if(patternPhiHits.begin(), patternPhiHits.end(), std::back_inserter(roadPhiHits), [&ids, this](const Trk::PrepRawData* prd){
            return !(ids.count(prd->identify()) || 
                    /// do not add CSCs as they really should be on a segment
                    m_idHelperSvc->isCsc(prd->identify()) || m_idHelperSvc->issTgc(prd->identify()));
        });
        

        if (roadPhiHits.size() + rots.size() > m_phiHitsMax) {
            ATH_MSG_VERBOSE(" too many pattern phi hits, not adding any " << roadPhiHits.size());
            roadPhiHits.clear();  // cleaning road hits as we do not want to add them but we do want to clean
        }
        if (msgLvl(MSG::VERBOSE)) {
            if (!roadPhiHits.empty()) {
                msg(MSG::VERBOSE) << " additional pattern phi hits " << std::endl;
                std::vector<const Trk::PrepRawData*>::const_iterator pit = roadPhiHits.begin();
                std::vector<const Trk::PrepRawData*>::const_iterator pit_end = roadPhiHits.end();
                for (; pit != pit_end; ++pit) {
                    msg(MSG::DEBUG) << "   " << m_printer->print(**pit);
                    if (pit + 1 != pit_end)
                        msg(MSG::DEBUG) << std::endl;
                    else
                        msg(MSG::DEBUG) << endmsg;
                }
            }
        }

        std::vector<std::unique_ptr<const Trk::MeasurementBase>> newMeasurements{m_phiHitSelector->select_rio(momentum, rots, roadPhiHits)};
       
        newMeasurements.insert(newMeasurements.end(), 
                               std::make_move_iterator(rotsNSW.begin()), 
                               std::make_move_iterator(rotsNSW.end()));

        ATH_MSG_VERBOSE(" selected phi hits " << newMeasurements.size());

        if (newMeasurements.empty()) {
            ATH_MSG_DEBUG(" empty list of phi hits return from phi hit selector: input size " << phiHits.size());
            return false;
        }

        // require the new hits to be within a certain distance of the hits already on the track candidate
        DistanceAlongParameters distAlongPars;
        constexpr double maxDistCut = 800.;
        std::vector<const Trk::MeasurementBase*> measurementsToBeAdded{};
        measurementsToBeAdded.reserve(newMeasurements.size());
        for (std::unique_ptr<const Trk::MeasurementBase>& meas : newMeasurements) {          
            const Identifier id = m_edmHelperSvc->getIdentifier(*meas);
            if (!id.is_valid()) {
                ATH_MSG_WARNING(" Phi measurement without valid Identifier! ");
                continue;
            }
            ATH_MSG_VERBOSE(" Test phi hit - " << m_printer->print(*meas));
            // only add hits if the hitList is not empty so we can calculate the distance of the new hit to the first and last hit on the
            // track
            if (fitterData.hitList.empty()) {
                ATH_MSG_VERBOSE("          discarding phi hit due to empty hitList ");
                break;
            }

            // calculate the distance of the phi hit to the first hit. If the phi hit lies before the first hit,
            // remove the hit if the distance is too large
            double dist = distAlongPars(fitterData.hitList.front()->parameters(), *meas);
            if (dist < -maxDistCut) {
                ATH_MSG_VERBOSE("          discarded phi hit, distance to first hit too large " << dist);
                continue;
            }

            // calculate the distance of the phi hit to the last hit. If the phi hit lies after the last hit,
            // remove the hit if the distance is too large
            double distBack = distAlongPars(fitterData.hitList.back()->parameters(), *meas);
            if (distBack > maxDistCut) {
                ATH_MSG_VERBOSE("          discarded phi hit, distance to last hit too large " << distBack);
                continue;
            }

            ATH_MSG_VERBOSE("          new phi hit, distance from start pars " << dist << " distance to last pars " << distBack);

            measurementsToBeAdded.push_back(meas.get());
            fitterData.garbage.push_back(std::move(meas));
        }

        // now remove all previous phi hits and replace them with the new ones

        for (const Trk::MeasurementBase* hit : phiHits) {
            ATH_MSG_VERBOSE(" Remove phi measurement " << m_printer->print(*hit));
            m_hitHandler->remove(*hit, fitterData.hitList);
                
        }

        // add the new phi hits to the hit list
        if (!measurementsToBeAdded.empty()) {
            ATH_MSG_VERBOSE(" adding measurements "<<std::endl<<m_printer->print(measurementsToBeAdded));
            MuPatHitList newHitList;
            m_hitHandler->create(ctx, fitterData.firstEntry->entryPars(), measurementsToBeAdded, newHitList);
            fitterData.hitList = m_hitHandler->merge(newHitList, fitterData.hitList);
        }

        ATH_MSG_VERBOSE(" done cleaning ");

        return true;
    }

    std::unique_ptr<Trk::Track> MooTrackFitter::cleanAndEvaluateTrack(const EventContext& ctx, Trk::Track& track,
                                                                      const std::set<Identifier>& excludedChambers) const {
        // preselection to get ride of really bad tracks
        if (!m_edmHelperSvc->goodTrack(track, m_preCleanChi2Cut)) {
            ATH_MSG_DEBUG(" Track rejected due to large chi2" << std::endl << m_printer->print(track));
            return nullptr;
        }

        // perform cleaning of track
        std::unique_ptr<Trk::Track> cleanTrack;
        if (excludedChambers.empty())
            cleanTrack = m_cleaner->clean(track, ctx);
        else {
            ATH_MSG_DEBUG(" Cleaning with exclusion list " << excludedChambers.size());
            cleanTrack = m_cleaner->clean(track, excludedChambers, ctx);
        }
        if (!cleanTrack) {
            ATH_MSG_DEBUG(" Cleaner returned a zero pointer, reject track ");
            return nullptr;
        }

        // selection to get ride of bad tracks after cleaning
        if (!m_edmHelperSvc->goodTrack(*cleanTrack, m_chi2Cut)) {
            ATH_MSG_DEBUG(" Track rejected after cleaning " << std::endl << m_printer->print(*cleanTrack));
            return nullptr;
        }

        ATH_MSG_DEBUG(" Track passed selection after cleaning! ");
        return cleanTrack;
    }

    bool MooTrackFitter::validMomentum(const Trk::TrackParameters& pars) const {
        const double p = pars.momentum().mag();
        if (p < m_pThreshold) {
            ATH_MSG_DEBUG(" momentum below threshold: momentum  " << pars.momentum().mag() << "  p " << pars.momentum().perp());
            return false;
        }
        return true;
    }

    void MooTrackFitter::cleanEntry(const MuPatCandidateBase& entry, std::set<Identifier>& removedIdentifiers) const {
        // if segment entry use segment directly
        const MuPatSegment* segEntry = dynamic_cast<const MuPatSegment*>(&entry);
        if (segEntry && segEntry->segment) {
            if (entry.hasSmallChamber() && entry.hasLargeChamber()) {
                ATH_MSG_DEBUG(" Segment with SL overlap, cannot perform cleaning ");
            } else {
                cleanSegment(*segEntry->segment, removedIdentifiers);
            }
        }
    }

    void MooTrackFitter::cleanSegment(const MuonSegment& seg, std::set<Identifier>& removedIdentifiers) const {
        TrkDriftCircleMath::DCOnTrackVec dcs;
        /* ********  Mdt hits  ******** */

        const MuonGM::MdtReadoutElement* detEl = nullptr;

        Amg::Transform3D gToStation;

        // set to get Identifiers of chambers with hits
        std::vector<std::pair<Identifier, bool>> indexIdMap;
        indexIdMap.reserve(seg.containedMeasurements().size());

        unsigned index = 0;
        float tubeRadius = 14.6;
        std::vector<const Trk::MeasurementBase*>::const_iterator it = seg.containedMeasurements().begin();
        std::vector<const Trk::MeasurementBase*>::const_iterator it_end = seg.containedMeasurements().end();
        ATH_MSG_DEBUG("loop through hits for segment");
        for (; it != it_end; ++it) {
            const MdtDriftCircleOnTrack* mdt = dynamic_cast<const MdtDriftCircleOnTrack*>(*it);

            if (!mdt) { continue; }
            Identifier id = mdt->identify();
            if (!detEl) {
                detEl = mdt->prepRawData()->detectorElement();
                gToStation = detEl->GlobalToAmdbLRSTransform();
            }
            if (!detEl) {
                ATH_MSG_WARNING(" error aborting not detEl found ");
                break;
            }

            ATH_MSG_DEBUG("detector element of station type " << detEl->getStationType());
            tubeRadius = detEl->innerTubeRadius();

            // calculate local AMDB position
            Amg::Vector3D LocVec2D = gToStation * mdt->prepRawData()->globalPosition();
            TrkDriftCircleMath::LocVec2D lpos(LocVec2D.y(), LocVec2D.z());

            double r = mdt->localParameters()[Trk::locR];
            double dr = Amg::error(mdt->localCovariance(), Trk::locR);

            // create identifier
            TrkDriftCircleMath::MdtId mdtid(m_idHelperSvc->mdtIdHelper().isBarrel(id), m_idHelperSvc->mdtIdHelper().multilayer(id) - 1,
                                            m_idHelperSvc->mdtIdHelper().tubeLayer(id) - 1, m_idHelperSvc->mdtIdHelper().tube(id) - 1);

            // create new DriftCircle
            TrkDriftCircleMath::DriftCircle dc(lpos, r, dr, TrkDriftCircleMath::DriftCircle::InTime, mdtid, index, mdt);
            TrkDriftCircleMath::DCOnTrack dcOnTrack(dc, 1., 1.);
            ATH_MSG_VERBOSE(" new MDT hit " << m_idHelperSvc->toString(id));

            dcs.push_back(dcOnTrack);
            indexIdMap.push_back(std::make_pair(id, false));

            ++index;
        }

        double angleYZ = seg.localDirection().angleYZ();
        const Amg::Vector3D lpos = gToStation * seg.globalPosition();

        ATH_MSG_DEBUG(" cleaning segment " << m_printer->print(seg) << std::endl
                                           << " local parameters " << lpos.y() << " " << lpos.z() << "  phi " << angleYZ << "  with "
                                           << dcs.size() << " hits  ");

        TrkDriftCircleMath::LocVec2D segPos(lpos.y(), lpos.z());
        TrkDriftCircleMath::Line segPars(segPos, angleYZ);

        TrkDriftCircleMath::Segment segment(TrkDriftCircleMath::Line(0., 0., 0.), TrkDriftCircleMath::DCOnTrackVec());
        TrkDriftCircleMath::DCSLFitter fitter;
        fitter.fit(segment, segPars, dcs);
        segment.hitsOnTrack(dcs.size());
        ATH_MSG_DEBUG(" segment after fit " << segment.chi2() << " ndof " << segment.ndof() << " local parameters " << segment.line().x0()
                                            << " " << segment.line().y0() << "  phi " << segment.line().phi());

        bool hasDroppedHit = false;
        unsigned int dropDepth = 0;
        TrkDriftCircleMath::SegmentFinder finder;
        bool success = finder.dropHits(segment, hasDroppedHit, dropDepth);
        if (!success) {
            ATH_MSG_DEBUG(" drop hits failed ");
            return;
        }

        if (dcs.size() == segment.ndof() + 2) {
            ATH_MSG_DEBUG(" segment unchanges ");
        } else if (dcs.size() != segment.ndof() + 3) {
            ATH_MSG_DEBUG(" more than one hit removed, keeping old segment ");
        } else {
            ATH_MSG_DEBUG(" removed single hit, not using it in fit ");

            TrkDriftCircleMath::MatchDCWithLine matchDC(segment.line(), 5., TrkDriftCircleMath::MatchDCWithLine::Pull, tubeRadius);
            const TrkDriftCircleMath::DCOnTrackVec& matchedDCs = matchDC.match(segment.dcs());

            for (TrkDriftCircleMath::DCOnTrackCit dcit = matchedDCs.begin(); dcit != matchedDCs.end(); ++dcit) {
                if (dcit->state() == TrkDriftCircleMath::DCOnTrack::OnTrack) continue;
                if ((unsigned int)dcit->index() >= indexIdMap.size()) continue;
                indexIdMap[dcit->index()].second = true;
            }

            std::vector<std::pair<Identifier, bool>>::iterator iit = indexIdMap.begin();
            std::vector<std::pair<Identifier, bool>>::iterator iit_end = indexIdMap.end();
            for (; iit != iit_end; ++iit) {
                if (iit->second) {
                    ATH_MSG_DEBUG(" removing hit " << m_idHelperSvc->toString(iit->first));
                    removedIdentifiers.insert(iit->first);
                }
            }
        }
    }

    void MooTrackFitter::removeSegmentOutliers(FitterData& fitterData) const {
        if (fitterData.startPars->momentum().mag() < 4000.) return;

        std::set<Identifier> removedIdentifiers;
        if (fitterData.firstEntry) cleanEntry(*fitterData.firstEntry, removedIdentifiers);
        if (fitterData.secondEntry) cleanEntry(*fitterData.secondEntry, removedIdentifiers);

        ATH_MSG_DEBUG(" removing hits  " << removedIdentifiers.size());

        for (const Identifier& id : removedIdentifiers) {
            ATH_MSG_VERBOSE("Remove hit "<<m_idHelperSvc->toString(id));
            m_hitHandler->remove(id, fitterData.hitList);
        }
    }

    std::pair<std::unique_ptr<Trk::Track>, std::unique_ptr<Trk::Track>> MooTrackFitter::splitTrack(const EventContext& ctx, const Trk::Track& track) const {
     
        // access TSOS of track
        const Trk::TrackStates* oldTSOT = track.trackStateOnSurfaces();
        if (!oldTSOT) return std::make_pair<std::unique_ptr<Trk::Track>, std::unique_ptr<Trk::Track>>(nullptr, nullptr);

        // check whether the perigee is expressed at the point of closes approach or at muon entry
        const Trk::Perigee* perigee = track.perigeeParameters();
        if (perigee) {
            bool atIP = std::abs(perigee->position().dot(perigee->momentum().unit())) < 10 ? true : false;
            if (atIP) {
                ATH_MSG_DEBUG(" track extressed at perigee, cannot split it ");
                return std::make_pair<std::unique_ptr<Trk::Track>, std::unique_ptr<Trk::Track>>(nullptr, nullptr);
            }
        }

        // loop over content input track and count perigees
        unsigned int nperigees(0);
        Trk::TrackStates::const_iterator tit = oldTSOT->begin();
        Trk::TrackStates::const_iterator tit_end = oldTSOT->end();
        for (; tit != tit_end; ++tit) {
            // look whether state is a perigee
            if ((*tit)->type(Trk::TrackStateOnSurface::Perigee) && dynamic_cast<const Trk::Perigee*>((*tit)->trackParameters()))
                ++nperigees;
        }
        if (nperigees != 2) {
            ATH_MSG_DEBUG(" Number of perigees is not one, cannot split it " << nperigees);
            return std::make_pair<std::unique_ptr<Trk::Track>, std::unique_ptr<Trk::Track>>(nullptr, nullptr);
        }

        struct TrackContent {
            TrackContent() : firstParameters(nullptr), tsos(), track() {}
            const Trk::TrackParameters* firstParameters;
            std::vector<const Trk::TrackStateOnSurface*> tsos;
            std::unique_ptr<Trk::Track> track;
            std::set<MuonStationIndex::StIndex> stations;
        };

        // store content of split tracks
        TrackContent firstTrack;
        firstTrack.tsos.reserve(oldTSOT->size());

        TrackContent secondTrack;
        secondTrack.tsos.reserve(oldTSOT->size());

        // keep track of the current set that is being filled
        TrackContent* currentTrack = &firstTrack;

        // count perigees on track
        nperigees = 0;

        // loop over content input track and extract the content of the two split tracks
        tit = oldTSOT->begin();
        tit_end = oldTSOT->end();
        for (; tit != tit_end; ++tit) {
            const Trk::TrackParameters* pars = (*tit)->trackParameters();

            // look whether state is a perigee
            if ((*tit)->type(Trk::TrackStateOnSurface::Perigee) && dynamic_cast<const Trk::Perigee*>(pars)) {
                ++nperigees;

                if (msgLvl(MSG::DEBUG) && nperigees == 1) msg(MSG::DEBUG) << MSG::DEBUG << " found first perigee on track " << endmsg;

                // if this is the second perigee, switch to second part of the track
                if (nperigees == 2) {
                    ATH_MSG_DEBUG(" found second perigee, switch to second track");
                    currentTrack = &secondTrack;

                } else if (nperigees > 2) {
                    ATH_MSG_WARNING(" Unexpected number of perigees: " << nperigees);
                }
            }

            // we should drop all states inbetween the two perigees
            if (nperigees == 1) { ATH_MSG_VERBOSE(" state between the two perigees "); }

            // check whether the current TrackContent has firstParameters, if not set them
            if (!currentTrack->firstParameters) {
                ATH_MSG_VERBOSE(" found first parameters " << m_printer->print(*pars));
                currentTrack->firstParameters = pars;
            }

            // check whether there are track parameters on the current state
            if (!pars) {
                ATH_MSG_VERBOSE(" adding state without parameters ");
                currentTrack->tsos.push_back(*tit);
                continue;
            }

            // check whether state is a measurement, if not continue
            const Trk::MeasurementBase* meas = (*tit)->measurementOnTrack();
            if (!meas) {
                ATH_MSG_VERBOSE(" adding state without measurement " << m_printer->print(*pars));
                currentTrack->tsos.push_back(*tit);
                continue;
            }

            // get identifier, if it has no identifier or is not a muon hit continue
            Identifier id = m_edmHelperSvc->getIdentifier(*meas);
            if (!id.is_valid() || !m_idHelperSvc->isMuon(id)) {
                ATH_MSG_VERBOSE(" adding state with measurement without valid identifier " << m_printer->print(*pars));
                currentTrack->tsos.push_back(*tit);
                continue;
            }

            if (nperigees == 1) ATH_MSG_WARNING(" found muon measurement inbetween the two perigees, this should not happen ");

            // add station index for MDT and CSC hits
            if (!m_idHelperSvc->isTrigger(id)) currentTrack->stations.insert(m_idHelperSvc->stationIndex(id));

            ATH_MSG_VERBOSE(" adding  " << m_printer->print(*meas));

            currentTrack->tsos.push_back(*tit);
        }

        if (firstTrack.firstParameters)
            ATH_MSG_DEBUG(" first track content: states " << firstTrack.tsos.size() << " stations " << firstTrack.stations.size() << endmsg
                                                          << " first pars " << m_printer->print(*firstTrack.firstParameters));

        else if (secondTrack.firstParameters)
            ATH_MSG_DEBUG(" second track content: states " << secondTrack.tsos.size() << " stations " << secondTrack.stations.size()
                                                           << endmsg << " first pars " << m_printer->print(*secondTrack.firstParameters));

        // check whether both tracks have start parameters and sufficient stations
        if ((firstTrack.firstParameters && firstTrack.stations.size() > 1) &&
            (secondTrack.firstParameters && secondTrack.stations.size() > 1)) {
            ATH_MSG_DEBUG(" track candidate can be split, trying to fit split tracks ");
            // fit the two tracks
            firstTrack.track = fitSplitTrack(ctx, *firstTrack.firstParameters, firstTrack.tsos);
            if (firstTrack.track) {
                ATH_MSG_DEBUG(" fitted first track, trying second ");

                secondTrack.track = fitSplitTrack(ctx, *secondTrack.firstParameters, secondTrack.tsos);

                if (secondTrack.track) {
                    ATH_MSG_DEBUG(" fitted second track ");

                } else {
                    ATH_MSG_DEBUG(" failed to fit second track ");
                    firstTrack.track.reset();
                }
            } else {
                ATH_MSG_DEBUG(" failed to fit first track ");
            }
        }

        return std::make_pair<std::unique_ptr<Trk::Track>, std::unique_ptr<Trk::Track>>(std::move(firstTrack.track),
                                                                                        std::move(secondTrack.track));
    }

    std::unique_ptr<Trk::Track> MooTrackFitter::fitSplitTrack(const EventContext& ctx, const Trk::TrackParameters& startPars,
                                                              const std::vector<const Trk::TrackStateOnSurface*>& tsos) const {
        // first create track out of the constituent
        double phi = startPars.momentum().phi();
        double theta = startPars.momentum().theta();
        double qoverp = startPars.charge() / startPars.momentum().mag();
        if (m_slFit) qoverp = 0;
        Trk::PerigeeSurface persurf(startPars.position());
        std::unique_ptr<Trk::Perigee> perigee = std::make_unique<Trk::Perigee>(0, 0, phi, theta, qoverp, persurf);

        Trk::TrackStates trackStateOnSurfaces{};
        trackStateOnSurfaces.reserve(tsos.size() + 1);
        trackStateOnSurfaces.push_back(MuonTSOSHelper::createPerigeeTSOS(std::move(perigee)));
        for (const Trk::TrackStateOnSurface* tsos : tsos) trackStateOnSurfaces.push_back(tsos->clone());

        Trk::TrackInfo trackInfo(Trk::TrackInfo::Unknown, Trk::muon);
        std::unique_ptr<Trk::Track> track = std::make_unique<Trk::Track>(trackInfo, std::move(trackStateOnSurfaces), nullptr);

        unsigned int nphi = hasPhiConstrain(track.get());

        if (nphi > 1) {
            ATH_MSG_DEBUG("Track has sufficient phi constraints, fitting ");

        } else {
            // loop over the track and find the first and last measurement and the phi hit if any
            const Trk::TrackStateOnSurface* firstMeas = nullptr;
            const Trk::TrackStateOnSurface* lastMeas = nullptr;
            const Trk::TrackStateOnSurface* phiMeas = nullptr;

            std::vector<const Trk::TrackStateOnSurface*>::const_iterator tit = tsos.begin();
            std::vector<const Trk::TrackStateOnSurface*>::const_iterator tit_end = tsos.end();
            for (; tit != tit_end; ++tit) {
                // require track parameters;
                if (!(*tit)->trackParameters()) continue;

                const Trk::MeasurementBase* meas = (*tit)->measurementOnTrack();
                if (!meas) continue;

                Identifier id = m_edmHelperSvc->getIdentifier(*meas);
                if (!id.is_valid() || !m_idHelperSvc->isMuon(id)) continue;

                if (m_idHelperSvc->isMdt(id)) {
                    if (!firstMeas) firstMeas = *tit;
                    lastMeas = *tit;
                } else if (m_idHelperSvc->measuresPhi(id)) {
                    phiMeas = *tit;
                }
            }

            if (!firstMeas) {
                ATH_MSG_WARNING(" failed to find first MDT measurement with track parameters");
                return nullptr;
            }

            if (!lastMeas) {
                ATH_MSG_WARNING(" failed to find second MDT measurement with track parameters");
                return nullptr;
            }

            if (firstMeas == lastMeas) {
                ATH_MSG_WARNING(" first MDT measurement with track parameters equals to second");
                return nullptr;
            }

            const Trk::TrackStateOnSurface* positionFirstFake = nullptr;
            const Trk::TrackStateOnSurface* positionSecondFake = nullptr;

            if (phiMeas) {
                double distFirst = (firstMeas->trackParameters()->position() - phiMeas->trackParameters()->position()).mag();
                double distSecond = (lastMeas->trackParameters()->position() - phiMeas->trackParameters()->position()).mag();
                ATH_MSG_DEBUG("Track has one phi constraints, adding second: dist to first " << distFirst << " dist to second "
                                                                                             << distSecond);
                // add fake at position furthest away from phi measurement
                if (distFirst < distSecond) {
                    positionFirstFake = lastMeas;
                } else {
                    positionFirstFake = firstMeas;
                }

            } else {
                ATH_MSG_DEBUG("Track has no phi constraints, adding one at beginning and one at the end of the track ");

                positionFirstFake = firstMeas;
                positionSecondFake = lastMeas;
            }

            // clean up previous track and create new one with fake hits
            auto uniquePerigee = std::make_unique<const Trk::Perigee>(0, 0, phi, theta, qoverp, persurf);
            trackStateOnSurfaces = Trk::TrackStates();
            trackStateOnSurfaces.reserve(tsos.size() + 3);
            trackStateOnSurfaces.push_back(MuonTSOSHelper::createPerigeeTSOS(std::move(uniquePerigee)));

            tit = tsos.begin();
            tit_end = tsos.end();
            for (; tit != tit_end; ++tit) {
                // remove existing pseudo measurements
                const Trk::MeasurementBase* meas = (*tit)->measurementOnTrack();
                if (meas && dynamic_cast<const Trk::PseudoMeasurementOnTrack*>(meas)) {
                    ATH_MSG_DEBUG("removing existing pseudo measurement ");
                    continue;
                }

                trackStateOnSurfaces.push_back((*tit)->clone());
                if (*tit == positionFirstFake) {
                    double fakeError = 100.;
                    if (positionFirstFake->trackParameters()->covariance()) {
                        fakeError = Amg::error(*positionFirstFake->trackParameters()->covariance(), Trk::loc2);
                        ATH_MSG_DEBUG(" Using error of parameters " << fakeError);
                    }

                    Amg::Vector3D position = positionFirstFake->trackParameters()->position();
                    std::unique_ptr<Trk::MeasurementBase> fake =
                        createFakePhiForMeasurement(*(positionFirstFake->measurementOnTrack()), &position, nullptr, fakeError);
                    if (fake) {
                        // need to clone as fake is already added to garbage collection
                        trackStateOnSurfaces.push_back(MuonTSOSHelper::createMeasTSOSWithUpdate(
                            **tit, std::move(fake), positionFirstFake->trackParameters()->uniqueClone(), Trk::TrackStateOnSurface::Measurement));
                    } else {
                        ATH_MSG_WARNING(" failed to create fake at first measurement ");
                    }
                }
                if (*tit == positionSecondFake && positionSecondFake) {
                    double fakeError = 100.;
                    if (positionSecondFake->trackParameters()->covariance()) {
                        fakeError = Amg::error(*positionSecondFake->trackParameters()->covariance(), Trk::loc2);
                        ATH_MSG_DEBUG(" Using error of parameters " << fakeError);
                    }

                    Amg::Vector3D position = positionSecondFake->trackParameters()->position();
                    std::unique_ptr<Trk::MeasurementBase> fake =
                        createFakePhiForMeasurement(*(positionSecondFake->measurementOnTrack()), &position, nullptr, fakeError);
                    if (fake) {
                        // need to clone as fake is already added to garbage collection
                        trackStateOnSurfaces.push_back(MuonTSOSHelper::createMeasTSOSWithUpdate(
                            **tit, std::move(fake), positionSecondFake->trackParameters()->uniqueClone(), Trk::TrackStateOnSurface::Measurement));
                    } else {
                        ATH_MSG_WARNING(" failed to create fake at second measurement ");
                    }
                }
            }

            Trk::TrackInfo trackInfo(Trk::TrackInfo::Unknown, Trk::muon);
            track = std::make_unique<Trk::Track>(trackInfo, std::move(trackStateOnSurfaces), nullptr);
        }

        // refit track
        std::unique_ptr<Trk::Track> refittedTrack = refit(ctx, *track);
        if (refittedTrack) refittedTrack->info().setPatternRecognitionInfo(m_patRecInfo);

        return refittedTrack;
    }

}  // namespace Muon
