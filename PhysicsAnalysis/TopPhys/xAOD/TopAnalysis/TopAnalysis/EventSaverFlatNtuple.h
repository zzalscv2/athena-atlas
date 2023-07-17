/*
   Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
 */

#ifndef EVENTSAVERFLATNTUPLE_H_
#define EVENTSAVERFLATNTUPLE_H_

#include "TopAnalysis/EventSaverBase.h"
#include "TopCorrections/ScaleFactorRetriever.h"
#include "TopEventSelectionTools/TreeManager.h"

// Framework include(s):
#include "AsgTools/AsgTool.h"
#include "AthContainers/AuxElement.h"
#include "AthContainers/DataVector.h"
#include <unordered_map>


namespace top {

/**
 * @brief Some code that writes out a 'standard' flat ntuple.  This can be
 * extended if the user wants to inherit from this and implement the functions
 * themselves.
 */
  class EventSaverFlatNtuple: public top::EventSaverBase, public asg::AsgTool  {
  public:
    /**
     * @brief A default constructor so that this class can be created at
     * run-time by ROOT.
     */
    EventSaverFlatNtuple();

    /**
     * @brief Does nothing.
     */
    virtual ~EventSaverFlatNtuple();

    /**
     * @brief initialise is run before the event loop.
     *
     * @param config The configuration object (not used by the default flat
     * ntuple).
     * @param file The output file is used to put the TTrees in.
     * @param extraBranches Extra info requested (like which selections are
     * passed).
     */
    virtual void initialize(std::shared_ptr<top::TopConfig> config, TFile* file,
                            const std::vector<std::string>& extraBranches) override;

    /**
     * @brief Execute stuff common for reco/particle/parton level,
     * e.g. loading of weights, PDF sets, etc.
     */
    virtual void execute() override;

    //Keep the asg::AsgTool happy
    virtual StatusCode initialize() override {return StatusCode::SUCCESS;}

    /**
     * @brief Run for every event (actually every systematic for every event).
     *
     * This does the tree filling work.
     *
     * @param event The top::Event which has had object selection and overlap
     * removal (if requested) applied to it.
     */
    virtual void saveEvent(const top::Event& event) override; // calls the three next functions
    virtual void cleanEvent(); // (re-)initialise all relevant variables to default (dummy) values
    virtual void calculateEvent(const top::Event& event); // calculate the relevant variables
    virtual void fillEvent(const top::Event& event); // calls tree->Fill

    /**
     * @breif Run for every event
     *
     * This fills, if requested, the:
     *   MC truth record
     *   PDF info
     *   TopPartons
     */
    virtual void saveTruthEvent() override; // calls the three next functions
    virtual void cleanTruthEvent(); // (re-)initialise all relevant variables to default (dummy) values
    virtual void calculateTruthEvent(); // calculate the relevant variables
    virtual void fillTruthEvent(); // calls tree->Fill

    /*!
     * @brief Store the particle level event's content in the particle level
     * TTree as flat n-tuple.
     *
     * This function shall be executed once for each event which is intended for
     * storage. It writes out the pass/fail results of all EventSelection chains
     * that have been applied to that specific event object.
     *
     * @param plEvent The particle level event whose data content will be
     * written to the output.
     */
    virtual void saveParticleLevelEvent(const top::ParticleLevelEvent& plEvent) override; // calls the three next functions
    virtual void cleanParticleLevelEvent(); // (re-)initialise all relevant variables to default (dummy) values
    virtual void calculateParticleLevelEvent(const top::ParticleLevelEvent& plEvent); // calculate the relevant
                                                                                      // variables
    virtual void fillParticleLevelEvent(); // calls tree->Fill

    /**
     * @brief Not used by the flat ntuple code yet, but needed by the xAOD code.
     */
    virtual void finalize() override;

    /**
     * @brief shorten name of b-tagging working point (FixedCutBEff_*)
     */
    std::string shortBtagWP(std::string const& WP) const;
  protected:
    /**
     * @brief Allow child classes to access the configuration object.
     */
    std::shared_ptr<top::TopConfig> topConfig();

    /**
     * @brief Allow classes that build on top of this to access the tree managers.
     *
     * Why not just make the variable that contains them protected? Because of
     * http://programmers.stackexchange.com/questions/162643/why-is-clean-code-suggesting-avoiding-protected-variables
     *
     * @return A vector of the treeManagers (one for each systematic).
     */
    std::vector<std::shared_ptr<top::TreeManager> > treeManagers();
    std::shared_ptr<top::TreeManager> truthTreeManager();

    /*!
     * @brief Return a shared pointer to the top::TreeManager object that is
     * used for the particle level output tree.
     * @returns A shared pointer to the top::TreeManager object that is used to
     * write out the particle level event data.
     */
    std::shared_ptr<top::TreeManager> particleLevelTreeManager();

    /**
     * @brief Return a shared pointer to the top::ScaleFactorRetriever object
     * which allows us to easily access the SF decorations
     * @returns A shared pointer to the top::ScaleFactorRetriever object
     */
    //std::shared_ptr<top::ScaleFactorRetriever> scaleFactorRetriever();
    top::ScaleFactorRetriever* scaleFactorRetriever();

    /*!
     * @brief Function to access the branch filters - cf ANALYSISTO-61
     * @returns A shared pointer to the m_branchFilters object
     */
    std::vector<top::TreeManager::BranchFilter>& branchFilters();
    
    /*!
     * @brief Internal function which configures the particle level tree
     * manager. It does branch setup etc.
     */
    void setupParticleLevelTreeManager(/*const top::ParticleLevelEvent& plEvent*/);

  private:
    /**
     * @brief For each selection that we run in the top-xaod code, we "decorate"
     * event info with a variable saying if this event passed or failed it.
     *
     * This code takes the decoration from event info
     *
     * @param event The top event, need this so we can access the decorators on EventInfo
     * @param extraBranches The names of the branches we added to EventInfo (one per event)
     */
    void recordSelectionDecision(const top::Event& event);
    void recordTriggerDecision(const top::Event& event);

    void registerObjectIntoTruthTree(const SG::AuxElement& obj);
    void saveObjectIntoTruthTree(const SG::AuxElement& obj);

    /*!
     * @brief Helper function to load the PDF info from the current truth event
     *  and write it into the branch output variable(s).
     *
     * After calling this function, the `m_PDFinfo*` variables will contain the
     * current truth event's PDF info data (given that the truth event is
     * valid).
     */
    void loadPdfInfo();

    /*!
     * @brief Helper function to load the PDF weights from the current truth
     * event and write them into the branch output variable.
     *
     * After calling this function, the `m_PDF_eventWeights` variable will
     * contain the current truth event's LHA PDF weights for the requested PDF
     * sets (given that the truth event is valid).
     */
    void loadPdfWeights();

    /*!
     * @brief Helper function to load the OTF-computed MC generator weights.
     */
    void loadMCGeneratorWeights();

    ///Configuration
    std::shared_ptr<top::TopConfig> m_config;

    ///Scale factors
    //std::shared_ptr<top::ScaleFactorRetriever> m_sfRetriever;
    top::ScaleFactorRetriever* m_sfRetriever;

    ///The file where everything goes
    TFile* m_outputFile;

    ///A simple way to write out branches, without having to worry about the type.
    std::vector<std::shared_ptr<top::TreeManager> > m_treeManagers;
    std::shared_ptr<top::TreeManager> m_truthTreeManager;

    /// The top::TreeManager used for the particle level data output.
    std::shared_ptr<top::TreeManager> m_particleLevelTreeManager;
    /// The array of variables used for storing the particle level selection
    /// decisions. The std::pair contains:
    ///    .first  --- The name of the selection
    ///    .second --- The variable used for storing into the TTree
    std::vector< std::pair<std::string, int> > m_particleLevel_SelectionDecisions;

    ///names of the passed / failed branches.
    std::vector<std::string> m_extraBranches;

    /// branch filters - cf ANALYSISTO-61
    std::vector<top::TreeManager::BranchFilter> m_branchFilters;

    /// remove "FT_EFF_" and spaces in named systematics
    std::string betterBtagNamedSyst(const std::string& WP);

    /////////////////////////////////////
    /// Definition of event variables
    /////////////////////////////////////

    ///Decisions on if the event passed / failed a particular selection.
    std::vector<int> m_selectionDecisions;

    ///Decisions on if the event passed / failed a particular trigger.
    std::unordered_map<std::string, char> m_triggerDecisions;

    ///Pre-scale of the trigger menu for each event.
    std::unordered_map<std::string, float> m_triggerPrescales;

    //Store output PDF weights from LHAPDF
    std::unordered_map<std::string, std::vector<float> > m_PDF_eventWeights;

    // Store boosted jet tagger names
    std::vector<std::string> m_boostedJetTaggersNames;
    std::vector<std::string> m_boostedJetTaggersNamesCalibrated;

    //some event weights
    float m_weight_mc;
    float m_weight_beamspot;
    float m_weight_pileup;
    ///-- Pileup SF systematics --///
    float m_weight_pileup_UP;
    float m_weight_pileup_DOWN;
    ///---Fwd electrons --///
    float m_weight_fwdElSF;
    float m_weight_fwdElSF_FWDEL_SF_ID_UP;
    float m_weight_fwdElSF_FWDEL_SF_ID_DOWN;

    ///-- Lepton SF --///
    float m_weight_leptonSF;
    ///-- Lepton SF - electron SF systematics --///
    float m_weight_leptonSF_EL_SF_Reco_UP;
    float m_weight_leptonSF_EL_SF_Reco_DOWN;
    float m_weight_leptonSF_EL_SF_ID_UP;
    float m_weight_leptonSF_EL_SF_ID_DOWN;
    float m_weight_leptonSF_EL_SF_Isol_UP;
    float m_weight_leptonSF_EL_SF_Isol_DOWN;
    ///-- electron SF advanded correlation model systematics --///
    std::vector<float> m_weight_leptonSF_EL_SF_CorrModel_Reco_UP;
    std::vector<float> m_weight_leptonSF_EL_SF_CorrModel_Reco_DOWN;
    std::vector<float> m_weight_leptonSF_EL_SF_CorrModel_ID_UP;
    std::vector<float> m_weight_leptonSF_EL_SF_CorrModel_ID_DOWN;
    std::vector<float> m_weight_leptonSF_EL_SF_CorrModel_Iso_UP;
    std::vector<float> m_weight_leptonSF_EL_SF_CorrModel_Iso_DOWN;

