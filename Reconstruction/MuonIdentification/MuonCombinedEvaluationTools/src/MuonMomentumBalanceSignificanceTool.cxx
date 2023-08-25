/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonMomentumBalanceSignificanceTool.h"

#include "GaudiKernel/SystemOfUnits.h"
#include "TrkMaterialOnTrack/EnergyLoss.h"
#include "TrkMaterialOnTrack/MaterialEffectsOnTrack.h"
#include "TrkMaterialOnTrack/ScatteringAngles.h"
#include "TrkMeasurementBase/MeasurementBase.h"
#include "TrkTrack/Track.h"
#include "TrkTrack/TrackStateOnSurface.h"

using Gaudi::Units::GeV;

namespace Rec {

    MuonMomentumBalanceSignificanceTool::MuonMomentumBalanceSignificanceTool(const std::string& type, const std::string& name,
                                                                             const IInterface* parent) :
        AthAlgTool(type, name, parent) {
        declareInterface<IMuonMomentumBalanceSignificance>(this);
    }

    double MuonMomentumBalanceSignificanceTool::momentumBalanceSignificance(const xAOD::Muon& muon) const {
        // exclude statistical combination
        if (muon.author() == xAOD::Muon::STACO) return 0.;

        // get combined track
        const Trk::Track* theTrack =
            muon.trackParticle(xAOD::Muon::CombinedTrackParticle) ? muon.trackParticle(xAOD::Muon::CombinedTrackParticle)->track() : nullptr;

        if (theTrack == nullptr) {
            ATH_MSG_DEBUG("No primary author original track for refitted muon, stop calculation...");
            return 0.0;
        } else
            return momentumBalanceSignificance(*theTrack);
    }

    double MuonMomentumBalanceSignificanceTool::momentumBalanceSignificance(const Trk::Track& track) const {
        // find the TSOS carrying caloEnergy (TSOS type CaloDeposit)
        // compare parameters with those from previous TSOS
        double energyBalance = 0.;
        const Trk::EnergyLoss* energyLoss = nullptr;
        const Trk::TrackParameters* previousParameters = nullptr;
        for (Trk::TrackStates::const_iterator s = track.trackStateOnSurfaces()->begin();
             s != track.trackStateOnSurfaces()->end(); ++s) {
            if (!(**s).trackParameters()) continue;
            if ((**s).materialEffectsOnTrack()) {
                const Trk::MaterialEffectsOnTrack* meot = dynamic_cast<const Trk::MaterialEffectsOnTrack*>((**s).materialEffectsOnTrack());
                if (!meot) continue;
                energyLoss = meot->energyLoss();
                if ((**s).type(Trk::TrackStateOnSurface::CaloDeposit) && energyLoss && previousParameters) {
                    energyBalance = previousParameters->momentum().mag() - energyLoss->deltaE() - (**s).trackParameters()->momentum().mag();

                    if (fabs(energyBalance) < energyLoss->sigmaDeltaE() || energyLoss->sigmaMinusDeltaE() <= 0.0 ||
                        energyLoss->sigmaPlusDeltaE() <= 0.0)

                    {
                        ATH_MSG_VERBOSE(std::setiosflags(std::ios::fixed)
                                        << " momentum balance  " << std::setw(6) << std::setprecision(2) << energyBalance / GeV
                                        << "   significance " << std::setw(6) << std::setprecision(1)
                                        << energyBalance / energyLoss->sigmaDeltaE() << "  p before/after calo" << std::setw(7)
                                        << std::setprecision(2) << previousParameters->momentum().mag() / GeV << " /" << std::setw(7)
                                        << std::setprecision(2) << (**s).trackParameters()->momentum().mag() / GeV
                                        << "   energy deposit sigma  " << energyLoss->sigmaDeltaE() / GeV << " GeV");
                        energyBalance /= energyLoss->sigmaDeltaE();
                        if (fabs(energyBalance) > 1.0)
                            ATH_MSG_DEBUG(" momentum balance using symmetric error "
                                          << "as asymmetric error undefined ");
                    } else if (energyBalance < 0.) {
                        ATH_MSG_VERBOSE(std::setiosflags(std::ios::fixed)
                                        << " momentum balance  " << std::setw(6) << std::setprecision(2) << energyBalance / GeV
                                        << "   significance " << std::setw(6) << std::setprecision(1)
                                        << energyBalance / energyLoss->sigmaDeltaE() << "  p before/after calo" << std::setw(7)
                                        << std::setprecision(2) << previousParameters->momentum().mag() / GeV << " /" << std::setw(7)
                                        << std::setprecision(2) << (**s).trackParameters()->momentum().mag() / GeV
                                        << "   energy deposit sigma- " << energyLoss->sigmaMinusDeltaE() / GeV << " GeV");
                        energyBalance /= energyLoss->sigmaMinusDeltaE();
                    } else {
                        ATH_MSG_VERBOSE(std::setiosflags(std::ios::fixed)
                                        << " momentum balance  " << std::setw(6) << std::setprecision(2) << energyBalance / GeV
                                        << "   significance " << std::setw(6) << std::setprecision(1)
                                        << energyBalance / energyLoss->sigmaDeltaE() << "  p before/after calo" << std::setw(7)
                                        << std::setprecision(2) << previousParameters->momentum().mag() / GeV << " /" << std::setw(7)
                                        << std::setprecision(2) << (**s).trackParameters()->momentum().mag() / GeV
                                        << "   energy deposit sigma+ " << energyLoss->sigmaPlusDeltaE() / GeV << " GeV");
                        energyBalance /= energyLoss->sigmaPlusDeltaE();
                    }
                    break;
                }
            }

            // update previous parameters
            previousParameters = (**s).trackParameters();
        }

        return energyBalance;
    }

}  // namespace Rec
