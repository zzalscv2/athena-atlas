/*
 Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
 */

#include <sstream>
#include <TRandom3.h>
#include "TROOT.h"
#include "TH1.h"
#include "TH2.h"
#include "TFile.h"
#include "TKey.h"

#include "xAODMuon/MuonContainer.h"
#include "xAODMuon/MuonAuxContainer.h"
#include "xAODTrigger/MuonRoIContainer.h"
#include "MuonEfficiencyCorrections/MuonTriggerScaleFactors.h"

#include "AsgMessaging/StatusCode.h"
#include "PATInterfaces/SystematicRegistry.h"
#include "PATInterfaces/SystematicVariation.h"
#include "FourMomUtils/xAODP4Helpers.h"
#include "PathResolver/PathResolver.h"
#include "AsgDataHandles/ReadHandle.h"

#include <iostream>
#include <functional>
#include <string>
#include <cmath>

namespace CP {
    static const double muon_barrel_endcap_boundary = 1.05;
    MuonTriggerScaleFactors::MuonTriggerScaleFactors(const std::string& name) :
        asg::AsgTool(name),
        m_systFilter(),
        m_appliedSystematics(nullptr),
        m_fileName(),
        m_efficiencyMap(),
        m_efficiencyMapReplicaArray(),
        m_muonquality("Medium"),
        m_calibration_version("230222_Winter_r22"),
        m_custom_dir(),
        m_binning("fine"),
        m_allowZeroSF(false),
        m_experimental(false),
        m_forceYear(-1),
        m_forcePeriod(""),
        m_replicaTriggerList(),
        m_replicaSet(),
        m_nReplicas(100),
        m_ReplicaRandomSeed(12345) {
      
        declareProperty("MuonQuality", m_muonquality); // HighPt,Tight,Medium,Loose,LowPt
        declareProperty("CalibrationRelease", m_calibration_version);
        // these are for debugging / testing, *not* for general use!
        declareProperty("filename", m_fileName);
        declareProperty("CustomInputFolder", m_custom_dir);
        declareProperty("Binning", m_binning); // fine or coarse
        declareProperty("UseExperimental", m_experimental); // enable experimental features like single muon SF
        //Properties needed for TOY setup for a given trigger: No replicas if m_replicaTriggerList is empty
        declareProperty("ReplicaTriggerList", m_replicaTriggerList, "List of triggers on which we want to generate stat. uncertainty toy replicas.");
        declareProperty("NReplicas", m_nReplicas, "Number of generated toy replicas, if replicas are required.");
        declareProperty("ReplicaRandomSeed", m_ReplicaRandomSeed, "Random seed for toy replica generation.");
        declareProperty("AllowZeroSF", m_allowZeroSF, "If a trigger is not available will return 0 instead of throwing an error. More difficult to spot configuration issues. Use at own risk");
        declareProperty("forceYear", m_forceYear, "Only for developers. Never use this in any analysis!!!!!!");
        declareProperty("forcePeriod", m_forcePeriod, "Only for developers. Never use this in any analysis!!!!!!");
    }

    MuonTriggerScaleFactors::~MuonTriggerScaleFactors() { }

