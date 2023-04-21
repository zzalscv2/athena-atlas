/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// MuonPhysValMonitoringTool.cxx
// Implementation file for class MuonPhysValMonitoringTool
// Author: S.Binet<binet@cern.ch>
///////////////////////////////////////////////////////////////////

// PhysVal includes
#include "MuonPhysValMonitoringTool.h"



#include "GaudiKernel/IToolSvc.h"
#include "MuonHistUtils/MuonEnumDefs.h"

#include "xAODBase/IParticleHelpers.h"
#include "xAODCore/ShallowCopy.h"
#include "xAODMuon/MuonAuxContainer.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODMuon/SlowMuonAuxContainer.h"
#include "xAODMuon/SlowMuonContainer.h"
#include "xAODTrigMuon/L2CombinedMuonContainer.h"
#include "xAODTrigMuon/L2StandAloneMuonContainer.h"
#include "xAODTrigger/MuonRoI.h"
#include "xAODTrigger/MuonRoIContainer.h"

#include "xAODTruth/TruthVertexAuxContainer.h"
#include "xAODTruth/TruthVertexContainer.h"
#include "FourMomUtils/xAODP4Helpers.h"


#include "TString.h"
#include <cmath>
#include <limits>


namespace{
    using TrackLink = ElementLink<xAOD::TrackParticleContainer>;
    using MuonLink = ElementLink<xAOD::MuonContainer>;
    using MuonSegmentLink =  ElementLink<xAOD::MuonSegmentContainer>;
    using TruthLink = ElementLink<xAOD::TruthParticleContainer>;

    constexpr int comm_bit = (1<<xAOD::Muon::Commissioning);
}
using namespace xAOD::P4Helpers;

namespace MuonPhysValMonitoring {

    ///////////////////////////////////////////////////////////////////
    // Public methods:
    ///////////////////////////////////////////////////////////////////

    // utilities
    float getMatchingProbability(const xAOD::TrackParticle& trackParticle) {
        float result(std::numeric_limits<float>::quiet_NaN());
        if (trackParticle.isAvailable<float>("truthMatchProbability")) { result = trackParticle.auxdata<float>("truthMatchProbability"); }
        return result;
    }

    // Constructors
    ////////////////

    MuonPhysValMonitoringTool::MuonPhysValMonitoringTool(const std::string& type, const std::string& name, const IInterface* parent) :
        ManagedMonitorToolBase(type, name, parent),
        m_counterBits(),
        m_muonItems(),
        m_L1Seed() {
    }

    // Athena algtool's Hooks
    ////////////////////////////
    StatusCode MuonPhysValMonitoringTool::initialize() {
        ATH_MSG_INFO("Initializing " << name() << "...");
        ATH_CHECK(ManagedMonitorToolBase::initialize());

        if (!m_slowMuonsName.empty()) m_muonsName = m_slowMuonsName;

        for (unsigned int i = 0; i < m_selectHLTMuonItems.size(); i++) {
            if (m_selectHLTMuonItems[i][0] == "" || m_selectHLTMuonItems[i][1] == "") continue;
            m_muonItems.emplace_back(m_selectHLTMuonItems[i][0]);
            m_L1Seed.emplace_back(m_selectHLTMuonItems[i][1]);
        }

        // setup flags
        if (m_doTrigMuonValidation == false) {
            m_doTrigMuonL1Validation = false;
            m_doTrigMuonL2Validation = false;
            m_doTrigMuonEFValidation = false;
        }
        ATH_CHECK(m_trigDec.retrieve(DisableTool{!m_doTrigMuonValidation}));

        ATH_CHECK(m_eventInfo.initialize());
        ATH_CHECK(m_muonSelectionTool.retrieve());
        ATH_CHECK(m_trackSelector.retrieve());
        ATH_CHECK(m_isoTool.retrieve());

        return StatusCode::SUCCESS;
    }
    StatusCode MuonPhysValMonitoringTool::bookHistograms() {
        ATH_MSG_INFO("Booking hists " << name() << "...");

        if (m_selectMuonWPs.size() == 1 && m_selectMuonWPs[0] < 0) m_selectMuonWPs.clear();

        if (m_selectMuonAuthors.size() == 1 && m_selectMuonAuthors[0] == 0) m_selectMuonAuthors.clear();

        static const std::map<int,std::string> theMuonCategories = {
          {ALL, "All"},
          {PROMPT, "Prompt"},
          {INFLIGHT, "InFlight"},
          {NONISO, "NonIsolated"},
          {REST, "Rest"}
        };

        for (const auto& category : m_selectMuonCategories) m_selectMuonCategoriesStr.emplace_back(theMuonCategories.at(category));

        // no such muons in case of SlowMuon reco
        bool separateSAFMuons = m_slowMuonsName.empty();
        
        std::string muonContainerName = m_muonsName;
        for (const auto& category : m_selectMuonCategoriesStr) {
            std::string categoryPath = m_muonsName + "/" + category + "/";
            m_muonValidationPlots.emplace_back(std::make_unique<MuonValidationPlots>(
                nullptr, categoryPath, m_selectMuonWPs, m_selectMuonAuthors, m_isData,
                (category == theMuonCategories.at(ALL) ? false : m_doBinnedResolutionPlots.value()), separateSAFMuons, m_doMuonTree));
            if (!m_slowMuonsName.empty()) m_slowMuonValidationPlots.emplace_back(std::make_unique<SlowMuonValidationPlots>(nullptr, categoryPath, m_isData));
            if (m_doTrigMuonValidation) {
                if (category == "All") {
                    m_TriggerMuonValidationPlots.emplace_back(std::make_unique<TriggerMuonValidationPlots>(
                        nullptr, categoryPath, m_selectMuonAuthors, m_isData, m_doTrigMuonL1Validation, m_doTrigMuonL2Validation,
                        m_doTrigMuonEFValidation, m_selectHLTMuonItems, m_L1MuonItems));
                }
            }
            if (!m_muonTracksName.empty()) {
                m_muonMSTrackValidationPlots.emplace_back(std::make_unique<MuonTrackValidationPlots>(nullptr, categoryPath, "MSTrackParticles", m_isData));
                if (!m_isData)
                    m_oUnmatchedRecoMuonTrackPlots = std::make_unique<Muon::RecoMuonTrackPlotOrganizer>(nullptr, muonContainerName + "/UnmatchedRecoMuonTracks/");
            }
            if (!m_muonExtrapolatedTracksName.empty())
                m_muonMETrackValidationPlots.emplace_back(std::make_unique<MuonTrackValidationPlots>(nullptr, categoryPath, "METrackParticles", m_isData));
            if (!m_muonMSOnlyExtrapolatedTracksName.empty())
                m_muonMSOnlyMETrackValidationPlots.emplace_back(
                    std::make_unique<MuonTrackValidationPlots>(nullptr, categoryPath, "MSOnlyMETrackParticles", m_isData));

            if (!m_tracksName.empty()) {
                m_muonIDTrackValidationPlots.emplace_back(std::make_unique<MuonTrackValidationPlots>(nullptr, categoryPath, "IDTrackParticles", m_isData));
                m_muonIDSelectedTrackValidationPlots.emplace_back(
                    std::make_unique<MuonTrackValidationPlots>(nullptr, categoryPath, "IDSelectedTrackParticles", m_isData));
            }
            if (!m_fwdtracksName.empty())
                m_muonIDForwardTrackValidationPlots.emplace_back(
                    std::make_unique<MuonTrackValidationPlots>(nullptr, categoryPath, "IDForwardTrackParticles", m_isData));

            if (!m_muonSegmentsName.empty()) {
                if (category != theMuonCategories.at(ALL)) continue;  // cannot identify the truth origin of segments...
                m_muonSegmentValidationPlots.emplace_back(std::make_unique<MuonSegmentValidationPlots>(nullptr, categoryPath, m_isData));
                if (!m_isData)
                    m_oUnmatchedRecoMuonSegmentPlots.reset(
                        new Muon::MuonSegmentPlots(nullptr, Form("%s/UnmatchedRecoMuonSegments/", muonContainerName.c_str())));
            }
        }

        if (!m_isData) {
            m_oUnmatchedRecoMuonPlots = std::make_unique<Muon::RecoMuonPlotOrganizer>(nullptr, muonContainerName +"/UnmatchedRecoMuons/");
            m_oUnmatchedTruthMuonPlots = std::make_unique<Muon::TruthMuonPlotOrganizer>(nullptr, muonContainerName +"/UnmatchedTruthMuons/");
        }

        for (const auto& plots : m_muonValidationPlots) bookValidationPlots(*plots).ignore();
        for (const auto& plots : m_slowMuonValidationPlots) bookValidationPlots(*plots).ignore();
        for (const auto& plots : m_TriggerMuonValidationPlots) bookValidationPlots(*plots).ignore();
        for (const auto& plots : m_muonIDTrackValidationPlots) bookValidationPlots(*plots).ignore();
        for (const auto& plots : m_muonIDSelectedTrackValidationPlots) bookValidationPlots(*plots).ignore();
        for (const auto& plots : m_muonIDForwardTrackValidationPlots) bookValidationPlots(*plots).ignore();
        for (const auto& plots : m_muonMSTrackValidationPlots) bookValidationPlots(*plots).ignore();
        for (const auto& plots : m_muonMETrackValidationPlots) bookValidationPlots(*plots).ignore();
        for (const auto& plots : m_muonMSOnlyMETrackValidationPlots) bookValidationPlots(*plots).ignore();
        if (!m_isData) {
            bookValidationPlots(*m_oUnmatchedRecoMuonPlots).ignore();
            bookValidationPlots(*m_oUnmatchedTruthMuonPlots).ignore();
            if (m_oUnmatchedRecoMuonTrackPlots) bookValidationPlots(*m_oUnmatchedRecoMuonTrackPlots).ignore();
            for (const auto& plots : m_muonSegmentValidationPlots) bookValidationPlots(*plots).ignore();
            if (m_oUnmatchedRecoMuonSegmentPlots) bookValidationPlots(*m_oUnmatchedRecoMuonSegmentPlots).ignore();
        }
        // book overview hists
        m_h_overview_Z_mass = new TH1F(Form("%s_Overview_Z_mass", muonContainerName.c_str()), "", 20, 76, 106);
        ATH_CHECK(regHist(m_h_overview_Z_mass, Form("%s/Overview", muonContainerName.c_str()), all));
        m_h_overview_Z_mass_ME = new TH1F(Form("%s_Overview_Z_mass_ME", muonContainerName.c_str()), "", 20, 76, 106);
        ATH_CHECK(regHist(m_h_overview_Z_mass_ME, Form("%s/Overview", muonContainerName.c_str()), all));
        m_h_overview_Z_mass_ID = new TH1F(Form("%s_Overview_Z_mass_ID", muonContainerName.c_str()), "", 20, 76, 106);
        ATH_CHECK(regHist(m_h_overview_Z_mass_ID, Form("%s/Overview", muonContainerName.c_str()), all));

        m_h_overview_nObjects.clear();
        m_h_overview_nObjects.emplace_back(new TH1F(Form("%s_Overview_N_perevent_truth_muons", muonContainerName.c_str()),
                                                 "Number of truth Muons per event", 20, -0.5, 19.5));
        m_h_overview_nObjects.emplace_back(
            new TH1F(Form("%s_Overview_N_perevent_muons", muonContainerName.c_str()), "Number of Muons per event", 20, -0.5, 19.5));
        m_h_overview_nObjects.emplace_back(
            new TH1F(Form("%s_Overview_N_perevent_tracks", muonContainerName.c_str()), "Number of Tracks per event", 50, -0.5, 49.5));
        m_h_overview_nObjects.emplace_back(new TH1F(Form("%s_Overview_N_perevent_truth_segments", muonContainerName.c_str()),
                                                 "Number of truth Segments per event", 200, -0.5, 199.5));
        m_h_overview_nObjects.emplace_back(
            new TH1F(Form("%s_Overview_N_perevent_segments", muonContainerName.c_str()), "Number of Segments per event", 200, -0.5, 199.5));
        for (const auto& hist : m_h_overview_nObjects) {
            if (hist) ATH_CHECK(regHist(hist, Form("%s/Overview", muonContainerName.c_str()), all));
        }

        m_h_overview_reco_category =
            new TH1F(Form("%s_Overview_reco_category", muonContainerName.c_str()), "", 4, 0, 4);  // prompt/in-flight/non-isolated/other
        for (int i = 1; i < 4; i++) {                                                             // skip 'All'
            m_h_overview_reco_category->GetXaxis()->SetBinLabel(i, theMuonCategories.at(i).c_str());
        }
        m_h_overview_reco_category->GetXaxis()->SetBinLabel(4, "Other");  // of some other origin or fakes
        ATH_CHECK(regHist(m_h_overview_reco_category, Form("%s/Overview", muonContainerName.c_str()), all));

        int nAuth = xAOD::Muon::NumberOfMuonAuthors;
        for (int i = 1; i < 4; i++) {
            m_h_overview_reco_authors.emplace_back(new TH1F((m_muonsName + "_" + theMuonCategories.at(i) + "_reco_authors").c_str(),
                                                         (muonContainerName + "_" + theMuonCategories.at(i) + "_reco_authors").c_str(),
                                                         nAuth + 1, -0.5, nAuth + 0.5));
        }
        m_h_overview_reco_authors.emplace_back(new TH1F((m_muonsName + "_Overview_Other_reco_authors").c_str(),
                                                     (muonContainerName + "_Other_reco_authors").c_str(), nAuth + 1, -0.5, nAuth + 0.5));

        for (const auto& hist : m_h_overview_reco_authors) {
            if (hist) ATH_CHECK(regHist(hist, Form("%s/Overview", muonContainerName.c_str()), all));
        }

        return StatusCode::SUCCESS;
    }

