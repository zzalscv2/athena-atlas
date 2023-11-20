/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonSelectorTools/MuonSelectionTool.h"

#include "AsgDataHandles/ReadHandle.h"
#include "PathResolver/PathResolver.h"
#include "xAODTracking/TrackingPrimitives.h"

namespace {
    static constexpr double const MeVtoGeV = 1. / 1000.;

    // This function defines the order of chamber indices for the low-pT MVA,
    // i.e. defining the meaning of "the first two segments", which are used in the BDT
    std::vector<int> initializeChamberIdxOrder() {
        // This vector defines the order. The current order follows the order of the enum "ChIndex"
        // except for the CSCs, which appear first. Since the order is not strictly innermost-to-outermost,
        // a reordering could be considered for a rel. 22 retuning, which can then easily be achieved by
        // swapping around the elements in the below initialization.
        const std::vector<int> chamberIndicesOrdered = {
            Muon::MuonStationIndex::CSS, Muon::MuonStationIndex::CSL, Muon::MuonStationIndex::BIS, Muon::MuonStationIndex::BIL,
            Muon::MuonStationIndex::BMS, Muon::MuonStationIndex::BML, Muon::MuonStationIndex::BOS, Muon::MuonStationIndex::BOL,
            Muon::MuonStationIndex::BEE, Muon::MuonStationIndex::EIS, Muon::MuonStationIndex::EIL, Muon::MuonStationIndex::EMS,
            Muon::MuonStationIndex::EML, Muon::MuonStationIndex::EOS, Muon::MuonStationIndex::EOL, Muon::MuonStationIndex::EES,
            Muon::MuonStationIndex::EEL};

        // This vector will hold the equivalent information in a form that can be efficiently accessed in the
        // below function "chamberIndexCompare", using the chamber index as the vector index
        std::vector<int> chamberIndexOrder(Muon::MuonStationIndex::ChIndexMax);

        for (unsigned int i = 0; i < chamberIndicesOrdered.size(); i++) chamberIndexOrder[chamberIndicesOrdered[i]] = i;

        return chamberIndexOrder;
    }

    // This is the comparison function for the sorting of segments according to the chamber index
    bool chamberIndexCompare(const xAOD::MuonSegment* first, const xAOD::MuonSegment* second) {
        static const std::vector<int> chamberIndexOrder = initializeChamberIdxOrder();

        return (chamberIndexOrder[first->chamberIndex()] < chamberIndexOrder[second->chamberIndex()]);
    }

    static const SG::AuxElement::Accessor<float> mePt_acc("MuonSpectrometerPt");
    static const SG::AuxElement::Accessor<float> idPt_acc("InnerDetectorPt");
    static const SG::AuxElement::Accessor<uint8_t> eta1stgchits_acc("etaLayer1STGCHits");
    static const SG::AuxElement::Accessor<uint8_t> eta2stgchits_acc("etaLayer2STGCHits");
    static const SG::AuxElement::Accessor<uint8_t> mmhits_acc("MMHits");
}  // namespace

namespace CP {

    MuonSelectionTool::MuonSelectionTool(const std::string& tool_name) : asg::AsgTool(tool_name), m_acceptInfo("MuonSelection"){}

    MuonSelectionTool::~MuonSelectionTool() = default;

    StatusCode MuonSelectionTool::initialize() {
        // Greet the user:
        ATH_MSG_INFO("Initialising...");
        if(m_isRun3) ATH_MSG_INFO("MuonSelectionTool will assume run3 geometry is used");
        else ATH_MSG_INFO("MuonSelectionTool will assume run2 geometry is used");
        ATH_MSG_INFO("Maximum eta: " << m_maxEta);
        ATH_MSG_INFO("Muon quality: " << m_quality);
        if (m_toroidOff) ATH_MSG_INFO("!! CONFIGURED FOR TOROID-OFF COLLISIONS !!");
        if (m_SctCutOff) ATH_MSG_WARNING("!! SWITCHING SCT REQUIREMENTS OFF !! FOR DEVELOPMENT USE ONLY !!");
        if (m_PixCutOff) ATH_MSG_WARNING("!! SWITCHING PIXEL REQUIREMENTS OFF !! FOR DEVELOPMENT USE ONLY !!");
        if (m_SiHolesCutOff) ATH_MSG_WARNING("!! SWITCHING SILICON HOLES REQUIREMENTS OFF !! FOR DEVELOPMENT USE ONLY !!");
        if (m_custom_dir != "")
            ATH_MSG_WARNING("!! SETTING UP WITH USER SPECIFIED INPUT LOCATION \"" << m_custom_dir << "\"!! FOR DEVELOPMENT USE ONLY !! ");
        if (!m_useAllAuthors)
            ATH_MSG_WARNING(
                "Not using allAuthors variable as currently missing in many derivations; LowPtEfficiency working point will always return "
                "false, but this is expected at the moment. Have a look here: "
                "https://twiki.cern.ch/twiki/bin/view/Atlas/MuonSelectionToolR21#New_LowPtEfficiency_working_poin");

        // Print message to ensure that users excluding 2-station muons in the high-pT selection are aware of this
        if (!m_use2stationMuonsHighPt)
            ATH_MSG_INFO("You have opted to select only 3-station muons in the high-pT selection! "
                         << "Please feed 'HighPt3Layers' to the 'WorkingPoint' property to retrieve the appropriate scale-factors");

        // Only an MVA-based selection is defined for segment-tagged muons for the Low-pT working point
        if (m_useSegmentTaggedLowPt && !m_useMVALowPt) {
            ATH_MSG_WARNING("No cut-based selection is defined for segment-tagged muons in the Low-pT working point. "
                            << "Please set UseMVALowPt=true if you want to try the UseSegmentTaggedLowPt=true option.");
            m_useSegmentTaggedLowPt = false;
        }

        // Set up the TAccept object:
        m_acceptInfo.addCut("Eta", "Selection of muons according to their pseudorapidity");
        m_acceptInfo.addCut("IDHits", "Selection of muons according to whether they passed the MCP ID Hit cuts");
        m_acceptInfo.addCut("Preselection", "Selection of muons according to their type/author");
        m_acceptInfo.addCut("Quality", "Selection of muons according to their tightness");
        // Sanity check
        if (m_quality > 5) {
            ATH_MSG_ERROR(
                "Invalid quality (i.e. selection WP) set: "
                << m_quality
                << " - it must be an integer between 0 and 5! (0=Tight, 1=Medium, 2=Loose, 3=Veryloose, 4=HighPt, 5=LowPtEfficiency)");
            return StatusCode::FAILURE;
        }
        if (m_quality == 5 && !m_useAllAuthors) {
            ATH_MSG_ERROR("Cannot use lowPt working point if allAuthors is not available!");
            return StatusCode::FAILURE;
        }
        
        if(m_CaloScoreWP<1 || m_CaloScoreWP>4){
          ATH_MSG_FATAL("CaloScoreWP property must be set to 1, 2, 3 or 4");
          return StatusCode::FAILURE;
        }

        // Load Tight WP cut-map
        ATH_MSG_INFO("Initialising tight working point histograms...");
        std::string tightWP_rootFile_fullPath;
        if (!m_custom_dir.empty()) {
            tightWP_rootFile_fullPath = PathResolverFindCalibFile(m_custom_dir + "/muonSelection_tightWPHisto.root");
        } else {
            tightWP_rootFile_fullPath = PathResolverFindCalibFile(
                Form("MuonSelectorTools/%s/muonSelection_tightWPHisto.root", m_calibration_version.value().c_str()));
        }

        ATH_MSG_INFO("Reading muon tight working point histograms from " << tightWP_rootFile_fullPath);
        //
        std::unique_ptr<TFile> file(TFile::Open(tightWP_rootFile_fullPath.c_str(), "READ"));

        if (!file->IsOpen()) {
            ATH_MSG_ERROR("Cannot read tight working point file from " << tightWP_rootFile_fullPath);
            return StatusCode::FAILURE;
        }

        // Retrieve all the relevant histograms
        ATH_CHECK(getHist(file.get(), "tightWP_lowPt_rhoCuts", m_tightWP_lowPt_rhoCuts));
        ATH_CHECK(getHist(file.get(), "tightWP_lowPt_qOverPCuts", m_tightWP_lowPt_qOverPCuts));
        ATH_CHECK(getHist(file.get(), "tightWP_mediumPt_rhoCuts", m_tightWP_mediumPt_rhoCuts));
        ATH_CHECK(getHist(file.get(), "tightWP_highPt_rhoCuts", m_tightWP_highPt_rhoCuts));
        //
        file->Close();

        // Read bad muon veto efficiency histograms
        std::string BMVcutFile_fullPath = PathResolverFindCalibFile(m_BMVcutFile);

        ATH_MSG_INFO("Reading bad muon veto cut functions from " << BMVcutFile_fullPath);
        //
        std::unique_ptr<TFile> BMVfile(TFile::Open(BMVcutFile_fullPath.c_str(), "READ"));

        if (!BMVfile->IsOpen()) {
            ATH_MSG_ERROR("Cannot read bad muon veto cut function file from " << BMVcutFile_fullPath);
            return StatusCode::FAILURE;
        }

        m_BMVcutFunction_barrel = std::unique_ptr<TF1>((TF1*)BMVfile->Get("BMVcutFunction_barrel"));
        m_BMVcutFunction_endcap = std::unique_ptr<TF1>((TF1*)BMVfile->Get("BMVcutFunction_endcap"));

        BMVfile->Close();

        if (!m_BMVcutFunction_barrel || !m_BMVcutFunction_endcap) {
            ATH_MSG_ERROR("Cannot read bad muon veto cut functions");
            return StatusCode::FAILURE;
        }

        if (m_useMVALowPt) {
            // Set up TMVA readers for MVA-based low-pT working point
            // E and O refer to even and odd event numbers to avoid applying the MVA on events used for training
            TString weightPath_EVEN_MuidCB = PathResolverFindCalibFile(m_MVAreaderFile_EVEN_MuidCB);
            TString weightPath_ODD_MuidCB = PathResolverFindCalibFile(m_MVAreaderFile_ODD_MuidCB);
            TString weightPath_EVEN_MuGirl = PathResolverFindCalibFile(m_MVAreaderFile_EVEN_MuGirl);
            TString weightPath_ODD_MuGirl = PathResolverFindCalibFile(m_MVAreaderFile_ODD_MuGirl);

            auto make_mva_reader = [](TString file_path) {
                std::vector<std::string> mva_var_names{"momentumBalanceSignificance",
                                                       "scatteringCurvatureSignificance",
                                                       "scatteringNeighbourSignificance",
                                                       "EnergyLoss",
                                                       "middleLargeHoles+middleSmallHoles",
                                                       "muonSegmentDeltaEta",
                                                       "muonSeg1ChamberIdx",
                                                       "muonSeg2ChamberIdx"};
                std::unique_ptr<TMVA::Reader> reader = std::make_unique<TMVA::Reader>(mva_var_names);
                reader->BookMVA("BDTG", file_path);
                return reader;
            };
            m_readerE_MUID = make_mva_reader(weightPath_EVEN_MuidCB);

            m_readerO_MUID = make_mva_reader(weightPath_ODD_MuidCB);

            m_readerE_MUGIRL = make_mva_reader(weightPath_EVEN_MuGirl);

            m_readerO_MUGIRL = make_mva_reader(weightPath_ODD_MuGirl);

            if (m_useSegmentTaggedLowPt) {
                TString weightPath_MuTagIMO_etaBin1 = PathResolverFindCalibFile(m_MVAreaderFile_MuTagIMO_etaBin1);
                TString weightPath_MuTagIMO_etaBin2 = PathResolverFindCalibFile(m_MVAreaderFile_MuTagIMO_etaBin2);
                TString weightPath_MuTagIMO_etaBin3 = PathResolverFindCalibFile(m_MVAreaderFile_MuTagIMO_etaBin3);

                auto make_mva_reader_MuTagIMO = [](TString file_path, bool useSeg2ChamberIndex) {
                    std::vector<std::string> mva_var_names;
                    if (useSeg2ChamberIndex) mva_var_names.push_back("muonSeg2ChamberIndex");
                    mva_var_names.push_back("muonSeg1ChamberIndex");
                    mva_var_names.push_back("muonSeg1NPrecisionHits");
                    mva_var_names.push_back("muonSegmentDeltaEta");
                    mva_var_names.push_back("muonSeg1GlobalR");
                    mva_var_names.push_back("muonSeg1Chi2OverDoF");
                    mva_var_names.push_back("muonSCS");

                    std::unique_ptr<TMVA::Reader> reader = std::make_unique<TMVA::Reader>(mva_var_names);
                    reader->BookMVA("BDT", file_path);
                    return reader;
                };

                m_reader_MUTAGIMO_etaBin1 = make_mva_reader_MuTagIMO(weightPath_MuTagIMO_etaBin1, false);
                m_reader_MUTAGIMO_etaBin2 = make_mva_reader_MuTagIMO(weightPath_MuTagIMO_etaBin2, false);
                m_reader_MUTAGIMO_etaBin3 = make_mva_reader_MuTagIMO(weightPath_MuTagIMO_etaBin3, true);
            }
        }
        ATH_CHECK(m_eventInfo.initialize());

        // Return gracefully:
        return StatusCode::SUCCESS;
    }