    // Muon ID SF systematics (regular)
    float m_weight_leptonSF_MU_SF_ID_STAT_UP;
    float m_weight_leptonSF_MU_SF_ID_STAT_DOWN;
    float m_weight_leptonSF_MU_SF_ID_SYST_UP;
    float m_weight_leptonSF_MU_SF_ID_SYST_DOWN;
    // Muon ID SF systematics (low pT)
    float m_weight_leptonSF_MU_SF_ID_STAT_LOWPT_UP;
    float m_weight_leptonSF_MU_SF_ID_STAT_LOWPT_DOWN;
    float m_weight_leptonSF_MU_SF_ID_SYST_LOWPT_UP;
    float m_weight_leptonSF_MU_SF_ID_SYST_LOWPT_DOWN;
    //Muon breakdown systematics
    float m_weight_leptonSF_MU_SF_ID_BKG_FRACTION_UP;
    float m_weight_leptonSF_MU_SF_ID_BKG_FRACTION_DOWN;
    float m_weight_leptonSF_MU_SF_ID_FIT_MODEL_LOWPT_UP;
    float m_weight_leptonSF_MU_SF_ID_FIT_MODEL_LOWPT_DOWN;
    float m_weight_leptonSF_MU_SF_ID_LUMI_UNCERT_UP;
    float m_weight_leptonSF_MU_SF_ID_LUMI_UNCERT_DOWN;
    float m_weight_leptonSF_MU_SF_ID_MATCHING_UP;
    float m_weight_leptonSF_MU_SF_ID_MATCHING_DOWN;
    float m_weight_leptonSF_MU_SF_ID_MATCHING_LOWPT_UP;
    float m_weight_leptonSF_MU_SF_ID_MATCHING_LOWPT_DOWN;
    float m_weight_leptonSF_MU_SF_ID_MC_XSEC_UP;
    float m_weight_leptonSF_MU_SF_ID_MC_XSEC_DOWN;
    float m_weight_leptonSF_MU_SF_ID_PT_DEPENDENCY_UP;
    float m_weight_leptonSF_MU_SF_ID_PT_DEPENDENCY_DOWN;
    float m_weight_leptonSF_MU_SF_ID_QCD_TEMPLATE_UP;
    float m_weight_leptonSF_MU_SF_ID_QCD_TEMPLATE_DOWN;
    float m_weight_leptonSF_MU_SF_ID_SUPRESSION_SCALE_UP;
    float m_weight_leptonSF_MU_SF_ID_SUPRESSION_SCALE_DOWN;
    float m_weight_leptonSF_MU_SF_ID_TRUTH_UP;
    float m_weight_leptonSF_MU_SF_ID_TRUTH_DOWN;
    float m_weight_leptonSF_MU_SF_ID_TRUTH_LOWPT_UP;
    float m_weight_leptonSF_MU_SF_ID_TRUTH_LOWPT_DOWN;
    float m_weight_leptonSF_MU_SF_ID_BAD_MUON_VETO_UP;
    float m_weight_leptonSF_MU_SF_ID_BAD_MUON_VETO_DOWN;
    float m_weight_leptonSF_MU_SF_ID_CR1_UP;
    float m_weight_leptonSF_MU_SF_ID_CR1_DOWN;
    float m_weight_leptonSF_MU_SF_ID_CR2_UP;
    float m_weight_leptonSF_MU_SF_ID_CR2_DOWN;
    float m_weight_leptonSF_MU_SF_ID_CR3_UP;
    float m_weight_leptonSF_MU_SF_ID_CR3_DOWN;
    float m_weight_leptonSF_MU_SF_ID_HIGHETA_PROBEIP_UP;
    float m_weight_leptonSF_MU_SF_ID_HIGHETA_PROBEIP_DOWN;
    float m_weight_leptonSF_MU_SF_ID_HIGHETA_PROBEISO_UP;
    float m_weight_leptonSF_MU_SF_ID_HIGHETA_PROBEISO_DOWN;
    float m_weight_leptonSF_MU_SF_ID_TAGPT_UP;
    float m_weight_leptonSF_MU_SF_ID_TAGPT_DOWN;
    float m_weight_leptonSF_MU_SF_ID_EXTRAPOLATION_UP;
    float m_weight_leptonSF_MU_SF_ID_EXTRAPOLATION_DOWN;
    float m_weight_leptonSF_MU_SF_ID_EXTRAPOLATION_LOWPT_UP;
    float m_weight_leptonSF_MU_SF_ID_EXTRAPOLATION_LOWPT_DOWN;
    // Muon isolation SF systematics
    float m_weight_leptonSF_MU_SF_Isol_STAT_UP;
    float m_weight_leptonSF_MU_SF_Isol_STAT_DOWN;
    float m_weight_leptonSF_MU_SF_Isol_SYST_UP;
    float m_weight_leptonSF_MU_SF_Isol_SYST_DOWN;
    float m_weight_leptonSF_MU_SF_Isol_BKG_FRACTION_UP;
    float m_weight_leptonSF_MU_SF_Isol_BKG_FRACTION_DOWN;
    float m_weight_leptonSF_MU_SF_Isol_DRMUJ_UP;
    float m_weight_leptonSF_MU_SF_Isol_DRMUJ_DOWN;
    float m_weight_leptonSF_MU_SF_Isol_LUMI_UNCERT_UP;
    float m_weight_leptonSF_MU_SF_Isol_LUMI_UNCERT_DOWN;
    float m_weight_leptonSF_MU_SF_Isol_MC_XSEC_UP;
    float m_weight_leptonSF_MU_SF_Isol_MC_XSEC_DOWN;
    float m_weight_leptonSF_MU_SF_Isol_MLLWINDOW_UP;
    float m_weight_leptonSF_MU_SF_Isol_MLLWINDOW_DOWN;
    float m_weight_leptonSF_MU_SF_Isol_QCD_TEMPLATE_UP;
    float m_weight_leptonSF_MU_SF_Isol_QCD_TEMPLATE_DOWN;
    float m_weight_leptonSF_MU_SF_Isol_SHERPA_POWHEG_UP;
    float m_weight_leptonSF_MU_SF_Isol_SHERPA_POWHEG_DOWN;
    float m_weight_leptonSF_MU_SF_Isol_SUPRESSION_SCALE_UP;
    float m_weight_leptonSF_MU_SF_Isol_SUPRESSION_SCALE_DOWN;
    float m_weight_leptonSF_MU_SF_Isol_EXTRAPOLATION_UP;
    float m_weight_leptonSF_MU_SF_Isol_EXTRAPOLATION_DOWN;
    float m_weight_leptonSF_MU_SF_TTVA_STAT_UP;
    float m_weight_leptonSF_MU_SF_TTVA_STAT_DOWN;
    float m_weight_leptonSF_MU_SF_TTVA_SYST_UP;
    float m_weight_leptonSF_MU_SF_TTVA_SYST_DOWN;
    float m_weight_leptonSF_MU_SF_TTVA_BKG_FRACTION_UP;
    float m_weight_leptonSF_MU_SF_TTVA_BKG_FRACTION_DOWN;
    float m_weight_leptonSF_MU_SF_TTVA_LUMI_UNCERT_UP;
    float m_weight_leptonSF_MU_SF_TTVA_LUMI_UNCERT_DOWN;
    float m_weight_leptonSF_MU_SF_TTVA_MC_XSEC_UP;
    float m_weight_leptonSF_MU_SF_TTVA_MC_XSEC_DOWN;
    float m_weight_leptonSF_MU_SF_TTVA_QCD_TEMPLATE_UP;
    float m_weight_leptonSF_MU_SF_TTVA_QCD_TEMPLATE_DOWN;
    float m_weight_leptonSF_MU_SF_TTVA_SUPRESSION_SCALE_UP;
    float m_weight_leptonSF_MU_SF_TTVA_SUPRESSION_SCALE_DOWN;
    float m_weight_leptonSF_MU_SF_TTVA_EXTRAPOLATION_UP;
    float m_weight_leptonSF_MU_SF_TTVA_EXTRAPOLATION_DOWN;

    ///-- triggers --///
    float m_weight_trigger;
    float m_weight_trigger_EL_SF_UP;
    float m_weight_trigger_EL_SF_DOWN;
    float m_weight_trigger_MU_SF_STAT_UP;
    float m_weight_trigger_MU_SF_STAT_DOWN;
    float m_weight_trigger_MU_SF_SYST_UP;
    float m_weight_trigger_MU_SF_SYST_DOWN;
    float m_weight_trigger_PH_UNCERT_UP;
    float m_weight_trigger_PH_UNCERT_DOWN;

    // Taus
    float m_weight_tauSF;
    std::map<top::topSFSyst, float> m_weight_tauSF_variations;

    // Photons
    float m_weight_photonSF = 0.;
    float m_weight_photonSF_ID_UP = 0.;
    float m_weight_photonSF_ID_DOWN = 0.;
    float m_weight_photonSF_effIso_UP = 0.;
    float m_weight_photonSF_effIso_DOWN = 0.;
    float m_weight_photonSF_Trigger_UNCERT_UP = 0.;
    float m_weight_photonSF_Trigger_UNCERT_DOWN = 0.;

    // nominal b-tagging SF [WP]
    std::unordered_map<std::string, float> m_weight_bTagSF;
    std::unordered_map<std::string, float> m_weight_trackjet_bTagSF;

    // per-jet nominal btag SF
    std::unordered_map<std::string, std::vector<float> > m_perjet_weight_bTagSF;
    std::unordered_map<std::string, std::vector<float> > m_perjet_weight_trackjet_bTagSF;

    // JVT (c++11 initialization for fun)
    float m_weight_jvt = 0.0;
    float m_weight_jvt_up = 0.0;
    float m_weight_jvt_down = 0.0;
    // fJVT
    float m_weight_forwardjvt = 0.0;
    float m_weight_forwardjvt_up = 0.0;
    float m_weight_forwardjvt_down = 0.0;

    // Sherpa 2.2 weight
    float m_weight_sherpa_22_vjets = 0.;

    // eigen variations affecting b-jets [WP]
    std::unordered_map<std::string, std::vector<float> > m_weight_bTagSF_eigen_B_up;
    std::unordered_map<std::string, std::vector<float> > m_weight_bTagSF_eigen_B_down;
    std::unordered_map<std::string, std::vector<float> > m_weight_trackjet_bTagSF_eigen_B_up;
    std::unordered_map<std::string, std::vector<float> > m_weight_trackjet_bTagSF_eigen_B_down;

    std::unordered_map<std::string, std::vector<std::vector<float> > > m_perjet_weight_bTagSF_eigen_B_up;
    std::unordered_map<std::string, std::vector<std::vector<float> > > m_perjet_weight_bTagSF_eigen_B_down;
    std::unordered_map<std::string, std::vector<std::vector<float> > > m_perjet_weight_trackjet_bTagSF_eigen_B_up;
    std::unordered_map<std::string, std::vector<std::vector<float> > > m_perjet_weight_trackjet_bTagSF_eigen_B_down;

    // eigen variations affecting c-jets [WP]
    std::unordered_map<std::string, std::vector<float> > m_weight_bTagSF_eigen_C_up;
    std::unordered_map<std::string, std::vector<float> > m_weight_bTagSF_eigen_C_down;
    std::unordered_map<std::string, std::vector<float> > m_weight_trackjet_bTagSF_eigen_C_up;
    std::unordered_map<std::string, std::vector<float> > m_weight_trackjet_bTagSF_eigen_C_down;

    std::unordered_map<std::string, std::vector<std::vector<float> > > m_perjet_weight_bTagSF_eigen_C_up;
    std::unordered_map<std::string, std::vector<std::vector<float> > > m_perjet_weight_bTagSF_eigen_C_down;
    std::unordered_map<std::string, std::vector<std::vector<float> > > m_perjet_weight_trackjet_bTagSF_eigen_C_up;
    std::unordered_map<std::string, std::vector<std::vector<float> > > m_perjet_weight_trackjet_bTagSF_eigen_C_down;

    // eigen variations affecting light jets [WP]
    std::unordered_map<std::string, std::vector<float> > m_weight_bTagSF_eigen_Light_up;
    std::unordered_map<std::string, std::vector<float> > m_weight_bTagSF_eigen_Light_down;
    std::unordered_map<std::string, std::vector<float> > m_weight_trackjet_bTagSF_eigen_Light_up;
    std::unordered_map<std::string, std::vector<float> > m_weight_trackjet_bTagSF_eigen_Light_down;

    std::unordered_map<std::string, std::vector<std::vector<float> > > m_perjet_weight_bTagSF_eigen_Light_up;
    std::unordered_map<std::string, std::vector<std::vector<float> > > m_perjet_weight_bTagSF_eigen_Light_down;
    std::unordered_map<std::string, std::vector<std::vector<float> > > m_perjet_weight_trackjet_bTagSF_eigen_Light_up;
    std::unordered_map<std::string, std::vector<std::vector<float> > > m_perjet_weight_trackjet_bTagSF_eigen_Light_down;

    // named systematics [WP][name]
    std::unordered_map<std::string, std::unordered_map<std::string, float> > m_weight_bTagSF_named_up;
    std::unordered_map<std::string, std::unordered_map<std::string, float> > m_weight_bTagSF_named_down;
    std::unordered_map<std::string, std::unordered_map<std::string, float> > m_weight_trackjet_bTagSF_named_up;
    std::unordered_map<std::string, std::unordered_map<std::string, float> > m_weight_trackjet_bTagSF_named_down;

    std::unordered_map<std::string,
                       std::unordered_map<std::string, std::vector<float> > > m_perjet_weight_bTagSF_named_up;
    std::unordered_map<std::string,
                       std::unordered_map<std::string, std::vector<float> > > m_perjet_weight_bTagSF_named_down;
    std::unordered_map<std::string,
                       std::unordered_map<std::string, std::vector<float> > > m_perjet_weight_trackjet_bTagSF_named_up;
    std::unordered_map<std::string,
                       std::unordered_map<std::string,
                                          std::vector<float> > > m_perjet_weight_trackjet_bTagSF_named_down;

    ///-- weights for matrix-method fakes estimate, for each selection and configuration --///
    std::string m_ASMdecorName;
    std::vector<float> m_ASMweights;

    /// Weights for bootstrapping
    std::vector<int> m_weight_poisson;

    //event info
    unsigned long long m_eventNumber;
    unsigned int m_runNumber;
    unsigned int m_randomRunNumber;
    unsigned int m_mcChannelNumber;
    float m_mu_original;
    float m_mu;
    float m_mu_actual_original;
    float m_mu_actual;
    // non-collision background flag - usage:
    // https://twiki.cern.ch/twiki/bin/view/Atlas/NonCollisionBackgroundsRunTwo#Recommend_cuts_tools_and_procedu
    unsigned int m_backgroundFlags;
    // hasBadMuon flag - see:
    // https://twiki.cern.ch/twiki/bin/viewauth/Atlas/MuonSelectionTool#is_BadMuon_Flag_Event_Veto
    unsigned int m_hasBadMuon;