    StatusCode MuonPhysValMonitoringTool::bookValidationPlots(PlotBase& valPlots) {
        valPlots.initialize();
        std::vector<HistData> hists = valPlots.retrieveBookedHistograms();

        for (auto& hist : hists) {
            TString sHistName = hist.first->GetName();
            ATH_MSG_VERBOSE("Initializing " << hist.first << " " << sHistName << " " << hist.second << "...");

            // check for histograms that are useless and skip regHist:
            if (sHistName.Contains("momentumPulls")) {
                if (sHistName.Contains(Muon::EnumDefs::toString(xAOD::Muon::MuidSA))) continue;  // empty for standalone muons
                if (!(sHistName.Contains("Prompt") && (sHistName.Contains("AllMuons") || sHistName.Contains("SiAssocForward"))))
                    continue;  // don't need binned eloss plots for separate muon types, keep only for Prompt AllMuons
            }
            if ((sHistName.Contains("resolution") || sHistName.Contains("pulls")) && !sHistName.Contains("Prompt"))
                continue;  // don't need resolution plots except for prompt muons
            if (sHistName.Contains("trigger") && sHistName.Contains("wrt") && sHistName.Contains("Features")) continue;
            modifyHistogram(hist.first);
            ATH_CHECK(regHist(hist.first, hist.second, all));
        }

        // register trees
        std::vector<TreeData> trees = valPlots.retrieveBookedTrees();
        for (auto& tree : trees) {
            std::string sTreeName = tree.first->GetName();
            ATH_MSG_VERBOSE("Initializing " << tree.first << " " << sTreeName << " " << tree.second << "...");
            ATH_CHECK(regTree(tree.first, tree.second, all));
        }

        return StatusCode::SUCCESS;
    }