    StatusCode MuonSelectionTool::getHist(TFile* file, const std::string& histName, std::unique_ptr<TH1>& hist) const {
        //
        if (!file) {
            ATH_MSG_ERROR(" getHist(...) TFile is nullptr! Check that the Tight cut map is loaded correctly");
            return StatusCode::FAILURE;
        }
        TH1* h_ptr = nullptr;
        file->GetObject(histName.c_str(), h_ptr);
        //
        //
        if (!h_ptr) {
            ATH_MSG_ERROR("Cannot retrieve histogram " << histName);
            return StatusCode::FAILURE;
        }
        hist = std::unique_ptr<TH1>{h_ptr};
        hist->SetDirectory(nullptr);
        ATH_MSG_INFO("Successfully read tight working point histogram: " << hist->GetName());
        //
        return StatusCode::SUCCESS;
    }

    const asg::AcceptInfo& MuonSelectionTool::getAcceptInfo() const { return m_acceptInfo; }

    asg::AcceptData MuonSelectionTool::accept(const xAOD::IParticle* p) const {
        // Check if this is a muon:
        if (p->type() != xAOD::Type::Muon) {
            ATH_MSG_ERROR("accept(...) Function received a non-muon");
            return asg::AcceptData(&m_acceptInfo);
        }

        // Cast it to a muon:
        const xAOD::Muon* mu = dynamic_cast<const xAOD::Muon*>(p);
        if (!mu) {
            ATH_MSG_FATAL("accept(...) Failed to cast particle to muon");
            return asg::AcceptData(&m_acceptInfo);
        }

        // Let the specific function do the work:
        return accept(*mu);
    }

    asg::AcceptData MuonSelectionTool::accept(const xAOD::Muon& mu) const {
        // Verbose information
        ATH_MSG_VERBOSE("-----------------------------------");
        ATH_MSG_VERBOSE("New muon passed to accept function:");
        if (mu.muonType() == xAOD::Muon::Combined)
            ATH_MSG_VERBOSE("Muon type: combined");
        else if (mu.muonType() == xAOD::Muon::MuonStandAlone)
            ATH_MSG_VERBOSE("Muon type: stand-alone");
        else if (mu.muonType() == xAOD::Muon::SegmentTagged)
            ATH_MSG_VERBOSE("Muon type: segment-tagged");
        else if (mu.muonType() == xAOD::Muon::CaloTagged)
            ATH_MSG_VERBOSE("Muon type: calorimeter-tagged");
        else if (mu.muonType() == xAOD::Muon::SiliconAssociatedForwardMuon)
            ATH_MSG_VERBOSE("Muon type: silicon-associated forward");
        ATH_MSG_VERBOSE("Muon pT [GeV]: " << mu.pt() * MeVtoGeV);
        ATH_MSG_VERBOSE("Muon eta: " << mu.eta());
        ATH_MSG_VERBOSE("Muon phi: " << mu.phi());
        
        static std::atomic<bool> isFirstRun3Check{true};
        if(isFirstRun3Check)
        {
            int rn=getRunNumber(true);
            
            if(!m_isRun3 && rn>=399999)
            {
              if(m_geoOnTheFly)
              {
                ATH_MSG_WARNING("muonSelectionTool configured for run2 geometry, but rununmber "<<rn<<" is run3! configure properly the isRun3Geo property; geometry set to run2 on the fly");
              }
              else if(m_forceGeometry)
              {
                ATH_MSG_WARNING("muonSelectionTool configured for run2 geometry, but rununmber "<<rn<<" is run3! Since ForceGeometry is set to True, we'll keep using the wrong geometry, but this is an expert option, make sure you know what you're doing");
              }
              else
              {
                ATH_MSG_FATAL("muonSelectionTool configured for run2 geometry, but rununmber "<<rn<<" is run3! configure properly the isRun3Geo property");
                throw std::runtime_error("MuonSelectionTool() - wrong detector geometry");
              }
            }
            if(m_isRun3 && rn<399999)
            {
              if(m_geoOnTheFly)
              {
                ATH_MSG_WARNING("muonSelectionTool configured for run3 geometry, but rununmber "<<rn<<" is run2! configure properly the isRun3Geo property; geometry set to run2 on the fly");
              }
              else if(m_forceGeometry)
              {
                ATH_MSG_WARNING("muonSelectionTool configured for run3 geometry, but rununmber "<<rn<<" is run3! Since ForceGeometry is set to True, we'll keep using the wrong geometry, but this is an expert option, make sure you know what you're doing");
              }
              else
              {
                ATH_MSG_FATAL("muonSelectionTool configured for run3 geometry, but rununmber "<<rn<<" is run2! configure properly the isRun3Geo property");
                throw std::runtime_error("MuonSelectionTool() - wrong detector geometry");
              }
            }
            if(isRun3())
            {
                
                if(m_quality!=0 && m_quality!=1 && m_quality!=2 && m_quality!=4) ATH_MSG_WARNING("muonSelectionTool currently only supports loose, medium, tight and highpt WPs for run 3 data/MC, all other WPs can currently only be used for tests using Expert mode");
                if(m_quality==0 && !m_developMode && (m_excludeNSWFromPrecisionLayers || !m_recalcPrecisionLayerswNSW)) ATH_MSG_WARNING("for run3, Tight WP is only supported when ExcludeNSWFromPrecisionLayers=False and RecalcPrecisionLayerswNSW=True");
            }
            isFirstRun3Check=false;
        }
        
        asg::AcceptData acceptData(&m_acceptInfo);

        // Do the eta cut:
        if (std::abs(mu.eta()) >= m_maxEta) {
            ATH_MSG_VERBOSE("Failed eta cut");
            return acceptData;
        }
        acceptData.setCutResult("Eta", true);

        // Passes ID hit cuts
        bool passIDCuts = passedIDCuts(mu);
        ATH_MSG_VERBOSE("Passes ID Hit cuts " << passIDCuts);
        acceptData.setCutResult("IDHits", passIDCuts);

        // passes muon preselection
        bool passMuonCuts = passedMuonCuts(mu);
        ATH_MSG_VERBOSE("Passes preselection cuts " << passMuonCuts);
        acceptData.setCutResult("Preselection", passMuonCuts);

        if (!passIDCuts || !passMuonCuts) { return acceptData; }

        // Passes quality requirements
        xAOD::Muon::Quality thisMu_quality = getQuality(mu);
        bool thisMu_highpt = false;
        thisMu_highpt = passedHighPtCuts(mu);
        bool thisMu_lowptE = false;
        thisMu_lowptE = passedLowPtEfficiencyCuts(mu, thisMu_quality);
        ATH_MSG_VERBOSE("Summary of quality information for this muon: ");
        ATH_MSG_VERBOSE("Muon quality: " << thisMu_quality << " passes HighPt: " << thisMu_highpt
                                         << " passes LowPtEfficiency: " << thisMu_lowptE);
        if (m_quality < 4 && thisMu_quality > m_quality) { return acceptData; }
        if (m_quality == 4 && !thisMu_highpt) { return acceptData; }
        if (m_quality == 5 && !thisMu_lowptE) { return acceptData; }
        acceptData.setCutResult("Quality", true);
        // Return the result:
        return acceptData;
    }

    void MuonSelectionTool::setQuality(xAOD::Muon& mu) const {
        mu.setQuality(getQuality(mu));
        return;
    }
    void MuonSelectionTool::IdMsPt(const xAOD::Muon& mu, float& idPt, float& mePt) const {
        const xAOD::TrackParticle* idtrack = mu.trackParticle(xAOD::Muon::InnerDetectorTrackParticle);
        const xAOD::TrackParticle* metrack = mu.trackParticle(xAOD::Muon::ExtrapolatedMuonSpectrometerTrackParticle);
        if (!idtrack || !metrack) idPt = mePt = -1.;
        else if (m_TurnOffMomCorr) {
            mePt = metrack->pt();
            idPt = idtrack->pt();
        } else {
            if (!mePt_acc.isAvailable(mu) || !idPt_acc.isAvailable(mu)) {
                ATH_MSG_FATAL("The muon with pT " << mu.pt() * MeVtoGeV << " eta: " << mu.eta() << ", phi:" << mu.phi()
                                                  << " q:" << mu.charge() << ", author:" << mu.author()
                                                  << " is not decorated with calibrated momenta. Please fix");
                throw std::runtime_error("MuonSelectionTool() - qOverP significance calculation failed");
            }
            mePt = mePt_acc(mu);
            idPt = idPt_acc(mu);
        }
    }

