/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonTrackTagTestTool.h"

#include "GaudiKernel/MsgStream.h"
#include "InDetRIO_OnTrack/PixelClusterOnTrack.h"
#include "InDetRIO_OnTrack/TRT_DriftCircleOnTrack.h"
#include "TrkGeometry/TrackingVolume.h"
#include "TrkMeasurementBase/MeasurementBase.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkSurfaces/CylinderSurface.h"
#include "TrkSurfaces/DiscSurface.h"
#include "TrkSurfaces/StraightLineSurface.h"
#include "TrkTrack/Track.h"

#ifdef MUONCOMBDEBUG
#include "AtlasHepMC/GenParticle.h"
#include "TrkTruthData/TrackTruth.h"
#include "TrkTruthData/TrackTruthCollection.h"
#endif

namespace {
    constexpr double dummy_chi2 = 1.e25;
}
namespace MuonCombined {

    MuonTrackTagTestTool::MuonTrackTagTestTool(const std::string &type, const std::string &name, const IInterface *parent) :
        AthAlgTool(type, name, parent) {
        declareInterface<IMuonTrackTagTool>(this);
        declareProperty("Chi2Cut", m_chi2cut = 50.);
#ifdef MUONCOMBDEBUG
        declareProperty("Truth", m_truth = false);
#endif
    }

    StatusCode MuonTrackTagTestTool::initialize() {
        ATH_CHECK(m_extrapolator.retrieve());
        if (!m_trackingGeometryReadKey.empty()) {
            ATH_CHECK(m_trackingGeometryReadKey.initialize());
        } else {
            ATH_MSG_ERROR("Could not retrieve a valid tracking geometry");
        }

        ATH_MSG_INFO("Initialized successfully");

        return StatusCode::SUCCESS;
    }