    //electrons
    std::vector<float> m_el_pt;
    std::vector<float> m_el_eta;
    std::vector<float> m_el_cl_eta;
    std::vector<float> m_el_phi;
    std::vector<float> m_el_e;
    std::vector<float> m_el_charge;
    std::vector<float> m_el_faketype;
    std::vector<float> m_el_topoetcone20;
    std::vector<float> m_el_ptvarcone20;
    std::vector<float> m_el_etcone20;
    std::vector<float> m_el_ptcone30;
    std::vector<char>  m_el_isTight;
    std::vector<char>  m_el_CF; // pass charge ID selector (has no charge flip)
    std::unordered_map<std::string, std::vector<char> > m_el_trigMatched;
    std::vector<float> m_el_d0sig;
    std::vector<float> m_el_delta_z0_sintheta;
    std::vector<int>   m_el_true_type;
    std::vector<int>   m_el_true_origin;
    std::vector<int>   m_el_true_firstEgMotherTruthType;
    std::vector<int>   m_el_true_firstEgMotherTruthOrigin;
    std::vector<int>   m_el_true_firstEgMotherPdgId;
    std::vector<char>  m_el_true_isPrompt;
    std::vector<char>  m_el_true_isChargeFl;

    //forward electrons
    std::vector<float> m_fwdel_pt;
    std::vector<float> m_fwdel_eta;
    std::vector<float> m_fwdel_phi;
    std::vector<float> m_fwdel_e;
    std::vector<float> m_fwdel_etcone20;
    std::vector<float> m_fwdel_etcone30;
    std::vector<float> m_fwdel_etcone40;
    std::vector<char>  m_fwdel_isTight;

    //muons
    std::vector<float> m_mu_pt;
    std::vector<float> m_mu_eta;
    std::vector<float> m_mu_phi;
    std::vector<float> m_mu_e;
    std::vector<float> m_mu_charge;
    std::vector<float> m_mu_topoetcone20;
    std::vector<float> m_mu_ptvarcone30;
    std::vector<float> m_mu_etcone20;
    std::vector<float> m_mu_ptcone30;
    std::vector<char>  m_mu_isTight;
    std::unordered_map<std::string, std::vector<char> > m_mu_trigMatched;
    std::vector<float> m_mu_d0sig;
    std::vector<float> m_mu_delta_z0_sintheta;
    std::vector<int>   m_mu_true_type;
    std::vector<int>   m_mu_true_origin;
    std::vector<char>  m_mu_true_isPrompt;
    std::vector<float>  m_mu_prodVtx_z;
    std::vector<float>  m_mu_prodVtx_perp;
    std::vector<float>  m_mu_prodVtx_phi;

    //soft muons
    std::vector<float> m_softmu_pt;
    std::vector<float> m_softmu_eta;
    std::vector<float> m_softmu_phi;
    std::vector<float> m_softmu_e;
    std::vector<float> m_softmu_charge;
    std::vector<float> m_softmu_d0;
    std::vector<float> m_softmu_d0sig;
    std::vector<float> m_softmu_delta_z0_sintheta;
    std::vector<int> m_softmu_true_type;
    std::vector<int> m_softmu_true_origin;
    std::vector<int> m_softmu_true_isPrompt;
    std::vector<float> m_softmu_SF_ID;
    std::vector<float> m_softmu_SF_ID_STAT_UP;
    std::vector<float> m_softmu_SF_ID_STAT_DOWN;
    std::vector<float> m_softmu_SF_ID_SYST_UP;
    std::vector<float> m_softmu_SF_ID_SYST_DOWN;
    std::vector<float> m_softmu_SF_ID_STAT_LOWPT_UP;
    std::vector<float> m_softmu_SF_ID_STAT_LOWPT_DOWN;
    std::vector<float> m_softmu_SF_ID_SYST_LOWPT_UP;
    std::vector<float> m_softmu_SF_ID_SYST_LOWPT_DOWN;
    std::vector<int> m_softmu_parton_origin_flag;
    std::vector<int> m_softmu_particle_origin_flag;
    std::vector<int> m_softmu_parent_pdgid;
    std::vector<int> m_softmu_b_hadron_parent_pdgid;
    std::vector<int> m_softmu_c_hadron_parent_pdgid;

    //temporary: PLIV for electrons+muons
    std::vector<float> m_PLIV_el_PromptLeptonRNN_conversion;
    std::vector<float> m_PLIV_el_PromptLeptonRNN_non_prompt_b;
    std::vector<float> m_PLIV_el_PromptLeptonRNN_non_prompt_c;
    std::vector<float> m_PLIV_el_PromptLeptonRNN_prompt;
    std::vector<short> m_PLIV_el_PromptLeptonImprovedInput_MVAXBin;
    std::vector<float> m_PLIV_el_PromptLeptonImprovedInput_TrackJetNTrack;
    std::vector<float> m_PLIV_el_PromptLeptonImprovedInput_topoetcone30rel;
    std::vector<float> m_PLIV_el_PromptLeptonImprovedInput_ptvarcone30rel;
    std::vector<float> m_PLIV_el_PromptLeptonImprovedInput_PtFrac;
    std::vector<float> m_PLIV_el_PromptLeptonImprovedInput_DRlj;
    std::vector<float> m_PLIV_el_PromptLeptonImprovedInput_CaloClusterSumEtRel;
    std::vector<float> m_PLIV_el_PromptLeptonImprovedInput_PtRel;
    std::vector<float> m_PLIV_el_PromptLeptonImprovedInput_CandVertex_normDistToPriVtxLongitudinalBest_ThetaCutVtx;
    std::vector<float> m_PLIV_el_PromptLeptonImprovedVetoBARR;
    std::vector<float> m_PLIV_el_PromptLeptonImprovedVetoECAP;
    std::vector<float> m_PLIV_mu_PromptLeptonRNN_non_prompt_b;
    std::vector<float> m_PLIV_mu_PromptLeptonRNN_non_prompt_c;
    std::vector<float> m_PLIV_mu_PromptLeptonRNN_prompt;
    std::vector<short> m_PLIV_mu_PromptLeptonImprovedInput_MVAXBin;
    std::vector<float> m_PLIV_mu_PromptLeptonImprovedInput_topoetcone30rel;
    std::vector<float> m_PLIV_mu_PromptLeptonImprovedInput_PtFrac;
    std::vector<float> m_PLIV_mu_PromptLeptonImprovedInput_DRlj;
    std::vector<float> m_PLIV_mu_PromptLeptonImprovedInput_ptvarcone30_Nonprompt_All_MaxWeightTTVA_pt500rel;
    std::vector<float> m_PLIV_mu_PromptLeptonImprovedInput_CaloClusterERel;
    std::vector<float> m_PLIV_mu_PromptLeptonImprovedInput_CandVertex_normDistToPriVtxLongitudinalBest;
    std::vector<float> m_PLIV_mu_PromptLeptonImprovedVeto;

    //photons
    std::vector<float> m_ph_pt;
    std::vector<float> m_ph_eta;
    std::vector<float> m_ph_phi;
    std::vector<float> m_ph_e;
    std::vector<float> m_ph_true_type;
    std::vector<float> m_ph_true_origin;
    std::vector<float> m_ph_faketype;
    std::vector<float> m_ph_iso;
    std::unordered_map<std::string, std::vector<char> > m_ph_trigMatched;

    //taus
    std::vector<float> m_tau_pt;
    std::vector<float> m_tau_eta;
    std::vector<float> m_tau_phi;
    std::vector<float> m_tau_e;
    std::vector<float> m_tau_charge;
    std::vector<char> m_tau_isHadronic;

    //jets
    std::vector<float> m_jet_pt;
    std::vector<float> m_jet_eta;
    std::vector<float> m_jet_phi;
    std::vector<float> m_jet_e;
    std::vector<float> m_jet_jvt;
    std::vector<float> m_jet_fjvt;
    std::vector<char> m_jet_passfjvt; //Could be useful to check pass/fail when fJVT only used in MET
    std::vector<float> m_jet_ip3dsv1;
    std::vector<int>   m_jet_truthflav;
    std::vector<int>   m_jet_truthPartonLabel;
    std::vector<char>  m_jet_isTrueHS;
    std::vector<int>   m_jet_HadronConeExclExtendedTruthLabelID; // Newer jet truth flavour label
    std::unordered_map<std::string, std::vector<char> >  m_jet_isbtagged;//one vector per jet per WP
    std::unordered_map<std::string, std::vector<int> >   m_jet_tagWeightBin;// one vector per jet per tag-weight bin in
                                                                            // case Continuous WP is used

    // ghost tracks
    std::vector<std::vector<float> > m_jet_ghostTrack_pt;
    std::vector<std::vector<float> > m_jet_ghostTrack_eta;
    std::vector<std::vector<float> > m_jet_ghostTrack_phi;
    std::vector<std::vector<float> > m_jet_ghostTrack_e;
    std::vector<std::vector<float> > m_jet_ghostTrack_d0;
    std::vector<std::vector<float> > m_jet_ghostTrack_z0;
    std::vector<std::vector<float> > m_jet_ghostTrack_qOverP;

    // tracks
    std::vector<float> m_track_pt;
    std::vector<float> m_track_eta;
    std::vector<float> m_track_phi;
    std::vector<float> m_track_e;
    std::vector<float> m_track_d0;
    std::vector<float> m_track_z0;
    std::vector<float> m_track_qOverP;
    std::vector<float> m_track_charge;
    std::vector<float> m_track_d0_significance;
    std::vector<float> m_track_z0_significance;
    std::vector<float> m_track_phi0;
    std::vector<float> m_track_theta;
    std::vector<float> m_track_chiSquared;
    std::vector<uint8_t> m_track_numberDoF;

    // R21 b-tagging
    std::unordered_map<std::string, std::vector<float>> m_jet_DLx;
    std::unordered_map<std::string, std::vector<float>> m_jet_DLx_pb;
    std::unordered_map<std::string, std::vector<float>> m_jet_DLx_pc;
    std::unordered_map<std::string, std::vector<float>> m_jet_DLx_pu;

    // fail-JVT jets
    std::vector<float> m_failJvt_jet_pt;
    std::vector<float> m_failJvt_jet_eta;
    std::vector<float> m_failJvt_jet_phi;
    std::vector<float> m_failJvt_jet_e;
    std::vector<float> m_failJvt_jet_jvt;
    std::vector<float> m_failJvt_jet_fjvt;
    std::vector<char> m_failJvt_jet_passfjvt;
    std::vector<int>   m_failJvt_jet_truthflav;
    std::vector<int>   m_failJvt_jet_truthPartonLabel;
    std::vector<char>  m_failJvt_jet_isTrueHS;
    std::vector<int>   m_failJvt_jet_HadronConeExclExtendedTruthLabelID; // Newer jet truth flavour label
    std::vector<std::vector<float> > m_failJvt_jet_ghostTrack_pt;
    std::vector<std::vector<float> > m_failJvt_jet_ghostTrack_eta;
    std::vector<std::vector<float> > m_failJvt_jet_ghostTrack_phi;
    std::vector<std::vector<float> > m_failJvt_jet_ghostTrack_e;
    std::vector<std::vector<float> > m_failJvt_jet_ghostTrack_d0;
    std::vector<std::vector<float> > m_failJvt_jet_ghostTrack_z0;
    std::vector<std::vector<float> > m_failJvt_jet_ghostTrack_qOverP;

    // fail-FJVT jets
    std::vector<float> m_failFJvt_jet_pt;
    std::vector<float> m_failFJvt_jet_eta;
    std::vector<float> m_failFJvt_jet_phi;
    std::vector<float> m_failFJvt_jet_e;
    std::vector<float> m_failFJvt_jet_jvt;
    std::vector<float> m_failFJvt_jet_fjvt;
    std::vector<char> m_failFJvt_jet_passjvt;
    std::vector<int>   m_failFJvt_jet_truthflav;
    std::vector<int>   m_failFJvt_jet_truthPartonLabel;
    std::vector<char>  m_failFJvt_jet_isTrueHS;
    std::vector<int>   m_failFJvt_jet_HadronConeExclExtendedTruthLabelID; // Newer jet truth flavour label
    std::vector<std::vector<float> > m_failFJvt_jet_ghostTrack_pt;
    std::vector<std::vector<float> > m_failFJvt_jet_ghostTrack_eta;
    std::vector<std::vector<float> > m_failFJvt_jet_ghostTrack_phi;
    std::vector<std::vector<float> > m_failFJvt_jet_ghostTrack_e;
    std::vector<std::vector<float> > m_failFJvt_jet_ghostTrack_d0;
    std::vector<std::vector<float> > m_failFJvt_jet_ghostTrack_z0;
    std::vector<std::vector<float> > m_failFJvt_jet_ghostTrack_qOverP;

    //large-R jets
    std::vector<float> m_ljet_pt;
    std::vector<float> m_ljet_eta;
    std::vector<float> m_ljet_phi;
    std::vector<float> m_ljet_e;
    std::vector<float> m_ljet_m;
    std::vector<int> m_ljet_truthLabel;

    std::unordered_map<std::string, std::vector<float> > m_ljet_substructure;
    std::unordered_map<std::string, std::vector<char> > m_ljet_isTagged;
    std::unordered_map<std::string, std::vector<char> > m_ljet_taggingPassedRangeCheck;
    std::unordered_map<std::string, std::vector<float> > m_ljet_tagSF;
    std::unordered_map<std::string, std::vector<std::vector<float>> > m_ljet_tagSFSysVars;