  StatusCode MuonTriggerScaleFactors::LoadTriggerMap(unsigned int year) {
        std::string fileName = m_fileName;
        if (fileName.empty()) {
          if (year == 2015) fileName = "muontrigger_sf_2015_mc20a_v1.root";
          else if (year == 2016) fileName = "muontrigger_sf_2016_mc20a_v1.root";
          else if (year == 2017) fileName = "muontrigger_sf_2017_mc20d_v1.root";
          else if (year == 2018) fileName = "muontrigger_sf_2018_mc20e_v1.root";
          else if (year == 2022) fileName = "muontrigger_sf_2022_mc21_v05.root";
          else{
            ATH_MSG_WARNING("There is no SF file for year " << year << " yet");
            return StatusCode::SUCCESS;
          }
        }
    
        TDirectory* origDir = gDirectory;

        std::string filePath;

        if (m_custom_dir.empty()) {
            filePath = PathResolverFindCalibFile(Form("MuonEfficiencyCorrections/%s/%s", m_calibration_version.c_str(), fileName.c_str()));
            if (filePath.empty()) {
                ATH_MSG_ERROR("Unable to resolve the input file " << fileName << " via PathResolver.");
            }
        }
        else {
            ATH_MSG_INFO("Note: setting up with user specified input file location " << m_custom_dir << " - this is not encouraged!");
            filePath = PathResolverFindCalibFile(Form("%s/%s", m_custom_dir.c_str(), fileName.c_str()));
        }

        TFile* file = TFile::Open(filePath.c_str());

        if (file == nullptr || !file->IsOpen()) {
            ATH_MSG_FATAL("MuonTriggerScaleFactors::initialize Couldn't open file " << filePath);
            return StatusCode::FAILURE;
        }
        TDirectory* tempDir = getTemporaryDirectory();
        tempDir->cd();

        static const std::vector<std::string> type { "data", "mc" };
        static const std::vector<std::string> region { "barrel", "endcap" };
        static const std::vector<std::string> systematic { "nominal", "stat_up", "stat_down", "syst_up", "syst_down" };
        if(m_muonquality.compare("LowPt") == 0)
          m_muonquality = "Medium";
        const std::string quality = m_muonquality;
        TDirectory* qualityDirectory = file->GetDirectory(m_muonquality.c_str());
        if (qualityDirectory == nullptr) {
            ATH_MSG_FATAL("MuonTriggerScaleFactors::initialize cannot find directory with selected quality");
            return StatusCode::FAILURE;
        }
        TKey* periodKey;
        TIter nextPeriod(qualityDirectory->GetListOfKeys());
        while ((periodKey = (TKey*) nextPeriod())) {
            if (not periodKey->IsFolder()) continue;
            TDirectory* periodDirectory = qualityDirectory->GetDirectory(periodKey->GetName());
            std::string periodName = std::string(periodKey->GetName());
            std::string periodName_InMap = periodName.substr(std::string("Period").size(), periodName.size());
            YearPeriod period = YearPeriod(year, periodName_InMap);
            TKey* triggerKey;
            TIter nextTrigger(periodDirectory->GetListOfKeys());
            while ((triggerKey = (TKey*) nextTrigger())) {
                if (not triggerKey->IsFolder()) continue;
                TDirectory* triggerDirectory = periodDirectory->GetDirectory(triggerKey->GetName());
                std::string triggerName = std::string(triggerKey->GetName());
                if(!std::set<std::string>{"HLT_mu26_ivarmedium", "HLT_mu50", "HLT_mu26_ivarmedium_OR_HLT_mu50"}.count(triggerName) && m_binning == "coarse"){
                  ATH_MSG_DEBUG("Coarse binning not supported for di-muon trigger legs at the moment");
                  continue;
                }
                for (const auto& iregion : region) {
                    bool isBarrel = iregion.find("barrel") != std::string::npos;
                    for (const auto& itype : type) {
                        bool isData = itype.find("data") != std::string::npos;
                        std::string histname = ("_MuonTrigEff_" + periodName + "_" + triggerName + "_" + quality + "_" + "_EtaPhi_" + m_binning + "_" + iregion + "_" + itype);
                        for (const auto& isys : systematic) {
                            if (itype.find("data") != std::string::npos && isys.find("syst") != std::string::npos) continue;
                            std::string path = "eff_etaphi_" + m_binning + "_" + iregion + "_" + itype + "_" + isys;
                            TH2* hist = dynamic_cast<TH2*>(triggerDirectory->Get(path.c_str()));
                            if (not hist) {
			      
                                ATH_MSG_FATAL("MuonTriggerScaleFactors::initialize " << path << " not found under trigger " << triggerName << " and period " << periodName << " for year: " << year);
                                continue;
                            }
                            hist->SetDirectory(0);

                            EffiHistoIdent HistoId = EffiHistoIdent(period, encodeHistoName(periodName_InMap, triggerName, isData, isys, isBarrel));
                            if (m_efficiencyMap.find(HistoId) != m_efficiencyMap.end()) {
                                ATH_MSG_FATAL("MuonTriggerScaleFactors::initialize(): histogram " << path << " is duplicated for year" << year << " in period " << periodName);
                                return StatusCode::FAILURE;
                            }
                            m_efficiencyMap.insert(std::pair<EffiHistoIdent, TH1_Ptr>(HistoId, std::shared_ptr < TH1 > (hist)));

                        }
                        //If the trigger is chosen for toy evaluation, generate all the replicas from
                        // NOMINAL with STAT variations stored in the data hist, load them in corresponding vector
                        if (m_replicaSet.find(triggerName) != m_replicaSet.end() && itype.find("data") != std::string::npos) {

                            TH1_Ptr Nominal_H = getEfficiencyHistogram(year, periodName, triggerName, isData, "nominal", isBarrel);
                            TH1_Ptr StatUp_H = getEfficiencyHistogram(year, periodName, triggerName, isData, "stat_up", isBarrel);

                            TH1_Ptr tmp_h2 = TH1_Ptr(dynamic_cast<TH2F*>(Nominal_H->Clone(Form("tmp_h2_%s", Nominal_H->GetName()))));
                            const int xbins = tmp_h2->GetNbinsX(), ybins = tmp_h2->GetNbinsY();
                            for (int x_i = 0; x_i <= xbins; ++x_i) {
                                for (int y_i = 0; y_i <= ybins; ++y_i) {
                                    double statErr = std::abs(tmp_h2->GetBinContent(x_i, y_i) - StatUp_H->GetBinContent(x_i, y_i));
                                    tmp_h2->SetBinError(x_i, y_i, statErr);
                                }
                            }
                            m_efficiencyMapReplicaArray[EffiHistoIdent(period, encodeHistoName(periodName, triggerName, isData, "repl", isBarrel))] = generateReplicas(tmp_h2, m_nReplicas, m_ReplicaRandomSeed);
                        }
                    }
                }
            }
        }
        file->Close();
        delete file;
        origDir->cd();
        return StatusCode::SUCCESS;
    }
    // ==================================================================================
    // == MuonTriggerScaleFactors::initialize()
    // ==================================================================================
    StatusCode MuonTriggerScaleFactors::initialize() {

        ATH_MSG_INFO("MuonQuality = '" << m_muonquality << "'");
        ATH_MSG_INFO("Binning = '" << m_binning << "'");
        ATH_MSG_INFO("CalibrationRelease = '" << m_calibration_version << "'");
        ATH_MSG_INFO("CustomInputFolder = '" << m_custom_dir << "'");
        ATH_MSG_INFO("AllowZeroSF = " << m_allowZeroSF);
        ATH_MSG_INFO("experimental = " << m_experimental);

        ATH_CHECK(m_eventInfo.initialize());

        if (registerSystematics() != StatusCode::SUCCESS) {
            return StatusCode::FAILURE;
        }

        if (applySystematicVariation(CP::SystematicSet()) != StatusCode::SUCCESS) {
            ATH_MSG_ERROR("Could not configure for nominal settings");
            return StatusCode::FAILURE;
        }
        // Initialize indexes of replicas for trigges which are asked
        for (auto trigToy : m_replicaTriggerList)
            m_replicaSet.insert(trigToy);

        ATH_MSG_INFO("MuonTriggerScaleFactors::initialize");
        static const int years_to_run[5] = {2015, 2016, 2017, 2018, 2022};
        for (const int &year: years_to_run) {

            // skip 2022 Tight WP as it is not supported
            if ((year == 2022) && (m_muonquality == "Tight")) continue;
            ATH_CHECK(LoadTriggerMap(year));
        }
        return StatusCode::SUCCESS;
    }
  