    double MuonTrackTagTestTool::chi2(const Trk::Track &idTrack, const Trk::Track &msTrack, const EventContext &ctx) const {
        const Trk::TrackingVolume *msEntrance = getVolume("MuonSpectrometerEntrance", ctx);
        if (!msEntrance) {
            ATH_MSG_ERROR("No MS entrance available");
            return dummy_chi2;
        }
        if (idTrack.perigeeParameters() == nullptr) {
            ATH_MSG_WARNING("Skipping track combination - no perigee parameters for ID track");
            return dummy_chi2;
        }
        if (msTrack.perigeeParameters() == nullptr) {
            ATH_MSG_WARNING("Skipping track combination - no perigee parameters for MS track");
            return dummy_chi2;
        }
        // skip tracks from backtracking
        if (dynamic_cast<const Trk::StraightLineSurface *>(&(**idTrack.measurementsOnTrack()->begin()).associatedSurface())) return 0;
        if (idTrack.measurementsOnTrack()->size() < 7 ||
            dynamic_cast<const Trk::StraightLineSurface *>(&(*idTrack.measurementsOnTrack())[6]->associatedSurface()) ||
            !dynamic_cast<const InDet::PixelClusterOnTrack *>(*idTrack.measurementsOnTrack()->begin()))
            return 0;
        Trk::TrackStates::const_iterator itStates = idTrack.trackStateOnSurfaces()->begin();
        Trk::TrackStates::const_iterator endState = idTrack.trackStateOnSurfaces()->end();
        int noutl = 0, ntrt = 0;
        for (; itStates != endState; ++itStates) {
            if ((**itStates).measurementOnTrack()) {
                const InDet::TRT_DriftCircleOnTrack *trthit =
                    dynamic_cast<const InDet::TRT_DriftCircleOnTrack *>((**itStates).measurementOnTrack());
                if (trthit) {
                    if ((**itStates).type(Trk::TrackStateOnSurface::Outlier))
                        noutl++;
                    else
                        ntrt++;
                }
            }
        }
        double eta = idTrack.perigeeParameters()->eta();
        ATH_MSG_DEBUG("ntrt: " << ntrt << " ntrtoutl: " << noutl << " eta: " << eta);
        if (noutl >= 15 || (ntrt == 0 && std::abs(eta) > .1 && std::abs(eta) < 1.9)) return 0;

        // skip tracks below 2.5 GeV
        double idqoverp = idTrack.perigeeParameters()->parameters()[Trk::qOverP];
        if (idqoverp != 0 && std::abs(1 / idqoverp) < 2500) return 0;

        const Trk::TrackParameters *muonpar = msTrack.trackParameters()->front();

        bool checkphiflip = false, muonisstraight = false;

        if (std::abs(muonpar->parameters()[Trk::qOverP]) < 1.e-9) checkphiflip = muonisstraight = true;

        double phiID = (**idTrack.trackParameters()->rbegin()).parameters()[Trk::phi], phiMS = muonpar->position().phi();
        double thetaID = (**idTrack.trackParameters()->rbegin()).parameters()[Trk::theta], thetaMS = muonpar->parameters()[Trk::theta];

        ATH_MSG_DEBUG("phi ID: " << phiID << " phi MS: " << phiMS << " diff: " << phiID - phiMS
                                 << " pt ID: " << idTrack.perigeeParameters()->pT() << " pt ms: " << muonpar->pT());
        ATH_MSG_DEBUG("theta ID: " << thetaID << " theta MS: " << thetaMS << " diff: " << thetaID - thetaMS);
        double phidiff = std::abs(phiID - phiMS);
        if (std::abs(phidiff - 2 * M_PI) < phidiff) phidiff = 2 * M_PI - phidiff;
        if (checkphiflip && std::abs(phidiff - M_PI) < phidiff) phidiff = std::abs(M_PI - phidiff);
        double thetalimit = .6, philimit = .8;
        if (muonisstraight) {
            thetalimit = 1.;
            philimit = 2;
        }
        if (!(std::abs(thetaID - thetaMS) < thetalimit && std::abs(phidiff) < philimit)) return 0;

        const Trk::TrackParameters *lastmeasidpar = nullptr;
        int index = (int)idTrack.trackParameters()->size();
        while (!lastmeasidpar && index > 0) {
            index--;
            lastmeasidpar = (*idTrack.trackParameters())[index]->covariance() ? (*idTrack.trackParameters())[index] : nullptr;
        }
        if (!lastmeasidpar) {
            ATH_MSG_WARNING("ID track parameters don't have error matrix!");
            return 0;
        }

        const Trk::TrackParameters *mspar = nullptr;
        Trk::TrackStates::const_iterator tsosit = msTrack.trackStateOnSurfaces()->begin();

        while (tsosit != msTrack.trackStateOnSurfaces()->end() && !mspar) {
            if ((**tsosit).type(Trk::TrackStateOnSurface::Measurement) && !(**tsosit).type(Trk::TrackStateOnSurface::Outlier)) {
                mspar = (**tsosit).trackParameters();
            }
            ++tsosit;
        }

        if (!mspar) {
            ATH_MSG_WARNING("Could not find muon track parameters!");
            return 0;
        }

        std::unique_ptr<const Trk::TrackParameters> idextrapolatedpar = std::unique_ptr<const Trk::TrackParameters>(
            m_extrapolator->extrapolateToVolume(ctx, *lastmeasidpar, *msEntrance, Trk::alongMomentum, Trk::muon));

        if (!idextrapolatedpar && lastmeasidpar->parameters()[Trk::qOverP] != 0 &&
            std::abs(1. / lastmeasidpar->parameters()[Trk::qOverP]) < 5. * CLHEP::GeV) {
            ATH_MSG_DEBUG("Extrapolating with p=5 GeV");
            AmgVector(5) params = lastmeasidpar->parameters();
            double sign = (params[Trk::qOverP] > 0) ? 1 : -1;
            double newqoverp = sign / (5. * CLHEP::GeV);
            params[Trk::qOverP] = newqoverp;
            std::unique_ptr<const Trk::TrackParameters> newlastidpar = lastmeasidpar->associatedSurface().createUniqueTrackParameters(
                params[0], params[1], params[2], params[3], params[4], AmgSymMatrix(5)(*lastmeasidpar->covariance()));
            if (newlastidpar) {
                idextrapolatedpar = std::unique_ptr<const Trk::TrackParameters>(
                    m_extrapolator->extrapolateToVolume(ctx, *newlastidpar, *msEntrance, Trk::alongMomentum, Trk::muon));
            }
        }

        if (!idextrapolatedpar || !idextrapolatedpar->covariance()) {
            ATH_MSG_DEBUG("ID extrapolated par null or missing error matrix, par: " << idextrapolatedpar.get());
            return 0;
        }
        const Trk::TrackParameters *msparforextrapolator = mspar;
        std::unique_ptr<const Trk::TrackParameters> created_mspar;
        if (muonisstraight) {
            const AmgSymMatrix(5) &idcovmat = *idextrapolatedpar->covariance();
            AmgVector(5) params = mspar->parameters();
            params[Trk::qOverP] = idextrapolatedpar->parameters()[Trk::qOverP];
            if (!mspar->covariance()) {
                ATH_MSG_DEBUG("Muons parameters missing Error matrix: " << mspar);
                return 1e5;  // Sometimes it's 0, sometimes 1e15. Maybe for comparison of chi2? Just in case, will copy this
                             // value from earlier check on ms track. EJWM.
            }
            AmgSymMatrix(5) newcovmat = AmgSymMatrix(5)(*mspar->covariance());
            for (int i = 0; i < 5; i++) (newcovmat)(i, 4) = idcovmat(i, 4);
            created_mspar = msparforextrapolator->associatedSurface().createUniqueTrackParameters(params[0], params[1], params[2],
                                                                                                  params[3], params[4], newcovmat);
            msparforextrapolator = created_mspar.get();
        }
        Trk::PropDirection propdir = Trk::oppositeMomentum;
        Trk::DistanceSolution distsol = idextrapolatedpar->associatedSurface().straightLineDistanceEstimate(
            msparforextrapolator->position(), msparforextrapolator->momentum().unit());
        double distance = 0;
        if (distsol.numberOfSolutions() == 1)
            distance = distsol.first();
        else if (distsol.numberOfSolutions() == 2) {
            distance = (std::abs(distsol.first()) < std::abs(distsol.second())) ? distsol.first() : distsol.second();
        }

        if (distance > 0 && distsol.numberOfSolutions() > 0) propdir = Trk::alongMomentum;

        std::unique_ptr<const Trk::TrackParameters> msextrapolatedpar = std::unique_ptr<const Trk::TrackParameters>(
            m_extrapolator->extrapolate(ctx, *msparforextrapolator, idextrapolatedpar->associatedSurface(), propdir, false, Trk::muon));

        if (muonisstraight) { ATH_MSG_DEBUG("Muon track is straight line"); }

        if ((!msextrapolatedpar && !muonisstraight)) {
            ATH_MSG_DEBUG("extrapolation failed, id:" << idextrapolatedpar.get() << " ms: " << msextrapolatedpar.get());
            return 0;
        }
        double mychi2 = 1e15;
        if (msextrapolatedpar) mychi2 = chi2(*idextrapolatedpar, *msextrapolatedpar);
        if (muonisstraight) {
            std::unique_ptr<const Trk::TrackParameters> idpar_firsthit = std::unique_ptr<const Trk::TrackParameters>(
                m_extrapolator->extrapolate(ctx, *idextrapolatedpar, mspar->associatedSurface(), Trk::alongMomentum, false, Trk::muon));
            if (idpar_firsthit) {
                double chi2_2 = chi2(*idpar_firsthit, *mspar);
                if (chi2_2 < mychi2) mychi2 = chi2_2;
            }
        }
        return mychi2;
    }