    float MuonSelectionTool::qOverPsignificance(const xAOD::Muon& muon) const {
        if (m_disablePtCuts) {
            ATH_MSG_VERBOSE(__FILE__ << ":"<<__LINE__
                                     << " Momentum dependent cuts are disabled. Return 0.");
            return 0.;
        }
        const xAOD::TrackParticle* idtrack = muon.trackParticle(xAOD::Muon::InnerDetectorTrackParticle);
        const xAOD::TrackParticle* metrack = muon.trackParticle(xAOD::Muon::ExtrapolatedMuonSpectrometerTrackParticle);
        if (!idtrack || !metrack) {
            ATH_MSG_VERBOSE("No ID / MS track. Return dummy large value of 1 mio");
            return 1.e6;
        }
        float mePt{-1.}, idPt{-1.};
        IdMsPt(muon, idPt, mePt);

        const float meP = mePt / std::sin(metrack->theta());
        const float idP = idPt / std::sin(idtrack->theta());

        float qOverPsigma = std::sqrt(idtrack->definingParametersCovMatrix()(4, 4) + metrack->definingParametersCovMatrix()(4, 4));
        return std::abs((metrack->charge() / meP) - (idtrack->charge() / idP)) / qOverPsigma;
    }
    float MuonSelectionTool::rhoPrime(const xAOD::Muon& muon) const {
        if (m_disablePtCuts) {
            ATH_MSG_VERBOSE(__FILE__ << ":"<<__LINE__
                                     << "Momentum dependent cuts are disabled. Return 0.");
            return 0.;
        }
        float mePt{-1.}, idPt{-1.};
        IdMsPt(muon, idPt, mePt);
        return std::abs(idPt - mePt) / muon.pt();
    }

    xAOD::Muon::Quality MuonSelectionTool::getQuality(const xAOD::Muon& mu) const {
        ATH_MSG_VERBOSE("Evaluating muon quality...");
        static const std::set<int> run3_qual{xAOD::Muon::Loose, xAOD::Muon::Medium, xAOD::Muon::Tight, 4};
        //currently allow only tight, medium, loose and highpt when not in expert mode for run3
        if(isRun3() && !m_developMode && !run3_qual.count(m_quality))
        {
          ATH_MSG_VERBOSE("tool configured with quality="<<m_quality<<" which is currently only supported in expert mode for run3");
          return xAOD::Muon::VeryLoose;
        }
        if (isRun3() && mu.isAuthor(xAOD::Muon::Author::Commissioning) && !m_allowComm) {
            ATH_MSG_VERBOSE("Reject authors from the commissioning chain");
            return xAOD::Muon::VeryLoose;
        }

        // SegmentTagged muons
        if (mu.muonType() == xAOD::Muon::SegmentTagged) {
            ATH_MSG_VERBOSE("Muon is segment-tagged");

            if (std::abs(mu.eta()) < 0.1) {
                ATH_MSG_VERBOSE("Muon is loose");
                return xAOD::Muon::Loose;
            } else {
                ATH_MSG_VERBOSE("Do not allow segment-tagged muon at |eta| > 0.1 - return VeryLoose");
                return xAOD::Muon::VeryLoose;
            }
        }

        // CaloTagged muons
        if (mu.muonType() == xAOD::Muon::CaloTagged) {
            ATH_MSG_VERBOSE("Muon is calorimeter-tagged");

            if (std::abs(mu.eta()) < 0.1 && passedCaloTagQuality(mu)) {
                ATH_MSG_VERBOSE("Muon is loose");
                return xAOD::Muon::Loose;
            }
        }

        // Combined muons
        hitSummary summary{};
        fillSummary(mu, summary);
        
        if (mu.muonType() == xAOD::Muon::Combined) {
            ATH_MSG_VERBOSE("Muon is combined");
            if (mu.author() == xAOD::Muon::STACO) {
                ATH_MSG_VERBOSE("Muon is STACO - return VeryLoose");
                return xAOD::Muon::VeryLoose;
            }

            // rejection muons with out-of-bounds hits
            uint8_t combinedTrackOutBoundsPrecisionHits{0};
            retrieveSummaryValue(mu, combinedTrackOutBoundsPrecisionHits, xAOD::MuonSummaryType::combinedTrackOutBoundsPrecisionHits);

            if (combinedTrackOutBoundsPrecisionHits > 0) {
                ATH_MSG_VERBOSE("Muon has out-of-bounds precision hits - return VeryLoose");
                return xAOD::Muon::VeryLoose;
            }

            // LOOSE / MEDIUM / TIGHT WP
            const xAOD::TrackParticle* idtrack = mu.trackParticle(xAOD::Muon::InnerDetectorTrackParticle);
            const xAOD::TrackParticle* metrack = mu.trackParticle(xAOD::Muon::ExtrapolatedMuonSpectrometerTrackParticle);
            if (idtrack && metrack && metrack->definingParametersCovMatrix()(4, 4) > 0) {
                const float qOverPsignif = qOverPsignificance(mu);
                const float rho = rhoPrime(mu);
                const float reducedChi2 = mu.primaryTrackParticle()->chiSquared() / mu.primaryTrackParticle()->numberDoF();

                ATH_MSG_VERBOSE("Relevant cut variables:");
                ATH_MSG_VERBOSE("number of precision layers = " << (int)summary.nprecisionLayers);
                ATH_MSG_VERBOSE("reduced Chi2 = " << reducedChi2);
                ATH_MSG_VERBOSE("qOverP significance = " << qOverPsignif);

                // NEW TIGHT WP
                if (summary.nprecisionLayers > 1 && reducedChi2 < 8 && std::abs(qOverPsignif) < 7) {
                    if (passTight(mu, rho, qOverPsignif)) {
                        ATH_MSG_VERBOSE("Muon is tight");
                        return xAOD::Muon::Tight;
                    }
                }

                ATH_MSG_VERBOSE("Muon did not pass requirements for tight combined muon");

                // MEDIUM WP
                if ((std::abs(qOverPsignif) < 7 || m_toroidOff) &&
                    (summary.nprecisionLayers > 1 ||(summary.nprecisionLayers == 1 && summary.nprecisionHoleLayers < 2 && std::abs(mu.eta()) < 0.1))
                    
                   ) {
                    ATH_MSG_VERBOSE("Muon is medium");
                    return xAOD::Muon::Medium;
                }

                ATH_MSG_VERBOSE("Muon did not pass requirements for medium combined muon");

            } else {
                ATH_MSG_VERBOSE("Muon is missing the ID and/or ME tracks...");

                // CB muons with missing ID or ME track
                if ((summary.nprecisionLayers > 1 ||
                     (summary.nprecisionLayers == 1 && summary.nprecisionHoleLayers < 2 && std::abs(mu.eta()) < 0.1))) {
                    // In toroid-off data ME/MS tracks often missing - need special treatment  => flagging as "Medium"
                    // In toroid-on data ME/MS tracks missing only for <1% of CB muons, mostly MuGirl (to be fixed) => flagging as "Loose"
                    if (m_toroidOff) {
                        ATH_MSG_VERBOSE("...this is toroid-off data - returning medium");
                        return xAOD::Muon::Medium;
                    } else {
                        ATH_MSG_VERBOSE("...this is not toroid-off data - returning loose");
                        return xAOD::Muon::Loose;
                    }
                }
            }

            // Improvement for Loose targeting low-pT muons (pt<7 GeV)
            if ((m_disablePtCuts || mu.pt() * MeVtoGeV < 7.) && std::abs(mu.eta()) < 1.3 && summary.nprecisionLayers > 0 &&
                (mu.author() == xAOD::Muon::MuGirl && mu.isAuthor(xAOD::Muon::MuTagIMO))) {
                ATH_MSG_VERBOSE("Muon passed selection for loose working point at low pT");
                return xAOD::Muon::Loose;
            }

            // didn't pass the set of requirements for a medium or tight combined muon
            ATH_MSG_VERBOSE("Did not pass selections for combined muon - returning VeryLoose");
            return xAOD::Muon::VeryLoose;
        }

        // SA muons
        if (mu.author() == xAOD::Muon::MuidSA) {
            ATH_MSG_VERBOSE("Muon is stand-alone");
            
            if(isRun3() && !m_developMode){
                ATH_MSG_VERBOSE("Standalone muons currently only used when in expert mode for run3");
                return xAOD::Muon::VeryLoose; //SA muons currently disabled for run3
             }

            if (std::abs(mu.eta()) > 2.5) {
                ATH_MSG_VERBOSE("number of precision layers = " << (int)summary.nprecisionLayers);

                // 3 station requirement for medium
                if (summary.nprecisionLayers > 2 && !m_toroidOff) {
                    ATH_MSG_VERBOSE("Muon is medium");
                    return xAOD::Muon::Medium;
                }
            }

            // didn't pass the set of requirements for a medium SA muon
            ATH_MSG_VERBOSE("Muon did not pass selection for medium stand-alone muon - return VeryLoose");
            return xAOD::Muon::VeryLoose;
        }

        // SiliconAssociatedForward (SAF) muons
        if (mu.muonType() == xAOD::Muon::SiliconAssociatedForwardMuon) {
            ATH_MSG_VERBOSE("Muon is silicon-associated forward muon");
            
            if(isRun3() && !m_developMode){
                ATH_MSG_VERBOSE("Silicon-associated forward muon muons currently only used when in expert mode for run3");
                return xAOD::Muon::VeryLoose; //SAF muons currently disabled for run3
            }

            const xAOD::TrackParticle* cbtrack = mu.trackParticle(xAOD::Muon::CombinedTrackParticle);
            const xAOD::TrackParticle* metrack = mu.trackParticle(xAOD::Muon::ExtrapolatedMuonSpectrometerTrackParticle);

            if (cbtrack && metrack) {
                if (std::abs(cbtrack->eta()) > 2.5) {
                    ATH_MSG_VERBOSE("number of precision layers = " << (int)summary.nprecisionLayers);

                    if (summary.nprecisionLayers > 2 && !m_toroidOff) {
                        if (mu.trackParticle(xAOD::Muon::Primary) == mu.trackParticle(xAOD::Muon::InnerDetectorTrackParticle) &&
                            !m_developMode) {
                            ATH_MSG_FATAL(
                                "SiliconForwardAssociated muon has ID track as primary track particle. "
                                << "This is a bug fixed starting with xAODMuon-00-17-07, which should be present in this release. "
                                << "Please report this to the Muon CP group!");
                        }
                        ATH_MSG_VERBOSE("Muon is medium");
                        return xAOD::Muon::Medium;
                    }
                }
            }

            // didn't pass the set of requirements for a medium SAF muon
            ATH_MSG_VERBOSE("Muon did not pass selection for medium silicon-associated forward muon - return VeryLoose");
            return xAOD::Muon::VeryLoose;
        }

        ATH_MSG_VERBOSE("Muon did not pass selection for loose/medium/tight for any muon type - return VeryLoose");
        return xAOD::Muon::VeryLoose;
    }