    StatusCode MuonPhysValMonitoringTool::fillHistograms() {
        ATH_MSG_DEBUG("Filling hists " << name() << "...");
        m_vMatchedTruthMuons.clear();
        m_vMatchedMuons.clear();
        m_vMatchedSlowMuons.clear();
        m_vMatchedMuonTracks.clear();
        m_vMatchedMuonSegments.clear();
        m_vZmumuIDTracks.clear();
        m_vZmumuMETracks.clear();
        m_vZmumuMuons.clear();
        m_vEFMuons.clear();
        m_vEFMuonsSelected.clear();
        m_vL2SAMuons.clear();
        m_vL2SAMuonsSelected.clear();
        m_vL2CBMuons.clear();
        m_vL2CBMuonsSelected.clear();
        m_vRecoMuons.clear();
        m_vRecoMuons_EffDen.clear();
        m_vRecoMuons_EffDen_CB.clear();
        m_vRecoMuons_EffDen_MS.clear();

        SG::ReadHandle<xAOD::EventInfo> eventInfoHandle(m_eventInfo);
        if (!eventInfoHandle.isValid()) {
            ATH_MSG_WARNING("Could not retrieve EventInfo, returning");
            return StatusCode::SUCCESS;
        }
        const xAOD::EventInfo* eventInfo = eventInfoHandle.cptr();
        m_isData = !eventInfo->eventType(xAOD::EventInfo::IS_SIMULATION);
        float beamSpotWeight = eventInfo->beamSpotWeight();

        const xAOD::TruthParticleContainer* TruthMuons(nullptr);

        if (!m_isData) {
            TruthMuons = evtStore()->tryConstRetrieve<xAOD::TruthParticleContainer>(m_muonsTruthName);
            if (!TruthMuons) {
                ATH_MSG_ERROR("Couldn't retrieve TruthMuons container with key: " << m_muonsTruthName);
                return StatusCode::FAILURE;
            }
            ATH_MSG_DEBUG("Retrieved truth muons " << TruthMuons->size());
            m_h_overview_nObjects[0]->Fill(TruthMuons->size(), beamSpotWeight);
        }

        const xAOD::MuonContainer* Muons = nullptr;
        const xAOD::SlowMuonContainer* SlowMuons = nullptr;
        if (!m_slowMuonsName.empty()) {
            SlowMuons = getContainer<xAOD::SlowMuonContainer>(m_slowMuonsName);
            if (!SlowMuons) {
                ATH_MSG_WARNING("Couldn't retrieve SlowMuons container with key: " << m_slowMuonsName);
                return StatusCode::SUCCESS;
            }
            ATH_MSG_DEBUG("Retrieved slow muons " << SlowMuons->size());
            m_h_overview_nObjects[1]->Fill(SlowMuons->size(), beamSpotWeight);
        } else {
            Muons = getContainer<xAOD::MuonContainer>(m_muonsName);
            if (!Muons) { return StatusCode::SUCCESS; }
            ATH_MSG_DEBUG("Retrieved muons " << Muons->size());
            m_h_overview_nObjects[1]->Fill(Muons->size(), beamSpotWeight);
        }

        /////////////////////////////////////////////////////////////////////// @@@
        // @@@ Temp hack to get the MuonSpectrometerTrackParticle (@MS Entry, not extrapolated), needed for eloss plots
        // Remove when the link to the real MuonSpectrometerTrackParticle appears in the xAOD muon
        if (evtStore()->contains<xAOD::TrackParticleContainer>("MuonSpectrometerTrackParticles")) {
            m_MSTracks = getContainer<xAOD::TrackParticleContainer>("MuonSpectrometerTrackParticles");
            if (!m_MSTracks) {
                ATH_MSG_WARNING("Couldn't retrieve MS Tracks container");
                return StatusCode::SUCCESS;
            } else
                ATH_MSG_DEBUG("Retrieved muon tracks " << m_MSTracks->size());
        } else
            ATH_MSG_DEBUG("Couldn't find MS Tracks container");

        /////////////////////////////////////////////////////////////////////// @@@

        // Do resonance selection
        std::vector<std::pair<const xAOD::Muon*, const xAOD::Muon*> > pairs;
        if (Muons) {
            // Use iterator loop to avoid double counting
            for (xAOD::MuonContainer::const_iterator mu1_itr = Muons->begin(); mu1_itr != Muons->end(); ++mu1_itr) {
                const xAOD::Muon* mu1 = (*mu1_itr);
                if (!m_selectComissioning && mu1->allAuthors() & comm_bit) continue;
                for (xAOD::MuonContainer::const_iterator mu2_itr = Muons->begin(); mu2_itr != mu1_itr; ++mu2_itr) {
                    const xAOD::Muon* mu2 = (*mu2_itr);
                    if (!m_selectComissioning && mu2->allAuthors() & comm_bit) continue;                
                    if (mu1->charge() * mu2->charge() >= 0) continue;
                    pairs.emplace_back(std::make_pair(mu1, mu2));
                }
            }
        }

        float dMmin {1e10}, mZ{0.};
        for (std::pair<const xAOD::Muon*, const xAOD::Muon*>& x : pairs) {
            // select best Z
            const TLorentzVector mu1{x.first->p4()}, mu2{x.second->p4()};
            const float M = (mu1 + mu2).M();
            if (M < 66000. || M > 116000.) continue;

            // choose the Z candidate closest to the Z pole - if multiple exist
            float dM = std::abs(M - 91187.);
            if (dM > dMmin) continue;
            dMmin = dM;
            mZ = M;

            m_vZmumuMuons.clear();
            m_vZmumuMuons.emplace_back(x.first);
            m_vZmumuMuons.emplace_back(x.second);
        }

        if (m_vZmumuMuons.size() == 2) {
            m_h_overview_Z_mass->Fill(mZ / 1000., beamSpotWeight);

            const xAOD::TrackParticle* metr1 = m_vZmumuMuons[0]->trackParticle(xAOD::Muon::ExtrapolatedMuonSpectrometerTrackParticle);
            const xAOD::TrackParticle* metr2 = m_vZmumuMuons[1]->trackParticle(xAOD::Muon::ExtrapolatedMuonSpectrometerTrackParticle);
            if (metr1 && metr2) {
                const TLorentzVector mu1ME{metr1->p4()}, mu2ME{metr2->p4()};
                m_h_overview_Z_mass_ME->Fill((mu1ME + mu2ME).M() / 1000., beamSpotWeight);
                if (m_isData) {
                    m_vZmumuMETracks.clear();
                    m_vZmumuMETracks.emplace_back(metr1);
                    m_vZmumuMETracks.emplace_back(metr2);
                }
            }

            const xAOD::TrackParticle* tr1 = m_vZmumuMuons[0]->trackParticle(xAOD::Muon::InnerDetectorTrackParticle);
            const xAOD::TrackParticle* tr2 = m_vZmumuMuons[1]->trackParticle(xAOD::Muon::InnerDetectorTrackParticle);
            if (tr1 && tr2) {
                const TLorentzVector mu1ID{tr1->p4()}, mu2ID{tr2->p4()};
                m_h_overview_Z_mass_ID->Fill((mu1ID + mu2ID).M() / 1000., beamSpotWeight);
                if (m_isData) {
                    m_vZmumuIDTracks.clear();
                    m_vZmumuIDTracks.emplace_back(tr1);
                    m_vZmumuIDTracks.emplace_back(tr2);
                }
            }
        }

        if (!m_isData) {
            for (const auto& truthMu : *TruthMuons) handleTruthMuon(truthMu, beamSpotWeight);
        }

        if (SlowMuons) {
            for (const auto& smu : *SlowMuons) {
                if (!smu) continue;
                const MuonLink link = smu->muonLink();
                if (!link.isValid()) continue;
                handleMuon(*link, smu, beamSpotWeight);
            }
        } else if (Muons) {
            for (const auto& mu : *Muons) handleMuon(mu, nullptr, beamSpotWeight);
        }

        if (m_doMuonTree) { handleMuonTrees(eventInfo, m_isData); }

        if (!m_tracksName.empty()) {
            auto IDTracks = getContainer<xAOD::TrackParticleContainer>(m_tracksName);
            if (!IDTracks) return StatusCode::FAILURE;
            ATH_MSG_DEBUG("handling " << IDTracks->size() << " " << m_tracksName);
            for (const auto& tp : *IDTracks) handleMuonTrack(tp, xAOD::Muon::InnerDetectorTrackParticle, beamSpotWeight);
        }
        if (!m_fwdtracksName.empty()) {
            auto FwdIDTracks = getContainer<xAOD::TrackParticleContainer>(m_fwdtracksName);
            if (!FwdIDTracks) return StatusCode::FAILURE;
            ATH_MSG_DEBUG("handling " << FwdIDTracks->size() << " " << m_fwdtracksName);
            for (const auto& tp : *FwdIDTracks) handleMuonTrack(tp, xAOD::Muon::InnerDetectorTrackParticle, beamSpotWeight);
        }
        if (!m_muonTracksName.empty()) {
            auto MuonTracks = getContainer<xAOD::TrackParticleContainer>(m_muonTracksName);
            if (!MuonTracks) return StatusCode::FAILURE;
            ATH_MSG_DEBUG("handling " << MuonTracks->size() << " " << m_muonTracksName);
            m_h_overview_nObjects[2]->Fill(MuonTracks->size(), beamSpotWeight);
            for (const auto& tp : *MuonTracks) handleMuonTrack(tp, xAOD::Muon::MuonSpectrometerTrackParticle, beamSpotWeight);
        }
        if (!m_muonExtrapolatedTracksName.empty()) {
            auto MuonExtrapolatedTracks = getContainer<xAOD::TrackParticleContainer>(m_muonExtrapolatedTracksName);
            if (!MuonExtrapolatedTracks) return StatusCode::FAILURE;
            ATH_MSG_DEBUG("handling " << MuonExtrapolatedTracks->size() << " " << m_muonExtrapolatedTracksName);
            for (const auto& tp : *MuonExtrapolatedTracks)
                handleMuonTrack(tp, xAOD::Muon::ExtrapolatedMuonSpectrometerTrackParticle, beamSpotWeight);
        }

        if (!m_muonMSOnlyExtrapolatedTracksName.empty() &&
            evtStore()->contains<xAOD::TrackParticleContainer>(m_muonMSOnlyExtrapolatedTracksName)) {
            auto MSOnlyMuonExtrapolatedTracks = getContainer<xAOD::TrackParticleContainer>(m_muonMSOnlyExtrapolatedTracksName);
            if (!MSOnlyMuonExtrapolatedTracks) return StatusCode::FAILURE;
            ATH_MSG_DEBUG("handling " << MSOnlyMuonExtrapolatedTracks->size() << " " << m_muonMSOnlyExtrapolatedTracksName);
            for (const auto& tp : *MSOnlyMuonExtrapolatedTracks)
                handleMuonTrack(tp, xAOD::Muon::MSOnlyExtrapolatedMuonSpectrometerTrackParticle, beamSpotWeight);
        }

        if (!m_muonSegmentsName.empty()) {
            const xAOD::MuonSegmentContainer* TruthMuonSegments(nullptr);
            if (!m_isData) {
                TruthMuonSegments = getContainer<xAOD::MuonSegmentContainer>(m_muonSegmentsTruthName);
                if (!TruthMuonSegments) { return StatusCode::SUCCESS; }
                m_h_overview_nObjects[3]->Fill(TruthMuonSegments->size(), beamSpotWeight);
                ATH_MSG_DEBUG("handling " << TruthMuonSegments->size() << " " << m_muonSegmentsTruthName);
                for (const auto& truthMuSeg : *TruthMuonSegments) handleTruthMuonSegment(truthMuSeg, TruthMuons, beamSpotWeight);
            }

            const xAOD::MuonSegmentContainer* MuonSegments = getContainer<xAOD::MuonSegmentContainer>(m_muonSegmentsName);
            if (!MuonSegments) { return StatusCode::SUCCESS; }
            m_h_overview_nObjects[4]->Fill(MuonSegments->size(), beamSpotWeight);
            ATH_MSG_DEBUG("handling " << MuonSegments->size() << " " << m_muonSegmentsName);
            for (const auto& muSeg : *MuonSegments) handleMuonSegment(muSeg, beamSpotWeight);
        }

        //@@@@@@@@@@@@@@@@ TRIGGER MONITORING IMPLEMENTATION @@@@@@@@@@@@@@@@@
        if (m_doTrigMuonValidation) {            
            auto chainGroups = m_trigDec->getChainGroup("HLT_.*mu.*");
            for (auto& trig : chainGroups->getListOfTriggers()) {
                if (m_trigDec->isPassed(trig, TrigDefs::EF_passedRaw)) {
                    ATH_MSG_DEBUG("Chain " << trig << " is passed: YES");
                } else
                    ATH_MSG_DEBUG("Chain " << trig << " is passed: NO");
            }
            auto L1chainGroups = m_trigDec->getChainGroup("L1_MU.*");
            for (auto& L1trig : L1chainGroups->getListOfTriggers()) {
                if (m_trigDec->isPassed(L1trig, TrigDefs::EF_passedRaw))
                    ATH_MSG_DEBUG("Chain " << L1trig << " is passed: YES");
                else
                    ATH_MSG_DEBUG("Chain " << L1trig << " is passed: NO");
            }
            for (auto mu : m_vRecoMuons) {
                if (passesAcceptanceCuts(mu) && std::abs(mu->eta()) < 2.4) {
                    if (mu->author() == 1) {
                        m_vRecoMuons_EffDen_CB.emplace_back(mu);
                        ATH_MSG_DEBUG("##### m_vRecoMuons_EffDen_CB  pt:" << mu->pt() << "   phi:" << mu->phi() << "   eta:" << mu->eta());
                    } else if (mu->author() == 5) {
                        m_vRecoMuons_EffDen_MS.emplace_back(mu);
                        ATH_MSG_DEBUG("##### m_vRecoMuons_EffDen_MS  pt:" << mu->pt() << "   phi:" << mu->phi() << "   eta:" << mu->eta());
                    }
                }
            }

            //@@@@@ L1 @@@@@
            if (m_doTrigMuonL1Validation) {
                const xAOD::MuonRoIContainer* L1TrigMuons = getContainer<xAOD::MuonRoIContainer>(m_muonL1TrigName);
                if (!L1TrigMuons) { return StatusCode::SUCCESS; }
                ATH_MSG_DEBUG("Retrieved L1 triggered muons " << L1TrigMuons->size());
                for (const auto& TrigL1mu : *L1TrigMuons) handleMuonL1Trigger(TrigL1mu);
            }

            //@@@@@ L2 @@@@@
            if (m_doTrigMuonL2Validation) {
                //@@@@@ L2SA @@@@@
                const xAOD::L2StandAloneMuonContainer* L2SAMuons = getContainer<xAOD::L2StandAloneMuonContainer>(m_muonL2SAName);
                if (!L2SAMuons) { return StatusCode::SUCCESS; }
                ATH_MSG_DEBUG("Retrieved L2 StandAlone triggered muons " << L2SAMuons->size());
                if (L2SAMuons->size() != 0) {
                    for (const auto& L2SAmu : *L2SAMuons) {
                        ATH_MSG_DEBUG("Muon L2SA Trigger: pt " << L2SAmu->pt() << " phi " << L2SAmu->phi() << " eta " << L2SAmu->eta()
                                                               << " roiWord " << L2SAmu->roiWord() << " sAddress " << L2SAmu->sAddress());
                        m_vL2SAMuons.emplace_back(L2SAmu);
                    }
                    for (const auto& mu : m_vL2SAMuons) {
                        if (mu->pt() != 0.) {
                            m_vL2SAMuonsSelected.emplace_back(mu);
                            break;
                        }
                    }
                    for (unsigned int i = 0; i < m_vL2SAMuons.size(); i++) {
                        unsigned int cont = 0;
                        for (unsigned int j = 0; j < m_vL2SAMuonsSelected.size(); j++) {
                            if (((m_vL2SAMuons.at(i)->pt()) != 0.) && ((deltaR(m_vL2SAMuonsSelected.at(j), m_vL2SAMuons.at(i))) > 0.1))
                                cont++;
                            if (cont == m_vL2SAMuonsSelected.size()) {
                                m_vL2SAMuonsSelected.emplace_back(m_vL2SAMuons.at(i));
                                break;
                            }
                        }
                    }
                    for (unsigned int i = 0; i < m_vL2SAMuonsSelected.size(); i++) { handleMuonL2Trigger(m_vL2SAMuonsSelected.at(i)); }
                    L2SATriggerResolution();
                }
                for (const auto& muonItem : m_muonItems) {
                    std::vector<Trig::Feature<xAOD::L2StandAloneMuonContainer> > vec_muons;
                    TString muonItem_str = (TString)muonItem;
                    if (muonItem_str.Contains("_OR_")) {
                        muonItem_str.ReplaceAll("_OR_", " ");
                        TString delim = " ";
                        std::vector<TString> v_subchains;
                        SplitString(muonItem_str, delim, v_subchains);
                        for (int i = 0; i < (int)v_subchains.size(); i++) {
                            Trig::FeatureContainer fc1 = m_trigDec->features((std::string)v_subchains.at(i));
                            std::vector<Trig::Feature<xAOD::L2StandAloneMuonContainer> > vec_muons_1 =
                                fc1.get<xAOD::L2StandAloneMuonContainer>();
                            for (const auto& mufeat : vec_muons_1) { vec_muons.emplace_back(mufeat); }
                        }
                    } else {
                        Trig::FeatureContainer fc = m_trigDec->features(muonItem);
                        vec_muons = fc.get<xAOD::L2StandAloneMuonContainer>();
                    }
                    ATH_MSG_DEBUG("Size of vector Trig::Feature<xAOD::L2StandAloneMuonContainer> for chain " << muonItem << " = "
                                                                                                             << vec_muons.size());
                    for (const auto& mufeat : vec_muons) {
                        ATH_MSG_DEBUG(muonItem << "  vec_muons.size() = " << vec_muons.size()
                                               << "  mufeat.cptr()->size() = " << mufeat.cptr()->size());
                        for (unsigned int i = 0; i < mufeat.cptr()->size(); i++) {
                            ATH_MSG_DEBUG("#####" << muonItem << "  L2SA feature pt: " << (*mufeat.cptr())[i]->pt()
                                                  << "   eta: " << (*mufeat.cptr())[i]->eta() << "   phi: " << (*mufeat.cptr())[i]->phi());
                        }
                    }
                }

                //@@@@@ L2CB @@@@@
                const xAOD::L2CombinedMuonContainer* L2CBMuons = getContainer<xAOD::L2CombinedMuonContainer>(m_muonL2CBName);
                if (!L2CBMuons) { return StatusCode::SUCCESS; }
                ATH_MSG_DEBUG("Retrieved L2 Combined triggered muons " << L2CBMuons->size());
                if (L2CBMuons->size() != 0) {
                    for (const auto& L2CBmu : *L2CBMuons) {
                        ATH_MSG_DEBUG("Muon L2CB Trigger: pt " << L2CBmu->pt() << " phi " << L2CBmu->phi() << " eta " << L2CBmu->eta());
                        m_vL2CBMuons.emplace_back(L2CBmu);
                    }
                    for (unsigned int i = 0; i < m_vL2CBMuons.size(); i++) {
                        if ((m_vL2CBMuons.at(i)->pt()) != 0.) {
                            m_vL2CBMuonsSelected.emplace_back(m_vL2CBMuons.at(i));
                            break;
                        }
                    }
                    for (unsigned int i = 0; i < m_vL2CBMuons.size(); i++) {
                        unsigned int cont = 0;
                        for (unsigned int j = 0; j < m_vL2CBMuonsSelected.size(); j++) {
                            if (((m_vL2CBMuons.at(i)->pt()) != 0.) && ((deltaR(m_vL2CBMuonsSelected.at(j), m_vL2CBMuons.at(i))) > 0.1))
                                cont++;
                            if (cont == m_vL2CBMuonsSelected.size()) {
                                m_vL2CBMuonsSelected.emplace_back(m_vL2CBMuons.at(i));
                                break;
                            }
                        }
                    }
                    for (unsigned int i = 0; i < m_vL2CBMuonsSelected.size(); i++) { handleMuonL2Trigger(m_vL2CBMuonsSelected.at(i)); }
                    L2CBTriggerResolution();
                }
                for (const auto& muonItem : m_muonItems) {
                    std::vector<Trig::Feature<xAOD::L2CombinedMuonContainer> > vec_muons;
                    TString muonItem_str = (TString)muonItem;
                    if (muonItem_str.Contains("_OR_")) {
                        muonItem_str.ReplaceAll("_OR_", " ");
                        TString delim = " ";
                        std::vector<TString> v_subchains;
                        SplitString(muonItem_str, delim, v_subchains);
                        for (int i = 0; i < (int)v_subchains.size(); i++) {
                            Trig::FeatureContainer fc1 = m_trigDec->features((std::string)v_subchains.at(i));
                            std::vector<Trig::Feature<xAOD::L2CombinedMuonContainer> > vec_muons_1 =
                                fc1.get<xAOD::L2CombinedMuonContainer>();
                            for (const auto& mufeat : vec_muons_1) { vec_muons.emplace_back(mufeat); }
                        }
                    } else {
                        Trig::FeatureContainer fc = m_trigDec->features(muonItem);
                        vec_muons = fc.get<xAOD::L2CombinedMuonContainer>();
                    }
                    ATH_MSG_DEBUG("Size of vector Trig::Feature<xAOD::L2CombinedMuonContainer> for chain " << muonItem << " = "
                                                                                                           << vec_muons.size());
                    for (const auto& mufeat : vec_muons) {
                        ATH_MSG_DEBUG(muonItem << "  vec_muons.size() = " << vec_muons.size()
                                               << "  mufeat.cptr()->size() = " << mufeat.cptr()->size());
                        for (unsigned int i = 0; i < mufeat.cptr()->size(); i++) {
                            ATH_MSG_DEBUG("#####" << muonItem << "  L2CB feature pt: " << (*mufeat.cptr())[i]->pt()
                                                  << "   eta: " << (*mufeat.cptr())[i]->eta() << "   phi: " << (*mufeat.cptr())[i]->phi());
                        }
                    }
                }
            }  // close if(m_doTrigMuonL2Validation)

            //@@@@@ EF @@@@@
            if (m_doTrigMuonEFValidation) {
                const xAOD::MuonContainer* EFCombTrigMuons = getContainer<xAOD::MuonContainer>(m_muonEFCombTrigName);
                const xAOD::MuonRoIContainer* L1TrigMuons = getContainer<xAOD::MuonRoIContainer>(m_muonL1TrigName);
                if (!EFCombTrigMuons) { return StatusCode::SUCCESS; }
                ATH_MSG_DEBUG("Retrieved EF triggered muons " << EFCombTrigMuons->size());
                if (EFCombTrigMuons->size() != 0) {
                    for (const auto& Trigmu : *EFCombTrigMuons) {
                        ATH_MSG_DEBUG("Muon EF Trigger: pt " << Trigmu->pt() << " phi " << Trigmu->phi() << " eta " << Trigmu->eta()
                                                             << "   author" << Trigmu->author());
                        m_vEFMuons.emplace_back(Trigmu);
                    }
                    m_vEFMuonsSelected.emplace_back(m_vEFMuons.at(0));
                    for (unsigned int i = 0; i < m_vEFMuons.size(); i++) {
                        unsigned int cont = 0;
                        for (unsigned int j = 0; j < m_vEFMuonsSelected.size(); j++) {
                            if (((deltaR(m_vEFMuonsSelected.at(j), m_vEFMuons.at(i))) > 0.1) ||
                                ((m_vEFMuons.at(i)->author() - m_vEFMuonsSelected.at(j)->author()) != 0))
                                cont++;
                            if (cont == m_vEFMuonsSelected.size()) {
                                m_vEFMuonsSelected.emplace_back(m_vEFMuons.at(i));
                                break;
                            }
                        }
                    }
                    for (unsigned int i = 0; i < m_vEFMuonsSelected.size(); i++) { handleMuonTrigger(m_vEFMuonsSelected.at(i)); }
                    EFTriggerResolution();
                }
                if (!m_isData) {
                    for (const auto& truthMu : *TruthMuons) {
                        ATH_MSG_DEBUG("TRUTH:: pt=" << truthMu->pt() << "  eta=" << truthMu->eta() << "  phi=" << truthMu->phi());
                    }
                }
                // handleMuonTrigger_ResoWRTTruth(m_vEFMuonsSelected,)
                //@@@@@ chains efficiency @@@@@
                for (const auto& muonItem : m_muonItems) {
                    m_vRecoMuons_EffDen = m_vRecoMuons_EffDen_CB;
                    m_SelectedAuthor = 1;
                    if ((muonItem.find("msonly") != std::string::npos)) {
                        m_vRecoMuons_EffDen = m_vRecoMuons_EffDen_MS;
                        m_SelectedAuthor = 5;
                    }
                    std::vector<Trig::Feature<xAOD::MuonContainer> > vec_muons;
                    TString muonItem_str = (TString)muonItem;
                    if (muonItem_str.Contains("_OR_")) {
                        muonItem_str.ReplaceAll("_OR_", " ");
                        TString delim = " ";
                        std::vector<TString> v_subchains;
                        SplitString(muonItem_str, delim, v_subchains);
                        for (int i = 0; i < (int)v_subchains.size(); i++) {
                            Trig::FeatureContainer fc1 = m_trigDec->features((std::string)v_subchains.at(i));
                            std::vector<Trig::Feature<xAOD::MuonContainer> > vec_muons_1 = fc1.get<xAOD::MuonContainer>();
                            for (const auto& mufeat : vec_muons_1) { vec_muons.emplace_back(mufeat); }
                        }
                    } else {
                        Trig::FeatureContainer fc = m_trigDec->features(muonItem);
                        vec_muons = fc.get<xAOD::MuonContainer>();
                    }
                    ATH_MSG_DEBUG("Size of vector Trig::Feature<xAOD::MuonContainer> for chain " << muonItem << " = " << vec_muons.size());
                    for (const auto& mufeat : vec_muons) {
                        ATH_MSG_DEBUG(muonItem << "  vec_muons.size() = " << vec_muons.size()
                                               << "  mufeat.cptr()->size() = " << mufeat.cptr()->size());
                        for (unsigned int i = 0; i < mufeat.cptr()->size(); i++) {
                            ATH_MSG_DEBUG("#####" << muonItem << "  EF feature pt: " << (*mufeat.cptr())[i]->pt()
                                                  << "   eta: " << (*mufeat.cptr())[i]->eta() << "   phi: " << (*mufeat.cptr())[i]->phi()
                                                  << "   author: " << (*mufeat.cptr())[i]->author());
                            for (unsigned int j = 0; j < m_selectMuonCategories.size(); j++) {
                                if (m_selectMuonCategories[j] == ALL) {
                                    if (((*mufeat.cptr())[i]->author()) == m_SelectedAuthor)
                                        m_TriggerMuonValidationPlots[j]->fillFeatPlots(*(*mufeat.cptr())[i], muonItem);
                                }  // if categ=ALL
                            }      // categories
                        }          // mufeat.cptr
                    }              // mufeat
                    for (unsigned int k = 0; k < m_vRecoMuons_EffDen.size(); k++) {
                        bool break_flag = false;
                        for (unsigned int j = 0; j < m_selectMuonCategories.size(); j++) {
                            if (m_selectMuonCategories[j] == ALL) {
                                m_TriggerMuonValidationPlots[j]->fillDenEff(*m_vRecoMuons_EffDen.at(k), muonItem);
                            }
                        }
                        for (const auto& mufeat : vec_muons) {
                            for (unsigned int i = 0; i < mufeat.cptr()->size(); i++) {
                                if ((((*mufeat.cptr())[i]->author()) == m_SelectedAuthor) &&
                                    (deltaR((*mufeat.cptr())[i], m_vRecoMuons_EffDen.at(k)) < 0.1)) {
                                    break_flag = true;
                                    ATH_MSG_DEBUG("   $$$ match Reco_EffDen "
                                                  << muonItem << "         pt: " << m_vRecoMuons_EffDen.at(k)->pt() << "   eta: "
                                                  << m_vRecoMuons_EffDen.at(k)->eta() << "   phi: " << m_vRecoMuons_EffDen.at(k)->phi()
                                                  << "   author: " << m_vRecoMuons_EffDen.at(k)->author());
                                    ATH_MSG_DEBUG("   $$$ match EF MuidCo feature "
                                                  << muonItem << "   pt: " << (*mufeat.cptr())[i]->pt()
                                                  << "   eta: " << (*mufeat.cptr())[i]->eta() << "   phi: " << (*mufeat.cptr())[i]->phi()
                                                  << "   author: " << (*mufeat.cptr())[i]->author() << "    rel_p "
                                                  << (std::abs(((*mufeat.cptr())[i]->pt() - m_vRecoMuons_EffDen.at(k)->pt()) /
                                                               (m_vRecoMuons_EffDen.at(k)->pt()))));
                                    for (unsigned int j = 0; j < m_selectMuonCategories.size(); j++) {
                                        if (m_selectMuonCategories[j] == ALL) {
                                            m_TriggerMuonValidationPlots[j]->fillNumEff(*m_vRecoMuons_EffDen.at(k), muonItem);
                                            // if (muonItem=="HLT_2mu10") m_TriggerMuonValidationPlots[j]->fill(
                                            // *(*mufeat.cptr())[i],*m_vRecoMuons_EffDen.at(k));

                                        }  // if categ=ALL
                                    }      // categories
                                    break;
                                }  // if(Delta_R)
                            }      // mufeat
                            if (break_flag) break;
                        }  // vec_muons
                    }      // m_vRecoMuons_EffDen
                }          // m_muonItems
                //@@@@@ L1 items efficiency @@@@@
                for (const auto& L1MuonItem : m_L1MuonItems) {
                    m_vRecoMuons_EffDen = m_vRecoMuons_EffDen_CB;
                    m_SelectedAuthor = 1;
                    float treshold = 0.;
                    if (L1MuonItem == "L1_MU4") treshold = 4000;
                    else if (L1MuonItem == "L1_MU6") treshold = 6000;
                    else if (L1MuonItem == "L1_MU10") treshold = 10000;
                    else if (L1MuonItem == "L1_MU11") treshold = 11000;
                    else if (L1MuonItem == "L1_MU15") treshold = 15000;
                    else if (L1MuonItem == "L1_MU20") treshold = 20000;
                    for (const auto& TrigL1mu : *L1TrigMuons) {
                        for (unsigned int j = 0; j < m_selectMuonCategories.size(); j++) {
                            if (m_selectMuonCategories[j] == ALL) {
                                if ((TrigL1mu->thrValue()) >= treshold)
                                    m_TriggerMuonValidationPlots[j]->fillFeatPlots(*TrigL1mu, L1MuonItem);
                            }  // if categ=ALL
                        }      // categories
                    }          // L1TrigMuons
                    for (unsigned int k = 0; k < m_vRecoMuons_EffDen.size(); k++) {
                        for (unsigned int j = 0; j < m_selectMuonCategories.size(); j++) {
                            if (m_selectMuonCategories[j] == ALL) {
                                m_TriggerMuonValidationPlots[j]->fillDenL1Eff(*m_vRecoMuons_EffDen.at(k), L1MuonItem);
                            }
                        }
                        for (const auto& TrigL1mu : *L1TrigMuons) {
                            if (((TrigL1mu->thrValue()) >= treshold) &&
                                (sqrt(pow(m_vRecoMuons_EffDen.at(k)->eta() - TrigL1mu->eta(), 2.) +
                                      pow(m_vRecoMuons_EffDen.at(k)->phi() - TrigL1mu->phi(), 2.)) < 0.2)) {
                                ATH_MSG_DEBUG("   $$$ match Reco_EffDen "
                                              << L1MuonItem << "         pt: " << m_vRecoMuons_EffDen.at(k)->pt() << "   eta: "
                                              << m_vRecoMuons_EffDen.at(k)->eta() << "   phi: " << m_vRecoMuons_EffDen.at(k)->phi()
                                              << "   author: " << m_vRecoMuons_EffDen.at(k)->author());
                                ATH_MSG_DEBUG("   $$$ L1 feature " << L1MuonItem << "   pt: " << TrigL1mu->thrValue()
                                                                   << "   eta: " << TrigL1mu->eta() << "   phi: " << TrigL1mu->phi());
                                for (unsigned int j = 0; j < m_selectMuonCategories.size(); j++) {
                                    if (m_selectMuonCategories[j] == ALL) {
                                        m_TriggerMuonValidationPlots[j]->fillNumL1Eff(*m_vRecoMuons_EffDen.at(k), L1MuonItem);
                                    }  // if categ=ALL
                                }      // categories
                                break;
                            }  // if(Delta_R)
                        }      // L1TrigMuons
                    }          // m_vRecoMuons_EffDen
                }
                //@@@@@ chains efficiency w.r.t. L1 @@@@@
                for (unsigned int m = 0; m < m_muonItems.size(); m++) {
                    std::vector<Trig::Feature<xAOD::MuonContainer> > vec_muons;
                    TString muonItem_str = (TString)m_muonItems[m];
                    if (muonItem_str.Contains("_OR_")) {
                        muonItem_str.ReplaceAll("_OR_", " ");
                        TString delim = " ";
                        std::vector<TString> v_subchains;
                        SplitString(muonItem_str, delim, v_subchains);
                        for (int i = 0; i < (int)v_subchains.size(); i++) {
                            Trig::FeatureContainer fc1 = m_trigDec->features((std::string)v_subchains.at(i));
                            std::vector<Trig::Feature<xAOD::MuonContainer> > vec_muons_1 = fc1.get<xAOD::MuonContainer>();
                            for (const auto& mufeat : vec_muons_1) { vec_muons.emplace_back(mufeat); }
                        }
                    } else {
                        Trig::FeatureContainer fc = m_trigDec->features(m_muonItems[m]);
                        vec_muons = fc.get<xAOD::MuonContainer>();
                    }
                    m_vRecoMuons_EffDen = m_vRecoMuons_EffDen_CB;
                    m_SelectedAuthor = 1;
                    if ((m_muonItems[m].find("msonly") != std::string::npos)) {
                        m_vRecoMuons_EffDen = m_vRecoMuons_EffDen_MS;
                        m_SelectedAuthor = 5;
                    }
                    float treshold = 0.;
                    if (m_L1Seed[m] == "L1_MU4") treshold = 4000;
                    if (m_L1Seed[m] == "L1_MU6") treshold = 6000;
                    if (m_L1Seed[m] == "L1_MU10") treshold = 10000;
                    if (m_L1Seed[m] == "L1_MU11") treshold = 11000;
                    if (m_L1Seed[m] == "L1_MU15") treshold = 15000;
                    if (m_L1Seed[m] == "L1_MU20") treshold = 20000;
                    for (unsigned int k = 0; k < m_vRecoMuons_EffDen.size(); k++) {
                        bool break_flag = false;
                        for (const auto& TrigL1mu : *L1TrigMuons) {
                            if (((TrigL1mu->thrValue()) >= treshold) &&
                                (sqrt(pow(m_vRecoMuons_EffDen.at(k)->eta() - TrigL1mu->eta(), 2.) +
                                      pow(m_vRecoMuons_EffDen.at(k)->phi() - TrigL1mu->phi(), 2.)) < 0.2)) {
                                for (unsigned int j = 0; j < m_selectMuonCategories.size(); j++) {
                                    if (m_selectMuonCategories[j] == ALL) {
                                        m_TriggerMuonValidationPlots[j]->fillDenRELEff(*m_vRecoMuons_EffDen.at(k), m_muonItems[m]);
                                    }  // if categ=ALL
                                }      // categories
                                for (const auto& mufeat : vec_muons) {
                                    for (unsigned int i = 0; i < mufeat.cptr()->size(); i++) {
                                        if ((((*mufeat.cptr())[i]->author()) == m_SelectedAuthor) &&
                                            (deltaR((*mufeat.cptr())[i], m_vRecoMuons_EffDen.at(k)) < 0.1)) {
                                            break_flag = true;
                                            for (unsigned int j = 0; j < m_selectMuonCategories.size(); j++) {
                                                if (m_selectMuonCategories[j] == ALL) {
                                                    m_TriggerMuonValidationPlots[j]->fillNumRELEff(*m_vRecoMuons_EffDen.at(k),
                                                                                                   m_muonItems[m]);
                                                }  // if categ=ALL
                                            }      // categories
                                            break;
                                        }  // if(Delta_R)
                                    }      // mufeat
                                    if (break_flag) break;
                                }  // vec_muons
                                break;
                            }  // if(Delta_R)
                        }      // L1TrigMuons
                    }          // m_vRecoMuons_EffDen
                }              // m_muonItems.size()
            }                  // m_doTrigMuonEFValidation
        }
        return StatusCode::SUCCESS;
    }