    CorrectionCode MuonTriggerScaleFactors::getTriggerScaleFactor(const xAOD::Muon& muon, Double_t& triggersf, const std::string& trigger) const {
      if(!m_experimental){
	ATH_MSG_ERROR("MuonTriggerScaleFactors::getTriggerScaleFactor This is an experimental function. If you really know what you are doing set UseExperimental property.");
      return CorrectionCode::Error;
      }
	
      if (trigger.empty()) {
	ATH_MSG_ERROR("MuonTriggerScaleFactors::getTriggerScaleFactor Trigger must have value.");
	return CorrectionCode::Error;
      }

      TrigMuonEff::Configuration configuration;

      if (trigger == "HLT_mu8noL1")
	ATH_MSG_WARNING("What you are trying to do is not correct. For di-muon triggers you should get the efficiency with getTriggerEfficiency and compute the SF by yourself.");
      else if (trigger.find("HLT_2mu10") != std::string::npos || trigger.find("HLT_2mu14") != std::string::npos)
	ATH_MSG_WARNING("Di-muon trigger scale factors for single reco muons are not supported!");
      else
	return GetTriggerSF(triggersf, configuration, muon, trigger);
      return CorrectionCode::Ok;
    }
    
    CorrectionCode MuonTriggerScaleFactors::getTriggerScaleFactor(const xAOD::MuonContainer& mucont, Double_t& triggersf, const std::string& trigger) const{
        if (trigger.empty()) {
            ATH_MSG_ERROR("MuonTriggerScaleFactors::getTriggerScaleFactor Trigger must have value.");
            return CorrectionCode::Error;
        }

        TrigMuonEff::Configuration configuration;

        if (trigger == "HLT_mu8noL1") {
            ATH_MSG_WARNING("What you are trying to do is not correct. For di-muon triggers you should get the efficiency with getTriggerEfficiency and compute the SF by yourself.");
        }
	else if (trigger.find("HLT_2mu10") != std::string::npos || trigger.find("HLT_2mu14") != std::string::npos) {
	  CorrectionCode cc = GetTriggerSF_dimu(triggersf, configuration, mucont, trigger);
	  return cc;
        } else {
            CorrectionCode cc = GetTriggerSF(triggersf, configuration, mucont, trigger);
            return cc;
        }
        return CorrectionCode::Ok;
    }

    // ==================================================================================
    // == MuonTriggerScaleFactors::getReplica_index
    // ==================================================================================
    // Gets  replica index correponding to the toy.
    // Also checks if the sys_name contains "MCTOY" and if the trigger has replicas generated.
    // Returns -1 if conditions are note satisfied
    int MuonTriggerScaleFactors::getReplica_index(const std::string& sysBaseName, const std::string& trigStr) const{
        if (m_replicaSet.find(trigStr) == m_replicaSet.end()) return -1; //No toys for this trigger
        std::size_t pos = sysBaseName.find("MCTOY");
        if (pos == std::string::npos) return -1; //sys variation not affected by TOYS
        return atoi(sysBaseName.substr(pos + 5, pos + 8).c_str()); //toys for this trigger are around get the 3-digit number
    }

    CorrectionCode MuonTriggerScaleFactors::getTriggerEfficiency(const xAOD::Muon& mu, Double_t& efficiency, const std::string& trigger, Bool_t dataType) const{
        if (trigger.empty()) {
            ATH_MSG_ERROR("MuonTriggerScaleFactors::getTriggerEfficiency Trigger must have value.");
            return CorrectionCode::Error;
        }
        TrigMuonEff::Configuration configuration;
        configuration.isData = dataType;
        configuration.replicaIndex = -1;
        Int_t threshold;
        CorrectionCode result = getThreshold(threshold, trigger);
        if (result != CorrectionCode::Ok) return result;
        if (mu.pt() < threshold) {
            efficiency = 0;
            return CorrectionCode::Ok;
        }

        // Pre-define uncertainty variations
        static const CP::SystematicVariation stat_up("MUON_EFF_TrigStatUncertainty", 1);
        static const CP::SystematicVariation stat_down("MUON_EFF_TrigStatUncertainty", -1);
        static const CP::SystematicVariation syst_up("MUON_EFF_TrigSystUncertainty", 1);
        static const CP::SystematicVariation syst_down("MUON_EFF_TrigSystUncertainty", -1);

        std::string systype = "";
        if (appliedSystematics().matchSystematic(syst_down) && !dataType) {
            systype = "syst_down";
        } else if (appliedSystematics().matchSystematic(syst_up) && !dataType) {
            systype = "syst_up";
        } else if (appliedSystematics().matchSystematic(stat_down)) {
            systype = "stat_down";
        } else if (appliedSystematics().matchSystematic(stat_up)) {
            systype = "stat_up";
        } else {
            systype = "nominal";
        }

        // Toys, if found, will overwrite the data hists stat with the generated toy
        //+++++++++++++
        // The best way is the use of filterByName with the 000MCTOY at the end. See:
        // if( !(appliedSystematics().filterByBaseName("MUON_EFF_Trig_MCTOY000")).empty()){//The following is a hack!!!
        //++++++++++THE FOLLOWING IS A PARTIAL HACK!!!
        if (!appliedSystematics().empty() && configuration.isData == true) {
            configuration.replicaIndex = getReplica_index(appliedSystematics().begin()->basename(), trigger);
            if (configuration.replicaIndex != -1) systype = "replicas";
        }
        CorrectionCode cc = getMuonEfficiency(efficiency, configuration, mu, trigger, systype);
        return cc;
    }

    ///////////////////////
    // Private functions //
    ///////////////////////

