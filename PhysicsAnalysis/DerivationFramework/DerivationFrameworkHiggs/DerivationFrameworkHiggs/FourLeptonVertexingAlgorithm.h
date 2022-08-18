/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#ifndef DERIVATIONFRAMEWORK_FourLeptonVertexingAlgorithm_H
#define DERIVATIONFRAMEWORK_FourLeptonVertexingAlgorithm_H

// Gaudi & Athena basics
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "EgammaAnalysisInterfaces/IAsgElectronLikelihoodTool.h"
#include "MuonAnalysisInterfaces/IMuonSelectionTool.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "TrkVertexFitterInterfaces/IVertexFitter.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODEventInfo/EventInfo.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODTracking/VertexContainer.h"

/**
 * @brief Algorithm that refits all possible four lepton combinations to a common space point.
 *        Leptons are selected by exploting the common CP tools / Loose cut on the delta Z0
 *        of the associated tracks is applied to avoid hopeless fits
 *
 */
namespace DerivationFramework {
    class FourLeptonVertexingAlgorithm : public AthReentrantAlgorithm {
    public:
        FourLeptonVertexingAlgorithm(const std::string& n, ISvcLocator* p);
        StatusCode initialize() override;
        StatusCode execute(const EventContext& ctx) const override;

        using LeptonQuadruplet = std::array<const xAOD::IParticle*, 4>;

    private:
        bool passSelection(const xAOD::Electron* elec) const;
        bool passSelection(const xAOD::Muon* muon) const;
        bool passSelection(const LeptonQuadruplet& quad) const;

        std::vector<LeptonQuadruplet> buildAllQuadruplets(const EventContext& ctx) const;

        std::unique_ptr<xAOD::Vertex> fitQuadruplet(const EventContext& ctx, const LeptonQuadruplet& quad) const;

        const xAOD::TrackParticle* trackParticle(const xAOD::IParticle* part) const;

        /// Input containers
        SG::ReadHandleKey<xAOD::MuonContainer> m_muonKey{this, "MuonContainer", "Muons",
                                                         "Location of the input muon container in the StoreGate"};
        SG::ReadHandleKey<xAOD::ElectronContainer> m_elecKey{this, "ElectronContainer", "Electrons",
                                                             "Location of the electron container in the store gate"};
        SG::ReadHandleKey<xAOD::EventInfo> m_evtKey{this, "EventInfoKey", "EventInfo"};

        ToolHandle<CP::IMuonSelectionTool> m_muonSelTool{this, "MuonSelectionTool", "",
                                                         "Applied preselection on the muons. Will be ignored if empty"};
        ToolHandle<IAsgElectronLikelihoodTool> m_elecSelTool{this, "ElectronSelectionTool", "",
                                                             "Applied preselection on the electrons. Will be ignored if empty"};

        ToolHandle<Trk::IVertexFitter> m_fitter{this, "VertexFitter", "", "Vertex fitter tool"};

        Gaudi::Property<float> m_minMuonPt{this, "MinMuonPt", 3. * Gaudi::Units::GeV,
                                           "Minimum pt cut applied on the muon -- Unfortunately not handled by the MST"};
        Gaudi::Property<float> m_minElecPt{this, "MinElecPt", 4.5 * Gaudi::Units::GeV, " Minimum pt cut applied on the electron"};

        using MuonTrk = xAOD::Muon::TrackParticleType;
        Gaudi::Property<int> m_muonTrkProp{this, "PickMuonTrk", MuonTrk::InnerDetectorTrackParticle,
                                           "Pick the proper track particle from the muon"};
        MuonTrk m_muonTrk{MuonTrk::Primary};

        Gaudi::Property<bool> m_elecUseGSF{this, "PickElecGsfTrk", true,
                                           "If this property is set to true it will pick up the electron GSF track"};

        Gaudi::Property<float> m_z0Cut{this, "DeltaZ0Cut", 5. * Gaudi::Units::GeV,
                                       "All leptons in the quadruplet have to be seperated from each other by maximum"};

        SG::WriteHandleKey<xAOD::VertexContainer> m_vtxKey{this, "OutVertexContainer", "FourLeptonVertices", "Output vertex container"};
    };

}  // namespace DerivationFramework
#endif