    void MuonPhysValMonitoringTool::handleTruthMuonSegment(const xAOD::MuonSegment* truthMuSeg,
                                                           const xAOD::TruthParticleContainer* /*muonTruthContainer*/, float weight) {
        const xAOD::MuonSegment* muSeg = findRecoMuonSegment(truthMuSeg);
        ////if (msgLvl(MSG::DEBUG)) printTruthMuonDebug(truthMu, mu);

        unsigned int thisMuonCategory = ALL;  // getMuonSegmentTruthCategory(truthMuSeg, muonTruthContainer) @@@ Does not work...

        for (unsigned int i = 0; i < m_selectMuonCategories.size(); i++) {
            if (m_selectMuonCategories[i] == ALL || m_selectMuonCategories[i] == thisMuonCategory) {
                m_muonSegmentValidationPlots[i]->fill(truthMuSeg, muSeg,
                                                      weight);  // if no reco muon segment is found a protection inside
                                                                // MuonSegmentValidationPlots will ensure, its plots won't be filled
            }
        }
    }

    void MuonPhysValMonitoringTool::handleMuonSegment(const xAOD::MuonSegment* muSeg, float weight) {
        if (!muSeg) {
            ATH_MSG_WARNING("No muon segment found");
            return;
        }
        if (std::find(std::begin(m_vMatchedMuonSegments), std::end(m_vMatchedMuonSegments), muSeg) != std::end(m_vMatchedMuonSegments))
            return;
        if (!m_isData) m_oUnmatchedRecoMuonSegmentPlots->fill(*muSeg, weight);
        for (unsigned int i = 0; i < m_selectMuonCategories.size(); i++) {
            if (m_selectMuonCategories[i] == ALL) {
                m_muonSegmentValidationPlots[i]->fill(muSeg, weight);
                break;
            }
        }
    }

