/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IsoCloseByCorrectionTrkSelAlg_H
#define IsoCloseByCorrectionTrkSelAlg_H

// Gaudi & Athena basics
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AthContainers/ConstDataVector.h"
#include "CxxUtils/checker_macros.h"
#include "EgammaAnalysisInterfaces/IAsgElectronLikelihoodTool.h"
#include "EgammaAnalysisInterfaces/IAsgPhotonIsEMSelector.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "InDetTrackSelectionTool/IInDetTrackSelectionTool.h"
#include "IsolationSelection/IIsolationCloseByCorrectionTool.h"
#include "MuonAnalysisInterfaces/IMuonSelectionTool.h"
#include "StoreGate/ReadDecorHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ThinningHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODEgamma/PhotonContainer.h"
#include "xAODMuon/MuonContainer.h"
/**
 * @brief The algorithm selects Inner detector tracks from leptons that are poluting the isolation of
 *        other close-by leptons. The tracks can be either written to a VIEW_ELEMENTS container or
 *        marked as passed for the track particle thinning of a particular derivation *
 */

namespace CP {

    class IsoCloseByCorrectionTrkSelAlg : public AthReentrantAlgorithm {
    public:
        IsoCloseByCorrectionTrkSelAlg(const std::string& name, ISvcLocator* svcLoc);

        StatusCode execute(const EventContext& ctx) const override;
        StatusCode initialize() override;
        StatusCode finalize() override;

    private:
        bool passSelection(const EventContext& ctx, const xAOD::Electron* elec) const;
        bool passSelection(const EventContext& ctx, const xAOD::Photon* phot) const;
        bool passSelection(const EventContext& ctx, const xAOD::Muon* muon) const;
        using LepContainer = std::set<const xAOD::IParticle*>;
        template <class CONT_TYPE>
        StatusCode loadTracks(const EventContext& ctx, const SG::ReadHandleKey<CONT_TYPE>& key, TrackSet& tracks,
                              LepContainer& prim_objs) const;

        /// Input containers to retrieve from the storegate
        SG::ReadHandleKey<xAOD::MuonContainer> m_muonKey{this, "MuonContainer", "Muons"};
        SG::ReadHandleKey<xAOD::ElectronContainer> m_elecKey{this, "EleContainer", "Electrons"};
        SG::ReadHandleKey<xAOD::PhotonContainer> m_photKey{this, "PhotContainer", "Photons"};

        /// External selection criteria
        Gaudi::Property<std::string> m_selDecoration{this, "SelectionDecorator", "",
                                                     "Optional char decorator flag that the leptons have to pass in order to be selected"};
        /// The keys serve to declare the data dependency on the optional selection decorator properly. The key is overwritten if the
        /// decorator is set
        SG::ReadDecorHandleKey<xAOD::MuonContainer> m_mounSelKey{this, "MuonSelectionKey", ""};
        SG::ReadDecorHandleKey<xAOD::ElectronContainer> m_elecSelKey{this, "ElecSelectionKey", ""};
        SG::ReadDecorHandleKey<xAOD::PhotonContainer> m_photSelKey{this, "PhotSelectionKey", ""};
        /// Optionally the user can also parse the elec / muon / photon selection tools
        ToolHandle<CP::IMuonSelectionTool> m_muonSelTool{this, "MuonSelectionTool", ""};
        ToolHandle<IAsgElectronLikelihoodTool> m_elecSelTool{this, "ElectronSelectionTool", ""};
        ToolHandle<IAsgPhotonIsEMSelector> m_photSelTool{this, "PhotonSelectionTool", ""};

        /// These tools shall be configured to pick up the same Inner detector tracks as for the isolation building
        ToolHandle<CP::IIsolationCloseByCorrectionTool> m_closeByCorrTool{this, "IsoCloseByCorrectionTool", "",
                                                                          "The isolation close by correction tool."};

        /// Kinematic cuts. The selection tools do not support kinematic cut selection unfortunately
        Gaudi::Property<float> m_minElecPt{this, "MinElecPt", 4.5 * Gaudi::Units::GeV,
                                           "Minimum pt cut that the electron needs to pass in order to be selected"};
        Gaudi::Property<float> m_minMuonPt{this, "MinMuonPt", 3. * Gaudi::Units::GeV,
                                           "Minimum pt cut that the muon needs to pass in order to be selected"};
        Gaudi::Property<float> m_minPhotPt{this, "MinPhotPt", 25. * Gaudi::Units::GeV,
                                           "Minimum pt cut that the photon needs to pass in order to be selected"};

        Gaudi::Property<float> m_maxConeSize{this, "ConeSize", 0.3};

        /// Output stream to be used for the thinning decision
        Gaudi::Property<std::string> m_stream{this, "OutputStream", "", "Stream"};
        SG::ThinningHandleKey<xAOD::TrackParticleContainer> m_thinKey{this, "ThinninKey", "InDetTrackParticles",
                                                                      "Apply the thinning decision to. Decision is set to OR"};
        /// Optionally the user can also dump a TrackParticleContainer containing all the tracks entring the cone 0.3
        SG::WriteHandleKey<ConstDataVector<xAOD::TrackParticleContainer>> m_trkKey{this, "OutContainerKey", "",
                                                                                   "The associated track particles can be written to an outpt container"};

        /// Array counting the number of accepted tracks per object type
        ///    muon[0], electron[1], photon[2]
        mutable std::array<std::atomic<Long64_t>, 3> m_accepted_trks ATLAS_THREAD_SAFE;
        mutable std::array<std::atomic<Long64_t>, 3> m_selected_obj ATLAS_THREAD_SAFE;
        /// Total track counter
        mutable std::atomic<Long64_t> m_tot_trks{};
    };
}  // namespace CP
#endif
