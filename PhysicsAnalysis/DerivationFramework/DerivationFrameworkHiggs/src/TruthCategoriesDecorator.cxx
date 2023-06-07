/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "DerivationFrameworkHiggs/TruthCategoriesDecorator.h"

#include <GaudiKernel/SystemOfUnits.h>
#include <MuonCondSvc/MdtStringUtils.h>
#include <PathResolver/PathResolver.h>
#include <StoreGate/ReadHandle.h>
#include <StoreGate/WriteDecorHandle.h>
#include <TEnv.h>
#include <TLorentzVector.h>
#include <TString.h>
#include <TruthUtils/HepMCHelpers.h>
#include <xAODJet/JetContainer.h>
#include <xAODTruth/TruthParticleContainer.h>
#include <xAODTruth/TruthVertex.h>

namespace DerivationFramework {

    TruthCategoriesDecorator::TruthCategoriesDecorator(const std::string& n, ISvcLocator* p) : AthReentrantAlgorithm(n, p) {}

    StatusCode TruthCategoriesDecorator::initialize() {
        ATH_MSG_DEBUG("Initialize ");

        // FOR xAOD->HEPMC ::  xAODtoHepMC tool
        ATH_CHECK(m_xAODtoHepMCTool.retrieve());
        ATH_CHECK(m_higgsTruthCatTool.retrieve());

        ATH_CHECK(m_truthEvtKey.initialize());
        ATH_CHECK(m_evtInfoKey.initialize());

        std::set<std::string> decor_keys{
            "HTXS_prodMode",
            "HTXS_errorCode",
            "HTXS_Stage0_Category",
            // Stage 1 binning
            "HTXS_Stage1_Category_pTjet25",
            "HTXS_Stage1_Category_pTjet30",
            "HTXS_Stage1_FineIndex_pTjet30",
            "HTXS_Stage1_FineIndex_pTjet25",
            // Stage 1.2 binning
            "HTXS_Stage1_2_Category_pTjet25",
            "HTXS_Stage1_2_Category_pTjet30",
            "HTXS_Stage1_2_FineIndex_pTjet30",
            "HTXS_Stage1_2_FineIndex_pTjet25",
            // stage 1.2. finer binning
            "HTXS_Stage1_2_Fine_Category_pTjet25",
            "HTXS_Stage1_2_Fine_Category_pTjet30",
            "HTXS_Stage1_2_Fine_FineIndex_pTjet30",
            "HTXS_Stage1_2_Fine_FineIndex_pTjet25",

            "HTXS_Njets_pTjet25",
            "HTXS_Njets_pTjet30",
            "HTXS_isZ2vvDecay",
        };

        if (!m_detailLevel) decor_keys.insert("HTXS_Higgs_pt");
        // The Higgs and the associated V (last instances prior to decay)
        if (m_detailLevel > 0) {
            m_p4_decors.insert(std::make_pair("HTXS_Higgs", FourMomDecoration{m_evtInfoKey, "HTXS_Higgs"}));
            m_p4_decors.insert(std::make_pair("HTXS_V", FourMomDecoration{m_evtInfoKey, "HTXS_V"}));
        }
        // Jets built excluding Higgs decay products
        if (m_detailLevel > 1) {
            m_p4_decors.insert(std::make_pair("HTXS_V_jets25", FourMomDecoration{m_evtInfoKey, "HTXS_V_jets25"}));
            m_p4_decors.insert(std::make_pair("HTXS_V_jets30", FourMomDecoration{m_evtInfoKey, "HTXS_V_jets30"}));
        }
        // Everybody might not want this ... but good for validation
        if (m_detailLevel > 2) {
            m_p4_decors.insert(std::make_pair("HTXS_Higgs_decay", FourMomDecoration{m_evtInfoKey, "HTXS_Higgs_decay"}));
            m_p4_decors.insert(std::make_pair("HTXS_V_decay", FourMomDecoration{m_evtInfoKey, "HTXS_V_decay"}));
        }

        for (const std::string& key : decor_keys) m_STXSDecors.emplace_back(m_evtInfoKey.key() + "." + key);
        for (auto& p4_pair : m_p4_decors) {
            std::vector<EvtInfoDecorKey> keys = p4_pair.second.vect();
            for (EvtInfoDecorKey key : keys) m_STXSDecors.push_back(key);
            ATH_CHECK(p4_pair.second.initialize());
               
        }
        /// Declare everything that's written by the algorithm
        ATH_CHECK(m_STXSDecors.initialize());
        // Open the TEnv configuration file
        TEnv config{};
        if (config.ReadFile(PathResolverFindCalibFile(m_configPath).c_str(), EEnvLevel(0))) {
            ATH_MSG_FATAL("Failed to open TEnv file " << m_configPath);
            return StatusCode::FAILURE;
        }

        for (const std::string prod_mode : {"GGF", "VBF", "WH", "QQ2ZH", "GG2ZH", "TTH", "BBH", "TH", "THQB", "WHT"}) {
            HTXSSample smp{};
            if (prod_mode == "GGF")
                smp.prod = HTXS::HiggsProdMode::GGF;
            else if (prod_mode == "VBF")
                smp.prod = HTXS::HiggsProdMode::VBF;
            else if (prod_mode == "WH")
                smp.prod = HTXS::HiggsProdMode::WH;
            else if (prod_mode == "QQ2ZH")
                smp.prod = HTXS::HiggsProdMode::QQ2ZH;
            else if (prod_mode == "GG2ZH")
                smp.prod = HTXS::HiggsProdMode::GG2ZH;
            else if (prod_mode == "TTH")
                smp.prod = HTXS::HiggsProdMode::TTH;
            else if (prod_mode == "BBH")
                smp.prod = HTXS::HiggsProdMode::BBH;
            else if (prod_mode == "TH")
                smp.prod = HTXS::HiggsProdMode::TH;
            else if (prod_mode == "THQB") {
                smp.th_type = HTXS::tH_type::THQB;
                smp.prod = HTXS::HiggsProdMode::TH;
            } else if (prod_mode == "WHT") {
                smp.th_type = HTXS::tH_type::TWH;
                smp.prod = HTXS::HiggsProdMode::TH;
            }
            std::vector<std::string> dsid_str{};
            MuonCalib::MdtStringUtils::tokenize(config.GetValue(Form("HTXS.MCsamples.%s", prod_mode.c_str()), ""), dsid_str, " ");
            for (const std::string& dsid : dsid_str) { smp.dsids.insert(std::atoi(dsid.c_str())); }
            m_htxs_samples.push_back(std::move(smp));
        }
        return StatusCode::SUCCESS;
    }