    void MuonPhysValMonitoringTool::handleMuon(const xAOD::Muon* mu, const xAOD::SlowMuon* smu, float weight) {
        if (!mu) return;
        if (!m_selectComissioning && mu->allAuthors() & comm_bit) return;

        if (msgLvl(MSG::DEBUG)) printMuonDebug(mu);

        // make deep copy of muon and decorate with quality
        std::unique_ptr<xAOD::Muon> mu_c;
        try {
            mu_c = getCorrectedMuon(*mu);
        } catch (const SG::ExcBadAuxVar&) {
            ATH_MSG_ERROR("Cannot retrieve aux-item - rejecting muon");
            return;
        }

        if (m_isData) {
            MUCATEGORY thisMuonCategory = ALL;
            // for events with a Zmumu candidate, separate Z muons from the rest:
            if (m_vZmumuMuons.size() > 0) {
                thisMuonCategory = REST;
                if (std::find(m_vZmumuMuons.begin(), m_vZmumuMuons.end(), mu) != m_vZmumuMuons.end()) { thisMuonCategory = PROMPT; }
            }
            // fill plots
            for (unsigned int i = 0; i < m_selectMuonCategories.size(); i++) {
                if (m_selectMuonCategories[i] == ALL or m_selectMuonCategories[i] == thisMuonCategory) {
                  if (mu_c){
                    // histos
                    m_muonValidationPlots[i]->fill(*mu_c, weight);
                    if (smu) m_slowMuonValidationPlots[i]->fill(*smu, *mu_c, weight);
                    // tree branches
                    m_muonValidationPlots[i]->fillTreeBranches(*mu_c);
                  }
                }
            }
        }

        ///////////////////////////////////////////////////////
        // SELECT MUON MEDIUM QUALITY FOR TRIGGER VALIDATION
        xAOD::Muon::Quality my_quality = m_muonSelectionTool->getQuality(*mu_c);
        if (my_quality <= xAOD::Muon::Medium && m_isoTool->accept(*mu_c)) m_vRecoMuons.emplace_back(mu);
        ///////////////////////////////////////////////////////

        if (smu) {
            if (std::find(std::begin(m_vMatchedSlowMuons), std::end(m_vMatchedSlowMuons), smu) != std::end(m_vMatchedSlowMuons)) { return; }
        } else {
            if (std::find(std::begin(m_vMatchedMuons), std::end(m_vMatchedMuons), mu) != std::end(m_vMatchedMuons)) { return; }
        }

        // unmatched reco muons (not matched with any kind of truth particle, fakes)
        if (!m_isData) m_oUnmatchedRecoMuonPlots->fill(*mu_c, weight);

        m_h_overview_reco_category->Fill("Other", weight);
        m_h_overview_reco_authors[3]->Fill(mu->author(), weight);

        for (unsigned int i = 0; i < m_selectMuonCategories.size(); i++) {
            if (m_selectMuonCategories[i] == ALL) {
                if (mu_c){// histos
                  m_muonValidationPlots[i]->fill(*mu_c);
                  if (smu) m_slowMuonValidationPlots[i]->fill(*smu, *mu_c, weight);
                  // tree branches
                  m_muonValidationPlots[i]->fillTreeBranches(*mu_c);
                  break;
                }
            }
        }
    }

    void MuonPhysValMonitoringTool::handleTruthMuon(const xAOD::TruthParticle* truthMu, float weight) {
        const xAOD::SlowMuon* smu = nullptr;
        const xAOD::Muon* mu = nullptr;
        if (!m_slowMuonsName.empty()) {
            smu = findRecoSlowMuon(truthMu);
            if (smu) {
                const MuonLink muLink = smu->muonLink();
                if (muLink.isValid()) mu = *muLink;

                if (!mu) {
                    ATH_MSG_WARNING("Found SlowMuon without valid muon link");
                    smu = nullptr;
                }
            }
        } else {
            mu = findRecoMuon(truthMu);
        }

        if (msgLvl(MSG::DEBUG)) printTruthMuonDebug(truthMu, mu);
        if (!passesAcceptanceCuts(truthMu)) return;

        std::unique_ptr<xAOD::Muon> mu_c;
        if (mu) {
            try {
                mu_c = getCorrectedMuon(*mu);
            } catch (const SG::ExcBadAuxVar&) {
                ATH_MSG_ERROR("Cannot retrieve aux-item - rejecting muon");
                return;
            }
        }

        unsigned int thisMuonCategory = getMuonTruthCategory(truthMu);
        for (unsigned int i = 0; i < m_selectMuonCategories.size(); i++) {
            if (m_selectMuonCategories[i] == ALL || m_selectMuonCategories[i] == thisMuonCategory) {
                // histos
                m_muonValidationPlots[i]->fill(
                    truthMu, mu_c.get(), m_MSTracks,
                    weight);  // if no muon is found a protection inside MuonValidationPlots will ensure, its plots won't be filled
                if (!m_slowMuonsName.empty()) m_slowMuonValidationPlots[i]->fill(truthMu, smu, mu_c.get(), weight);
                // tree branches
                m_muonValidationPlots[i]->fillTreeBranches(truthMu, mu_c.get(), m_MSTracks);
            }
        }
        if (mu_c) {
            m_h_overview_reco_category->Fill(thisMuonCategory - 1, weight);
            m_h_overview_reco_authors[thisMuonCategory - 1]->Fill(mu_c->author(), weight);
        } else if (!m_isData)
            m_oUnmatchedTruthMuonPlots->fill(*truthMu, weight);
    }

    // This method MUST be called after all muon object (reco, truth) related variables (usually, vectors or arrays of vectors) of branches
    // of tree are already initialized. The method fills the event related branches (basic types like int, float, double, etc), fills the
    // TTree object and at the end resets all branch variables for the next event.
    void MuonPhysValMonitoringTool::handleMuonTrees(const xAOD::EventInfo* eventInfo, bool isData) {
        ATH_MSG_DEBUG("Filling MuonTree " << name() << "...");

        for (unsigned int i = 0; i < m_selectMuonCategories.size(); i++) {
            m_muonValidationPlots[i]->fillTree(eventInfo, isData);

            ATH_MSG_DEBUG("MuonTree is filled for muon category = " << m_selectMuonCategories[i] << " with event # "
                                                                    << eventInfo->eventNumber());
        }
    }