    //track jets
    std::vector<float> m_tjet_pt;
    std::vector<float> m_tjet_eta;
    std::vector<float> m_tjet_phi;
    std::vector<float> m_tjet_e;
    std::unordered_map<std::string, SG::AuxElement::ConstAccessor<float>> DLx;
    std::unordered_map<std::string, std::vector<float>> m_tjet_DLx;
    std::unordered_map<std::string, std::vector<float>> m_tjet_DLx_pb;
    std::unordered_map<std::string, std::vector<float>> m_tjet_DLx_pc;
    std::unordered_map<std::string, std::vector<float>> m_tjet_DLx_pu;
    std::unordered_map<std::string, std::vector<char> >  m_tjet_isbtagged;//one vector per jet per WP
    std::unordered_map<std::string, std::vector<int> >   m_tjet_tagWeightBin;//one vector per jet tag-weight bin in case
                                                                             // Continuous WP is used

    //re-clustered jets
    //  -> need unordered map for systematics
    bool m_makeRCJets; // making re-clustered jets
    bool m_makeVarRCJets; // making VarRC jets
    bool m_useRCJSS; // write RCJSS variables
    bool m_useRCAdditionalJSS; // write RCJSS additional variables
    bool m_useVarRCJSS; // write Variable-R RCJSS variables
    bool m_useVarRCAdditionalJSS; // write Variable-R RCJSS additional variables
    std::string m_RCJetContainer;       // name for RC jets container in TStore
    std::vector<std::string> m_VarRCJetRho;
    std::vector<std::string> m_VarRCJetMassScale;
    std::string m_egamma;      // egamma systematic naming scheme
    std::string m_muonsyst;    // muon systematic naming scheme
    std::string m_softmuonsyst;    // soft muon systematic naming scheme
    std::string m_jetsyst;     // jet systematic naming scheme
    std::map<std::string, std::vector<float> > m_VarRCjetBranches;
    std::map<std::string, std::vector<std::vector<float> > > m_VarRCjetsubBranches;
    std::map<std::string, std::vector<float> > m_VarRCjetBranchesParticle;
    std::map<std::string, std::vector<std::vector<float> > > m_VarRCjetsubBranchesParticle;
    std::vector<int> m_rcjet_nsub;
    std::vector<float> m_rcjet_pt;
    std::vector<float> m_rcjet_eta;
    std::vector<float> m_rcjet_phi;
    std::vector<float> m_rcjet_e;
    std::vector<float> m_rcjet_d12;
    std::vector<float> m_rcjet_d23;
    std::vector<std::vector<float> > m_rcjetsub_pt;
    std::vector<std::vector<float> > m_rcjetsub_eta;
    std::vector<std::vector<float> > m_rcjetsub_phi;
    std::vector<std::vector<float> > m_rcjetsub_e;

    std::vector<float> m_rrcjet_pt;
    std::vector<float> m_rrcjet_eta;
    std::vector<float> m_rrcjet_phi;
    std::vector<float> m_rrcjet_e;

    std::vector<float> m_rcjet_tau32_clstr;
    std::vector<float> m_rcjet_tau21_clstr;
    std::vector<float> m_rcjet_tau3_clstr;
    std::vector<float> m_rcjet_tau2_clstr;
    std::vector<float> m_rcjet_tau1_clstr;

    std::vector<float> m_rcjet_D2_clstr;
    std::vector<float> m_rcjet_ECF1_clstr;
    std::vector<float> m_rcjet_ECF2_clstr;
    std::vector<float> m_rcjet_ECF3_clstr;

    std::vector<float> m_rcjet_d12_clstr;
    std::vector<float> m_rcjet_d23_clstr;
    std::vector<float> m_rcjet_Qw_clstr;
    std::vector<float> m_rcjet_nconstituent_clstr;

    std::vector<float> m_rcjet_gECF332_clstr;
    std::vector<float> m_rcjet_gECF461_clstr;
    std::vector<float> m_rcjet_gECF322_clstr;
    std::vector<float> m_rcjet_gECF331_clstr;
    std::vector<float> m_rcjet_gECF422_clstr;
    std::vector<float> m_rcjet_gECF441_clstr;
    std::vector<float> m_rcjet_gECF212_clstr;
    std::vector<float> m_rcjet_gECF321_clstr;
    std::vector<float> m_rcjet_gECF311_clstr;
    std::vector<float> m_rcjet_L1_clstr;
    std::vector<float> m_rcjet_L2_clstr;
    std::vector<float> m_rcjet_L3_clstr;
    std::vector<float> m_rcjet_L4_clstr;
    std::vector<float> m_rcjet_L5_clstr;


    //met
    float m_met_met;
    float m_met_sumet;
    float m_met_phi;
    float m_met_sig;
    float m_met_sigHT;
    float m_met_sigET;
    float m_met_sigRho;
    float m_met_sigVarL;
    float m_met_sigVarT;
    //these are for specific studies on the met, turned off by default, and turned on with the WriteMETBuiltWithLooseObjects option
    float m_met_met_withLooseObjects;
    float m_met_phi_withLooseObjects;

    //KLFitter
    short m_klfitter_selected;
    /// Selection
    std::vector<std::string> m_klfitter_selection;
    /// Error flags
    std::vector<short> m_klfitter_minuitDidNotConverge;
    std::vector<short> m_klfitter_fitAbortedDueToNaN;
    std::vector<short> m_klfitter_atLeastOneFitParameterAtItsLimit;
    std::vector<short> m_klfitter_invalidTransferFunctionAtConvergence;

    /// Global result
    std::vector<unsigned int> m_klfitter_bestPermutation;
    std::vector<float> m_klfitter_logLikelihood;
    std::vector<float> m_klfitter_eventProbability;
    std::vector<unsigned int> m_klfitter_parameters_size;
    std::vector<std::vector<double> > m_klfitter_parameters;
    std::vector<std::vector<double> > m_klfitter_parameterErrors;

    /// Model
    std::vector<float> m_klfitter_model_bhad_pt;
    std::vector<float> m_klfitter_model_bhad_eta;
    std::vector<float> m_klfitter_model_bhad_phi;
    std::vector<float> m_klfitter_model_bhad_E;
    std::vector<unsigned int> m_klfitter_model_bhad_jetIndex;

    std::vector<float> m_klfitter_model_blep_pt;
    std::vector<float> m_klfitter_model_blep_eta;
    std::vector<float> m_klfitter_model_blep_phi;
    std::vector<float> m_klfitter_model_blep_E;
    std::vector<unsigned int> m_klfitter_model_blep_jetIndex;

    std::vector<float> m_klfitter_model_lq1_pt;
    std::vector<float> m_klfitter_model_lq1_eta;
    std::vector<float> m_klfitter_model_lq1_phi;
    std::vector<float> m_klfitter_model_lq1_E;
    std::vector<unsigned int> m_klfitter_model_lq1_jetIndex;

    std::vector<float> m_klfitter_model_lq2_pt;
    std::vector<float> m_klfitter_model_lq2_eta;
    std::vector<float> m_klfitter_model_lq2_phi;
    std::vector<float> m_klfitter_model_lq2_E;
    std::vector<unsigned int> m_klfitter_model_lq2_jetIndex;

    std::vector<float> m_klfitter_model_Higgs_b1_pt;
    std::vector<float> m_klfitter_model_Higgs_b1_eta;
    std::vector<float> m_klfitter_model_Higgs_b1_phi;
    std::vector<float> m_klfitter_model_Higgs_b1_E;
    std::vector<unsigned int> m_klfitter_model_Higgs_b1_jetIndex;

    std::vector<float> m_klfitter_model_Higgs_b2_pt;
    std::vector<float> m_klfitter_model_Higgs_b2_eta;
    std::vector<float> m_klfitter_model_Higgs_b2_phi;
    std::vector<float> m_klfitter_model_Higgs_b2_E;
    std::vector<unsigned int> m_klfitter_model_Higgs_b2_jetIndex;

    std::vector<float> m_klfitter_model_lep_pt;
    std::vector<float> m_klfitter_model_lep_eta;
    std::vector<float> m_klfitter_model_lep_phi;
    std::vector<float> m_klfitter_model_lep_E;
    std::vector<unsigned int> m_klfitter_model_lep_index;

    std::vector<float> m_klfitter_model_lepZ1_pt;
    std::vector<float> m_klfitter_model_lepZ1_eta;
    std::vector<float> m_klfitter_model_lepZ1_phi;
    std::vector<float> m_klfitter_model_lepZ1_E;
    std::vector<unsigned int> m_klfitter_model_lepZ1_index;

    std::vector<float> m_klfitter_model_lepZ2_pt;
    std::vector<float> m_klfitter_model_lepZ2_eta;
    std::vector<float> m_klfitter_model_lepZ2_phi;
    std::vector<float> m_klfitter_model_lepZ2_E;
    std::vector<unsigned int> m_klfitter_model_lepZ2_index;

    std::vector<float> m_klfitter_model_nu_pt;
    std::vector<float> m_klfitter_model_nu_eta;
    std::vector<float> m_klfitter_model_nu_phi;
    std::vector<float> m_klfitter_model_nu_E;

    std::vector<float> m_klfitter_model_b_from_top1_pt;
    std::vector<float> m_klfitter_model_b_from_top1_eta;
    std::vector<float> m_klfitter_model_b_from_top1_phi;
    std::vector<float> m_klfitter_model_b_from_top1_E;
    std::vector<unsigned int> m_klfitter_model_b_from_top1_jetIndex;

    std::vector<float> m_klfitter_model_b_from_top2_pt;
    std::vector<float> m_klfitter_model_b_from_top2_eta;
    std::vector<float> m_klfitter_model_b_from_top2_phi;
    std::vector<float> m_klfitter_model_b_from_top2_E;
    std::vector<unsigned int> m_klfitter_model_b_from_top2_jetIndex;

    std::vector<float> m_klfitter_model_lj1_from_top1_pt;
    std::vector<float> m_klfitter_model_lj1_from_top1_eta;
    std::vector<float> m_klfitter_model_lj1_from_top1_phi;
    std::vector<float> m_klfitter_model_lj1_from_top1_E;
    std::vector<unsigned int> m_klfitter_model_lj1_from_top1_jetIndex;

    std::vector<float> m_klfitter_model_lj2_from_top1_pt;
    std::vector<float> m_klfitter_model_lj2_from_top1_eta;
    std::vector<float> m_klfitter_model_lj2_from_top1_phi;
    std::vector<float> m_klfitter_model_lj2_from_top1_E;
    std::vector<unsigned int> m_klfitter_model_lj2_from_top1_jetIndex;

    std::vector<float> m_klfitter_model_lj1_from_top2_pt;
    std::vector<float> m_klfitter_model_lj1_from_top2_eta;
    std::vector<float> m_klfitter_model_lj1_from_top2_phi;
    std::vector<float> m_klfitter_model_lj1_from_top2_E;
    std::vector<unsigned int> m_klfitter_model_lj1_from_top2_jetIndex;

    std::vector<float> m_klfitter_model_lj2_from_top2_pt;
    std::vector<float> m_klfitter_model_lj2_from_top2_eta;
    std::vector<float> m_klfitter_model_lj2_from_top2_phi;
    std::vector<float> m_klfitter_model_lj2_from_top2_E;
    std::vector<unsigned int> m_klfitter_model_lj2_from_top2_jetIndex;

    // calculated KLFitter variables for best perm
    float m_klfitter_bestPerm_topLep_pt;
    float m_klfitter_bestPerm_topLep_eta;
    float m_klfitter_bestPerm_topLep_phi;
    float m_klfitter_bestPerm_topLep_E;
    float m_klfitter_bestPerm_topLep_m;

    float m_klfitter_bestPerm_topHad_pt;
    float m_klfitter_bestPerm_topHad_eta;
    float m_klfitter_bestPerm_topHad_phi;
    float m_klfitter_bestPerm_topHad_E;
    float m_klfitter_bestPerm_topHad_m;

    float m_klfitter_bestPerm_ttbar_pt;
    float m_klfitter_bestPerm_ttbar_eta;
    float m_klfitter_bestPerm_ttbar_phi;
    float m_klfitter_bestPerm_ttbar_E;
    float m_klfitter_bestPerm_ttbar_m;

    // PseudoTop variables