    // ==================================================================================
    // == MuonTriggerScaleFactors::generateReplicas
    // ==================================================================================
    // Generate replicas of h for Toys with each bin of h varied with Gaussian distribution
    // with mean from bin content and sigma from bin error
    std::vector<TH1_Ptr> MuonTriggerScaleFactors::generateReplicas(TH1_Ptr h, int nrep, int seed) const {
        TRandom3 Rndm(seed);
        std::vector<TH1_Ptr> replica_v;
        const int xbins = h->GetNbinsX(), ybins = h->GetNbinsY();

        for (int t = 0; t < nrep; ++t) {
            TH2* replica = dynamic_cast<TH2*>(h->Clone(Form("rep%d_%s", t, h->GetName())));

            for (int x_i = 0; x_i <= xbins; ++x_i) {
                for (int y_i = 0; y_i <= ybins; ++y_i) {
                    replica->SetBinContent(x_i, y_i, Rndm.Gaus(h->GetBinContent(x_i, y_i), h->GetBinError(x_i, y_i)));
                }
            }
            replica_v.push_back(TH1_Ptr(replica));
        }
        return replica_v;
    }

   bool MuonTriggerScaleFactors::isTriggerSupported(const std::string& trigger) const{
       TH1_Ptr H1 = getEfficiencyHistogram(trigger, true, "nominal");
       return H1.get() != nullptr;
    }
    
  int MuonTriggerScaleFactors::getBinNumber(const xAOD::Muon& muon, const std::string& trigger) const{
    if(!m_experimental){
      ATH_MSG_ERROR("MuonTriggerScaleFactors::getTriggerScaleFactor This is an experimental function. If you really know what you are doing set UseExperimental property.");
      return CorrectionCode::Error;
    }

    const double mu_eta = muon.eta();
    const double mu_phi = muon.phi();
    bool isBarrel = std::abs(mu_eta) < muon_barrel_endcap_boundary;
    TH1_Ptr cit = getEfficiencyHistogram(trigger, true, "nominal", isBarrel);
    if(!cit.get()){
      if(!m_allowZeroSF)
	ATH_MSG_ERROR("Could not find efficiency map for muon with eta: " << mu_eta << " and phi: " << mu_phi << ". Something is inconsistent. Please check your settings for year, mc and trigger." );
      return -1;
    }
    auto eff_h2 = cit;
    double mu_phi_corr = mu_phi;
    if (mu_phi_corr < eff_h2->GetYaxis()->GetXmin()) mu_phi_corr += 2.0 * M_PI;
    if (mu_phi_corr > eff_h2->GetYaxis()->GetXmax()) mu_phi_corr -= 2.0 * M_PI;
    return eff_h2->FindFixBin(mu_eta, mu_phi_corr);
  }

    unsigned int MuonTriggerScaleFactors::encodeHistoName(const std::string& period, const std::string& Trigger, bool isData, const std::string& Systematic, bool isBarrel) const {
        //keep the string as short as possible
        const std::string histName = period + "_" + Trigger + "_" + (isBarrel ? "b" : "e") + "_" + (isData ? "data" : "mc") + Systematic;
        return std::hash<std::string>()(histName);
    }

    unsigned int MuonTriggerScaleFactors::encodeHistoName(const std::string& Trigger, const TrigMuonEff::Configuration& configuration, const std::string& Systematic, bool isBarrel) const {
        //keep the string as short as possible
        return encodeHistoName(getDataPeriod(), Trigger, configuration.isData, Systematic, isBarrel);

    }
    TH1_Ptr MuonTriggerScaleFactors::getEfficiencyHistogram(unsigned int year, const std::string& period, const std::string& trigger, bool isData, const std::string& Systematic, bool isBarrel) const {
        EffiHistoIdent Ident = EffiHistoIdent(YearPeriod(year, period), encodeHistoName(period, trigger, isData, Systematic, isBarrel));
        EfficiencyMap::const_iterator Itr = m_efficiencyMap.find(Ident);

        if (Itr == m_efficiencyMap.end()) {
            return TH1_Ptr();
        }
        return Itr->second;
    }
    TH1_Ptr MuonTriggerScaleFactors::getEfficiencyHistogram(const std::string& trigger, bool isData, const std::string& Systematic, bool isBarrel) const {
        unsigned int run = getRunNumber();
        return getEfficiencyHistogram(getYear(run), getDataPeriod(run), trigger, isData, Systematic, isBarrel);
    }