    void MuonSelectionTool::setPassesIDCuts(xAOD::Muon& mu) const { mu.setPassesIDCuts(passedIDCuts(mu)); }

    void MuonSelectionTool::setPassesHighPtCuts(xAOD::Muon& mu) const { mu.setPassesHighPtCuts(passedHighPtCuts(mu)); }

    bool MuonSelectionTool::passedIDCuts(const xAOD::Muon& mu) const {
        // do not apply the ID hit requirements for SA muons for |eta| > 2.5
        if (mu.author() == xAOD::Muon::MuidSA && std::abs(mu.eta()) > 2.5) {
            return true;
        } else if (mu.muonType() == xAOD::Muon::SiliconAssociatedForwardMuon) {
            const xAOD::TrackParticle* cbtrack = mu.trackParticle(xAOD::Muon::CombinedTrackParticle);
            if (cbtrack && std::abs(cbtrack->eta()) > 2.5) { return true; }
            return false;
        } else {
            if (mu.trackParticle(xAOD::Muon::InnerDetectorTrackParticle))
                return passedIDCuts(*mu.trackParticle(xAOD::Muon::InnerDetectorTrackParticle));
            else if (mu.primaryTrackParticle())
                return passedIDCuts(*mu.primaryTrackParticle());
        }
        return false;
    }

    bool MuonSelectionTool::isBadMuon(const xAOD::Muon& mu) const {
        if (mu.muonType() != xAOD::Muon::Combined) return false;
        // ::
        const xAOD::TrackParticle* idtrack = mu.trackParticle(xAOD::Muon::InnerDetectorTrackParticle);
        const xAOD::TrackParticle* metrack = mu.trackParticle(xAOD::Muon::ExtrapolatedMuonSpectrometerTrackParticle);
        const xAOD::TrackParticle* cbtrack = mu.trackParticle(xAOD::Muon::CombinedTrackParticle);
        // ::
        // Some spurious muons are found to have negative ME track fit covariance, and are typically poorly reconstructed
        if (metrack && metrack->definingParametersCovMatrix()(4, 4) < 0.0) return true;
        // ::
        bool IsBadMuon = false;
        if (idtrack && metrack && cbtrack) {
            // ::
            double qOverP_ID = idtrack->qOverP();
            double qOverPerr_ID = std::sqrt(idtrack->definingParametersCovMatrix()(4, 4));
            double qOverP_ME = metrack->qOverP();
            double qOverPerr_ME = std::sqrt(metrack->definingParametersCovMatrix()(4, 4));
            double qOverP_CB = cbtrack->qOverP();
            double qOverPerr_CB = std::sqrt(cbtrack->definingParametersCovMatrix()(4, 4));
            // ::
            if (m_quality == 4) {
                // recipe for high-pt selection
                IsBadMuon = !passedErrorCutCB(mu);

                hitSummary summary{};
                fillSummary(mu, summary);

                // temporarily apply same recipe as for other working points in addition to CB error
                // cut for 2-station muons, pending better treatment of ID/MS misalignments
                if (m_use2stationMuonsHighPt && summary.nprecisionLayers == 2) {
                    double IdCbRatio = std::abs((qOverPerr_ID / qOverP_ID) / (qOverPerr_CB / qOverP_CB));
                    double MeCbRatio = std::abs((qOverPerr_ME / qOverP_ME) / (qOverPerr_CB / qOverP_CB));
                    IsBadMuon = (IdCbRatio < 0.8 || MeCbRatio < 0.8 || IsBadMuon);
                }
            } else {
                // recipe for other WP
                double IdCbRatio = std::abs((qOverPerr_ID / qOverP_ID) / (qOverPerr_CB / qOverP_CB));
                double MeCbRatio = std::abs((qOverPerr_ME / qOverP_ME) / (qOverPerr_CB / qOverP_CB));
                IsBadMuon = (IdCbRatio < 0.8 || MeCbRatio < 0.8);
            }
        } else {
            return true;
        }
        return IsBadMuon;
    }

    bool MuonSelectionTool::passedLowPtEfficiencyCuts(const xAOD::Muon& mu) const {
        xAOD::Muon::Quality thisMu_quality = getQuality(mu);
        return passedLowPtEfficiencyCuts(mu, thisMu_quality);
    }

    bool MuonSelectionTool::passedLowPtEfficiencyCuts(const xAOD::Muon& mu, xAOD::Muon::Quality thisMu_quality) const {
        ATH_MSG_VERBOSE("Checking whether muon passes low-pT selection...");
        
        //LowPt Not supported in run3 for the time being
        if(isRun3() && !m_developMode){
            ATH_MSG_VERBOSE("LowPt WP currently not supported for run3 if not in expert mode");
            return false;
        }
        if (!m_useAllAuthors) {  // no allAuthors, always fail the WP
            ATH_MSG_VERBOSE("Do not have allAuthors variable - fail low-pT");
            return false;
        }

        // requiring combined muons, unless segment-tags are included
        if (!m_useSegmentTaggedLowPt) {
            if (mu.muonType() != xAOD::Muon::Combined) {
                ATH_MSG_VERBOSE("Muon is not combined - fail low-pT");
                return false;
            }
        } else {
            if (mu.muonType() != xAOD::Muon::Combined && mu.muonType() != xAOD::Muon::SegmentTagged) {
                ATH_MSG_VERBOSE("Muon is not combined or segment-tagged - fail low-pT");
                return false;
            }
        }

        // author check
        if (!m_useSegmentTaggedLowPt) {
            if (mu.author() != xAOD::Muon::MuGirl && mu.author() != xAOD::Muon::MuidCo) {
                ATH_MSG_VERBOSE("Muon is neither MuGirl nor MuidCo - fail low-pT");
                return false;
            }
        } else {
            if (mu.author() != xAOD::Muon::MuGirl && mu.author() != xAOD::Muon::MuidCo && mu.author() != xAOD::Muon::MuTagIMO) {
                ATH_MSG_VERBOSE("Muon is neither MuGirl / MuidCo / MuTagIMO - fail low-pT");
                return false;
            }
        }

        // applying Medium selection above pT = 18 GeV
        if (mu.pt() * MeVtoGeV > 18.) {
            ATH_MSG_VERBOSE("pT > 18 GeV - apply medium selection");
            if (thisMu_quality <= xAOD::Muon::Medium) {
                ATH_MSG_VERBOSE("Muon passed low-pT selection");
                return true;
            } else {
                ATH_MSG_VERBOSE("Muon failed low-pT selection");
                return false;
            }
        }

        // requiring Medium in forward regions
        if (!m_useMVALowPt && std::abs(mu.eta()) > 1.55 && thisMu_quality > xAOD::Muon::Medium) {
            ATH_MSG_VERBOSE("Not using MVA selection, failing low-pT selection due to medium requirement in forward region");
            return false;
        }

        // rejection of muons with out-of-bounds hits
        uint8_t combinedTrackOutBoundsPrecisionHits{0};
        retrieveSummaryValue(mu, combinedTrackOutBoundsPrecisionHits, xAOD::MuonSummaryType::combinedTrackOutBoundsPrecisionHits);
        if (combinedTrackOutBoundsPrecisionHits > 0) {
            ATH_MSG_VERBOSE("Muon has out-of-bounds precision hits - fail low-pT");
            return false;
        }

        // requiring explicitely >=1 station (2 in the |eta|>1.3 region when Medium selection is not explicitely required)
        if (mu.muonType() == xAOD::Muon::Combined) {
            hitSummary summary{};
            fillSummary(mu, summary);
            uint nStationsCut = (std::abs(mu.eta()) > 1.3 && std::abs(mu.eta()) < 1.55) ? 2 : 1;
            if (summary.nprecisionLayers < nStationsCut) {
                ATH_MSG_VERBOSE("number of precision layers = " << (int)summary.nprecisionLayers << " is lower than cut value " << nStationsCut
                                                                << " - fail low-pT");
                return false;
            }
        }

        // reject MuGirl muon if not found also by MuTagIMO
        if (m_useAllAuthors) {
            if (mu.author() == xAOD::Muon::MuGirl && !mu.isAuthor(xAOD::Muon::MuTagIMO)) {
                ATH_MSG_VERBOSE("MuGirl muon is not confirmed by MuTagIMO - fail low-pT");
                return false;
            }
        } else
            return false;

        if (m_useMVALowPt) {
            ATH_MSG_VERBOSE("Applying MVA-based selection");
            return passedLowPtEfficiencyMVACut(mu);
        }

        ATH_MSG_VERBOSE("Applying cut-based selection");

        // apply some loose quality requirements
        float momentumBalanceSignificance{0.}, scatteringCurvatureSignificance{0.}, scatteringNeighbourSignificance{0.};

        retrieveParam(mu, momentumBalanceSignificance, xAOD::Muon::momentumBalanceSignificance);
        retrieveParam(mu, scatteringCurvatureSignificance, xAOD::Muon::scatteringCurvatureSignificance);
        retrieveParam(mu, scatteringNeighbourSignificance, xAOD::Muon::scatteringNeighbourSignificance);

        ATH_MSG_VERBOSE("momentum balance significance: " << momentumBalanceSignificance);
        ATH_MSG_VERBOSE("scattering curvature significance: " << scatteringCurvatureSignificance);
        ATH_MSG_VERBOSE("scattering neighbour significance: " << scatteringNeighbourSignificance);

        if (std::abs(momentumBalanceSignificance) > 3. || std::abs(scatteringCurvatureSignificance) > 3. ||
            std::abs(scatteringNeighbourSignificance) > 3.) {
            ATH_MSG_VERBOSE("Muon failed cut-based low-pT selection");
            return false;
        }

        // passed low pt selection
        ATH_MSG_VERBOSE("Muon passed cut-based low-pT selection");
        return true;
    }

    std::vector<const xAOD::MuonSegment*> MuonSelectionTool::getSegmentsSorted(const xAOD::Muon& mu) const {
        std::vector<const xAOD::MuonSegment*> segments_sorted;
        segments_sorted.reserve(mu.nMuonSegments());

        for (unsigned int i = 0; i < mu.nMuonSegments(); i++) {
            if (!mu.muonSegment(i))
                ATH_MSG_WARNING("The muon reports more segments than are available. Please report this to the muon software community!");
            else
                segments_sorted.push_back(mu.muonSegment(i));
        }

        std::sort(segments_sorted.begin(), segments_sorted.end(), chamberIndexCompare);

        return segments_sorted;
    }