    void MuonPhysValMonitoringTool::handleMuonTrack(const xAOD::TrackParticle* tp, xAOD::Muon::TrackParticleType type, float weight) {
        if (!tp) {
            ATH_MSG_WARNING("No track particle found");
            return;
        }

        // if ID track, check that it passes standard combined mu reco selections
        bool passesMuonTrackSel = false;
        if (type == xAOD::Muon::InnerDetectorTrackParticle) { passesMuonTrackSel = m_trackSelector->decision(*tp); }

        if (m_isData) {
            if (type == xAOD::Muon::InnerDetectorTrackParticle) {
                MUCATEGORY thisTrkCategory = ALL;
                // for events with a Zmumu candidate, separate Z muon tracks from the rest:
                if (m_vZmumuIDTracks.size() > 0) {
                    thisTrkCategory = REST;
                    for (const auto& zmu : m_vZmumuIDTracks) {
                        if (deltaR(zmu,  tp) < 0.01) {
                            thisTrkCategory = PROMPT;
                            break;
                        }
                    }
                }

                for (unsigned int i = 0; i < m_selectMuonCategories.size(); i++) {
                    if (m_selectMuonCategories[i] == ALL || m_selectMuonCategories[i] == thisTrkCategory) {
                        m_muonIDTrackValidationPlots[i]->fill(*tp, weight);
                        if (passesMuonTrackSel) m_muonIDSelectedTrackValidationPlots[i]->fill(*tp, weight);
                    }
                }
            } else if (type == xAOD::Muon::MuonSpectrometerTrackParticle)
                m_muonMSTrackValidationPlots[ALL]->fill(*tp, weight);
            else if (type == xAOD::Muon::ExtrapolatedMuonSpectrometerTrackParticle)
                m_muonMETrackValidationPlots[ALL]->fill(*tp, weight);
            else if (type == xAOD::Muon::MSOnlyExtrapolatedMuonSpectrometerTrackParticle)
                m_muonMSOnlyMETrackValidationPlots[ALL]->fill(*tp, weight);

            return;
        }

        TruthLink truthLink;
        if (tp->isAvailable<TruthLink>("truthParticleLink")) { truthLink = tp->auxdata<TruthLink>("truthParticleLink"); }

        // int truthType = tp->isAvailable<int>("truthType")? tp->auxdata< int >("truthType") :0;
        // float truthMatchProb = getMatchingProbability(*tp);

        if (!truthLink.isValid()) {
            ATH_MSG_DEBUG("No truth link available");
            if (type == xAOD::Muon::InnerDetectorTrackParticle) return;
            if (!passesAcceptanceCuts(tp)) return;

            for (unsigned int i = 0; i < m_selectMuonCategories.size(); i++) {
                if (m_selectMuonCategories[i] == ALL) {
                    // ID track plots for any track
                    // if (type==xAOD::Muon::InnerDetectorTrackParticle) {
                    //   m_muonIDTrackValidationPlots[i]->fill(*tp);
                    //   if (passesMuonTrackSel) m_muonIDSelectedTrackValidationPlots[i]->fill(*tp);
                    // } else
                    if (type == xAOD::Muon::MuonSpectrometerTrackParticle) {
                        if (!m_isData) m_oUnmatchedRecoMuonTrackPlots->fill(*tp, weight);
                        m_muonMSTrackValidationPlots[i]->fill(*tp, weight);
                    } else if (type == xAOD::Muon::ExtrapolatedMuonSpectrometerTrackParticle) {
                        m_muonMETrackValidationPlots[i]->fill(*tp, weight);
                    } else if (type == xAOD::Muon::MSOnlyExtrapolatedMuonSpectrometerTrackParticle) {
                        m_muonMSOnlyMETrackValidationPlots[i]->fill(*tp, weight);
                    }
                    break;
                }
            }
        }       // end if no valid truth link
        else {  // here: valid truth link and truthType

            if (type ==
                xAOD::Muon::InnerDetectorTrackParticle) {  // don't fill histograms for any ID track, only for muons; buys a lot of time
                if ((*truthLink)->absPdgId() != 13 || (*truthLink)->status() != 1) return;     // not a muon
                if ((*truthLink)->barcode() == 0 || (*truthLink)->barcode() >= 200e3) return;  // must have valid barcode
            }

            if (!passesAcceptanceCuts(*truthLink)) return;
            unsigned int thisMuonCategory = getMuonTruthCategory(tp);

            for (unsigned int i = 0; i < m_selectMuonCategories.size(); i++) {
                if (m_selectMuonCategories[i] == ALL || m_selectMuonCategories[i] == thisMuonCategory) {
                    if (type == xAOD::Muon::InnerDetectorTrackParticle) {
                        if (!m_fwdtracksName.empty() && std::abs((*truthLink)->eta()) > 2.5)
                            m_muonIDForwardTrackValidationPlots[i]->fill(*truthLink, tp, weight);
                        else if (!m_tracksName.empty()) {
                            m_muonIDTrackValidationPlots[i]->fill(*truthLink, tp, weight);
                            if (passesMuonTrackSel) m_muonIDSelectedTrackValidationPlots[i]->fill(*truthLink, tp, weight);
                        }
                    } else if (type == xAOD::Muon::MuonSpectrometerTrackParticle) {
                        m_muonMSTrackValidationPlots[i]->fill(*truthLink, tp, weight);
                    } else if (type == xAOD::Muon::ExtrapolatedMuonSpectrometerTrackParticle) {
                        m_muonMETrackValidationPlots[i]->fill(*truthLink, tp, weight);
                    } else if (type == xAOD::Muon::MSOnlyExtrapolatedMuonSpectrometerTrackParticle) {
                        m_muonMSOnlyMETrackValidationPlots[i]->fill(*truthLink, tp, weight);
                    }
                }
            }
        }

        return;
    }

    void MuonPhysValMonitoringTool::handleMuonL1Trigger(const xAOD::MuonRoI* TrigL1mu) {
        ATH_MSG_DEBUG("MuonRoI L1 Trigger: ptThr " << TrigL1mu->thrValue() << " phi " << TrigL1mu->phi() << " eta " << TrigL1mu->eta());
        for (unsigned int i = 0; i < m_selectMuonCategories.size(); i++) {
            if (m_selectMuonCategories[i] == ALL) {
                m_TriggerMuonValidationPlots[i]->fill(*TrigL1mu);
                break;
            }
        }
    }

    void MuonPhysValMonitoringTool::handleMuonL2Trigger(const xAOD::L2StandAloneMuon* L2SAMu) {
        for (unsigned int i = 0; i < m_selectMuonCategories.size(); i++) {
            if (m_selectMuonCategories[i] == ALL) {
                m_TriggerMuonValidationPlots[i]->fill(*L2SAMu);
                break;
            }
        }
        ATH_MSG_DEBUG("   ==> Geometrical selection of Muon L2SA Trigger :  pt " << L2SAMu->pt() << " phi " << L2SAMu->phi() << " eta "
                                                                                 << L2SAMu->eta() << " roiWord " << L2SAMu->roiWord()
                                                                                 << " sAddress " << L2SAMu->sAddress());
    }

    void MuonPhysValMonitoringTool::L2SATriggerResolution() {
        int k_L2SAMu_MinDeltaR = -1;
        float MinDeltaR = 0.;
        ATH_MSG_DEBUG(" m_vL2SAMuons.size()" << m_vL2SAMuons.size());
        for (unsigned int i = 0; i < m_vRecoMuons.size(); i++) {
            ATH_MSG_DEBUG(":: TEST: listing all Recomu pt=" << m_vRecoMuons.at(i)->pt() << "  eta=" << m_vRecoMuons.at(i)->eta() << "  phi="
                                                            << m_vRecoMuons.at(i)->phi() << "    auth=" << m_vRecoMuons.at(i)->author());
        }
        for (unsigned int i = 0; i < m_vRecoMuons.size(); i++) {
            if ((m_vRecoMuons.at(i)->author() != 1) || (std::abs(m_vRecoMuons.at(i)->eta()) > 2.4)) continue;
            ATH_MSG_DEBUG(":::: TEST: Recomu pt=" << m_vRecoMuons.at(i)->pt() << "  eta=" << m_vRecoMuons.at(i)->eta()
                                                  << "  phi=" << m_vRecoMuons.at(i)->phi() << "    auth=" << m_vRecoMuons.at(i)->author());
            k_L2SAMu_MinDeltaR = -1;
            MinDeltaR = 1000;
            ATH_MSG_DEBUG("==============>>>>      k_L2SAMu_MinDeltaR=" << k_L2SAMu_MinDeltaR << "    MinDeltaR" << MinDeltaR);
            for (unsigned int k = 0; k < m_vL2SAMuons.size(); k++) {
                ATH_MSG_DEBUG("  :::::::: TEST: L2SA pt=" << m_vL2SAMuons.at(k)->pt() << "  eta=" << m_vL2SAMuons.at(k)->eta()
                                                          << "  phi=" << m_vL2SAMuons.at(k)->phi()
                                                          << "  DeltaR=" << deltaR(m_vRecoMuons.at(i), m_vL2SAMuons.at(k)));
                if ((deltaR(m_vRecoMuons.at(i), m_vL2SAMuons.at(k)) < 0.1 &&
                     (deltaR(m_vRecoMuons.at(i), m_vL2SAMuons.at(k)) < MinDeltaR))) {
                    k_L2SAMu_MinDeltaR = k;
                    MinDeltaR = deltaR(m_vRecoMuons.at(i), m_vL2SAMuons.at(k));
                    ATH_MSG_DEBUG("==============>>>>   taken!!!!     k_L2SAMu_MinDeltaR=" << k_L2SAMu_MinDeltaR << "    MinDeltaR"
                                                                                           << MinDeltaR);
                }
            }
            if (k_L2SAMu_MinDeltaR == -1) continue;
            for (unsigned int c = 0; c < m_selectMuonCategories.size(); c++) {
                if (m_selectMuonCategories[c] == ALL) {
                    m_TriggerMuonValidationPlots[c]->fill(*m_vL2SAMuons.at(k_L2SAMu_MinDeltaR), *m_vRecoMuons.at(i));  /////work in progress
                }
            }
        }
    }

    void MuonPhysValMonitoringTool::handleMuonL2Trigger(const xAOD::L2CombinedMuon* L2CBMu) {
        for (unsigned int i = 0; i < m_selectMuonCategories.size(); i++) {
            if (m_selectMuonCategories[i] == ALL) {
                m_TriggerMuonValidationPlots[i]->fill(*L2CBMu);
                break;
            }
        }
        ATH_MSG_DEBUG("   ==> Geometrical selection of Muon L2CB Trigger :  pt " << L2CBMu->pt() << " phi " << L2CBMu->phi() << " eta "
                                                                                 << L2CBMu->eta());
    }

    void MuonPhysValMonitoringTool::L2CBTriggerResolution() {
        int k_L2CBMu_MinDeltaR = -1;
        float MinDeltaR = 0.;
        ATH_MSG_DEBUG(" m_vL2CBMuons.size()" << m_vL2CBMuons.size());
        for (unsigned int i = 0; i < m_vRecoMuons.size(); i++) {
            ATH_MSG_DEBUG(":: TEST: listing all Recomu pt=" << m_vRecoMuons.at(i)->pt() << "  eta=" << m_vRecoMuons.at(i)->eta() << "  phi="
                                                            << m_vRecoMuons.at(i)->phi() << "    auth=" << m_vRecoMuons.at(i)->author());
        }
        for (unsigned int i = 0; i < m_vRecoMuons.size(); i++) {
            if ((m_vRecoMuons.at(i)->author() != 1) || (std::abs(m_vRecoMuons.at(i)->eta()) > 2.4)) continue;
            ATH_MSG_DEBUG(":::: TEST: Recomu pt=" << m_vRecoMuons.at(i)->pt() << "  eta=" << m_vRecoMuons.at(i)->eta()
                                                  << "  phi=" << m_vRecoMuons.at(i)->phi() << "    auth=" << m_vRecoMuons.at(i)->author());
            k_L2CBMu_MinDeltaR = -1;
            MinDeltaR = 1000;
            ATH_MSG_DEBUG("==============>>>>      k_L2CBMu_MinDeltaR=" << k_L2CBMu_MinDeltaR << "    MinDeltaR" << MinDeltaR);
            for (unsigned int k = 0; k < m_vL2CBMuons.size(); k++) {
                ATH_MSG_DEBUG("  :::::::: TEST: L2CB pt=" << m_vL2CBMuons.at(k)->pt() << "  eta=" << m_vL2CBMuons.at(k)->eta()
                                                          << "  phi=" << m_vL2CBMuons.at(k)->phi()
                                                          << "  DeltaR=" << deltaR(m_vRecoMuons.at(i), m_vL2CBMuons.at(k)));
                if ((deltaR(m_vRecoMuons.at(i), m_vL2CBMuons.at(k)) < 0.1 &&
                     (deltaR(m_vRecoMuons.at(i), m_vL2CBMuons.at(k)) < MinDeltaR))) {
                    k_L2CBMu_MinDeltaR = k;
                    MinDeltaR = deltaR(m_vRecoMuons.at(i), m_vL2CBMuons.at(k));
                    ATH_MSG_DEBUG("==============>>>>   taken!!!!     k_L2CBMu_MinDeltaR=" << k_L2CBMu_MinDeltaR << "    MinDeltaR"
                                                                                           << MinDeltaR);
                }
            }
            if (k_L2CBMu_MinDeltaR == -1) continue;
            for (unsigned int c = 0; c < m_selectMuonCategories.size(); c++) {
                if (m_selectMuonCategories[c] == ALL) {
                    m_TriggerMuonValidationPlots[c]->fill(*m_vL2CBMuons.at(k_L2CBMu_MinDeltaR), *m_vRecoMuons.at(i));  /////work in progress
                }
            }
        }
    }

    void MuonPhysValMonitoringTool::handleMuonTrigger(const xAOD::Muon* EFMu) {
        for (unsigned int i = 0; i < m_selectMuonCategories.size(); i++) {
            if (m_selectMuonCategories[i] == ALL) {
                m_TriggerMuonValidationPlots[i]->fill(*EFMu);
                break;
            }
        }
        ATH_MSG_DEBUG("==> Geometrical selection of EF Trigger muons: pt " << EFMu->pt() << " phi " << EFMu->phi() << " eta " << EFMu->eta()
                                                                           << " author " << EFMu->author());
    }