    CorrectionCode MuonTriggerScaleFactors::getMuonEfficiency(Double_t& eff, const TrigMuonEff::Configuration& configuration, const xAOD::Muon& muon, const std::string& trigger, const std::string& systematic) const{
        const double mu_eta = muon.eta();
        const double mu_phi = muon.phi();
        bool isBarrel = std::abs(mu_eta) < muon_barrel_endcap_boundary;

        TH1_Ptr eff_h2 = nullptr;
        if (configuration.replicaIndex >= 0) { //Only look into the replicas if asking for them
            
            unsigned int run = getRunNumber();            
            EffiHistoIdent Ident = EffiHistoIdent(YearPeriod(getYear(run), getDataPeriod(run)), encodeHistoName(getDataPeriod(run), trigger, configuration.isData, "repl", isBarrel));
            std::map<EffiHistoIdent, std::vector<TH1_Ptr> >::const_iterator cit = m_efficiencyMapReplicaArray.find(Ident);
            if (cit == m_efficiencyMapReplicaArray.end()) {
                if (m_allowZeroSF) {
                    ATH_MSG_WARNING("Could not find what you are looking for in the efficiency map. The trigger you are looking for, year and mc are not consistent, or the trigger is unavailable in this data period. Returning efficiency = 0.");
                    eff = 0.;
                    return CorrectionCode::Ok;
                }

                else {
                    ATH_MSG_ERROR("Could not find what you are looking for in the efficiency map. The trigger you are looking for, year and mc are not consistent, or the trigger is unavailable in this data period. Please check how you set up the tool.");
                    return CorrectionCode::OutOfValidityRange;
                }
            }

            if (configuration.replicaIndex >= (int) cit->second.size()) {
                ATH_MSG_ERROR("MuonTriggerScaleFactors::getMuonEfficiency ; index for replicated histograms is out of range.");
                return CorrectionCode::OutOfValidityRange;
            }

            eff_h2 = cit->second[configuration.replicaIndex];
        } else { //Standard case, look into the usual eff map
            TH1_Ptr cit = getEfficiencyHistogram(trigger, configuration.isData, systematic, isBarrel);
            if (cit.get() == nullptr) {
                if (m_allowZeroSF) {
                    ATH_MSG_WARNING("Could not find what you are looking for in the efficiency map. The trigger you are looking for, year and mc are not consistent, or the trigger is unavailable in this data period. Returning efficiency = 0.");
                    eff = 0.;
                    return CorrectionCode::Ok;
                } else {
                    ATH_MSG_ERROR("Could not find what you are looking for in the efficiency map. The trigger you are looking for, year and mc are not consistent, or the trigger is unavailable in this data period. Please check how you set up the tool.");
                    return CorrectionCode::OutOfValidityRange;
                }
            }
            eff_h2 = cit;
        }

        double mu_phi_corr = mu_phi;
        if (mu_phi_corr < eff_h2->GetYaxis()->GetXmin()) mu_phi_corr += 2.0 * M_PI;
        if (mu_phi_corr > eff_h2->GetYaxis()->GetXmax()) mu_phi_corr -= 2.0 * M_PI;

        const int bin = eff_h2->FindFixBin(mu_eta, mu_phi_corr);
        const double efficiency = eff_h2->GetBinContent(bin);

        eff = efficiency;

        ATH_MSG_DEBUG("getMuonEfficiency [eta,phi,phi_corr]=[" << mu_eta << "," << mu_phi << "," << mu_phi_corr << "], ibin=" << bin << " eff=" << eff);

        return CorrectionCode::Ok;

    }

    CorrectionCode MuonTriggerScaleFactors::GetTriggerSF_dimu(Double_t& TriggerSF, TrigMuonEff::Configuration& configuration, const xAOD::MuonContainer& mucont, const std::string& trigger) const{

        if (mucont.size() != 2) {
            ATH_MSG_FATAL("MuonTriggerScaleFactors::GetTriggerSF;Currently dimuon trigger chains only implemented for events with exactly 2 muons.");
        }
        ATH_MSG_DEBUG("The trigger that you choose : " << trigger);

        Double_t eff_data = 0;
        Double_t eff_mc = 0;

        std::string data_err = "";
        std::string mc_err = "";

        if (appliedSystematics().matchSystematic(CP::SystematicVariation("MUON_EFF_TrigSystUncertainty", -1))) {
            data_err = "nominal";
            mc_err = "syst_up";
        } else if (appliedSystematics().matchSystematic(CP::SystematicVariation("MUON_EFF_TrigSystUncertainty", 1))) {
            data_err = "nominal";
            mc_err = "syst_down";
        } else if (appliedSystematics().matchSystematic(CP::SystematicVariation("MUON_EFF_TrigStatUncertainty", -1))) {
            data_err = "stat_down";
            mc_err = "nominal";
        } else if (appliedSystematics().matchSystematic(CP::SystematicVariation("MUON_EFF_TrigStatUncertainty", 1))) {
            data_err = "stat_up";
            mc_err = "nominal";
        } else {
            data_err = "nominal";
            mc_err = "nominal";
        }

        //Toys, if found, will overwrite the data hists with the sys generated with one toy
        //+++++++++++++
        //The best way is the use of filterByName with the 000MCTOY at the end. See:
        // if( !(appliedSystematics().filterByBaseName("MUON_EFF_Trig_MCTOY000")).empty()){//The following is a hack!!!
        //++++++++++THE FOLLOWING IS A PARTIAL HACK!!!
        if (!appliedSystematics().empty()) {
            configuration.replicaIndex = getReplica_index(appliedSystematics().begin()->basename(), trigger);
            if (configuration.replicaIndex != -1) data_err = "replicas";
        }

        configuration.isData = true;
        CorrectionCode result = getDimuonEfficiency(eff_data, configuration, mucont, trigger, data_err);
        if (result != CorrectionCode::Ok) return result;

        configuration.isData = false;
        configuration.replicaIndex = -1;
        result = getDimuonEfficiency(eff_mc, configuration, mucont, trigger, mc_err);
        if (result != CorrectionCode::Ok) return result;

        double event_SF = 1.;

        if (std::abs(1. - eff_mc) > 0.0001) {
            event_SF = eff_data / eff_mc;
        }

        TriggerSF = event_SF;
        return CorrectionCode::Ok;
    }