    bool MuonSelectionTool::passedLowPtEfficiencyMVACut(const xAOD::Muon& mu) const {
        //LowPt Not supported in run3 for the time being
        if(isRun3() && !m_developMode){
            ATH_MSG_VERBOSE("LowPt WP currently not supported for run3 if not in expert mode");
            return false;
        }
        if (!m_useMVALowPt) {
            ATH_MSG_DEBUG("Low pt MVA disabled. Return... ");
            return false;
        }
        std::lock_guard<std::mutex> guard(m_low_pt_mva_mutex);
        // set values for all BDT input variables from the muon in question
        float momentumBalanceSig{-1}, CurvatureSig{-1}, energyLoss{-1}, muonSegmentDeltaEta{-1}, scatteringNeigbour{-1};
        retrieveParam(mu, momentumBalanceSig, xAOD::Muon::momentumBalanceSignificance);
        retrieveParam(mu, CurvatureSig, xAOD::Muon::scatteringCurvatureSignificance);
        retrieveParam(mu, scatteringNeigbour, xAOD::Muon::scatteringNeighbourSignificance);
        retrieveParam(mu, energyLoss, xAOD::Muon::EnergyLoss);
        retrieveParam(mu, muonSegmentDeltaEta, xAOD::Muon::segmentDeltaEta);      

        uint8_t middleSmallHoles{0}, middleLargeHoles{0};
        retrieveSummaryValue(mu, middleSmallHoles, xAOD::MuonSummaryType::middleSmallHoles);
        retrieveSummaryValue(mu, middleLargeHoles, xAOD::MuonSummaryType::middleLargeHoles);

        float seg1ChamberIdx{-1.}, seg2ChamberIdx{-1.}, middleHoles{-1.}, seg1NPrecisionHits{-1.}, seg1GlobalR{-1.}, seg1Chi2OverDoF{-1.};

        std::vector<const xAOD::MuonSegment*> muonSegments = getSegmentsSorted(mu);

        if (mu.author() == xAOD::Muon::MuTagIMO && muonSegments.size() == 0)
            ATH_MSG_WARNING("passedLowPtEfficiencyMVACut - found segment-tagged muon with no segments!");

        seg1ChamberIdx = (!muonSegments.empty()) ? muonSegments[0]->chamberIndex() : -9;
        seg2ChamberIdx = (muonSegments.size() > 1) ? muonSegments[1]->chamberIndex() : -9;

        // these variables are only used for MuTagIMO
        if (mu.author() == xAOD::Muon::MuTagIMO) {
            seg1NPrecisionHits = (!muonSegments.empty()) ? muonSegments[0]->nPrecisionHits() : -1;
            seg1GlobalR = (!muonSegments.empty())
                              ? std::hypot(muonSegments[0]->x(), muonSegments[0]->y(), muonSegments[0]->z())
                              : 0;
            seg1Chi2OverDoF = (!muonSegments.empty()) ? muonSegments[0]->chiSquared() / muonSegments[0]->numberDoF() : -1;
        }

        middleHoles = middleSmallHoles + middleLargeHoles;

        // get event number from event info
        SG::ReadHandle<xAOD::EventInfo> eventInfo(m_eventInfo);

        // variables for the BDT
        std::vector<float> var_vector;
        if (mu.author() == xAOD::Muon::MuidCo || mu.author() == xAOD::Muon::MuGirl) {
            var_vector = {momentumBalanceSig, CurvatureSig,        scatteringNeigbour, energyLoss,
                          middleHoles,        muonSegmentDeltaEta, seg1ChamberIdx,     seg2ChamberIdx};
        } else {
            if (std::abs(mu.eta()) >= 1.3)
                var_vector = {seg2ChamberIdx, seg1ChamberIdx,  seg1NPrecisionHits,    muonSegmentDeltaEta,
                              seg1GlobalR,    seg1Chi2OverDoF, std::abs(CurvatureSig)};
            else
                var_vector = {seg1ChamberIdx, seg1NPrecisionHits, muonSegmentDeltaEta,
                              seg1GlobalR,    seg1Chi2OverDoF,    std::abs(CurvatureSig)};
        }

        // use different trainings for even/odd numbered events
        TMVA::Reader *reader_MUID, *reader_MUGIRL;
        if (eventInfo->eventNumber() % 2 == 1) {
            reader_MUID = m_readerE_MUID.get();
            reader_MUGIRL = m_readerE_MUGIRL.get();
        } else {
            reader_MUID = m_readerO_MUID.get();
            reader_MUGIRL = m_readerO_MUGIRL.get();
        }

        // BDT for MuTagIMO is binned in |eta|
        TMVA::Reader* reader_MUTAGIMO;
        if (std::abs(mu.eta()) < 0.7)
            reader_MUTAGIMO = m_reader_MUTAGIMO_etaBin1.get();
        else if (std::abs(mu.eta()) < 1.3)
            reader_MUTAGIMO = m_reader_MUTAGIMO_etaBin2.get();
        else
            reader_MUTAGIMO = m_reader_MUTAGIMO_etaBin3.get();

        // get the BDT discriminant response
        float BDTdiscriminant;

        if (mu.author() == xAOD::Muon::MuidCo)
            BDTdiscriminant = reader_MUID->EvaluateMVA(var_vector, "BDTG");
        else if (mu.author() == xAOD::Muon::MuGirl)
            BDTdiscriminant = reader_MUGIRL->EvaluateMVA(var_vector, "BDTG");
        else if (mu.author() == xAOD::Muon::MuTagIMO && m_useSegmentTaggedLowPt)
            BDTdiscriminant = reader_MUTAGIMO->EvaluateMVA(var_vector, "BDT");
        else {
            ATH_MSG_WARNING("Invalid author for low-pT MVA, failing selection...");
            return false;
        }

        // cut on dicriminant
        float BDTcut = (mu.author() == xAOD::Muon::MuTagIMO) ? 0.12 : -0.6;

        if (BDTdiscriminant > BDTcut) {
            ATH_MSG_VERBOSE("Passed low-pT MVA cut");
            return true;
        } else {
            ATH_MSG_VERBOSE("Failed low-pT MVA cut");
            return false;
        }
    }

    bool MuonSelectionTool::passedHighPtCuts(const xAOD::Muon& mu) const {
        ATH_MSG_VERBOSE("Checking whether muon passes high-pT selection...");

        // :: Request combined muons
        if (mu.muonType() != xAOD::Muon::Combined) {
            ATH_MSG_VERBOSE("Muon is not combined - fail high-pT");
            return false;
        }
        if (mu.author() == xAOD::Muon::STACO) {
            ATH_MSG_VERBOSE("Muon is STACO - fail high-pT");
            return false;
        }

        // :: Reject muons with out-of-bounds hits
        uint8_t combinedTrackOutBoundsPrecisionHits{0};
        retrieveSummaryValue(mu, combinedTrackOutBoundsPrecisionHits, xAOD::MuonSummaryType::combinedTrackOutBoundsPrecisionHits);
        if (combinedTrackOutBoundsPrecisionHits > 0) {
            ATH_MSG_VERBOSE("Muon has out-of-bounds precision hits - fail high-pT");
            return false;
        }

        // :: Access MS hits information
        hitSummary summary{};
        fillSummary(mu, summary);
        
        
        ATH_MSG_VERBOSE("number of precision layers: " << (int)summary.nprecisionLayers);

        //::: Apply MS Chamber Vetoes
        // Given according to their eta-phi locations in the muon spectrometer
        // FORM: CHAMBERNAME[ array of four values ] = { eta 1, eta 2, phi 1, phi 2}
        // The vetoes are applied based on the MS track if available. If the MS track is not available,
        // the vetoes are applied according to the combined track, and runtime warning is printed to
        // the command line.
        const xAOD::TrackParticle* CB_track = mu.trackParticle(xAOD::Muon::CombinedTrackParticle);
        const xAOD::TrackParticle* MS_track = mu.trackParticle(xAOD::Muon::MuonSpectrometerTrackParticle);
        if (!MS_track) {
            ATH_MSG_VERBOSE("passedHighPtCuts - No MS track available for muon. Using combined track.");
            MS_track = mu.trackParticle(xAOD::Muon::CombinedTrackParticle);
        }

        if (MS_track && CB_track) {
            float etaMS = MS_track->eta();
            float phiMS = MS_track->phi();
            float etaCB = CB_track->eta();

            //::: no unspoiled clusters in CSC
            if (!isRun3() && (std::abs(etaMS) > 2.0 || std::abs(etaCB) > 2.0)) {
                if (summary.cscUnspoiledEtaHits == 0) {
                    ATH_MSG_VERBOSE("Muon has only spoiled CSC clusters - fail high-pT");
                    return false;
                }
            }

            // veto bad CSC giving troubles with scale factors
            if (!isRun3() && mu.eta() < -1.899 && std::abs(mu.phi()) < 0.211) {
                ATH_MSG_VERBOSE("Muon is in eta/phi region vetoed due to disabled chambers in MC - fail high-pT");
                return false;
            }

            //::: Barrel/Endcap overlap region
            if ((1.01 < std::abs(etaMS) && std::abs(etaMS) < 1.1) || (1.01 < std::abs(etaCB) && std::abs(etaCB) < 1.1)) {
                ATH_MSG_VERBOSE("Muon is in barrel/endcap overlap region - fail high-pT");
                return false;
            }

            //::: BIS78
            if (isBIS78(etaMS, phiMS)) {
                ATH_MSG_VERBOSE("Muon is in BIS7/8 eta/phi region - fail high-pT");
                return false;
            }
            
            //// tentatively removed for r22, to be rechecked
            ////::: BMG - only veto in 2017+2018 data and corresponding MC
            //if (getRunNumber(true) >= 324320) {
                //if (isBMG(etaMS, phiMS)) {
                    //ATH_MSG_VERBOSE("Muon is in BMG eta/phi region - fail high-pT");
                    //return false;
                //}
            //}

            //::: BEE
            if (isBEE(etaMS, phiMS)) {
                // in Run3, large mis-alignment on the BEE chamber was found. temporarily mask the BEE region
                if (isRun3()) {
                    ATH_MSG_VERBOSE("Muon is in BEE eta/phi region - fail high-pT");
                    return false;
                }
                // Muon falls in the BEE eta-phi region: asking for 4 good precision layers
                // if( nGoodPrecLayers < 4 ) return false; // postponed (further studies needed)
                if (summary.nprecisionLayers < 4) {
                    ATH_MSG_VERBOSE("Muon is in BEE eta/phi region and does not have 4 precision layers - fail high-pT");
                    return false;
                }
            }
            if (std::abs(etaCB) > 1.4) {
                // Veto residual 3-station muons in BEE region due to MS eta/phi resolution effects
                // if( nGoodPrecLayers<4 && (extendedSmallHits>0||extendedSmallHoles>0) ) return false; // postponed (further studies
                // needed)
                if (summary.nprecisionLayers < 4 && (summary.extendedSmallHits > 0 || summary.extendedSmallHoles > 0)) {
                    ATH_MSG_VERBOSE("Muon is in BEE eta/phi region and does not have 4 precision layers - fail high-pT");
                    return false;
                }
            }
        } else {
            ATH_MSG_WARNING("passedHighPtCuts - MS or CB track missing in muon! Failing High-pT selection...");
            return false;
        }

        //::: Apply 1/p significance cut
        const xAOD::TrackParticle* idtrack = mu.trackParticle(xAOD::Muon::InnerDetectorTrackParticle);
        const xAOD::TrackParticle* metrack = mu.trackParticle(xAOD::Muon::ExtrapolatedMuonSpectrometerTrackParticle);
        if (idtrack && metrack && metrack->definingParametersCovMatrix()(4, 4) > 0) {            const float qOverPsignif = qOverPsignificance(mu);

            ATH_MSG_VERBOSE("qOverP significance: " << qOverPsignif);

            if (std::abs(qOverPsignif) > 7) {
                ATH_MSG_VERBOSE("Muon failed qOverP significance cut");
                return false;
            }
        } else {
            ATH_MSG_VERBOSE("Muon missing ID or ME tracks - fail high-pT");
            return false;
        }

        // Accept good 2-station muons if the user has opted to include these
        if (m_use2stationMuonsHighPt && summary.nprecisionLayers == 2) {
            // should not accept EM+EO muons due to ID/MS alignment issues
            if (std::abs(mu.eta()) > 1.2 && summary.extendedSmallHits < 3 && summary.extendedLargeHits < 3) {
                ATH_MSG_VERBOSE("2-station muon with EM+EO - fail high-pT");
                return false;
            }

            // only select muons missing the inner precision layer
            // apply strict veto on overlap between small and large sectors

            if (summary.innerLargeHits == 0 && summary.middleLargeHits == 0 && summary.outerLargeHits == 0 &&
                summary.extendedLargeHits == 0 && summary.middleSmallHits > 2 &&
                (summary.outerSmallHits > 2 || summary.extendedSmallHits > 2)) {
                ATH_MSG_VERBOSE("Accepted 2-station muon in small sector");
                return true;
            }

            if (summary.innerSmallHits == 0 && summary.middleSmallHits == 0 && summary.outerSmallHits == 0 &&
                summary.extendedSmallHits == 0 && summary.middleLargeHits > 2 &&
                (summary.outerLargeHits > 2 || summary.extendedLargeHits > 2)) {
                ATH_MSG_VERBOSE("Accepted 2-station muon in large sector");
                return true;
            }
        }

        //::: Require 3 (good) station muons
        if (summary.nprecisionLayers < 3) {
            ATH_MSG_VERBOSE("Muon has less than 3 precision layers - fail high-pT");
            return false;
        }

        // Remove 3-station muons with small-large sectors overlap
        if (summary.isSmallGoodSectors) {
            if (!(summary.innerSmallHits > 2 && summary.middleSmallHits > 2 &&
                  (summary.outerSmallHits > 2 || summary.extendedSmallHits > 2))) {
                ATH_MSG_VERBOSE("Muon has small/large sectors overlap - fail high-pT");
                return false;
            }
        } else {
            if (!(summary.innerLargeHits > 2 && summary.middleLargeHits > 2 &&
                  (summary.outerLargeHits > 2 || summary.extendedLargeHits > 2))) {
                ATH_MSG_VERBOSE("Muon has small/large sectors overlap - fail high-pT");
                return false;
            }
        }

        ATH_MSG_VERBOSE("Muon passed high-pT selection");
        return true;
    }