    void MuonPhysValMonitoringTool::EFTriggerResolution() {
        int k_EFMu_MinDeltaR = -1;
        float MinDeltaR = 0.;
        std::vector<int> vAvailableAuthors;
        vAvailableAuthors.clear();
        vAvailableAuthors.emplace_back(m_vEFMuons[0]->author());
        unsigned int iter = 0;
        for (unsigned int k = 0; k < m_vEFMuons.size(); k++) {
            iter = 0;
            for (unsigned int l = 0; l < vAvailableAuthors.size(); l++) {
                if (m_vEFMuons[k]->author() != vAvailableAuthors[l]) iter++;
            }
            if (iter == vAvailableAuthors.size()) vAvailableAuthors.emplace_back(m_vEFMuons[k]->author());
        }
        ATH_MSG_DEBUG(" m_vEFMuons.size()" << m_vEFMuons.size());
        for (unsigned int i = 0; i < m_vRecoMuons.size(); i++) {
            ATH_MSG_DEBUG(":: TEST: listing all Recomu pt=" << m_vRecoMuons.at(i)->pt() << "  eta=" << m_vRecoMuons.at(i)->eta() << "  phi="
                                                            << m_vRecoMuons.at(i)->phi() << "    auth=" << m_vRecoMuons.at(i)->author());
        }
        for (unsigned int i = 0; i < m_vRecoMuons.size(); i++) {
            if ((m_vRecoMuons.at(i)->author() != 1) || (std::abs(m_vRecoMuons.at(i)->eta()) > 2.4)) continue;
            ATH_MSG_DEBUG(":::: TEST: Recomu pt=" << m_vRecoMuons.at(i)->pt() << "  eta=" << m_vRecoMuons.at(i)->eta()
                                                  << "  phi=" << m_vRecoMuons.at(i)->phi() << "    auth=" << m_vRecoMuons.at(i)->author());
            for (unsigned int l = 0; l < vAvailableAuthors.size(); l++) {
                k_EFMu_MinDeltaR = -1;
                MinDeltaR = 1000;
                for (unsigned int k = 0; k < m_vEFMuons.size(); k++) {
                    ATH_MSG_DEBUG("  :::::::: TEST: EF pt=" << m_vEFMuons.at(k)->pt() << "  eta=" << m_vEFMuons.at(k)->eta()
                                                            << "  phi=" << m_vEFMuons.at(k)->phi()
                                                            << "  DeltaR=" << deltaR(m_vRecoMuons.at(i), m_vEFMuons.at(k))
                                                            << "  author=" << m_vEFMuons.at(k)->author());
                    if (m_vEFMuons.at(k)->author() == vAvailableAuthors.at(l) &&
                        (deltaR(m_vRecoMuons.at(i), m_vEFMuons.at(k)) < 0.1 &&
                         (deltaR(m_vRecoMuons.at(i), m_vEFMuons.at(k)) < MinDeltaR))) {
                        k_EFMu_MinDeltaR = k;
                        MinDeltaR = deltaR(m_vRecoMuons.at(i), m_vEFMuons.at(k));
                    }
                }
                if (k_EFMu_MinDeltaR == -1) continue;
                for (unsigned int c = 0; c < m_selectMuonCategories.size(); c++) {
                    if (m_selectMuonCategories[c] == ALL) {
                        m_TriggerMuonValidationPlots[c]->fill(*m_vEFMuons.at(k_EFMu_MinDeltaR), *m_vRecoMuons.at(i));
                    }
                }
            }
        }
    }

    void MuonPhysValMonitoringTool::printMuonDebug(const xAOD::Muon* mu) {
        const xAOD::TrackParticle* tp = mu->primaryTrackParticle();
        TruthLink truthLink;
        if (tp) {
            if (!tp->isAvailable<TruthLink>("truthParticleLink"))
                ATH_MSG_VERBOSE("No truth link found");
            else
                truthLink = tp->auxdata<TruthLink>("truthParticleLink");
        }
        ATH_MSG_DEBUG("Muon: pt " << mu->pt() << " eta " << mu->eta() << " link " << truthLink.isValid());
    }

    const xAOD::Muon* MuonPhysValMonitoringTool::findRecoMuon(const xAOD::TruthParticle* truthMu) {
        static const SG::AuxElement::ConstAccessor<MuonLink> acc_muon("recoMuonLink");
        if (!acc_muon.isAvailable(*truthMu)) return nullptr;
        MuonLink link = acc_muon(*truthMu);
        if (!link.isValid()) return nullptr;
        const xAOD::Muon* reco_mu = (*link);
        if (!m_selectComissioning && reco_mu->allAuthors() & comm_bit) return nullptr;
        m_vMatchedMuons.emplace_back(reco_mu);
        return reco_mu;
    }

    const xAOD::SlowMuon* MuonPhysValMonitoringTool::findRecoSlowMuon(const xAOD::TruthParticle* truthMu) {
        if (m_slowMuonsName.empty()) return nullptr;

        const xAOD::SlowMuonContainer* SlowMuons = nullptr;
        SlowMuons = getContainer<xAOD::SlowMuonContainer>(m_slowMuonsName);
        for (const auto& smu : *SlowMuons) {
            const MuonLink muLink = smu->muonLink();
            if (!muLink.isValid()) continue;
            float DR = deltaR(*muLink , truthMu);
            if (DR < 0.005) {
                m_vMatchedSlowMuons.emplace_back(smu);
                return smu;
            }
        }

        return nullptr;
    }

    void MuonPhysValMonitoringTool::printTruthMuonDebug(const xAOD::TruthParticle* truthMu, const xAOD::Muon* mu) {
        ATH_MSG_DEBUG("Truth muon: " << truthMu->pt() << " eta " << truthMu->eta());
        if (!mu) return;
        ATH_MSG_DEBUG("Reco muon: " << mu->pt() << " eta " << mu->eta());
    }

    StatusCode MuonPhysValMonitoringTool::procHistograms() {
        ATH_MSG_INFO("Finalising hists " << name() << "...");
        for (const auto& plots : m_muonValidationPlots) plots->finalize();
        for (const auto& plots : m_TriggerMuonValidationPlots) plots->finalize();
        if (!m_isData) {
            m_oUnmatchedRecoMuonPlots->finalize();
            m_oUnmatchedTruthMuonPlots->finalize();
        }

        for (const auto& plots : m_muonMSTrackValidationPlots) plots->finalize();
        for (const auto& plots : m_muonMETrackValidationPlots) plots->finalize();
        for (const auto& plots : m_muonMSOnlyMETrackValidationPlots) plots->finalize();
        for (const auto& plots : m_muonIDTrackValidationPlots) plots->finalize();
        for (const auto& plots : m_muonIDSelectedTrackValidationPlots) plots->finalize();
        for (const auto& plots : m_muonIDForwardTrackValidationPlots) plots->finalize();
        if (!m_isData)
            if (m_oUnmatchedRecoMuonTrackPlots) m_oUnmatchedRecoMuonTrackPlots->finalize();

        for (const auto& plots : m_muonSegmentValidationPlots) plots->finalize();
        if (!m_isData)
            if (m_oUnmatchedRecoMuonSegmentPlots) m_oUnmatchedRecoMuonSegmentPlots->finalize();

        if (m_doMuonTree) {
            for (const auto& plots : m_muonValidationPlots) {
                if (plots->getMuonTree()) { plots->getMuonTree()->getTree()->Write(); }
            }
        }

        return StatusCode::SUCCESS;
    }

    const xAOD::MuonSegment* MuonPhysValMonitoringTool::findRecoMuonSegment(const xAOD::MuonSegment* truthMuSeg) {
        if (!truthMuSeg->isAvailable<MuonSegmentLink>("recoSegmentLink")) {
            ATH_MSG_DEBUG("recoSegmentLink not found");
            return nullptr;
        }
        MuonSegmentLink link = truthMuSeg->auxdata<MuonSegmentLink>("recoSegmentLink");
        if (!link.isValid()) {
            ATH_MSG_DEBUG("recoSegmentLink not valid");
            return nullptr;
        }
        m_vMatchedMuonSegments.emplace_back(*link);
        return (*link);
    }

    TH1F* MuonPhysValMonitoringTool::findHistogram(const std::vector<HistData>& hists, const std::string& hnameTag,
                                                   const std::string& hdirTag, const std::string& hNewName) {
        TH1F* h = nullptr;
        for (const auto& hist : hists) {
            if (hist.second.find(hdirTag) != std::string::npos || hdirTag.empty()) {
                std::string histname = hist.first->GetName();
                if (histname.find(hnameTag) != std::string::npos) {
                    h = (TH1F*)hist.first->Clone(hNewName.c_str());
                    return h;
                }
            }
        }
        return h;
    }

    MuonPhysValMonitoringTool::MUCATEGORY MuonPhysValMonitoringTool::getMuonSegmentTruthCategory(
        const xAOD::MuonSegment* truthMuSeg, const xAOD::TruthParticleContainer* muonTruthContainer) {
        TruthLink truthLink;
        if (truthMuSeg->isAvailable<TruthLink>("truthParticleLink")) {
            truthLink = truthMuSeg->auxdata<TruthLink>("truthParticleLink");
            if (truthLink.isValid()) {
                int theBarcode = (*truthLink)->barcode();
                if (std::abs((*truthLink)->pdgId()) != 13) return REST;

                for (const auto& muTruthPart : *muonTruthContainer) {
                    if (muTruthPart->barcode() == theBarcode) { return getMuonTruthCategory(muTruthPart); }
                }
            }
        } else
            ATH_MSG_WARNING("No truth link available for muon truth segment");

        return REST;
    }

    MuonPhysValMonitoringTool::MUCATEGORY MuonPhysValMonitoringTool::getMuonTruthCategory(const xAOD::IParticle* mu) {
        int truthType = mu->auxdata<int>("truthType");
        if (truthType == 6)
            return PROMPT;
        else if (truthType == 8 && (mu->auxdata<int>("truthOrigin") == 34 || mu->auxdata<int>("truthOrigin") == 35))
            return INFLIGHT;
        else if (truthType == 7)
            return NONISO;
        return REST;
    }

    std::unique_ptr<xAOD::Muon> MuonPhysValMonitoringTool::getCorrectedMuon(const xAOD::Muon& mu) {
        std::unique_ptr<xAOD::Muon> mu_c;
        if (mu.m() <= 0) return mu_c;
        mu_c = std::make_unique<xAOD::Muon>();
        mu_c->makePrivateStore(mu);

        // add decorations too fool the muon selector tool
        const xAOD::TrackParticle* idtrk{mu_c->trackParticle(xAOD::Muon::InnerDetectorTrackParticle)};
        const xAOD::TrackParticle* metrk{mu_c->trackParticle(xAOD::Muon::ExtrapolatedMuonSpectrometerTrackParticle)};
        if (idtrk && metrk) {
            mu_c->auxdecor<float>("InnerDetectorPt") = idtrk->pt();
            mu_c->auxdecor<float>("MuonSpectrometerPt") = metrk->pt();
        }
        m_muonSelectionTool->setQuality(*mu_c);
        m_muonSelectionTool->setPassesHighPtCuts(*mu_c);
        m_muonSelectionTool->setPassesIDCuts(*mu_c);
        return mu_c;
    }