    float m_PseudoTop_Reco_ttbar_pt;
    float m_PseudoTop_Reco_ttbar_eta;
    float m_PseudoTop_Reco_ttbar_phi;
    float m_PseudoTop_Reco_ttbar_m;
    float m_PseudoTop_Reco_top_had_pt;
    float m_PseudoTop_Reco_top_had_eta;
    float m_PseudoTop_Reco_top_had_phi;
    float m_PseudoTop_Reco_top_had_m;
    float m_PseudoTop_Reco_top_lep_pt;
    float m_PseudoTop_Reco_top_lep_eta;
    float m_PseudoTop_Reco_top_lep_phi;
    float m_PseudoTop_Reco_top_lep_m;
    float m_PseudoTop_Particle_ttbar_pt;
    float m_PseudoTop_Particle_ttbar_eta;
    float m_PseudoTop_Particle_ttbar_phi;
    float m_PseudoTop_Particle_ttbar_m;
    float m_PseudoTop_Particle_top_had_pt;
    float m_PseudoTop_Particle_top_had_eta;
    float m_PseudoTop_Particle_top_had_phi;
    float m_PseudoTop_Particle_top_had_m;
    float m_PseudoTop_Particle_top_lep_pt;
    float m_PseudoTop_Particle_top_lep_eta;
    float m_PseudoTop_Particle_top_lep_phi;
    float m_PseudoTop_Particle_top_lep_m;


    //MC
    std::vector<float> m_mc_pt;
    std::vector<float> m_mc_eta;
    std::vector<float> m_mc_phi;
    std::vector<float> m_mc_e;
    std::vector<double> m_mc_charge;
    std::vector<int> m_mc_pdgId;
    std::vector<int> m_mc_status;
    std::vector<int> m_mc_barcode;

    //PDFInfo
    std::vector<float> m_PDFinfo_X1;
    std::vector<float> m_PDFinfo_X2;
    std::vector<int> m_PDFinfo_PDGID1;
    std::vector<int> m_PDFinfo_PDGID2;
    std::vector<float> m_PDFinfo_Q;
    std::vector<float> m_PDFinfo_XF1;
    std::vector<float> m_PDFinfo_XF2;

    //the on-the-fly computed generator weights stored in EventInfo
    std::vector<float> m_mc_generator_weights;

    //Extra variables for Particle Level (bare lepton kinematics and b-Hadron
    //tagging information).
    std::vector<float> m_el_pt_bare;
    std::vector<float> m_el_eta_bare;
    std::vector<float> m_el_phi_bare;
    std::vector<float> m_el_e_bare;

    std::vector<float> m_mu_pt_bare;
    std::vector<float> m_mu_eta_bare;
    std::vector<float> m_mu_phi_bare;
    std::vector<float> m_mu_e_bare;

    std::vector<int> m_jet_Ghosts_BHadron_Final_Count;
    std::vector<int> m_jet_Ghosts_CHadron_Final_Count;
    std::vector<int> m_ljet_Ghosts_BHadron_Final_Count;
    std::vector<int> m_ljet_Ghosts_CHadron_Final_Count;
    std::vector<std::vector<int> > m_rcjetsub_Ghosts_BHadron_Final_Count;
    std::vector<std::vector<int> > m_rcjetsub_Ghosts_CHadron_Final_Count;

    // Truth tree inserted variables
    // This can be expanded as required
    // This is just a first pass at doing this sort of thing
    std::unordered_map<std::string, int*> m_extraTruthVars_int;
    std::unordered_map<std::string, float*> m_extraTruthVars_float;
  protected:
    /////////////////////////////////////
    /// const getters for the event variables
    /////////////////////////////////////

    ///Decisions on if the event passed / failed a particular selection.
    const std::vector<int>& selectionDecisions() const {return m_selectionDecisions;}

    ///Decisions on if the event passed / failed a particular trigger.
    const std::unordered_map<std::string, char>& triggerDecisions() const {return m_triggerDecisions;}

    ///Pre-scale of the trigger menu for each event.
    const std::unordered_map<std::string, float>& triggerPrescales() const {return m_triggerPrescales;}

    //Store output PDF weights from LHAPDF
    const std::unordered_map<std::string, std::vector<float> >& PDF_eventWeights() const {return m_PDF_eventWeights;}

    //some event weights
    const float& weight_mc() const {return m_weight_mc;}
    const float& weight_pileup() const {return m_weight_pileup;}

    ///-- Pileup SF systematics --///
    const float& weight_pileup_UP() const {return m_weight_pileup_UP;}
    const float& weight_pileup_DOWN() const {return m_weight_pileup_DOWN;}

    ///-- Lepton SF --///
    const float& weight_leptonSF() const {return m_weight_leptonSF;}

    ///-- Lepton SF - electron SF systematics --///
    const float& weight_leptonSF_EL_SF_Reco_UP() const {return m_weight_leptonSF_EL_SF_Reco_UP;}
    const float& weight_leptonSF_EL_SF_Reco_DOWN() const {return m_weight_leptonSF_EL_SF_Reco_DOWN;}
    const float& weight_leptonSF_EL_SF_ID_UP() const {return m_weight_leptonSF_EL_SF_ID_UP;}
    const float& weight_leptonSF_EL_SF_ID_DOWN() const {return m_weight_leptonSF_EL_SF_ID_DOWN;}
    const float& weight_leptonSF_EL_SF_Isol_UP() const {return m_weight_leptonSF_EL_SF_Isol_UP;}
    const float& weight_leptonSF_EL_SF_Isol_DOWN() const {return m_weight_leptonSF_EL_SF_Isol_DOWN;}

    ///-- Lepton SF - muon SF systematics --///
    // Muon ID SF systematics (regular)
    const float& weight_leptonSF_MU_SF_ID_STAT_UP() const {return m_weight_leptonSF_MU_SF_ID_STAT_UP;}
    const float& weight_leptonSF_MU_SF_ID_STAT_DOWN() const {return m_weight_leptonSF_MU_SF_ID_STAT_DOWN;}
    const float& weight_leptonSF_MU_SF_ID_SYST_UP() const {return m_weight_leptonSF_MU_SF_ID_SYST_UP;}
    const float& weight_leptonSF_MU_SF_ID_SYST_DOWN() const {return m_weight_leptonSF_MU_SF_ID_SYST_DOWN;}
    // Muon ID SF systematics (low pT)
    const float& weight_leptonSF_MU_SF_ID_STAT_LOWPT_UP() const {return m_weight_leptonSF_MU_SF_ID_STAT_LOWPT_UP;}
    const float& weight_leptonSF_MU_SF_ID_STAT_LOWPT_DOWN() const {return m_weight_leptonSF_MU_SF_ID_STAT_LOWPT_DOWN;}
    const float& weight_leptonSF_MU_SF_ID_SYST_LOWPT_UP() const {return m_weight_leptonSF_MU_SF_ID_SYST_LOWPT_UP;}
    const float& weight_leptonSF_MU_SF_ID_SYST_LOWPT_DOWN() const {return m_weight_leptonSF_MU_SF_ID_SYST_LOWPT_DOWN;}
    // Muon isolation SF systematics
    const float& weight_leptonSF_MU_SF_Isol_STAT_UP() const {return m_weight_leptonSF_MU_SF_Isol_STAT_UP;}
    const float& weight_leptonSF_MU_SF_Isol_STAT_DOWN() const {return m_weight_leptonSF_MU_SF_Isol_STAT_DOWN;}
    const float& weight_leptonSF_MU_SF_Isol_SYST_UP() const {return m_weight_leptonSF_MU_SF_Isol_SYST_UP;}
    const float& weight_leptonSF_MU_SF_Isol_SYST_DOWN() const {return m_weight_leptonSF_MU_SF_Isol_SYST_DOWN;}
    const float& weight_leptonSF_MU_SF_TTVA_STAT_UP() const {return m_weight_leptonSF_MU_SF_TTVA_STAT_UP;}
    const float& weight_leptonSF_MU_SF_TTVA_STAT_DOWN() const {return m_weight_leptonSF_MU_SF_TTVA_STAT_DOWN;}
    const float& weight_leptonSF_MU_SF_TTVA_SYST_UP() const {return m_weight_leptonSF_MU_SF_TTVA_SYST_UP;}
    const float& weight_leptonSF_MU_SF_TTVA_SYST_DOWN() const {return m_weight_leptonSF_MU_SF_TTVA_SYST_DOWN;}

    // Photons
    const float& weight_photonSF() const {return m_weight_photonSF;}
    const float& weight_photonSF_ID_UP() const {return m_weight_photonSF_ID_UP;}
    const float& weight_photonSF_ID_DOWN() const {return m_weight_photonSF_ID_DOWN;}
    const float& weight_photonSF_effIso_UP() const {return m_weight_photonSF_effIso_UP;}
    const float& weight_photonSF_effIso_DOWN() const {return m_weight_photonSF_effIso_DOWN;}

    // nominal b-tagging SF [WP]
    const std::unordered_map<std::string, float>& weight_bTagSF() const {return m_weight_bTagSF;}
    const std::unordered_map<std::string, std::vector<float> >& perjet_weight_bTagSF() const {return m_perjet_weight_bTagSF;}
    const std::unordered_map<std::string, float>& weight_trackjet_bTagSF() const {return m_weight_trackjet_bTagSF;}
    const std::unordered_map<std::string, std::vector<float> >& perjet_weight_trackjet_bTagSF() const {return m_perjet_weight_trackjet_bTagSF;}

    // JVT (c++11 initialization for fun)
    const float& weight_jvt() const {return m_weight_jvt;}
    const float& weight_jvt_up() const {return m_weight_jvt_up;}
    const float& weight_jvt_down() const {return m_weight_jvt_down;}

    // FJVT
    const float& weight_forwardjvt() const {return m_weight_forwardjvt;}
    const float& weight_forwardjvt_up() const {return m_weight_forwardjvt_up;}
    const float& weight_forwardjvt_down() const {return m_weight_forwardjvt_down;}

    // Sherpa 2.2 weight
    const float& weight_sherpa_22_vjets() const {return m_weight_sherpa_22_vjets;}

    // eigen variations affecting b-jets [WP]
    const std::unordered_map<std::string, std::vector<float> >& weight_bTagSF_eigen_B_up() const {return m_weight_bTagSF_eigen_B_up;}
    const std::unordered_map<std::string, std::vector<float> >& weight_bTagSF_eigen_B_down() const {return m_weight_bTagSF_eigen_B_down;}
    const std::unordered_map<std::string, std::vector<float> >& weight_trackjet_bTagSF_eigen_B_up() const {return m_weight_trackjet_bTagSF_eigen_B_up;}
    const std::unordered_map<std::string, std::vector<float> >& weight_trackjet_bTagSF_eigen_B_down() const {return m_weight_trackjet_bTagSF_eigen_B_down;}
    const std::unordered_map<std::string, std::vector<std::vector<float> > >& perjet_weight_bTagSF_eigen_B_up() const {return m_perjet_weight_bTagSF_eigen_B_up;}
    const std::unordered_map<std::string, std::vector<std::vector<float> > >& perjet_weight_bTagSF_eigen_B_down() const {return m_perjet_weight_bTagSF_eigen_B_down;}
    const std::unordered_map<std::string, std::vector<std::vector<float> > >& perjet_weight_trackjet_bTagSF_eigen_B_up() const {return m_perjet_weight_trackjet_bTagSF_eigen_B_up;}
    const std::unordered_map<std::string, std::vector<std::vector<float> > >& perjet_weight_trackjet_bTagSF_eigen_B_down() const {return m_perjet_weight_trackjet_bTagSF_eigen_B_down;}
    // eigen variations affecting c-jets [WP]
    const std::unordered_map<std::string, std::vector<float> >& weight_bTagSF_eigen_C_up() const {return m_weight_bTagSF_eigen_C_up;}
    const std::unordered_map<std::string, std::vector<float> >& weight_bTagSF_eigen_C_down() const {return m_weight_bTagSF_eigen_C_down;}
    const std::unordered_map<std::string, std::vector<float> >& weight_trackjet_bTagSF_eigen_C_up() const {return m_weight_trackjet_bTagSF_eigen_C_up;}
    const std::unordered_map<std::string, std::vector<float> >& weight_trackjet_bTagSF_eigen_C_down() const {return m_weight_trackjet_bTagSF_eigen_C_down;}
    const std::unordered_map<std::string, std::vector<std::vector<float> > >& perjet_weight_bTagSF_eigen_C_up() const {return m_perjet_weight_bTagSF_eigen_C_up;}
    const std::unordered_map<std::string, std::vector<std::vector<float> > >& perjet_weight_bTagSF_eigen_C_down() const {return m_perjet_weight_bTagSF_eigen_C_down;}
    const std::unordered_map<std::string, std::vector<std::vector<float> > >& perjet_weight_trackjet_bTagSF_eigen_C_up() const {return m_perjet_weight_trackjet_bTagSF_eigen_C_up;}
    const std::unordered_map<std::string, std::vector<std::vector<float> > >& perjet_weight_trackjet_bTagSF_eigen_C_down() const {return m_perjet_weight_trackjet_bTagSF_eigen_C_down;}
    // eigen variations affecting light jets [WP]
    const std::unordered_map<std::string, std::vector<float> >& weight_bTagSF_eigen_Light_up() const {return m_weight_bTagSF_eigen_Light_up;}
    const std::unordered_map<std::string, std::vector<float> >& weight_bTagSF_eigen_Light_down() const {return m_weight_bTagSF_eigen_Light_down;}
    const std::unordered_map<std::string, std::vector<float> >& weight_trackjet_bTagSF_eigen_Light_up() const {return m_weight_trackjet_bTagSF_eigen_Light_up;}
    const std::unordered_map<std::string, std::vector<float> >& weight_trackjet_bTagSF_eigen_Light_down() const {return m_weight_trackjet_bTagSF_eigen_Light_down;}
    const std::unordered_map<std::string, std::vector<std::vector<float> > >& perjet_weight_bTagSF_eigen_Light_up() const {return m_perjet_weight_bTagSF_eigen_C_up;}
    const std::unordered_map<std::string, std::vector<std::vector<float> > >& perjet_weight_bTagSF_eigen_Light_down() const {return m_perjet_weight_bTagSF_eigen_C_down;}
    const std::unordered_map<std::string, std::vector<std::vector<float> > >& perjet_weight_trackjet_bTagSF_eigen_Light_up() const {return m_perjet_weight_trackjet_bTagSF_eigen_Light_up;}
    const std::unordered_map<std::string, std::vector<std::vector<float> > >& perjet_weight_trackjet_bTagSF_eigen_Light_down() const {return m_perjet_weight_trackjet_bTagSF_eigen_Light_down;}
    // named systematics [WP][name]
    const std::unordered_map<std::string, std::unordered_map<std::string, float> >& weight_bTagSF_named_up() const {return m_weight_bTagSF_named_up;}
    const std::unordered_map<std::string, std::unordered_map<std::string, float> >& weight_bTagSF_named_down() const {return m_weight_bTagSF_named_down;}
    const std::unordered_map<std::string, std::unordered_map<std::string, float> >& weight_trackjet_bTagSF_named_up() const {return m_weight_trackjet_bTagSF_named_up;}
    const std::unordered_map<std::string, std::unordered_map<std::string, float> >& weight_trackjet_bTagSF_named_down() const {return m_weight_trackjet_bTagSF_named_down;}
    const std::unordered_map<std::string, std::unordered_map<std::string, std::vector<float> > >& perjet_weight_bTagSF_named_up() const {return m_perjet_weight_bTagSF_named_up;}
    const std::unordered_map<std::string, std::unordered_map<std::string, std::vector<float> > >& perjet_weight_bTagSF_named_down() const {return m_perjet_weight_bTagSF_named_down;}
    const std::unordered_map<std::string, std::unordered_map<std::string, std::vector<float> > >& perjet_weight_trackjet_bTagSF_named_up() const {return m_perjet_weight_trackjet_bTagSF_named_up;}
    const std::unordered_map<std::string, std::unordered_map<std::string, std::vector<float> > >& perjet_weight_trackjet_bTagSF_named_down() const {return m_perjet_weight_trackjet_bTagSF_named_down;}