    bool MuonSelectionTool::passedErrorCutCB(const xAOD::Muon& mu) const {
        // ::
        if (mu.muonType() != xAOD::Muon::Combined) return false;
        // ::
        double start_cut = 3.0;
        double end_cut = 1.6;
        double abs_eta = std::abs(mu.eta());
        
        // parametrization of expected q/p error as function of pT
        double p0(8.0), p1(0.), p2(0.);
        if(isRun3()) //MC21 optimization
        {
          if(abs_eta<=1.05){
              p1=0.046;
              p2=0.00005;
          }
          else if (abs_eta > 1.05 && abs_eta <= 1.3) {
              p1 = 0.052;
              p2 = 0.00008;
          } else if (abs_eta > 1.3 && abs_eta <= 1.7) {
              p1 = 0.068;
              p2 = 0.00006;
          } else if (abs_eta > 1.7 && abs_eta <= 2.0) {
              p1 = 0.048;
              p2 = 0.00006;
          } else if (abs_eta > 2.0) {
              p1 = 0.037;
              p2 = 0.00006;
          }
        }
        else
        {
          if(abs_eta<=1.05){
              p1=0.039;
              p2=0.00006;
          }
          else if (abs_eta > 1.05 && abs_eta <= 1.3) {
              p1 = 0.040;
              p2 = 0.00009;
          } else if (abs_eta > 1.3 && abs_eta <= 1.7) {
              p1 = 0.056;
              p2 = 0.00008;
          } else if (abs_eta > 1.7 && abs_eta <= 2.0) {
              p1 = 0.041;
              p2 = 0.00006;
          } else if (abs_eta > 2.0) {
              p1 = 0.031;
              p2 = 0.00006;
          }
        }
        // ::
        hitSummary summary{};
        fillSummary(mu, summary);

        // independent parametrization for 2-station muons
        if (m_use2stationMuonsHighPt && summary.nprecisionLayers == 2) {
            start_cut = 1.1;
            end_cut=0.7; 
            p1 = 0.0739568;
            p2 = 0.00012443;
            if (abs_eta > 1.05 && abs_eta < 1.3) {
                p1 = 0.0674484;
                p2 = 0.000119879;
            } else if (abs_eta >= 1.3 && abs_eta < 1.7) {
                p1 = 0.041669;
                p2 = 0.000178349;
            } else if (abs_eta >= 1.7 && abs_eta < 2.0) {
                p1 = 0.0488664;
                p2 = 0.000137648;
            } else if (abs_eta >= 2.0) {
                p1 = 0.028077;
                p2 = 0.000152707;
            }
        }
        // ::
        bool passErrorCutCB = false;
        const xAOD::TrackParticle* cbtrack = mu.trackParticle(xAOD::Muon::CombinedTrackParticle);
        if (cbtrack) {
            // ::
            double pt_CB = (cbtrack->pt() * MeVtoGeV < 5000.) ? cbtrack->pt() * MeVtoGeV : 5000.;  // GeV
            double qOverP_CB = cbtrack->qOverP();
            double qOverPerr_CB = std::sqrt(cbtrack->definingParametersCovMatrix()(4, 4));
            // sigma represents the average expected error at the muon's pt/eta
            double sigma = std::sqrt(std::pow(p0 / pt_CB, 2) + std::pow(p1, 2) + std::pow(p2 * pt_CB, 2));
            // cutting at start_cut*sigma for pt <=1 TeV depending on eta region,
            // then linearly tightening until end_cut*sigma is reached at pt >= 5TeV.
            double a = (end_cut - start_cut) / 4000.0;
            double b = end_cut - a * 5000.0;
            double coefficient = (pt_CB > 1000.) ? (a * pt_CB + b) : start_cut;
            if (std::abs(qOverPerr_CB / qOverP_CB) < coefficient * sigma) { passErrorCutCB = true; }
        }
        // ::
        if (m_use2stationMuonsHighPt && m_doBadMuonVetoMimic && summary.nprecisionLayers == 2) {
            SG::ReadHandle<xAOD::EventInfo> eventInfo(m_eventInfo);

            if (eventInfo->eventType(xAOD::EventInfo::IS_SIMULATION)) {
                ATH_MSG_DEBUG("The current event is a MC event. Use bad muon veto mimic.");
                return passErrorCutCB && passedBMVmimicCut(mu);
            }
        }

        // ::
        return passErrorCutCB;
    }

    bool MuonSelectionTool::passedBMVmimicCut(const xAOD::Muon& mu) const {
        TF1* cutFunction;
        double p1, p2;
        if (std::abs(mu.eta()) < 1.05) {
            cutFunction = m_BMVcutFunction_barrel.get();
            p1 = 0.066265;
            p2 = 0.000210047;
        } else {
            cutFunction = m_BMVcutFunction_endcap.get();
            p1 = 0.0629747;
            p2 = 0.000196466;
        }

        double qOpRelResolution = std::hypot(p1, p2 * mu.primaryTrackParticle()->pt() * MeVtoGeV);

        double qOverPabs_unsmeared = std::abs(mu.primaryTrackParticle()->definingParameters()[4]);
        double qOverPabs_smeared = 1.0 / (mu.pt() * std::cosh(mu.eta()));

        if ((qOverPabs_smeared - qOverPabs_unsmeared) / (qOpRelResolution * qOverPabs_unsmeared) <
            cutFunction->Eval(mu.primaryTrackParticle()->pt() * MeVtoGeV))
            return false;
        else
            return true;
    }

    bool MuonSelectionTool::passedMuonCuts(const xAOD::Muon& mu) const {
        // ::
        if (mu.muonType() == xAOD::Muon::Combined) { return mu.author() != xAOD::Muon::STACO; }
        // ::
        if (mu.muonType() == xAOD::Muon::CaloTagged && std::abs(mu.eta()) < 0.105)
            return passedCaloTagQuality(mu);
        // ::
        if (mu.muonType() == xAOD::Muon::SegmentTagged && (std::abs(mu.eta()) < 0.105 || m_useSegmentTaggedLowPt)) return true;
        // ::
        if (mu.author() == xAOD::Muon::MuidSA && std::abs(mu.eta()) > 2.4) return true;
        // ::
        if (mu.muonType() == xAOD::Muon::SiliconAssociatedForwardMuon) {
            const xAOD::TrackParticle* cbtrack = mu.trackParticle(xAOD::Muon::CombinedTrackParticle);
            return (cbtrack && std::abs(cbtrack->eta()) > 2.4);
        }
        // ::
        return false;
    }

    bool MuonSelectionTool::passedIDCuts(const xAOD::TrackParticle& track) const {
        uint8_t value1{0}, value2{0};

        if ((m_PixCutOff || m_SctCutOff || m_SiHolesCutOff) && !m_developMode)
            ATH_MSG_WARNING(
                " !! Tool configured with some of the ID hits requirements changed... FOR DEVELOPMENT ONLY: muon efficiency SF won't be "
                "valid !! ");

        retrieveSummaryValue(track, value1, xAOD::SummaryType::numberOfPixelHits);
        retrieveSummaryValue(track, value2, xAOD::SummaryType::numberOfPixelDeadSensors);
        if ((value1 + value2 == 0) && !m_PixCutOff) return false;

        retrieveSummaryValue(track, value1, xAOD::SummaryType::numberOfSCTHits);
        retrieveSummaryValue(track, value2, xAOD::SummaryType::numberOfSCTDeadSensors);
        if ((value1 + value2 <= 4) && !m_SctCutOff) return false;

        retrieveSummaryValue(track, value1, xAOD::SummaryType::numberOfPixelHoles);
        retrieveSummaryValue(track, value2, xAOD::SummaryType::numberOfSCTHoles);
        if ((value1 + value2 >= 3) && !m_SiHolesCutOff) return false;

        if (!m_TrtCutOff) {
            const float abseta = std::abs(track.eta());
            retrieveSummaryValue(track, value1, xAOD::SummaryType::numberOfTRTHits);
            retrieveSummaryValue(track, value2, xAOD::SummaryType::numberOfTRTOutliers);
            const uint8_t totTRThits = value1 + value2;
            if (!((0.1 < abseta && abseta <= 1.9 && totTRThits > 5 && value2 < (0.9 * totTRThits)) || (abseta <= 0.1 || abseta > 1.9)))
                return false;
        }
        // Reached end - all ID hit cuts are passed.
        return true;
    }  // passedIDCuts