    // Save a TLV as 4 floats
    StatusCode TruthCategoriesDecorator::decorateFourVec(const EventContext& ctx, const std::string& prefix,
                                                         const TLorentzVector& p4) const {
        P4DecorMap::const_iterator itr = m_p4_decors.find(prefix);
        if (itr == m_p4_decors.end()) {
            ATH_MSG_FATAL("decorateFourVec() -- Key " << prefix << " is unknown");
            return StatusCode::FAILURE;
        }
        using FloatDecorator = SG::WriteDecorHandle<xAOD::EventInfo, float>;
        FloatDecorator dec_pt{itr->second.pt, ctx};
        FloatDecorator dec_eta{itr->second.eta, ctx};
        FloatDecorator dec_phi{itr->second.phi, ctx};
        FloatDecorator dec_m{itr->second.m, ctx};
        const xAOD::EventInfo* eventInfo = dec_pt.cptr();
        dec_pt(*eventInfo) = p4.Pt() * Gaudi::Units::GeV;
        dec_eta(*eventInfo) = p4.Eta();
        dec_phi(*eventInfo) = p4.Phi();
        dec_m(*eventInfo) = p4.M() * Gaudi::Units::GeV;
        ATH_MSG_DEBUG("Decorate "<<prefix<<" with pT: "<<p4.Pt()<<" [GeV], eta: "<<p4.Eta()<<", phi: "<<p4.Phi()<<", M: "<<p4.M());
        return StatusCode::SUCCESS;
    }