    CorrectionCode MuonTriggerScaleFactors::GetTriggerSF(Double_t& TriggerSF, TrigMuonEff::Configuration& configuration, const xAOD::MuonContainer& mucont, const std::string& trigger) const{
        Int_t threshold;
        CorrectionCode result = getThreshold(threshold, trigger);
        if (result != CorrectionCode::Ok) return result;

        double rate_not_fired_data = 1.;
        double rate_not_fired_mc = 1.;

        for (const auto mu : mucont) {

            double eff_data = 0., eff_mc = 0.;

            if (mu->pt() < threshold) {

                eff_data = 0.;
                eff_mc = 0.;

            } else {

                std::string muon_trigger_name = trigger;
                std::string data_err = "";
                std::string mc_err = "";

                // Pre-define uncertainty variations
                static const CP::SystematicVariation stat_up("MUON_EFF_TrigStatUncertainty", 1);
                static const CP::SystematicVariation stat_down("MUON_EFF_TrigStatUncertainty", -1);
                static const CP::SystematicVariation syst_up("MUON_EFF_TrigSystUncertainty", 1);
                static const CP::SystematicVariation syst_down("MUON_EFF_TrigSystUncertainty", -1);

                if (appliedSystematics().matchSystematic(syst_down)) {
                    data_err = "nominal";
                    mc_err = "syst_up";
                } else if (appliedSystematics().matchSystematic(syst_up)) {
                    data_err = "nominal";
                    mc_err = "syst_down";
                } else if (appliedSystematics().matchSystematic(stat_down)) {
                    data_err = "stat_down";
                    mc_err = "nominal";
                } else if (appliedSystematics().matchSystematic(stat_up)) {
                    data_err = "stat_up";
                    mc_err = "nominal";
                } else {
                    data_err = "nominal";
                    mc_err = "nominal";
                }

                //Toys, if found, will overwrite the data hists, on which toys for stat uncertainty have been generated
                //+++++++++++++
                //The best way is the use of filterByName with the 000MCTOY at the end. See:
                // if( !(appliedSystematics().filterByBaseName("MUON_EFF_Trig_MCTOY000")).empty()){//The following is a hack!!!
                //++++++++++The following is a hack!!!
                if (!appliedSystematics().empty()) {
                    configuration.replicaIndex = getReplica_index(appliedSystematics().begin()->basename(), trigger);
                    if (configuration.replicaIndex != -1) data_err = "replicas";
                }

                configuration.isData = true;
                CorrectionCode result_data = getMuonEfficiency(eff_data, configuration, *mu, muon_trigger_name, data_err);
                if (result_data != CorrectionCode::Ok) return result_data;
                configuration.isData = false;
                configuration.replicaIndex = -1;
                CorrectionCode result_mc = getMuonEfficiency(eff_mc, configuration, *mu, muon_trigger_name, mc_err);
                if (result_mc != CorrectionCode::Ok) return result_mc;
            }
            rate_not_fired_data *= (1. - eff_data);
            rate_not_fired_mc *= (1. - eff_mc);
        }

        double event_SF = 1.;
        if (1 - rate_not_fired_data == 0) event_SF = 0;
        if ((mucont.size()) and (std::abs(1. - rate_not_fired_mc) > 0.0001)) {

            event_SF = (1. - rate_not_fired_data) / (1. - rate_not_fired_mc);
        }
        TriggerSF = event_SF;

        return CorrectionCode::Ok;
    }

  CorrectionCode MuonTriggerScaleFactors::GetTriggerSF(Double_t& TriggerSF, TrigMuonEff::Configuration& configuration, const xAOD::Muon& mu, const std::string& trigger) const{
        Int_t threshold;
        CorrectionCode result = getThreshold(threshold, trigger);
        if (result != CorrectionCode::Ok)
	  return result;

	double eff_data = 0., eff_mc = 0.;

	if (mu.pt() < threshold) {
	  eff_data = 0.;
	  eff_mc = 0.;
	  TriggerSF = 1.;
	  return CorrectionCode::Ok;
	}

	std::string muon_trigger_name = trigger;
	std::string data_err = "";
	std::string mc_err = "";

	// Pre-define uncertainty variations
	static const CP::SystematicVariation stat_up("MUON_EFF_TrigStatUncertainty", 1);
	static const CP::SystematicVariation stat_down("MUON_EFF_TrigStatUncertainty", -1);
	static const CP::SystematicVariation syst_up("MUON_EFF_TrigSystUncertainty", 1);
	static const CP::SystematicVariation syst_down("MUON_EFF_TrigSystUncertainty", -1);
	
	if (appliedSystematics().matchSystematic(syst_down)) {
	  data_err = "nominal";
	  mc_err = "syst_up";
	} else if (appliedSystematics().matchSystematic(syst_up)) {
	  data_err = "nominal";
	  mc_err = "syst_down";
	} else if (appliedSystematics().matchSystematic(stat_down)) {
	  data_err = "stat_down";
	  mc_err = "nominal";
	} else if (appliedSystematics().matchSystematic(stat_up)) {
	  data_err = "stat_up";
	  mc_err = "nominal";
	} else {
	  data_err = "nominal";
	  mc_err = "nominal";
	}

	if (!appliedSystematics().empty()) {
	  configuration.replicaIndex = getReplica_index(appliedSystematics().begin()->basename(), trigger);
	  if (configuration.replicaIndex != -1) data_err = "replicas";
	}

	configuration.isData = true;
	CorrectionCode result_data = getMuonEfficiency(eff_data, configuration, mu, muon_trigger_name, data_err);
	if (result_data != CorrectionCode::Ok)
	  return result_data;
	configuration.isData = false;
	configuration.replicaIndex = -1;
	CorrectionCode result_mc = getMuonEfficiency(eff_mc, configuration, mu, muon_trigger_name, mc_err);
	if (result_mc != CorrectionCode::Ok)
	  return result_mc;
	if (eff_data == 0)
	  TriggerSF =  0;
        if (std::abs(eff_mc) > 0.0001)
	  TriggerSF = eff_data / eff_mc;
        return CorrectionCode::Ok;
  }
  
    CorrectionCode MuonTriggerScaleFactors::getDimuonEfficiency(Double_t& eff, const TrigMuonEff::Configuration& configuration, const xAOD::MuonContainer& mucont, const std::string& chain, const std::string& systematic) const{

        std::string trigger = getTriggerCorrespondingToDimuonTrigger(chain);
        Int_t threshold;
        CorrectionCode result = getThreshold(threshold, trigger);
        if (result != CorrectionCode::Ok) return result;

        xAOD::MuonContainer::const_iterator mu1 = mucont.begin();
        xAOD::MuonContainer::const_iterator mu2 = mucont.begin() + 1;

        Double_t eff1 = 0;
        if ((**mu1).pt() > threshold) {
            CorrectionCode result1 = getMuonEfficiency(eff1, configuration, (**mu1), trigger, systematic);
            if (result1 != CorrectionCode::Ok) return result1;
        }
        Double_t eff2 = 0;
        if ((**mu2).pt() > threshold) {
            CorrectionCode result2 = getMuonEfficiency(eff2, configuration, (**mu2), trigger, systematic);
            if (result2 != CorrectionCode::Ok) return result2;
        }

        eff = eff1 * eff2;
        return CorrectionCode::Ok;
    }