    bool MuonSelectionTool::passedCaloTagQuality(const xAOD::Muon& mu) const {
        // Use CaloScore variable based on Neural Network if enabled
        // The neural network is only trained until eta = 1 
        // cf. https://cds.cern.ch/record/2802605/files/CERN-THESIS-2021-290.pdf
        constexpr float eta_range = 1.;
        if (std::abs(mu.eta()) < eta_range && m_useCaloScore) return passedCaloScore(mu);

        // Otherwise we use CaloMuonIDTag
        int CaloMuonIDTag = -20;

        // Extract CaloMuonIDTag variable
        bool readID = mu.parameter(CaloMuonIDTag, xAOD::Muon::CaloMuonIDTag);
        if (!readID) {
            ATH_MSG_WARNING("Unable to read CaloMuonIDTag Quality information! Rejecting the CALO muon!");
            return false;
        }

        // Cut on CaloMuonIDTag variable
        return (CaloMuonIDTag > 10);
    }

    bool MuonSelectionTool::passedCaloScore(const xAOD::Muon& mu) const {
        // We use a working point with a pT-dependent cut on the NN discriminant, designed to achieve a constant
        // fakes rejection as function of pT in Z->mumu MC

        // Extract the relevant score variable (NN discriminant)
        float CaloMuonScore{-999.0};
        retrieveParam(mu, CaloMuonScore, xAOD::Muon::CaloMuonScore);
        
        if(m_CaloScoreWP==1) return (CaloMuonScore >= 0.92);
        if(m_CaloScoreWP==2) return (CaloMuonScore >= 0.56);
        else if(m_CaloScoreWP==3 || m_CaloScoreWP==4)
        {
          // Cut on the score variable
          float pT = mu.pt() * MeVtoGeV;  // GeV

          if (pT > 20.0)  // constant cut above 20 GeV
              return (CaloMuonScore >= 0.77);
          else {
              // pT-dependent cut below 20 GeV
              // The pT-dependent cut is based on a fit of a third-degree polynomial, with coefficients as given below
              
              if(m_CaloScoreWP==3) return (CaloMuonScore >= (-1.98e-4 * std::pow(pT, 3) +6.04e-3 * std::pow(pT, 2) -6.13e-2 * pT + 1.16));
              if(m_CaloScoreWP==4) return (CaloMuonScore >= (-1.80e-4 * std::pow(pT, 3) +5.02e-3 * std::pow(pT, 2) -4.62e-2 * pT + 1.12));
          }
        }
        
        return false;
    }

    bool MuonSelectionTool::passTight(const xAOD::Muon& mu, float rho, float oneOverPSig) const {
      
        if(isRun3() && !m_developMode && (m_excludeNSWFromPrecisionLayers || !m_recalcPrecisionLayerswNSW)){
          ATH_MSG_VERBOSE("for run3, Tight WP is only supported when ExcludeNSWFromPrecisionLayers=False and RecalcPrecisionLayerswNSW=True");
          return false;
        }
        float symmetric_eta = std::abs(mu.eta());
        float pt = mu.pt() * MeVtoGeV;  // GeV

        // Impose pT and eta cuts; the bounds of the cut maps
        if (pt < 4.0 || symmetric_eta >= 2.5) return false;
        ATH_MSG_VERBOSE("Muon is passing tight WP kinematic cuts with pT,eta " << mu.pt() << "  ,  " << mu.eta());

        // ** Low pT specific cuts ** //
        if (pt < 20.0) {
            double rhoCut = m_tightWP_lowPt_rhoCuts->Interpolate(pt, symmetric_eta);
            double qOverPCut = m_tightWP_lowPt_qOverPCuts->Interpolate(pt, symmetric_eta);

            ATH_MSG_VERBOSE("Applying tight WP cuts to a low pt muon with (pt,eta) ( " << pt << " , " << mu.eta() << " ) ");
            ATH_MSG_VERBOSE("Rho value " << rho << ", required to be less than " << rhoCut);
            ATH_MSG_VERBOSE("Momentum significance value " << oneOverPSig << ", required to be less than " << qOverPCut);

            if (rho > rhoCut) return false;
            ATH_MSG_VERBOSE("Muon passed tight WP, low pT rho cut!");

            if (oneOverPSig > qOverPCut) return false;
            ATH_MSG_VERBOSE("Muon passed tight WP, low pT momentum significance cut");

            // Tight muon!
            return true;

        }

        // ** Medium pT specific cuts ** //
        else if (pt < 100.0) {
            double rhoCut = m_tightWP_mediumPt_rhoCuts->Interpolate(pt, symmetric_eta);
            //
            ATH_MSG_VERBOSE("Applying tight WP cuts to a medium pt muon with (pt,eta) (" << pt << "," << mu.eta() << ")");
            ATH_MSG_VERBOSE("Rho value " << rho << " required to be less than " << rhoCut);

            // Apply cut
            if (rho > rhoCut) return false;
            ATH_MSG_VERBOSE("Muon passed tight WP, medium pT rho cut!");

            // Tight muon!
            return true;
        }

        // ** High pT specific cuts
        else if (pt < 500.0) {
            //
            ATH_MSG_VERBOSE("Applying tight WP cuts to a high pt muon with (pt,eta) (" << pt << "," << mu.eta() << ")");
            // No interpolation, since bins with -1 mean we should cut really loose
            double rhoCut = m_tightWP_highPt_rhoCuts->GetBinContent(m_tightWP_highPt_rhoCuts->FindFixBin(pt, symmetric_eta));
            ATH_MSG_VERBOSE("Rho value " << rho << ", required to be less than " << rhoCut << " unless -1, in which no cut is applied");
            //
            if (rhoCut < 0.0) return true;
            if (rho > rhoCut) return false;
            ATH_MSG_VERBOSE("Muon passed tight WP, high pT rho cut!");

            return true;
        }
        // For muons with pT > 500 GeV, no extra cuts
        else {
            ATH_MSG_VERBOSE("Not applying any tight WP cuts to a very high pt muon with (pt,eta) (" << pt << "," << mu.eta() << ")");
            return true;
        }

        // you should never reach this point
        return false;
    }

    void MuonSelectionTool::fillSummary(const xAOD::Muon& muon, hitSummary& summary) const {
        retrieveSummaryValue(muon, summary.nprecisionLayers, xAOD::SummaryType::numberOfPrecisionLayers);
        retrieveSummaryValue(muon, summary.nprecisionHoleLayers, xAOD::SummaryType::numberOfPrecisionHoleLayers);
        retrieveSummaryValue(muon, summary.nGoodPrecLayers, xAOD::numberOfGoodPrecisionLayers);
        retrieveSummaryValue(muon, summary.innerSmallHits, xAOD::MuonSummaryType::innerSmallHits);
        retrieveSummaryValue(muon, summary.innerLargeHits, xAOD::MuonSummaryType::innerLargeHits);
        retrieveSummaryValue(muon, summary.middleSmallHits, xAOD::MuonSummaryType::middleSmallHits);
        retrieveSummaryValue(muon, summary.middleLargeHits, xAOD::MuonSummaryType::middleLargeHits);
        retrieveSummaryValue(muon, summary.outerSmallHits, xAOD::MuonSummaryType::outerSmallHits);
        retrieveSummaryValue(muon, summary.outerLargeHits, xAOD::MuonSummaryType::outerLargeHits);
        retrieveSummaryValue(muon, summary.extendedSmallHits, xAOD::MuonSummaryType::extendedSmallHits);
        retrieveSummaryValue(muon, summary.extendedLargeHits, xAOD::MuonSummaryType::extendedLargeHits);
        retrieveSummaryValue(muon, summary.extendedSmallHoles, xAOD::MuonSummaryType::extendedSmallHoles);
        retrieveSummaryValue(muon, summary.isSmallGoodSectors, xAOD::MuonSummaryType::isSmallGoodSectors);
        if(!isRun3(false)) retrieveSummaryValue(muon, summary.cscUnspoiledEtaHits, xAOD::MuonSummaryType::cscUnspoiledEtaHits); //setting allowForce to false for isRun3(bool) because otherwise that flag can be forced via tool properties to get a specific value, typically for testing purposes. But whatever you force that flag to be, you'll not have CSC hits in run-3 samples!

        if (!isRun3() && std::abs(muon.eta()) > 2.0) {
          ATH_MSG_VERBOSE("Recalculating number of precision layers for combined muon");
          summary.nprecisionLayers = 0;
          if (summary.innerSmallHits > 1 || summary.innerLargeHits > 1) summary.nprecisionLayers += 1;
          if (summary.middleSmallHits > 2 || summary.middleLargeHits > 2) summary.nprecisionLayers += 1;
          if (summary.outerSmallHits > 2 || summary.outerLargeHits > 2) summary.nprecisionLayers += 1;
        }
        if (isRun3() && m_excludeNSWFromPrecisionLayers && std::abs(muon.eta()) > 1.3) {
          summary.nprecisionLayers = 0;
          if (summary.middleSmallHits > 2 || summary.middleLargeHits > 2) summary.nprecisionLayers += 1;
          if (summary.outerSmallHits > 2 || summary.outerLargeHits > 2) summary.nprecisionLayers += 1;
          if (summary.extendedSmallHits > 2 || summary.extendedLargeHits > 2) summary.nprecisionLayers += 1;
        }
        if (isRun3() && !m_excludeNSWFromPrecisionLayers && m_recalcPrecisionLayerswNSW && std::abs(muon.eta()) > 1.3) {
          if (!eta1stgchits_acc.isAvailable(muon) || !eta2stgchits_acc.isAvailable(muon) || !mmhits_acc.isAvailable(muon)) {
            ATH_MSG_FATAL(__FILE__ << ":" << __LINE__ << " Failed to retrieve NSW hits!!"
                                   << " If you're using DxAODs (with smart slimming for muons), you should use p-tags >= p5834."
                                   << " OR set ExcludeNSWFromPrecisionLayers to True before crashing if you want to technically be able to run on old DAODs, noting that this is allowed only for testing purposes");
            throw std::runtime_error("Failed to retrieve NSW hits");
          }
          retrieveSummaryValue(muon, summary.etaLayer1STGCHits, xAOD::MuonSummaryType::etaLayer1STGCHits);
          retrieveSummaryValue(muon, summary.etaLayer2STGCHits, xAOD::MuonSummaryType::etaLayer2STGCHits);
          retrieveSummaryValue(muon, summary.MMHits, xAOD::MuonSummaryType::MMHits);
          summary.nprecisionLayers = 0;
          if (summary.middleSmallHits > 2 || summary.middleLargeHits > 2) summary.nprecisionLayers += 1;
          if (summary.outerSmallHits > 2 || summary.outerLargeHits > 2) summary.nprecisionLayers += 1;
          if (summary.extendedSmallHits > 2 || summary.extendedLargeHits > 2) summary.nprecisionLayers += 1;
          if (summary.etaLayer1STGCHits + summary.etaLayer2STGCHits > 3 || summary.MMHits > 3) summary.nprecisionLayers += 1;
        }

    }
    void MuonSelectionTool::retrieveParam(const xAOD::Muon& muon, float& value, const xAOD::Muon::ParamDef param) const {
        if (!muon.parameter(value, param)) {
            ATH_MSG_FATAL(__FILE__ << ":" << __LINE__ << " Failed to retrieve parameter " << param
                                   << " for muon with pT:" << muon.pt() * MeVtoGeV << ", eta:" << muon.eta() << ", phi: " << muon.phi()
                                   << ", q:" << muon.charge() << ", author: " << muon.author());
            throw std::runtime_error("Failed to retrieve Parameter");
        }
    }