    /// Weights for bootstrapping
    const std::vector<int>& weight_poisson() const {return m_weight_poisson;}

    //event info
    const unsigned long long& eventNumber() const {return m_eventNumber;}
    const unsigned int& runNumber() const {return m_runNumber;}
    const unsigned int& randomRunNumber() const {return m_randomRunNumber;}
    const unsigned int& mcChannelNumber() const {return m_mcChannelNumber;}
    const float& mu_original() const {return m_mu_original;}
    const float& mu() const {return m_mu;}

    // non-collision background flag - usage:
    // https://twiki.cern.ch/twiki/bin/view/Atlas/NonCollisionBackgroundsRunTwo#Recommend_cuts_tools_and_procedu
    const unsigned int& backgroundFlags() const {return m_backgroundFlags;}

    // hasBadMuon flag - see:
    // https://twiki.cern.ch/twiki/bin/viewauth/Atlas/MuonSelectionTool#is_BadMuon_Flag_Event_Veto
    const unsigned int& hasBadMuon() const {return m_hasBadMuon;}

    //electrons
    const std::vector<float>& el_pt() const {return m_el_pt;}
    const std::vector<float>& el_eta() const {return m_el_eta;}
    const std::vector<float>& el_cl_eta() const {return m_el_cl_eta;}
    const std::vector<float>& el_phi() const {return m_el_phi;}
    const std::vector<float>& el_e() const {return m_el_e;}
    const std::vector<float>& el_faketype() const {return m_el_faketype;}
    const std::vector<float>& el_charge() const {return m_el_charge;}
    const std::vector<float>& el_topoetcone20() const {return m_el_topoetcone20;}
    const std::vector<float>& el_ptvarcone20() const {return m_el_ptvarcone20;}
    const std::vector<char>& el_isTight() const {return m_el_isTight;}
    const std::vector<char>& el_CF() const {return m_el_CF;} // pass charge ID selector (has no charge flip)
    const std::unordered_map<std::string, std::vector<char> >& el_trigMatched() const {return m_el_trigMatched;}
    const std::vector<float>& el_d0sig() const {return m_el_d0sig;}
    const std::vector<float>& el_delta_z0_sintheta() const {return m_el_delta_z0_sintheta;}
    const std::vector<int>& el_true_type() const {return m_el_true_type;}
    const std::vector<int>& el_true_origin() const {return m_el_true_origin;}
    const std::vector<int>& el_true_firstEgMotherTruthType() const {return m_el_true_firstEgMotherTruthType;}
    const std::vector<int>& el_true_firstEgMotherTruthOrigin() const {return m_el_true_firstEgMotherTruthOrigin;}
    const std::vector<int>& el_true_firstEgMotherPdgId() const {return m_el_true_firstEgMotherPdgId;}
    const std::vector<char>& el_true_isPrompt() const {return m_el_true_isPrompt;}
    const std::vector<char>& el_true_isChargeFl() const {return m_el_true_isChargeFl;}

    //forward electrons
    const std::vector<float>& fwdel_pt() const {return m_fwdel_pt;}
    const std::vector<float>& fwdel_eta() const {return m_fwdel_eta;}
    const std::vector<float>& fwdel_phi() const {return m_fwdel_phi;}
    const std::vector<float>& fwdel_e() const {return m_fwdel_e;}
    const std::vector<float>& fwdel_etcone20() const {return m_fwdel_etcone20;}
    const std::vector<float>& fwdel_etcone30() const {return m_fwdel_etcone30;}
    const std::vector<float>& fwdel_etcone40() const {return m_fwdel_etcone40;}

    //muons
    const std::vector<float>& mu_pt() const {return m_mu_pt;}
    const std::vector<float>& mu_eta() const {return m_mu_eta;}
    const std::vector<float>& mu_phi() const {return m_mu_phi;}
    const std::vector<float>& mu_e() const {return m_mu_e;}
    const std::vector<float>& mu_charge() const {return m_mu_charge;}
    const std::vector<float>& mu_topoetcone20() const {return m_mu_topoetcone20;}
    const std::vector<float>& mu_ptvarcone30() const {return m_mu_ptvarcone30;}
    const std::vector<char>& mu_isTight() const {return m_mu_isTight;}
    const std::unordered_map<std::string, std::vector<char> >& mu_trigMatched() const {return m_mu_trigMatched;}
    const std::vector<float>& mu_d0sig() const {return m_mu_d0sig;}
    const std::vector<float>& mu_delta_z0_sintheta() const {return m_mu_delta_z0_sintheta;}
    const std::vector<int>& mu_true_type() const {return m_mu_true_type;}
    const std::vector<int>& mu_true_origin() const {return m_mu_true_origin;}
    const std::vector<char>& mu_true_isPrompt() const {return m_mu_true_isPrompt;}

    //soft muons
    const std::vector<float>& softmu_pt() const {return m_softmu_pt;}
    const std::vector<float>& softmu_eta() const {return m_softmu_eta;}
    const std::vector<float>& softmu_phi() const {return m_softmu_phi;}
    const std::vector<float>& softmu_e() const {return m_softmu_e;}
    const std::vector<float>& softmu_charge() const {return m_softmu_charge;}
    const std::vector<float>& softmu_d0() const {return m_softmu_d0;}
    const std::vector<float>& softmu_d0sig() const {return m_softmu_d0sig;}
    const std::vector<float>& softmu_delta_z0_sintheta() const {return m_softmu_delta_z0_sintheta;}
    const std::vector<int>& softmu_true_type() const {return m_softmu_true_type;}
    const std::vector<int>& softmu_true_origin() const {return m_softmu_true_origin;}
    const std::vector<int>& softmu_true_isPrompt() const {return m_softmu_true_isPrompt;}

    //photons
    const std::vector<float>& ph_pt() const {return m_ph_pt;}
    const std::vector<float>& ph_eta() const {return m_ph_eta;}
    const std::vector<float>& ph_phi() const {return m_ph_phi;}
    const std::vector<float>& ph_e() const {return m_ph_e;}
    const std::vector<float>& ph_true_type() const {return m_ph_true_type;}
    const std::vector<float>& ph_true_origin() const {return m_ph_true_origin;}
    const std::vector<float>& ph_faketype() const {return m_ph_faketype;}
    const std::vector<float>& ph_iso() const {return m_ph_iso;}

    //taus
    const std::vector<float>& tau_pt() const {return m_tau_pt;}
    const std::vector<float>& tau_eta() const {return m_tau_eta;}
    const std::vector<float>& tau_phi() const {return m_tau_phi;}
    const std::vector<float>& tau_charge() const {return m_tau_charge;}

    //jets
    const std::vector<float>& jet_pt() const {return m_jet_pt;}
    const std::vector<float>& jet_eta() const {return m_jet_eta;}
    const std::vector<float>& jet_phi() const {return m_jet_phi;}
    const std::vector<float>& jet_e() const {return m_jet_e;}
    const std::vector<float>& jet_jvt() const {return m_jet_jvt;}
    const std::vector<float>& jet_forwardjvt() const {return m_jet_fjvt;}
    const std::vector<char>& jet_passforwardjvt() const {return m_jet_passfjvt;}
    const std::vector<int>& jet_truthflav() const {return m_jet_truthflav;}
    const std::vector<int>& jet_truthPartonLabel() const {return m_jet_truthPartonLabel;}
    const std::vector<char>& jet_isTrueHS() const {return m_jet_isTrueHS;}
    const std::vector<int>& jet_truthflavExtended() const {return m_jet_HadronConeExclExtendedTruthLabelID;}
    const std::unordered_map<std::string, std::vector<char> >& jet_isbtagged() const {return m_jet_isbtagged;}//one
                                                                                                              // vector
                                                                                                              // per jet
                                                                                                              // per WP
    const std::unordered_map<std::string, std::vector<int> >& jet_tagWeightBin() const {return m_jet_tagWeightBin;}//one
                                                                                                                   // vector
                                                                                                                   // per
                                                                                                                   // jet
                                                                                                                   // tag-weight
                                                                                                                   // bin
                                                                                                                   // in
                                                                                                                   // case
                                                                                                                   // Continuous
                                                                                                                   // WP
                                                                                                                   // is
                                                                                                                   // used

    // fail-JVT jets
    const std::vector<float>& failJvt_jet_pt() const {return m_failJvt_jet_pt;}
    const std::vector<float>& failJvt_jet_eta() const {return m_failJvt_jet_eta;}
    const std::vector<float>& failJvt_jet_phi() const {return m_failJvt_jet_phi;}
    const std::vector<float>& failJvt_jet_e() const {return m_failJvt_jet_e;}
    const std::vector<float>& failJvt_jet_jvt() const {return m_failJvt_jet_jvt;}
    const std::vector<float>& failJvt_jet_forwardjvt() const {return m_failJvt_jet_fjvt;}
    const std::vector<char>& failJvt_jet_passforwardjvt() const {return m_failJvt_jet_passfjvt;}
    const std::vector<int>& failJvt_jet_truthflav() const {return m_failJvt_jet_truthflav;}
    const std::vector<int>& failJvt_jet_truthPartonLabel() const {return m_failJvt_jet_truthPartonLabel;}
    const std::vector<char>& failJvt_jet_isTrueHS() const {return m_failJvt_jet_isTrueHS;}
    const std::vector<int>& failJvt_jet_truthflavExtended() const {return m_failJvt_jet_HadronConeExclExtendedTruthLabelID;}

    // fail-FJVT jets
    const std::vector<float>& failFJvt_jet_pt() const {return m_failFJvt_jet_pt;}
    const std::vector<float>& failFJvt_jet_eta() const {return m_failFJvt_jet_eta;}
    const std::vector<float>& failFJvt_jet_phi() const {return m_failFJvt_jet_phi;}
    const std::vector<float>& failFJvt_jet_e() const {return m_failFJvt_jet_e;}
    const std::vector<float>& failFJvt_jet_jvt() const {return m_failFJvt_jet_jvt;}
    const std::vector<float>& failFJvt_jet_forwardjvt() const {return m_failFJvt_jet_fjvt;}
    const std::vector<char>& failFJvt_jet_passjvt() const {return m_failFJvt_jet_passjvt;}
    const std::vector<int>& failFJvt_jet_truthflav() const {return m_failFJvt_jet_truthflav;}
    const std::vector<int>& failFJvt_jet_truthPartonLabel() const {return m_failFJvt_jet_truthPartonLabel;}
    const std::vector<char>& failFJvt_jet_isTrueHS() const {return m_failFJvt_jet_isTrueHS;}
    const std::vector<int>& failFJvt_jet_truthflavExtended() const {return m_failFJvt_jet_HadronConeExclExtendedTruthLabelID;}