    std::string MuonTriggerScaleFactors::getTriggerCorrespondingToDimuonTrigger(const std::string& trigger) const {
        if (trigger.find("2mu10") != std::string::npos) return "HLT_mu10";
        if (trigger.find("2mu14") != std::string::npos) return "HLT_mu14";
        throw std::runtime_error("Unknown dimuon trigger");
    }

    CorrectionCode MuonTriggerScaleFactors::getThreshold(Int_t& threshold, const std::string& trigger) const{
        std::size_t index = trigger.find("HLT_mu");
        if (index != std::string::npos) {
            std::string rawNumber = trigger.substr(index + 6);
            if (!rawNumber.empty() && isdigit(rawNumber[0])) {
                std::stringstream(rawNumber) >> threshold;
                if (threshold < 10) threshold = 10000;
                else threshold = (threshold + 1) * 1000;
                return CorrectionCode::Ok;
            }
        }
        ATH_MSG_ERROR("MuonTriggerScaleFactors::getThreshold Could not extract threshold for trigger " << trigger);
        return CorrectionCode::Error;
    }

    unsigned int MuonTriggerScaleFactors::getYear(unsigned int run) const {
      if(m_forceYear != -1){
	return m_forceYear;
      }
      if (run <= 284484) return 2015;
      else if (run <= 311481) return 2016;
      else if (run <= 340453) return 2017;
      else if (run <= 364292) return 2018;
      else return 2022;
    }
  
    std::string MuonTriggerScaleFactors::getDataPeriod() const {
      return getDataPeriod(getRunNumber());
    }

    std::string MuonTriggerScaleFactors::getDataPeriod(unsigned int run) const {
        return getDataPeriod(run, getYear(run));
    }

    std::string MuonTriggerScaleFactors::getDataPeriod(unsigned int runNumber, unsigned year) const {
        if(!m_forcePeriod.empty())
            return m_forcePeriod;
        if (year == 2015) {
            if (runNumber >= 266904 && runNumber <= 272531) return "AC";
            else if (runNumber >= 276073 && runNumber <= 276954) return "D";
            else if (runNumber >= 278727 && runNumber <= 279928) return "E";
            else if (runNumber >= 279932 && runNumber <= 280422) return "F";
            else if (runNumber >= 280423 && runNumber <= 281075) return "G";
            else if (runNumber >= 281130 && runNumber <= 281411) return "H";
            else if (runNumber >= 281662 && runNumber <= 282482) return "I"; // special ALFA run
            else if (runNumber >= 282625 && runNumber <= 284484) return "J";
        }
        else if (year == 2016) {
            if (runNumber >= 296939 && runNumber <= 300287) return "A";
            else if (runNumber >= 300345 && runNumber <= 300908) return "B";
            else if (runNumber >= 301912 && runNumber <= 302393) return "C";
            else if (runNumber >= 302737 && runNumber <= 302872) return "D1D3";
            else if (runNumber >= 302919 && runNumber <= 303560) return "D4D8";
            else if (runNumber >= 303638 && runNumber <= 303892) return "E";
            else if (runNumber >= 303943 && runNumber <= 304494) return "F";
            else if (runNumber >= 305291 && runNumber <= 306714) return "G";
            else if (runNumber >= 307124 && runNumber <= 308084) return "I";
            else if (runNumber >= 309311 && runNumber <= 309759) return "K";
            else if (runNumber >= 310015 && runNumber <= 311481) return "L";
        }
        else if (year == 2017) {
            if (runNumber >= 324320 && runNumber <= 325558) return "A";
            else if (runNumber >= 325713 && runNumber <= 328393) return "B";
            else if (runNumber >= 329385 && runNumber <= 330470) return "C";
            else if (runNumber >= 330857 && runNumber <= 332304) return "D";
            else if (runNumber >= 332720 && runNumber <= 334779) return "E";
            else if (runNumber >= 334842 && runNumber <= 335290) return "F";
            else if (runNumber >= 336497 && runNumber <= 336782) return "H";
            else if (runNumber >= 336832 && runNumber <= 337833) return "I";
            else if (runNumber >= 338183 && runNumber <= 340453) return "K";
        }
        else if (year == 2018) {
            if (runNumber >= 348197 && runNumber <= 348836) return "A";
            else if (runNumber >= 348885 && runNumber <= 349533) return "B";
            else if (runNumber >= 349534 && runNumber <= 350220) return "C";
            else if (runNumber >= 350310 && runNumber <= 352107) return "D";
            else if (runNumber >= 352123 && runNumber <= 352137) return "E";
            else if (runNumber >= 352274 && runNumber <= 352514) return "F";
            else if (runNumber >= 354107 && runNumber <= 354494) return "G";
            else if (runNumber >= 354826 && runNumber <= 355224) return "H";
            else if (runNumber >= 355261 && runNumber <= 355273) return "I";
            else if (runNumber >= 355331 && runNumber <= 355468) return "J";
            else if (runNumber >= 355529 && runNumber <= 356259) return "K";
            else if (runNumber >= 357050 && runNumber <= 359171) return "L";
            else if (runNumber >= 359191 && runNumber <= 360414) return "M";
            else if (runNumber >= 361635 && runNumber <= 361696) return "N";
            else if (runNumber >= 361738 && runNumber <= 363400) return "O";
            else if (runNumber >= 363664 && runNumber <= 364292) return "Q";
        }
        else if (year == 2022) {
            if(runNumber >= 430536 && runNumber <= 432180) return "F";
            else if (runNumber >= 435816 && runNumber <= 439927) return "H";
            else if (runNumber >= 440407 && runNumber <= 440613) return "J";
        }
    
      ATH_MSG_FATAL("RunNumber: " << runNumber << " not known! Will stop the code to prevent using wrong SFs.");
      throw std::invalid_argument{""};
    }
  