    // Save a vector of TLVs as vectors of float
    StatusCode TruthCategoriesDecorator::decorateFourVecs(const EventContext& ctx, const std::string& prefix,
                                                          const std::vector<TLorentzVector>& p4s) const {
        P4DecorMap::const_iterator itr = m_p4_decors.find(prefix);
        if (itr == m_p4_decors.end()) {
            ATH_MSG_FATAL("decorateFourVecs() -- Key " << prefix << " is unknown");
            return StatusCode::FAILURE;
        }
        using FloatDecorator = SG::WriteDecorHandle<xAOD::EventInfo, std::vector<float>>;
        FloatDecorator dec_pt{itr->second.pt, ctx};
        FloatDecorator dec_eta{itr->second.eta, ctx};
        FloatDecorator dec_phi{itr->second.phi, ctx};
        FloatDecorator dec_m{itr->second.m, ctx};
        const xAOD::EventInfo* eventInfo = dec_pt.cptr();

        std::vector<float>&pt = dec_pt(*eventInfo) ;
        std::vector<float>&eta = dec_eta(*eventInfo);
        std::vector<float>&phi = dec_phi(*eventInfo) ;
        std::vector<float>&mass = dec_m(*eventInfo) ;
        for (const TLorentzVector& p4 : p4s) {
            pt.push_back(p4.Pt() * Gaudi::Units::GeV);
            eta.push_back(p4.Eta());
            phi.push_back(p4.Phi());
            mass.push_back(p4.M() * Gaudi::Units::GeV);
            ATH_MSG_DEBUG("Decorate "<<prefix<<" with pT: "<<p4.Pt()<<" [GeV], eta: "<<p4.Eta()<<", phi: "<<p4.Phi()<<", M: "<<p4.M());
        }
        return StatusCode::SUCCESS;
    }