    //large-R jets
    const std::vector<float>& ljet_pt() const {return m_ljet_pt;}
    const std::vector<float>& ljet_eta() const {return m_ljet_eta;}
    const std::vector<float>& ljet_phi() const {return m_ljet_phi;}
    const std::vector<float>& ljet_e() const {return m_ljet_e;}
    const std::vector<float>& ljet_m() const {return m_ljet_m;}
    const std::vector<int>& ljet_truthLabel() const {return m_ljet_truthLabel;}

    const std::unordered_map<std::string, std::vector<float> >& ljet_substructure() const {return m_ljet_substructure;}
    const std::unordered_map<std::string, std::vector<char> >& ljet_isTagged() const {return m_ljet_isTagged;}
    const std::unordered_map<std::string, std::vector<char> >& ljet_taggingPassedRangeCheck() const {return m_ljet_taggingPassedRangeCheck;}
    const std::vector<char>& ljet_isTagged(const std::string& taggerName) {return m_ljet_isTagged[taggerName];}

    //track jets
    const std::vector<float>& tjet_pt() const {return m_tjet_pt;}
    const std::vector<float>& tjet_eta() const {return m_tjet_eta;}
    const std::vector<float>& tjet_phi() const {return m_tjet_phi;}
    const std::vector<float>& tjet_e() const {return m_tjet_e;}
    const std::unordered_map<std::string, std::vector<char> >& tjet_isbtagged() const {return m_tjet_isbtagged;}//one
                                                                                                                // vector
                                                                                                                // per
                                                                                                                // jet
                                                                                                                // per
                                                                                                                // WP
    const std::unordered_map<std::string, std::vector<int> >& tjet_tagWeightBin() const {return m_tjet_tagWeightBin;}//one
                                                                                                                     // vector
                                                                                                                     // per
                                                                                                                     // jet
                                                                                                                     // tag-weight
                                                                                                                     // bin
                                                                                                                     // in
                                                                                                                     // case
                                                                                                                     // Continuous
                                                                                                                     // WP
                                                                                                                     // is
                                                                                                                     // used

    //re-clustered jets
    // -> need unordered map for systematics
    const bool& makeRCJets() const {return m_makeRCJets;} // making re-clustered jets
    const bool& makeVarRCJets() const {return m_makeVarRCJets;} // making VarRC jets
    const std::string& RCJetContainer() const {return m_RCJetContainer;} // name for RC jets container in TStore
    const std::vector<std::string>& VarRCJetRho() const {return m_VarRCJetRho;}
    const std::vector<std::string>& VarRCJetMassScale() const {return m_VarRCJetMassScale;}
    const std::string& egamma() const {return m_egamma;} // egamma systematic naming scheme
    const std::string& muonsyst() const {return m_muonsyst;} // muon systematic naming scheme
    const std::string& softmuonsyst() const {return m_softmuonsyst;} // soft muon systematic naming scheme
    const std::string& jetsyst() const {return m_jetsyst;} // jet systematic naming scheme
    const std::map<std::string, std::vector<float> >& VarRCjetBranches() const {return m_VarRCjetBranches;}
    const std::map<std::string, std::vector<std::vector<float> > >& VarRCjetsubBranches() const {return m_VarRCjetsubBranches;}
    const std::vector<int>& rcjet_nsub() const {return m_rcjet_nsub;}
    const std::vector<float>& rcjet_pt() const {return m_rcjet_pt;}
    const std::vector<float>& rcjet_eta() const {return m_rcjet_eta;}
    const std::vector<float>& rcjet_phi() const {return m_rcjet_phi;}
    const std::vector<float>& rcjet_e() const {return m_rcjet_e;}
    const std::vector<float>& rcjet_d12() const {return m_rcjet_d12;}
    const std::vector<float>& rcjet_d23() const {return m_rcjet_d23;}
    const std::vector<std::vector<float> >& rcjetsub_pt() const {return m_rcjetsub_pt;}
    const std::vector<std::vector<float> >& rcjetsub_eta() const {return m_rcjetsub_eta;}
    const std::vector<std::vector<float> >& rcjetsub_phi() const {return m_rcjetsub_phi;}
    const std::vector<std::vector<float> >& rcjetsub_e() const {return m_rcjetsub_e;}
    const std::vector<float>& rcjet_tau32_clstr() const {return m_rcjet_tau32_clstr;}
    const std::vector<float>& rcjet_tau21_clstr() const {return m_rcjet_tau21_clstr;}
    const std::vector<float>& rcjet_tau3_clstr() const {return m_rcjet_tau3_clstr;}
    const std::vector<float>& rcjet_tau2_clstr() const {return m_rcjet_tau2_clstr;}
    const std::vector<float>& rcjet_tau1_clstr() const {return m_rcjet_tau1_clstr;}
    const std::vector<float>& rcjet_D2_clstr() const {return m_rcjet_D2_clstr;}
    const std::vector<float>& rcjet_ECF1_clstr() const {return m_rcjet_ECF1_clstr;}
    const std::vector<float>& rcjet_ECF2_clstr() const {return m_rcjet_ECF2_clstr;}
    const std::vector<float>& rcjet_ECF3_clstr() const {return m_rcjet_ECF3_clstr;}
    const std::vector<float>& rcjet_d12_clstr() const {return m_rcjet_d12_clstr;}
    const std::vector<float>& rcjet_d23_clstr() const {return m_rcjet_d23_clstr;}
    const std::vector<float>& rcjet_Qw_clstr() const {return m_rcjet_Qw_clstr;}
    const std::vector<float>& rcjet_nconstituent_clstr() const {return m_rcjet_nconstituent_clstr;}
    const std::vector<float>& rcjet_gECF332_clstr() const {return m_rcjet_gECF332_clstr;}
    const std::vector<float>& rcjet_gECF461_clstr() const {return m_rcjet_gECF461_clstr;}
    const std::vector<float>& rcjet_gECF322_clstr() const {return m_rcjet_gECF322_clstr;}
    const std::vector<float>& rcjet_gECF331_clstr() const {return m_rcjet_gECF331_clstr;}
    const std::vector<float>& rcjet_gECF422_clstr() const {return m_rcjet_gECF422_clstr;}
    const std::vector<float>& rcjet_gECF441_clstr() const {return m_rcjet_gECF441_clstr;}
    const std::vector<float>& rcjet_gECF212_clstr() const {return m_rcjet_gECF212_clstr;}
    const std::vector<float>& rcjet_gECF321_clstr() const {return m_rcjet_gECF321_clstr;}
    const std::vector<float>& rcjet_gECF311_clstr() const {return m_rcjet_gECF311_clstr;}

    const std::vector<float>& rcjet_L1_clstr() const {return m_rcjet_L1_clstr;}
    const std::vector<float>& rcjet_L2_clstr() const {return m_rcjet_L2_clstr;}
    const std::vector<float>& rcjet_L3_clstr() const {return m_rcjet_L3_clstr;}
    const std::vector<float>& rcjet_L4_clstr() const {return m_rcjet_L4_clstr;}
    const std::vector<float>& rcjet_L5_clstr() const {return m_rcjet_L5_clstr;}

    //met
    const float& met_met() const {return m_met_met;}
    const float& met_phi() const {return m_met_phi;}

    ///KLFitter
    const short& klfitter_selected() const {return m_klfitter_selected;}
    /// Selection
    const std::vector<std::string> klfitter_selection() const {return m_klfitter_selection;}
    /// Error flags
    const std::vector<short>& klfitter_minuitDidNotConverge() const {return m_klfitter_minuitDidNotConverge;}
    const std::vector<short>& klfitter_fitAbortedDueToNaN() const {return m_klfitter_fitAbortedDueToNaN;}
    const std::vector<short>& klfitter_atLeastOneFitParameterAtItsLimit() const {return m_klfitter_atLeastOneFitParameterAtItsLimit;}
    const std::vector<short>& klfitter_invalidTransferFunctionAtConvergence() const {return m_klfitter_invalidTransferFunctionAtConvergence;}
    /// Global result
    const std::vector<unsigned int>& klfitter_bestPermutation() const {return m_klfitter_bestPermutation;}
    const std::vector<float>& klfitter_logLikelihood() const {return m_klfitter_logLikelihood;}
    const std::vector<float>& klfitter_eventProbability() const {return m_klfitter_eventProbability;}
    const std::vector<unsigned int>& klfitter_parameters_size() const {return m_klfitter_parameters_size;}
    const std::vector<std::vector<double> >& klfitter_parameters() const {return m_klfitter_parameters;}
    const std::vector<std::vector<double> >& klfitter_parameterErrors() const {return m_klfitter_parameterErrors;}
    /// Model
    const std::vector<float>& klfitter_model_bhad_pt() const {return m_klfitter_model_bhad_pt;}
    const std::vector<float>& klfitter_model_bhad_eta() const {return m_klfitter_model_bhad_eta;}
    const std::vector<float>& klfitter_model_bhad_phi() const {return m_klfitter_model_bhad_phi;}
    const std::vector<float>& klfitter_model_bhad_E() const {return m_klfitter_model_bhad_E;}
    const std::vector<unsigned int>& klfitter_model_bhad_jetIndex() const {return m_klfitter_model_bhad_jetIndex;}
    const std::vector<float>& klfitter_model_blep_pt() const {return m_klfitter_model_blep_pt;}
    const std::vector<float>& klfitter_model_blep_eta() const {return m_klfitter_model_blep_eta;}
    const std::vector<float>& klfitter_model_blep_phi() const {return m_klfitter_model_blep_phi;}
    const std::vector<float>& klfitter_model_blep_E() const {return m_klfitter_model_blep_E;}
    const std::vector<unsigned int>& klfitter_model_blep_jetIndex() const {return m_klfitter_model_blep_jetIndex;}
    const std::vector<float>& klfitter_model_lq1_pt() const {return m_klfitter_model_lq1_pt;}
    const std::vector<float>& klfitter_model_lq1_eta() const {return m_klfitter_model_lq1_eta;}
    const std::vector<float>& klfitter_model_lq1_phi() const {return m_klfitter_model_lq1_phi;}
    const std::vector<float>& klfitter_model_lq1_E() const {return m_klfitter_model_lq1_E;}
    const std::vector<unsigned int>& klfitter_model_lq1_jetIndex() const {return m_klfitter_model_lq1_jetIndex;}
    const std::vector<float>& klfitter_model_lq2_pt() const {return m_klfitter_model_lq2_pt;}
    const std::vector<float>& klfitter_model_lq2_eta() const {return m_klfitter_model_lq2_eta;}
    const std::vector<float>& klfitter_model_lq2_phi() const {return m_klfitter_model_lq2_phi;}
    const std::vector<float>& klfitter_model_lq2_E() const {return m_klfitter_model_lq2_E;}
    const std::vector<unsigned int>& klfitter_model_lq2_jetIndex() const {return m_klfitter_model_lq2_jetIndex;}
    const std::vector<float>& klfitter_model_Higgs_b1_pt() const {return m_klfitter_model_Higgs_b1_pt;}
    const std::vector<float>& klfitter_model_Higgs_b1_eta() const {return m_klfitter_model_Higgs_b1_eta;}
    const std::vector<float>& klfitter_model_Higgs_b1_phi() const {return m_klfitter_model_Higgs_b1_phi;}
    const std::vector<float>& klfitter_model_Higgs_b1_E() const {return m_klfitter_model_Higgs_b1_E;}
    const std::vector<unsigned int>& klfitter_model_Higgs_b1_jetIndex() const {return m_klfitter_model_Higgs_b1_jetIndex;}
    const std::vector<float>& klfitter_model_Higgs_b2_pt() const {return m_klfitter_model_Higgs_b2_pt;}
    const std::vector<float>& klfitter_model_Higgs_b2_eta() const {return m_klfitter_model_Higgs_b2_eta;}
    const std::vector<float>& klfitter_model_Higgs_b2_phi() const {return m_klfitter_model_Higgs_b2_phi;}
    const std::vector<float>& klfitter_model_Higgs_b2_E() const {return m_klfitter_model_Higgs_b2_E;}
    const std::vector<unsigned int>& klfitter_model_Higgs_b2_jetIndex() const {return m_klfitter_model_Higgs_b2_jetIndex;}
    const std::vector<float>& klfitter_model_lep_pt() const {return m_klfitter_model_lep_pt;}
    const std::vector<float>& klfitter_model_lep_eta() const {return m_klfitter_model_lep_eta;}
    const std::vector<float>& klfitter_model_lep_phi() const {return m_klfitter_model_lep_phi;}
    const std::vector<float>& klfitter_model_lep_E() const {return m_klfitter_model_lep_E;}
    const std::vector<float>& klfitter_model_nu_pt() const {return m_klfitter_model_nu_pt;}
    const std::vector<float>& klfitter_model_nu_eta() const {return m_klfitter_model_nu_eta;}
    const std::vector<float>& klfitter_model_nu_phi() const {return m_klfitter_model_nu_phi;}
    const std::vector<float>& klfitter_model_nu_E() const {return m_klfitter_model_nu_E;}
    const std::vector<float>& klfitter_model_b_from_top1_pt() const {return m_klfitter_model_b_from_top1_pt;}
    const std::vector<float>& klfitter_model_b_from_top1_eta() const {return m_klfitter_model_b_from_top1_eta;}
    const std::vector<float>& klfitter_model_b_from_top1_phi() const {return m_klfitter_model_b_from_top1_phi;}
    const std::vector<float>& klfitter_model_b_from_top1_E() const {return m_klfitter_model_b_from_top1_E;}
    const std::vector<unsigned int>& klfitter_model_b_from_top1_jetIndex() const {return m_klfitter_model_b_from_top1_jetIndex;}
    const std::vector<float>& klfitter_model_b_from_top2_pt() const {return m_klfitter_model_b_from_top2_pt;}
    const std::vector<float>& klfitter_model_b_from_top2_eta() const {return m_klfitter_model_b_from_top2_eta;}
    const std::vector<float>& klfitter_model_b_from_top2_phi() const {return m_klfitter_model_b_from_top2_phi;}
    const std::vector<float>& klfitter_model_b_from_top2_E() const {return m_klfitter_model_b_from_top2_E;}
    const std::vector<unsigned int>& klfitter_model_b_from_top2_jetIndex() const {return m_klfitter_model_b_from_top2_jetIndex;}
    const std::vector<float>& klfitter_model_lj1_from_top1_pt() const {return m_klfitter_model_lj1_from_top1_pt;}
    const std::vector<float>& klfitter_model_lj1_from_top1_eta() const {return m_klfitter_model_lj1_from_top1_eta;}
    const std::vector<float>& klfitter_model_lj1_from_top1_phi() const {return m_klfitter_model_lj1_from_top1_phi;}
    const std::vector<float>& klfitter_model_lj1_from_top1_E() const {return m_klfitter_model_lj1_from_top1_E;}
    const std::vector<unsigned int>& klfitter_model_lj1_from_top1_jetIndex() const {return m_klfitter_model_lj1_from_top1_jetIndex;}
    const std::vector<float>& klfitter_model_lj2_from_top1_pt() const {return m_klfitter_model_lj2_from_top1_pt;}
    const std::vector<float>& klfitter_model_lj2_from_top1_eta() const {return m_klfitter_model_lj2_from_top1_eta;}
    const std::vector<float>& klfitter_model_lj2_from_top1_phi() const {return m_klfitter_model_lj2_from_top1_phi;}
    const std::vector<float>& klfitter_model_lj2_from_top1_E() const {return m_klfitter_model_lj2_from_top1_E;}
    const std::vector<unsigned int>& klfitter_model_lj2_from_top1_jetIndex() const {return m_klfitter_model_lj2_from_top1_jetIndex;}
    const std::vector<float>& klfitter_model_lj1_from_top2_pt() const {return m_klfitter_model_lj1_from_top2_pt;}
    const std::vector<float>& klfitter_model_lj1_from_top2_eta() const {return m_klfitter_model_lj1_from_top2_eta;}
    const std::vector<float>& klfitter_model_lj1_from_top2_phi() const {return m_klfitter_model_lj1_from_top2_phi;}
    const std::vector<float>& klfitter_model_lj1_from_top2_E() const {return m_klfitter_model_lj1_from_top2_E;}
    const std::vector<unsigned int>& klfitter_model_lj1_from_top2_jetIndex() const {return m_klfitter_model_lj1_from_top2_jetIndex;}
    const std::vector<float>& klfitter_model_lj2_from_top2_pt() const {return m_klfitter_model_lj2_from_top2_pt;}
    const std::vector<float>& klfitter_model_lj2_from_top2_eta() const {return m_klfitter_model_lj2_from_top2_eta;}
    const std::vector<float>& klfitter_model_lj2_from_top2_phi() const {return m_klfitter_model_lj2_from_top2_phi;}
    const std::vector<float>& klfitter_model_lj2_from_top2_E() const {return m_klfitter_model_lj2_from_top2_E;}
    const std::vector<unsigned int>& klfitter_model_lj2_from_top2_jetIndex() const {return m_klfitter_model_lj2_from_top2_jetIndex;}