    unsigned int MuonTriggerScaleFactors::getRunNumber() const {
        static const SG::AuxElement::ConstAccessor<unsigned int> acc_rnd("RandomRunNumber");
        SG::ReadHandle<xAOD::EventInfo> info(m_eventInfo);
        if (info.operator->()==nullptr) {
            ATH_MSG_FATAL("Could not retrieve the xAOD::EventInfo with name: " << m_eventInfo.key() << ". Exiting the code.");
            throw std::invalid_argument{""};
        }
        if (!info->eventType(xAOD::EventInfo::IS_SIMULATION)) {
            ATH_MSG_DEBUG("The current event is a data event. Return runNumber instead.");
            return info->runNumber();
        }
        if (!acc_rnd.isAvailable(*info)) {
            if(m_forceYear == -1 && m_forcePeriod == "")
            ATH_MSG_FATAL("Failed to find the RandomRunNumber decoration. Please call the apply() method from the PileupReweightingTool beforehand in order to get period dependent SFs");
            throw std::invalid_argument{""};
        } else if (acc_rnd(*info) == 0) {
            ATH_MSG_FATAL("Pile up tool has given runNumber 0. Exiting the code.");
            throw std::invalid_argument{""};
        }

        // standard behaviour for MC, get the random RunNumber
        return acc_rnd(*info);
    }

    TDirectory* MuonTriggerScaleFactors::getTemporaryDirectory(void) const {
        gROOT->cd();
        TDirectory* tempDir = 0;
        int counter = 0;
        while (not tempDir) {
            std::stringstream dirname;
            dirname << "MuonTriggerScaleFactorsTempDir_%i" << counter;
            if (gROOT->GetDirectory((dirname.str()).c_str())) {
                ++counter;
                continue;
            }
            tempDir = gROOT->mkdir((dirname.str()).c_str());
            if (not tempDir) {
                ATH_MSG_ERROR("getTemporaryDirectory::Temporary directory could not be created");
            }
        }
        return tempDir;
    }

    //=======================================================================
    //   Systematics Interface
    //=======================================================================
    bool MuonTriggerScaleFactors::isAffectedBySystematic(const CP::SystematicVariation& systematic) const {
        if (!systematic.empty()) {
            CP::SystematicSet sys = affectingSystematics();
            return sys.find(systematic) != sys.end();
        }
        return true;
    }

    /// returns: the list of all systematics this tool can be affected by
    CP::SystematicSet MuonTriggerScaleFactors::affectingSystematics() const {
        CP::SystematicSet mySysSet;

        mySysSet.insert(CP::SystematicVariation("MUON_EFF_TrigSystUncertainty", 1));
        mySysSet.insert(CP::SystematicVariation("MUON_EFF_TrigSystUncertainty", -1));

        //Consider full statUncertainty if  TOY replicas are not used
        if (m_replicaTriggerList.size() == 0) {
            mySysSet.insert(CP::SystematicVariation("MUON_EFF_TrigStatUncertainty", 1));
            mySysSet.insert(CP::SystematicVariation("MUON_EFF_TrigStatUncertainty", -1));
        } else {
            for (int i = 0; i < m_nReplicas; ++i) { //TOFIX Hack with just up variations! needs ASG reserved words for a clean handling//+++++++
                mySysSet.insert(CP::SystematicVariation(Form("MUON_EFF_Trig_MCTOY%03d", i), 1));
            }
        }

        return mySysSet;
    }

    // Register the systematics with the registry and add them to the recommended list
    StatusCode MuonTriggerScaleFactors::registerSystematics() {
        CP::SystematicRegistry& registry = CP::SystematicRegistry::getInstance();
        if (registry.registerSystematics(*this) != StatusCode::SUCCESS) {
            ATH_MSG_ERROR("Failed to add systematic to list of recommended systematics.");
            return StatusCode::FAILURE;
        }
        return StatusCode::SUCCESS;
    }

    /// returns: the list of all systematics this tool recommends to use
    CP::SystematicSet MuonTriggerScaleFactors::recommendedSystematics() const {
        return affectingSystematics();
    }

    StatusCode MuonTriggerScaleFactors::applySystematicVariation(const CP::SystematicSet& systConfig){
        // First, check if we already know this systematic configuration
        auto itr = m_systFilter.find(systConfig);

        // If it's a new input set, we need to filter it
        if (itr == m_systFilter.end()) {

            // New systematic. We need to parse it.
            static const CP::SystematicSet affectingSys = affectingSystematics();
            CP::SystematicSet filteredSys;
            if (!CP::SystematicSet::filterForAffectingSystematics(systConfig, affectingSys, filteredSys)) {
                ATH_MSG_ERROR("Unsupported combination of systematics passed to the tool!");
                return StatusCode::FAILURE;
            }

            // Insert filtered set into the map
            itr = m_systFilter.insert(std::make_pair(systConfig, filteredSys)).first;
        }

        CP::SystematicSet& mySysConf = itr->second;

        // Check to see if the set of variations tries to add in the uncertainty up and down. Since the errors
        // are symetric this would result in 0 and so should not be done.
        static const CP::SystematicVariation stat_up("MUON_EFF_TrigStatUncertainty", 1);
        static const CP::SystematicVariation stat_down("MUON_EFF_TrigStatUncertainty", -1);
        static const CP::SystematicVariation syst_up("MUON_EFF_TrigSystUncertainty", 1);
        static const CP::SystematicVariation syst_down("MUON_EFF_TrigSystUncertainty", -1);

        if ((mySysConf.matchSystematic(stat_up) && mySysConf.matchSystematic(stat_down)) || (mySysConf.matchSystematic(syst_up) && mySysConf.matchSystematic(syst_down))) {
            return StatusCode::FAILURE;
        }

        m_appliedSystematics = &mySysConf;
        return StatusCode::SUCCESS;
    }

} /* namespace CP */