    // Returns an integer corresponding to categorization of muons with different resolutions
    int MuonSelectionTool::getResolutionCategory(const xAOD::Muon& mu) const {
        // Resolutions have only been evaluated for medium combined muons
        if (mu.muonType() != xAOD::Muon::Combined || getQuality(mu) > xAOD::Muon::Medium) return ResolutionCategory::unclassified;

        // :: Access MS hits information
        hitSummary summary{};
        fillSummary(mu, summary);

        // For muons passing the high-pT working point, distinguish between 2-station tracks and the rest
        if (passedHighPtCuts(mu)) {
            if (summary.nprecisionLayers == 2)
                return ResolutionCategory::highPt2station;
            else
                return ResolutionCategory::highPt;
        }

        const xAOD::TrackParticle* CB_track = mu.trackParticle(xAOD::Muon::CombinedTrackParticle);
        const xAOD::TrackParticle* MS_track = mu.trackParticle(xAOD::Muon::MuonSpectrometerTrackParticle);
        if (!MS_track) {
            ATH_MSG_VERBOSE("getResolutionCategory - No MS track available for muon. Using combined track.");
            MS_track = mu.trackParticle(xAOD::Muon::CombinedTrackParticle);
        }

        if (!MS_track || !CB_track) return ResolutionCategory::unclassified;
        const float etaMS = MS_track->eta();
        const float etaCB = CB_track->eta();
        const float phiMS = MS_track->phi();

        int category = ResolutionCategory::unclassified;

        if ((summary.isSmallGoodSectors && summary.innerSmallHits < 3) || (!summary.isSmallGoodSectors && summary.innerLargeHits < 3))
            category = ResolutionCategory::missingInner;  // missing-inner

        if ((summary.isSmallGoodSectors && summary.middleSmallHits < 3) || (!summary.isSmallGoodSectors && summary.middleLargeHits < 3))
            category = ResolutionCategory::missingMiddle;  // missing-middle

        if ((summary.isSmallGoodSectors && summary.outerSmallHits < 3 && summary.extendedSmallHits < 3) ||
            (!summary.isSmallGoodSectors && summary.outerLargeHits < 3 && summary.extendedLargeHits < 3))
            category = ResolutionCategory::missingOuter;  // missing-outer

        if ((std::abs(etaMS) > 2.0 || std::abs(etaCB) > 2.0) && summary.cscUnspoiledEtaHits == 0)
            category = ResolutionCategory::spoiledCSC;  // spoiled CSC

        if ((1.01 < std::abs(etaMS) && std::abs(etaMS) < 1.1) || (1.01 < std::abs(etaCB) && std::abs(etaCB) < 1.1))
            category = ResolutionCategory::BEoverlap;  // barrel-end-cap overlap

        if (isBIS78(etaMS, phiMS)) category = ResolutionCategory::BIS78;  // BIS7/8

        //::: BEE
        if (isBEE(etaMS, phiMS) || (std::abs(etaCB) > 1.4 && (summary.extendedSmallHits > 0 || summary.extendedSmallHoles > 0))) {
            if (summary.extendedSmallHits < 3 && summary.middleSmallHits >= 3 && summary.outerSmallHits >= 3)
                category = ResolutionCategory::missingBEE;  // missing-BEE

            if (summary.extendedSmallHits >= 3 && summary.outerSmallHits < 3) category = ResolutionCategory::missingOuter;  // missing-outer

            if (!summary.isSmallGoodSectors)
                category = ResolutionCategory::unclassified;  // ambiguity due to eta/phi differences between MS and CB track
        }

        if (summary.nprecisionLayers == 1) category = ResolutionCategory::oneStation;  // one-station track

        return category;
    }

    // need run number (or random run number) to apply period-dependent selections
    unsigned int MuonSelectionTool::getRunNumber(bool needOnlyCorrectYear) const {
        static const SG::AuxElement::ConstAccessor<unsigned int> acc_rnd("RandomRunNumber");

        SG::ReadHandle<xAOD::EventInfo> eventInfo(m_eventInfo);

        if (!eventInfo->eventType(xAOD::EventInfo::IS_SIMULATION)) {
            ATH_MSG_DEBUG("The current event is a data event. Return runNumber.");
            return eventInfo->runNumber();
        }

        if (!acc_rnd.isAvailable(*eventInfo)) {
            if (needOnlyCorrectYear) 
            {
                if (eventInfo->runNumber() < 300000) 
                {
                    ATH_MSG_DEBUG("Random run number not available and this is mc16a or mc20a, returning dummy 2016 run number.");
                    return 311071;
                } 
                else if (eventInfo->runNumber() < 310000) 
                {
                    ATH_MSG_DEBUG("Random run number not available and this is mc16d or mc20d, returning dummy 2017 run number.");
                    return 340072;
                } 
                else if (eventInfo->runNumber() < 320000)
                {
                    ATH_MSG_DEBUG("Random run number not available and this is mc16e or mc20e, returning dummy 2018 run number.");
                    return 351359;
                }
                else if (eventInfo->runNumber() < 500000) //mc21 is 330000, mc23a is 410000, mc23c is 4500000
                {
                    ATH_MSG_DEBUG("Random run number not available and this is mc21/mc23, for the time being we're returing a dummy run number.");
                    return 399999;
                }
                else{
                  ATH_MSG_FATAL("Random run number not available, fallback option of using runNumber failed since "<<eventInfo->runNumber()<<" cannot be recognised");
                  throw std::runtime_error("MuonSelectionTool() - need RandomRunNumber decoration from PileupReweightingTool");
                }
            }//end of if (needOnlyCorrectYear) 
            else 
            {
                ATH_MSG_FATAL("Failed to find the RandomRunNumber decoration. Please call the apply() method from the PileupReweightingTool before");
                throw std::runtime_error("MuonSelectionTool() - need RandomRunNumber decoration from PileupReweightingTool");
            }
        } //end of if (!acc_rnd.isAvailable(*eventInfo))
        else if (acc_rnd(*eventInfo) == 0) 
        {
            static std::atomic<bool> firstPRWWarning{false};
            if(firstPRWWarning) ATH_MSG_WARNING("Pile up tool has given runNumber 0. Returning dummy 2017 run number.");
            ATH_MSG_DEBUG("Pile up tool has given runNumber 0. Returning dummy 2017 run number.");
            return 340072;
        }

        return acc_rnd(*eventInfo);
    }

    // Check if eta/phi coordinates correspond to BIS7/8 chambers
    bool MuonSelectionTool::isBIS78(const float eta, const float phi) const {
        static constexpr std::array<float, 2> BIS78_eta{1.05, 1.3};
        static constexpr std::array<float, 8> BIS78_phi{0.21, 0.57, 1.00, 1.33, 1.78, 2.14, 2.57, 2.93};

        float abs_eta = std::abs(eta);
        float abs_phi = std::abs(phi);

        if (abs_eta >= BIS78_eta[0] && abs_eta <= BIS78_eta[1]) {
            if ((abs_phi >= BIS78_phi[0] && abs_phi <= BIS78_phi[1]) || (abs_phi >= BIS78_phi[2] && abs_phi <= BIS78_phi[3]) ||
                (abs_phi >= BIS78_phi[4] && abs_phi <= BIS78_phi[5]) || (abs_phi >= BIS78_phi[6] && abs_phi <= BIS78_phi[7])) {
                return true;
            }
        }

        return false;
    }

    // Check if eta/phi coordinates correspond to BEE chambers
    bool MuonSelectionTool::isBEE(const float eta, const float phi) const {
        static constexpr std::array<float, 2> BEE_eta{1.440, 1.692};
        static constexpr std::array<float, 8> BEE_phi{0.301, 0.478, 1.086, 1.263, 1.872, 2.049, 2.657, 2.834};

        float abs_eta = std::abs(eta);
        float abs_phi = std::abs(phi);

        if (abs_eta >= BEE_eta[0] && abs_eta <= BEE_eta[1]) {
            if ((abs_phi >= BEE_phi[0] && abs_phi <= BEE_phi[1]) || (abs_phi >= BEE_phi[2] && abs_phi <= BEE_phi[3]) ||
                (abs_phi >= BEE_phi[4] && abs_phi <= BEE_phi[5]) || (abs_phi >= BEE_phi[6] && abs_phi <= BEE_phi[7])) {
                return true;
            }
        }

        return false;
    }

    // Check if eta/phi coordinates correspond to BMG chambers
    bool MuonSelectionTool::isBMG(const float eta, const float phi) const {
        static constexpr std::array<float, 6> BMG_eta{0.35, 0.47, 0.68, 0.80, 0.925, 1.04};
        static constexpr std::array<float, 4> BMG_phi{-1.93, -1.765, -1.38, -1.21};

        float abs_eta = std::abs(eta);

        if ((abs_eta >= BMG_eta[0] && abs_eta <= BMG_eta[1]) || (abs_eta >= BMG_eta[2] && abs_eta <= BMG_eta[3]) ||
            (abs_eta >= BMG_eta[4] && abs_eta <= BMG_eta[5])) {
            if ((phi >= BMG_phi[0] && phi <= BMG_phi[1]) || (phi >= BMG_phi[2] && phi <= BMG_phi[3])) { return true; }
        }

        return false;
    }

}  // namespace CP