    StatusCode TruthCategoriesDecorator::execute(const EventContext& ctx) const {
        using IntDecorator = SG::AuxElement::Decorator<int>;
        using FloatDecorator = SG::AuxElement::Decorator<float>;
        // Retrieve the xAOD event info
        SG::ReadHandle<xAOD::EventInfo> eventInfo{m_evtInfoKey, ctx};
        if (!eventInfo.isValid()) {
            ATH_MSG_FATAL("Failed to retrieve " << m_evtInfoKey.fullKey());
            return StatusCode::FAILURE;
        }

        int mcChannelNumber = eventInfo->mcChannelNumber();
        if (!mcChannelNumber) mcChannelNumber = eventInfo->runNumber();  // EVNT input

        static const IntDecorator dec_prodMode{"HTXS_prodMode"};
        std::vector<HTXSSample>::const_iterator smp_itr =
            std::find_if(m_htxs_samples.begin(), m_htxs_samples.end(),
                         [mcChannelNumber](const HTXSSample& smp) { return smp.dsids.count(mcChannelNumber); });
        if (smp_itr == m_htxs_samples.end()) {
            dec_prodMode(*eventInfo) = HTXS::HiggsProdMode::UNKNOWN;
            ATH_MSG_DEBUG("The sample " << mcChannelNumber
                                        << " is not a Higgs sample and not of further interest. Decorate the prod mode information only");
            return StatusCode::SUCCESS;
        }

        const HTXS::HiggsProdMode prodMode = smp_itr->prod;
        const HTXS::tH_type th_type = smp_itr->th_type;

        // Retrieve the xAOD truth
        SG::ReadHandle<xAOD::TruthEventContainer> xTruthEventContainer{m_truthEvtKey, ctx};
        if (!xTruthEventContainer.isValid()) {
            ATH_MSG_FATAL("Failed to retrieve " << m_truthEvtKey.fullKey());
            return StatusCode::FAILURE;
        }
        // convert xAOD -> HepMC
        std::vector<HepMC::GenEvent> hepmc_evts = m_xAODtoHepMCTool->getHepMCEvents(xTruthEventContainer.cptr(), eventInfo.cptr());

        if (hepmc_evts.empty()) {
            // ANGRY MESSAGE HERE
            ATH_MSG_FATAL("The HEP MC GenEvent conversion failed");
            return StatusCode::FAILURE;
        } else {
            ATH_MSG_DEBUG("Found "<<hepmc_evts.size()<<" HepMC events.");
        }

        // classify event according to simplified template cross section
        std::unique_ptr<HTXS::HiggsClassification> htxs{m_higgsTruthCatTool->getHiggsTruthCategoryObject(hepmc_evts[0], prodMode)};
        ATH_MSG_DEBUG("Truth categorization done ");
        // Decorate the enums
        static const IntDecorator dec_errorCode{"HTXS_errorCode"};
        static const IntDecorator dec_stage0Cat{"HTXS_Stage0_Category"};

        dec_prodMode(*eventInfo) = htxs->prodMode;
        dec_errorCode(*eventInfo) = htxs->errorCode;
        dec_stage0Cat(*eventInfo) = htxs->stage0_cat;

        // Stage-1 binning
        static const IntDecorator dec_stage1CatPt25{"HTXS_Stage1_Category_pTjet25"};
        static const IntDecorator dec_stage1CatPt30{"HTXS_Stage1_Category_pTjet30"};
        static const IntDecorator dec_stage1IdxPt25{"HTXS_Stage1_FineIndex_pTjet25"};
        static const IntDecorator dec_stage1IdxPt30{"HTXS_Stage1_FineIndex_pTjet30"};

        dec_stage1CatPt25(*eventInfo) = htxs->stage1_cat_pTjet25GeV;
        dec_stage1CatPt30(*eventInfo) = htxs->stage1_cat_pTjet30GeV;
        /// Last argument switches between 25 GeV (true) and 30 GeV jets (false)
        dec_stage1IdxPt25(*eventInfo) = HTXSstage1_to_HTXSstage1FineIndex(*htxs, th_type, true);
        dec_stage1IdxPt30(*eventInfo) = HTXSstage1_to_HTXSstage1FineIndex(*htxs, th_type, false);

        // Stage-1.2 binning
        static const IntDecorator dec_stage1p2_CatPt25{"HTXS_Stage1_2_Category_pTjet25"};
        static const IntDecorator dec_stage1p2_CatPt30{"HTXS_Stage1_2_Category_pTjet30"};
        static const IntDecorator dec_stage1p2_IdxPt25{"HTXS_Stage1_2_FineIndex_pTjet25"};
        static const IntDecorator dec_stage1p2_IdxPt30{"HTXS_Stage1_2_FineIndex_pTjet30"};

        dec_stage1p2_CatPt25(*eventInfo) = htxs->stage1_2_cat_pTjet25GeV;
        dec_stage1p2_CatPt30(*eventInfo) = htxs->stage1_2_cat_pTjet30GeV;
        /// Last argument switches between 25 (true) / 30 (false) GeV jets.
        dec_stage1p2_IdxPt25(*eventInfo) = HTXSstage1_2_to_HTXSstage1_2_FineIndex(*htxs, th_type, true);
        dec_stage1p2_IdxPt30(*eventInfo) = HTXSstage1_2_to_HTXSstage1_2_FineIndex(*htxs, th_type, false);

        // Stage-1.2 finer binning
        static const IntDecorator dec_stage1p2_Fine_CatPt25{"HTXS_Stage1_2_Fine_Category_pTjet25"};
        static const IntDecorator dec_stage1p2_Fine_CatPt30{"HTXS_Stage1_2_Fine_Category_pTjet30"};
        static const IntDecorator dec_stage1p2_Fine_IdxPt25{"HTXS_Stage1_2_Fine_FineIndex_pTjet25"};
        static const IntDecorator dec_stage1p2_Fine_IdxPt30{"HTXS_Stage1_2_Fine_FineIndex_pTjet30"};

        dec_stage1p2_Fine_CatPt25(*eventInfo) = htxs->stage1_2_fine_cat_pTjet25GeV;
        dec_stage1p2_Fine_CatPt30(*eventInfo) = htxs->stage1_2_fine_cat_pTjet30GeV;
        dec_stage1p2_Fine_IdxPt25(*eventInfo) = HTXSstage1_2_Fine_to_HTXSstage1_2_Fine_FineIndex(*htxs, th_type, true);
        dec_stage1p2_Fine_IdxPt30(*eventInfo) = HTXSstage1_2_Fine_to_HTXSstage1_2_Fine_FineIndex(*htxs, th_type, false);

        static const IntDecorator dec_NJets25{"HTXS_Njets_pTjet25"};
        static const IntDecorator dec_NJets30{"HTXS_Njets_pTjet30"};
        dec_NJets25(*eventInfo) = htxs->jets25.size();
        dec_NJets30(*eventInfo) = htxs->jets30.size();

        static const IntDecorator dec_isZnunu{"HTXS_isZ2vvDecay"};
        dec_isZnunu(*eventInfo) = htxs->isZ2vvDecay;

        // At the very least, save the Higgs boson pT
        if (!m_detailLevel) {
            static const FloatDecorator dec_higgsPt{"HTXS_Higgs_pt"};
            dec_higgsPt(*eventInfo) = htxs->higgs.Pt() * Gaudi::Units::GeV;
        }
        if (m_detailLevel > 0) {
            // The Higgs and the associated V (last instances prior to decay)
            ATH_CHECK(decorateFourVec(ctx, "HTXS_Higgs", htxs->higgs));
            ATH_CHECK(decorateFourVec(ctx, "HTXS_V", htxs->V));
        }

        if (m_detailLevel > 1) {
            // Jets built excluding Higgs decay products
            ATH_CHECK(decorateFourVecs(ctx, "HTXS_V_jets25", htxs->jets25));
            ATH_CHECK(decorateFourVecs(ctx, "HTXS_V_jets30", htxs->jets30));
        }

        if (m_detailLevel > 2) {
            // Everybody might not want this ... but good for validation
            ATH_CHECK(decorateFourVec(ctx, "HTXS_Higgs_decay", htxs->p4decay_higgs));
            ATH_CHECK(decorateFourVec(ctx, "HTXS_V_decay", htxs->p4decay_V));
        }
        /// Summary of the HTXS categorization. Used mainly in the testing algorithm
        ATH_MSG_DEBUG("production mode: " << dec_prodMode(*eventInfo) << ", errorCode: " << dec_errorCode(*eventInfo) << ", Stage0: " << dec_stage0Cat(*eventInfo) 
                                         << ",  " << dec_stage1CatPt25(*eventInfo) << ", Stage1  -- Jet25 " << dec_stage1CatPt25(*eventInfo) << " Idx " << dec_stage1IdxPt25(*eventInfo) 
                                         << ", Jet30: " << dec_stage1CatPt30(*eventInfo) << " Idx "<< dec_stage1IdxPt30(*eventInfo) << ", Stage 1.2 -- Jet25: " 
                                         << dec_stage1p2_CatPt25(*eventInfo) << " Idx: " << dec_stage1p2_IdxPt25(*eventInfo) << " Jet30: " << dec_stage1p2_CatPt30(*eventInfo) 
                                         << " Idx: " << dec_stage1p2_IdxPt30(*eventInfo) << ", HTXS NJets 25" << dec_NJets25(*eventInfo) 
                                         << " HTXS NJets 30 " << dec_NJets30(*eventInfo) << " Z->nunu" << dec_isZnunu(*eventInfo));

        return StatusCode::SUCCESS;
    }

}  // namespace DerivationFramework