    void MuonPhysValMonitoringTool::modifyHistogram(TH1* hist) {
        std::string histname = hist->GetName();

        if (histname.find("trigger_L1_pt") != std::string::npos) {  // if (histname=="Muons_All_trigger_L1_pt"){
            hist->SetTitle("L1Trigger Muons pt treshold");
            hist->GetXaxis()->SetTitle("L1Trigger Muons pt treshold [GeV]");
            hist->GetXaxis()->Set(30, -0.5, 29.5);
        }
        if (histname.find("trigger_L1_eta_pt") != std::string::npos) {  // if (histname=="Muons_All_trigger_L1_eta_pt") {
            hist->SetTitle("L1Trigger Muons pt treshold vs eta");
            hist->GetYaxis()->Set(90, -0.5, 29.5);
            hist->GetYaxis()->SetTitle("L1Trigger Muons pt treshold [GeV]");
        }

        if (histname.find("trigger") != std::string::npos &&
            ((histname.find("Denom_pt") != std::string::npos) || (histname.find("Numer_pt") != std::string::npos) ||
             (histname.find("Features_pt") != std::string::npos)))
            hist->GetXaxis()->Set(200, 0., 200.);

        if (histname.find("hits") != std::string::npos) {
            if (histname.find("etaLayer2") != std::string::npos)
                hist->GetXaxis()->Set(15, -0.5, 14.5);
            else if (histname.find("etaLayer") != std::string::npos)
                hist->GetXaxis()->Set(11, -0.5, 10.5);
        }

        //////////////
        bool is2D = !(histname.find("_vs_") == std::string::npos);
        //////////////

        if (histname.find("METrackParticles") != std::string::npos) {
            if (histname.find("Res_eta") != std::string::npos) {
                if (is2D)
                    hist->GetYaxis()->Set(50, -0.025, 0.025);
                else
                    hist->GetXaxis()->Set(50, -0.025, 0.025);
            } else if (histname.find("Res_phi") != std::string::npos) {
                if (is2D)
                    hist->GetYaxis()->Set(50, -0.02, 0.02);
                else
                    hist->GetXaxis()->Set(50, -0.02, 0.02);
            }
        } else if (histname.find("MSTrackParticles") != std::string::npos) {
            if (histname.find("Res_eta") != std::string::npos) {
                if (is2D)
                    hist->GetYaxis()->Set(50, -0.025, 0.025);
                else
                    hist->GetXaxis()->Set(50, -0.025, 0.025);
            } else if (histname.find("Res_phi") != std::string::npos) {
                if (is2D)
                    hist->GetYaxis()->Set(50, -0.05, 0.05);
                else
                    hist->GetXaxis()->Set(50, -0.05, 0.05);
            }
        } else {
            if (histname.find("Res_eta") != std::string::npos) {
                if (is2D)
                    hist->GetYaxis()->Set(50, -0.005, 0.005);
                else
                    hist->GetXaxis()->Set(50, -0.005, 0.005);
            } else if (histname.find("Res_phi") != std::string::npos) {
                if (is2D)
                    hist->GetYaxis()->Set(50, -0.002, 0.002);
                else
                    hist->GetXaxis()->Set(50, -0.002, 0.002);
            }
        }

        if (histname.find("trigger") != std::string::npos) {
            // RESO EF - MC
            if ((!m_isData) && histname.find("MuidCo") != std::string::npos &&
                (histname.find("BARREL") != std::string::npos || histname.find("WHOLE_DETECT") != std::string::npos)) {
                if (histname.find("Res_pT") != std::string::npos && histname.find("_vs_") == std::string::npos) {
                    hist->GetXaxis()->Set(100, -0.04, 0.04);
                }
                if (histname.find("Res_pT_vs_") != std::string::npos) { hist->GetYaxis()->Set(100, -0.04, 0.04); }
                if (histname.find("Res_eta") != std::string::npos && histname.find("_vs_") == std::string::npos) {
                    hist->GetXaxis()->Set(100, -0.0005, 0.0005);
                }
                if (histname.find("Res_eta_vs_") != std::string::npos) { hist->GetYaxis()->Set(100, -0.0005, 0.0005); }

                if (histname.find("Res_phi") != std::string::npos && histname.find("_vs_") == std::string::npos) {
                    hist->GetXaxis()->Set(100, -0.0002, 0.0002);
                }
                if (histname.find("Res_phi_vs_") != std::string::npos) { hist->GetYaxis()->Set(100, -0.0002, 0.0002); }
            }
            if ((!m_isData) && histname.find("MuidCo") != std::string::npos && (histname.find("ENDCAPS") != std::string::npos)) {
                if (histname.find("Res_pT") != std::string::npos && histname.find("_vs_") == std::string::npos) {
                    hist->GetXaxis()->Set(100, -0.05, 0.05);
                }
                if (histname.find("Res_pT_vs_") != std::string::npos) { hist->GetYaxis()->Set(100, -0.05, 0.05); }
                if (histname.find("Res_eta") != std::string::npos && histname.find("_vs_") == std::string::npos) {
                    hist->GetXaxis()->Set(100, -0.001, 0.001);
                }
                if (histname.find("Res_eta_vs_") != std::string::npos) { hist->GetYaxis()->Set(100, -0.001, 0.001); }

                if (histname.find("Res_phi") != std::string::npos && histname.find("_vs_") == std::string::npos) {
                    hist->GetXaxis()->Set(100, -0.0003, 0.0003);
                }
                if (histname.find("Res_phi_vs_") != std::string::npos) { hist->GetYaxis()->Set(100, -0.0003, 0.0003); }
            }
            if ((!m_isData) && histname.find("MuidSA") != std::string::npos) {
                if (histname.find("Res_eta") != std::string::npos && histname.find("_vs_") == std::string::npos) {
                    hist->GetXaxis()->Set(100, -0.03, 0.03);
                }
                if (histname.find("Res_eta_vs_") != std::string::npos) { hist->GetYaxis()->Set(100, -0.03, 0.03); }

                if (histname.find("Res_phi") != std::string::npos && histname.find("_vs_") == std::string::npos) {
                    hist->GetXaxis()->Set(100, -0.03, 0.03);
                }
                if (histname.find("Res_phi_vs_") != std::string::npos) { hist->GetYaxis()->Set(100, -0.03, 0.03); }
            }
            if ((!m_isData) && histname.find("MuidSA") != std::string::npos && (histname.find("BARREL") != std::string::npos)) {
                if (histname.find("Res_pT") != std::string::npos && histname.find("_vs_") == std::string::npos) {
                    hist->GetXaxis()->Set(100, -0.15, 0.15);
                }
                if (histname.find("Res_pT_vs_") != std::string::npos) { hist->GetYaxis()->Set(100, -0.15, 0.15); }
            }
            if ((!m_isData) && histname.find("MuidSA") != std::string::npos &&
                (histname.find("ENDCAPS") != std::string::npos || histname.find("WHOLE_DETECT") != std::string::npos)) {
                if (histname.find("Res_pT") != std::string::npos && histname.find("_vs_") == std::string::npos) {
                    hist->GetXaxis()->Set(100, -0.2, 0.2);
                }
                if (histname.find("Res_pT_vs_") != std::string::npos) { hist->GetYaxis()->Set(100, -0.2, 0.2); }
            }

            // RESO EF - DATA
            if ((m_isData) && histname.find("MuidCo") != std::string::npos &&
                (histname.find("BARREL") != std::string::npos || histname.find("WHOLE_DETECT") != std::string::npos)) {
                if (histname.find("Res_pT") != std::string::npos && histname.find("_vs_") == std::string::npos) {
                    hist->GetXaxis()->Set(100, -0.06, 0.06);
                }
                if (histname.find("Res_pT_vs_") != std::string::npos) { hist->GetYaxis()->Set(100, -0.06, 0.06); }
                if (histname.find("Res_eta") != std::string::npos && histname.find("_vs_") == std::string::npos) {
                    hist->GetXaxis()->Set(100, -0.001, 0.001);
                }
                if (histname.find("Res_eta_vs_") != std::string::npos) { hist->GetYaxis()->Set(100, -0.001, 0.001); }

                if (histname.find("Res_phi") != std::string::npos && histname.find("_vs_") == std::string::npos) {
                    hist->GetXaxis()->Set(100, -0.0005, 0.0005);
                }
                if (histname.find("Res_phi_vs_") != std::string::npos) { hist->GetYaxis()->Set(100, -0.0005, 0.0005); }
            }
            if ((m_isData) && histname.find("MuidCo") != std::string::npos && (histname.find("ENDCAPS") != std::string::npos)) {
                if (histname.find("Res_pT") != std::string::npos && histname.find("_vs_") == std::string::npos) {
                    hist->GetXaxis()->Set(100, -0.1, 0.1);
                }
                if (histname.find("Res_pT_vs_") != std::string::npos) { hist->GetYaxis()->Set(100, -0.1, 0.1); }
                if (histname.find("Res_eta") != std::string::npos && histname.find("_vs_") == std::string::npos) {
                    hist->GetXaxis()->Set(100, -0.0015, 0.0015);
                }
                if (histname.find("Res_eta_vs_") != std::string::npos) { hist->GetYaxis()->Set(100, -0.0015, 0.0015); }

                if (histname.find("Res_phi") != std::string::npos && histname.find("_vs_") == std::string::npos) {
                    hist->GetXaxis()->Set(100, -0.0005, 0.0005);
                }
                if (histname.find("Res_phi_vs_") != std::string::npos) { hist->GetYaxis()->Set(100, -0.0005, 0.0005); }
            }
            if ((m_isData) && histname.find("MuidSA") != std::string::npos) {
                if (histname.find("Res_eta") != std::string::npos && histname.find("_vs_") == std::string::npos) {
                    hist->GetXaxis()->Set(100, -0.03, 0.03);
                }
                if (histname.find("Res_eta_vs_") != std::string::npos) { hist->GetYaxis()->Set(100, -0.03, 0.03); }

                if (histname.find("Res_phi") != std::string::npos && histname.find("_vs_") == std::string::npos) {
                    hist->GetXaxis()->Set(100, -0.03, 0.03);
                }
                if (histname.find("Res_phi_vs_") != std::string::npos) { hist->GetYaxis()->Set(100, -0.03, 0.03); }
            }
            if ((m_isData) && histname.find("MuidSA") != std::string::npos && (histname.find("BARREL") != std::string::npos)) {
                if (histname.find("Res_pT") != std::string::npos && histname.find("_vs_") == std::string::npos) {
                    hist->GetXaxis()->Set(100, -0.3, 0.3);
                }
                if (histname.find("Res_pT_vs_") != std::string::npos) { hist->GetYaxis()->Set(100, -0.3, 0.3); }
            }
            if ((m_isData) && histname.find("MuidSA") != std::string::npos &&
                (histname.find("ENDCAPS") != std::string::npos || histname.find("WHOLE_DETECT") != std::string::npos)) {
                if (histname.find("Res_pT") != std::string::npos && histname.find("_vs_") == std::string::npos) {
                    hist->GetXaxis()->Set(100, -0.5, 0.5);
                }
                if (histname.find("Res_pT_vs_") != std::string::npos) { hist->GetYaxis()->Set(100, -0.5, 0.5); }
            }

            // LEVEL2
            if ((histname.find("L2_StandAlone") != std::string::npos)) {
                if (histname.find("Res_eta") != std::string::npos && histname.find("_vs_") == std::string::npos) {
                    hist->GetXaxis()->Set(100, -0.03, 0.03);
                }
                if (histname.find("Res_eta_vs_") != std::string::npos) { hist->GetYaxis()->Set(100, -0.03, 0.03); }

                if (histname.find("Res_phi") != std::string::npos && histname.find("_vs_") == std::string::npos) {
                    hist->GetXaxis()->Set(100, -0.03, 0.03);
                }
                if (histname.find("Res_phi_vs_") != std::string::npos) { hist->GetYaxis()->Set(100, -0.03, 0.03); }
            }
            if ((histname.find("L2_StandAlone") != std::string::npos) && (histname.find("BARREL") != std::string::npos)) {
                if (histname.find("Res_pT") != std::string::npos && histname.find("_vs_") == std::string::npos) {
                    hist->GetXaxis()->Set(100, -0.3, 0.3);
                }
                if (histname.find("Res_pT_vs_") != std::string::npos) { hist->GetYaxis()->Set(100, -0.3, 0.3); }
            }
            if ((histname.find("L2_StandAlone") != std::string::npos) &&
                ((histname.find("ENDCAPS") != std::string::npos) || (histname.find("WHOLE_DETECT") != std::string::npos))) {
                if (histname.find("Res_pT") != std::string::npos && histname.find("_vs_") == std::string::npos) {
                    hist->GetXaxis()->Set(100, -0.5, 0.5);
                }
                if (histname.find("Res_pT_vs_") != std::string::npos) { hist->GetYaxis()->Set(100, -0.5, 0.5); }
            }

            if ((histname.find("L2_Combined") != std::string::npos)) {
                if (histname.find("Res_eta") != std::string::npos && histname.find("_vs_") == std::string::npos) {
                    hist->GetXaxis()->Set(100, -0.002, 0.002);
                }
                if (histname.find("Res_eta_vs_") != std::string::npos) { hist->GetYaxis()->Set(100, -0.002, 0.002); }

                if (histname.find("Res_phi") != std::string::npos && histname.find("_vs_") == std::string::npos) {
                    hist->GetXaxis()->Set(100, -0.001, 0.001);
                }
                if (histname.find("Res_phi_vs_") != std::string::npos) { hist->GetYaxis()->Set(100, -0.001, 0.001); }
                if (histname.find("Res_pT") != std::string::npos && histname.find("_vs_") == std::string::npos) {
                    hist->GetXaxis()->Set(100, -0.2, 0.2);
                }
                if (histname.find("Res_pT_vs_") != std::string::npos) { hist->GetYaxis()->Set(100, -0.2, 0.2); }
            }
        }
    }

    bool MuonPhysValMonitoringTool::passesAcceptanceCuts(const xAOD::IParticle* prt) {
        if (prt->pt() < 2000.) return false;
        if (std::abs(prt->eta()) > 2.7) return false;
        return true;
    }

    void MuonPhysValMonitoringTool::SplitString(TString x, const TString& delim, std::vector<TString>& v) {
        v.clear();
        int stringLength = x.Length();
        int delimLength = delim.Length();

        int stop = 1;
        TString temp = "---";
        while (stop != -1) {
            stop = x.First(delim);

            if (stop != -1) {
                temp = x(0, stop);
                TSubString newString = x(stop + delimLength, stringLength);
                x = newString;
                stringLength = x.Length();
            } else {
                stringLength = x.Length();
                temp = x(0, stringLength);
            }

            v.emplace_back(temp);
        }
    }

}  // namespace MuonPhysValMonitoring