    double MuonTrackTagTestTool::chi2(const Trk::TrackParameters &idextrapolatedpar, const Trk::TrackParameters &msextrapolatedpar) const {
        double loc1ID = idextrapolatedpar.parameters()[Trk::loc1];
        double loc2ID = idextrapolatedpar.parameters()[Trk::loc2];
        double phiID = idextrapolatedpar.parameters()[Trk::phi];
        double thetaID = idextrapolatedpar.parameters()[Trk::theta];

        double loc1MS = msextrapolatedpar.parameters()[Trk::loc1];
        double loc2MS = msextrapolatedpar.parameters()[Trk::loc2];
        double phiMS = msextrapolatedpar.parameters()[Trk::phi];
        double thetaMS = msextrapolatedpar.parameters()[Trk::theta];

        if (!idextrapolatedpar.covariance() || !msextrapolatedpar.covariance()) {
            ATH_MSG_DEBUG("track parameters don't have error matrix! id: " << idextrapolatedpar.covariance()
                                                                           << " ms: " << msextrapolatedpar.covariance());
            return 1e15;
        }
        const AmgSymMatrix(5) &idcovmat = *idextrapolatedpar.covariance();
        const AmgSymMatrix(5) &mscovmat = *msextrapolatedpar.covariance();

        double loc1diff = std::abs(loc1ID - loc1MS);
        double loc2diff = std::abs(loc2ID - loc2MS);
        const Trk::CylinderSurface *cylsurf = dynamic_cast<const Trk::CylinderSurface *>(&idextrapolatedpar.associatedSurface());
        const Trk::DiscSurface *discsurf = dynamic_cast<const Trk::DiscSurface *>(&idextrapolatedpar.associatedSurface());

        if (cylsurf) {
            double length = 2 * M_PI * cylsurf->bounds().r();
            if (std::abs(loc1diff - length) < loc1diff) loc1diff = length - loc1diff;
        }
        if (discsurf) {
            if (std::abs(loc2diff - 2 * M_PI) < loc2diff) loc2diff = 2 * M_PI - loc2diff;
        }
        double phidiff = std::abs(phiID - phiMS);
        if (std::abs(phidiff - 2 * M_PI) < phidiff) phidiff = 2 * M_PI - phidiff;
        if (std::abs(phidiff - M_PI) < phidiff) phidiff -= M_PI;  // catch singularity in phi near theta=0

        double thetadiff = thetaID - thetaMS;

        double chi2 = loc1diff * loc1diff / (idcovmat(0, 0) + mscovmat(0, 0)) + loc2diff * loc2diff / (idcovmat(1, 1) + mscovmat(1, 1)) +
                      phidiff * phidiff / (idcovmat(2, 2) + mscovmat(2, 2)) + thetadiff * thetadiff / (idcovmat(3, 3) + mscovmat(3, 3));
        chi2 = std::abs(chi2);

        ATH_MSG_DEBUG(" chi2: " << chi2);
        return chi2;
    }
}  // namespace MuonCombined