    // calculated KLFitter variables for best perm
    const float& klfitter_bestPerm_topLep_pt() const {return m_klfitter_bestPerm_topLep_pt;}
    const float& klfitter_bestPerm_topLep_eta() const {return m_klfitter_bestPerm_topLep_eta;}
    const float& klfitter_bestPerm_topLep_phi() const {return m_klfitter_bestPerm_topLep_phi;}
    const float& klfitter_bestPerm_topLep_E() const {return m_klfitter_bestPerm_topLep_E;}
    const float& klfitter_bestPerm_topLep_m() const {return m_klfitter_bestPerm_topLep_m;}
    const float& klfitter_bestPerm_topHad_pt() const {return m_klfitter_bestPerm_topHad_pt;}
    const float& klfitter_bestPerm_topHad_eta() const {return m_klfitter_bestPerm_topHad_eta;}
    const float& klfitter_bestPerm_topHad_phi() const {return m_klfitter_bestPerm_topHad_phi;}
    const float& klfitter_bestPerm_topHad_E() const {return m_klfitter_bestPerm_topHad_E;}
    const float& klfitter_bestPerm_topHad_m() const {return m_klfitter_bestPerm_topHad_m;}
    const float& klfitter_bestPerm_ttbar_pt() const {return m_klfitter_bestPerm_ttbar_pt;}
    const float& klfitter_bestPerm_ttbar_eta() const {return m_klfitter_bestPerm_ttbar_eta;}
    const float& klfitter_bestPerm_ttbar_phi() const {return m_klfitter_bestPerm_ttbar_phi;}
    const float& klfitter_bestPerm_ttbar_E() const {return m_klfitter_bestPerm_ttbar_E;}
    const float& klfitter_bestPerm_ttbar_m() const {return m_klfitter_bestPerm_ttbar_m;}
    // PseudoTop variables
    const float& PseudoTop_Reco_ttbar_pt() const {return m_PseudoTop_Reco_ttbar_pt;}
    const float& PseudoTop_Reco_ttbar_eta() const {return m_PseudoTop_Reco_ttbar_eta;}
    const float& PseudoTop_Reco_ttbar_phi() const {return m_PseudoTop_Reco_ttbar_phi;}
    const float& PseudoTop_Reco_ttbar_m() const {return m_PseudoTop_Reco_ttbar_m;}
    const float& PseudoTop_Reco_top_had_pt() const {return m_PseudoTop_Reco_top_had_pt;}
    const float& PseudoTop_Reco_top_had_eta() const {return m_PseudoTop_Reco_top_had_eta;}
    const float& PseudoTop_Reco_top_had_phi() const {return m_PseudoTop_Reco_top_had_phi;}
    const float& PseudoTop_Reco_top_had_m() const {return m_PseudoTop_Reco_top_had_m;}
    const float& PseudoTop_Reco_top_lep_pt() const {return m_PseudoTop_Reco_top_lep_pt;}
    const float& PseudoTop_Reco_top_lep_eta() const {return m_PseudoTop_Reco_top_lep_eta;}
    const float& PseudoTop_Reco_top_lep_phi() const {return m_PseudoTop_Reco_top_lep_phi;}
    const float& PseudoTop_Reco_top_lep_m() const {return m_PseudoTop_Reco_top_lep_m;}
    const float& PseudoTop_Particle_ttbar_pt() const {return m_PseudoTop_Particle_ttbar_pt;}
    const float& PseudoTop_Particle_ttbar_eta() const {return m_PseudoTop_Particle_ttbar_eta;}
    const float& PseudoTop_Particle_ttbar_phi() const {return m_PseudoTop_Particle_ttbar_phi;}
    const float& PseudoTop_Particle_ttbar_m() const {return m_PseudoTop_Particle_ttbar_m;}
    const float& PseudoTop_Particle_top_had_pt() const {return m_PseudoTop_Particle_top_had_pt;}
    const float& PseudoTop_Particle_top_had_eta() const {return m_PseudoTop_Particle_top_had_eta;}
    const float& PseudoTop_Particle_top_had_phi() const {return m_PseudoTop_Particle_top_had_phi;}
    const float& PseudoTop_Particle_top_had_m() const {return m_PseudoTop_Particle_top_had_m;}
    const float& PseudoTop_Particle_top_lep_pt() const {return m_PseudoTop_Particle_top_lep_pt;}
    const float& PseudoTop_Particle_top_lep_eta() const {return m_PseudoTop_Particle_top_lep_eta;}
    const float& PseudoTop_Particle_top_lep_phi() const {return m_PseudoTop_Particle_top_lep_phi;}
    const float& PseudoTop_Particle_top_lep_m() const {return m_PseudoTop_Particle_top_lep_m;}

    //MC
    const std::vector<float>& mc_pt() const {return m_mc_pt;}
    const std::vector<float>& mc_eta() const {return m_mc_eta;}
    const std::vector<float>& mc_phi() const {return m_mc_phi;}
    const std::vector<float>& mc_e() const {return m_mc_e;}
    const std::vector<double>& mc_charge() const {return m_mc_charge;}
    const std::vector<int>& mc_pdgId() const {return m_mc_pdgId;}
    const std::vector<int>& mc_status() const {return m_mc_status;}
    const std::vector<int>& mc_barcode() const {return m_mc_barcode;}

    //PDFInfo
    const std::vector<float>& PDFinfo_X1() const {return m_PDFinfo_X1;}
    const std::vector<float>& PDFinfo_X2() const {return m_PDFinfo_X2;}
    const std::vector<int>& PDFinfo_PDGID1() const {return m_PDFinfo_PDGID1;}
    const std::vector<int>& PDFinfo_PDGID2() const {return m_PDFinfo_PDGID2;}
    const std::vector<float>& PDFinfo_Q() const {return m_PDFinfo_Q;}
    const std::vector<float>& PDFinfo_XF1() const {return m_PDFinfo_XF1;}
    const std::vector<float>& PDFinfo_XF2() const {return m_PDFinfo_XF2;}

    //the on-the-fly computed generator weights stored in EventInfo
    const std::vector<float>& mc_generator_weights() const {return m_mc_generator_weights;}

    //Extra variables for Particle Level (bare lepton kinematics and b-Hadron
    //tagging information).
    const std::vector<float>& el_pt_bare() const {return m_el_pt_bare;}
    const std::vector<float>& el_eta_bare() const {return m_el_eta_bare;}
    const std::vector<float>& el_phi_bare() const {return m_el_phi_bare;}
    const std::vector<float>& el_e_bare() const {return m_el_e_bare;}
    const std::vector<float>& mu_pt_bare() const {return m_mu_pt_bare;}
    const std::vector<float>& mu_eta_bare() const {return m_mu_eta_bare;}
    const std::vector<float>& mu_phi_bare() const {return m_mu_phi_bare;}
    const std::vector<float>& mu_e_bare() const {return m_mu_e_bare;}
    const std::vector<int>& jet_Ghosts_BHadron_Final_Count() const {return m_jet_Ghosts_BHadron_Final_Count;}
    const std::vector<int>& jet_Ghosts_CHadron_Final_Count() const {return m_jet_Ghosts_CHadron_Final_Count;}
    const std::vector<int>& ljet_Ghosts_BHadron_Final_Count() const {return m_ljet_Ghosts_BHadron_Final_Count;}
    const std::vector<int>& ljet_Ghosts_CHadron_Final_Count() const {return m_ljet_Ghosts_CHadron_Final_Count;}
    const std::vector<std::vector<int> >& rcjetsub_Ghosts_BHadron_Final_Count() const {return m_rcjetsub_Ghosts_BHadron_Final_Count;}
    const std::vector<std::vector<int> >& rcjetsub_Ghosts_CHadron_Final_Count() const {return m_rcjetsub_Ghosts_CHadron_Final_Count;}

    // Truth tree inserted variables
    // This can be expanded as required
    // This is just a first pass at doing this sort of thing
    const std::unordered_map<std::string, int*>& extraTruthVars_int() const {return m_extraTruthVars_int;}

    // Prompt lepton definition for event saver
    std::pair<bool, bool> isPromptElectron(int type, int origin, int egMotherType, int egMotherOrigin, int egMotherPdgId, int RecoCharge);
    bool isPromptMuon(int type, int origin);

    int filterBranches(const top::TreeManager*, const std::string& variable);

    ClassDefOverride(top::EventSaverFlatNtuple, 0);
  };
}

#endif
