/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
 */

#include <IsolationSelection/Interp3D.h>
#include <IsolationSelection/IsolationConditionCombined.h>
#include <IsolationSelection/IsolationConditionFormula.h>
#include <IsolationSelection/IsolationConditionHist.h>
#include <IsolationSelection/IsolationSelectionTool.h>
#include <TF2.h>
#include <TFile.h>
#include <TH3.h>
#include <TObjString.h>
#include <TROOT.h>
#include <xAODPrimitives/IsolationType.h>

#include "PathResolver/PathResolver.h"

namespace CP {
    IsolationSelectionTool::IsolationSelectionTool(const std::string& name) : asg::AsgTool(name) {}
    const std::vector<std::unique_ptr<IsolationWP>>& IsolationSelectionTool::getMuonWPs() const { return m_muWPs; }
    const std::vector<std::unique_ptr<IsolationWP>>& IsolationSelectionTool::getElectronWPs() const { return m_elWPs; }
    const std::vector<std::unique_ptr<IsolationWP>>& IsolationSelectionTool::getPhotonWPs() const { return m_phWPs; }
    const std::vector<std::unique_ptr<IsolationWP>>& IsolationSelectionTool::getObjWPs() const { return m_objWPs; }
    IsolationSelectionTool::~IsolationSelectionTool() = default;

    StatusCode IsolationSelectionTool::initialize() {
        /// Greet the user:
        ATH_MSG_INFO("Initialising...");

        if (!m_calibFileName.empty()) {
            std::string filename = PathResolverFindCalibFile(m_calibFileName);

            ATH_MSG_INFO("Reading input file " << m_calibFileName << " from " << filename);
            m_calibFile = std::make_unique<TFile>(filename.c_str(), "READ");

            TObjString* versionInfo{nullptr};
            m_calibFile->GetObject("VersionInfo", versionInfo);
            if (versionInfo)
                ATH_MSG_INFO("VersionInfo:" << versionInfo->String());
            else
                ATH_MSG_WARNING("VersionInfo of input file (" << filename << ") is missing.");
        }

        if (m_doInterpE || m_doInterpM) {
            // special setting for electrons
            // do not apply interpolation in crack vicinity for topoetcone
            std::vector<std::pair<double, double>> rangeEtaNoInt;
            std::pair<double, double> apair(1.26, 1.665);
            rangeEtaNoInt.push_back(apair);
            // do not apply interpolation between Z defined and J/Psi defined cuts (pT < > 15 GeV/c) for both calo and track iso
            std::vector<std::pair<double, double>> rangePtNoInt;
            apair.first = 12.5;
            apair.second = 17.5;
            rangePtNoInt.push_back(apair);
            std::map<std::string, Interp3D::VetoInterp> amap;
            Interp3D::VetoInterp veto;
            veto.xRange = rangePtNoInt;
            veto.yRange = std::vector<std::pair<double, double>>();
            amap.insert(std::make_pair(std::string("el_cutValues_ptvarcone20"), veto));
            veto.yRange = rangeEtaNoInt;
            amap.insert(std::make_pair(std::string("el_cutValues_topoetcone20"), veto));
            m_Interp = std::make_unique<Interp3D>(amap);
            m_Interp->debug(false);
        }

        /// setup working points
        if (m_phWPname != "Undefined") ATH_CHECK(addPhotonWP(m_phWPname));
        if (m_elWPname != "Undefined") ATH_CHECK(addElectronWP(m_elWPname));
        if (m_muWPname != "Undefined") ATH_CHECK(addMuonWP(m_muWPname));
        for (const std::string& c : m_muWPvec) ATH_CHECK(addMuonWP(c));
        for (const std::string& c : m_elWPvec) ATH_CHECK(addElectronWP(c));
        for (const std::string& c : m_phWPvec) ATH_CHECK(addPhotonWP(c));

        m_calibFile.reset();
        ATH_CHECK(m_isoDecors.initialize());
        /// Return gracefully:
        return StatusCode::SUCCESS;
    }
    void IsolationSelectionTool::addDependencies(const std::string& container, const IsolationWP& wp) {
        if (container.empty()) return;
        for (const std::unique_ptr<IsolationCondition>& cond : wp.conditions()) {
            for (unsigned int acc = 0; acc < cond->num_types(); ++acc) {
                m_isoDecors.emplace_back(container + "." + SG::AuxTypeRegistry::instance().getName(cond->accessor(acc).auxid()));
            }
        }
    }

    StatusCode IsolationSelectionTool::setIParticleCutsFrom(xAOD::Type::ObjectType ObjType) {
        if (ObjType == xAOD::Type::Electron) {
            m_iparAcceptInfo = &m_electronAccept;
            m_iparWPs = &m_elWPs;
        } else if (ObjType == xAOD::Type::Muon) {
            m_iparAcceptInfo = &m_muonAccept;
            m_iparWPs = &m_muWPs;
        } else if (ObjType == xAOD::Type::Photon) {
            m_iparAcceptInfo = &m_photonAccept;
            m_iparWPs = &m_phWPs;
        } else {
            return StatusCode::FAILURE;
        }

        return StatusCode::SUCCESS;
    }

    StatusCode IsolationSelectionTool::addCutToWP(IsolationWP* wp, std::string key, const xAOD::Iso::IsolationType t,
                                                  const std::string expression, const xAOD::Iso::IsolationType isoCutRemap) {
        if (!m_calibFile) {
            ATH_MSG_ERROR("Calibration File (" << m_calibFileName << ") is missing.");
            return StatusCode::FAILURE;
        }

        std::string varname(xAOD::Iso::toCString(isoCutRemap));
        key += varname;

        TH3F* calibHisto{nullptr};
        m_calibFile->GetObject(key.c_str(), calibHisto);
        if (!calibHisto) {
            ATH_MSG_FATAL(" Failed to load " << key << " from " << m_calibFile->GetName());
            return StatusCode::FAILURE;
        }
        calibHisto->SetDirectory(nullptr);
        std::unique_ptr<TH3F> histogram(calibHisto);
        std::unique_ptr<IsolationConditionHist> ich =
            std::make_unique<IsolationConditionHist>(varname, t, expression, std::move(histogram));
        if ((m_doInterpM && key.find("Muon") != std::string::npos) || (m_doInterpE && key.find("Electron") != std::string::npos))
            ich->setInterp(m_Interp);
        wp->addCut(std::move(ich));

        return StatusCode::SUCCESS;
    }

    StatusCode IsolationSelectionTool::addCutToWP(IsolationWP* wp, std::string key, const xAOD::Iso::IsolationType t,
                                                  const std::string expression) {
        return addCutToWP(wp, key, t, expression, t);
    }

    StatusCode IsolationSelectionTool::addMuonWP(std::string muWPname) {
        std::unique_ptr<IsolationWP> wp = std::make_unique<IsolationWP>(muWPname);
        if (muWPname == "HighPtTrackOnly") {
            wp->addCut(std::make_unique<IsolationConditionFormula>(
                "ptcone20_Tight_1p25", xAOD::Iso::ptcone20_Nonprompt_All_MaxWeightTTVA_pt1000, "1.25E03"));  // units are MeV!
        } else if (muWPname == "TightTrackOnly_FixedRad") {
            wp->addCut(std::make_unique<IsolationConditionFormula>(
                "MuonFixedCutHighMuTrackOnly_lowPt", xAOD::Iso::ptvarcone30_Nonprompt_All_MaxWeightTTVA_pt1000, "0.06*(x>50e3?1e9:x)"));
            wp->addCut(std::make_unique<IsolationConditionFormula>(
                "MuonFixedCutHighMuTrackOnly_highPt", xAOD::Iso::ptcone20_Nonprompt_All_MaxWeightTTVA_pt1000, "0.06*(x>50e3?x:1e9)"));
        } else if (muWPname == "Tight_FixedRad") {
            wp->addCut(std::make_unique<IsolationConditionFormula>(
                "MuonFixedCutHighMuTight_track_lowPt", xAOD::Iso::ptvarcone30_Nonprompt_All_MaxWeightTTVA_pt1000, "0.04*(x>50e3?1e9:x)"));
            wp->addCut(std::make_unique<IsolationConditionFormula>(
                "MuonFixedCutHighMuTight_track_highPt", xAOD::Iso::ptcone20_Nonprompt_All_MaxWeightTTVA_pt1000, "0.04*(x>50e3?x:1e9)"));
            wp->addCut(std::make_unique<IsolationConditionFormula>("MuonFixedCutHighMuTight_calo", xAOD::Iso::topoetcone20, "0.15*x"));
        } else if (muWPname == "Loose_FixedRad") {
            wp->addCut(std::make_unique<IsolationConditionFormula>(
                "MuonFixedCutHighMuLoose_track_lowPt", xAOD::Iso::ptvarcone30_Nonprompt_All_MaxWeightTTVA_pt1000, "0.15*(x>50e3?1e9:x)"));
            wp->addCut(std::make_unique<IsolationConditionFormula>(
                "MuonFixedCutHighMuLoose_track_highPt", xAOD::Iso::ptcone20_Nonprompt_All_MaxWeightTTVA_pt1000, "0.15*(x>50e3?x:1e9)"));
            wp->addCut(std::make_unique<IsolationConditionFormula>("MuonFixedCutHighMuLoose_calo", xAOD::Iso::topoetcone20, "0.30*x"));
        } else if (muWPname == "TightTrackOnly_VarRad") {
            wp->addCut(std::make_unique<IsolationConditionFormula>("MuonFixedCutHighMuTrackOnly",
                                                                   xAOD::Iso::ptvarcone30_Nonprompt_All_MaxWeightTTVA_pt1000, "0.06*x"));
        } else if (muWPname == "Tight_VarRad") {
            wp->addCut(std::make_unique<IsolationConditionFormula>("MuonFixedCutHighMuTight_track",
                                                                   xAOD::Iso::ptvarcone30_Nonprompt_All_MaxWeightTTVA_pt1000, "0.04*x"));
            wp->addCut(std::make_unique<IsolationConditionFormula>("MuonFixedCutHighMuTight_calo", xAOD::Iso::topoetcone20, "0.15*x"));
        } else if (muWPname == "Loose_VarRad") {
            wp->addCut(std::make_unique<IsolationConditionFormula>("MuonFixedCutHighMuLoose_track",
                                                                   xAOD::Iso::ptvarcone30_Nonprompt_All_MaxWeightTTVA_pt1000, "0.15*x"));
            wp->addCut(std::make_unique<IsolationConditionFormula>("MuonFixedCutHighMuLoose_calo", xAOD::Iso::topoetcone20, "0.30*x"));
        } else if (muWPname == "PflowTight_FixedRad") {
            std::vector<xAOD::Iso::IsolationType> isoTypesHighPt{xAOD::Iso::ptcone20_Nonprompt_All_MaxWeightTTVA_pt500,
                                                                 xAOD::Iso::neflowisol20};
            std::vector<xAOD::Iso::IsolationType> isoTypesLowPt{xAOD::Iso::ptvarcone30_Nonprompt_All_MaxWeightTTVA_pt500,
                                                                xAOD::Iso::neflowisol20};
            wp->addCut(std::make_unique<IsolationConditionCombined>("MuonPFlowTightLowPt", isoTypesLowPt,
                                                                    std::make_unique<TF2>("pflowTFunctionLowPt", "fabs(x)+0.4*(y>0?y:0)"),
                                                                    "0.045*(x>50e3?1e9:x)"));
            wp->addCut(std::make_unique<IsolationConditionCombined>("MuonPFlowTightHighPt", isoTypesHighPt,
                                                                    std::make_unique<TF2>("pflowTFunctionHighPt", "fabs(x)+0.4*(y>0?y:0)"),
                                                                    "0.045*(x>50e3?x:1e9)"));
        } else if (muWPname == "PflowTight_VarRad") {
            std::vector<xAOD::Iso::IsolationType> isoTypes{xAOD::Iso::ptvarcone30_Nonprompt_All_MaxWeightTTVA_pt500,
                                                           xAOD::Iso::neflowisol20};
            wp->addCut(std::make_unique<IsolationConditionCombined>(
                "MuonPFlowTight", isoTypes, std::make_unique<TF2>("pflowTFunction", "fabs(x)+0.4*(y>0?y:0)"), "0.045*x"));
        } else if (muWPname == "PflowLoose_FixedRad") {
            std::vector<xAOD::Iso::IsolationType> isoTypesHighPt{xAOD::Iso::ptcone20_Nonprompt_All_MaxWeightTTVA_pt500,
                                                                 xAOD::Iso::neflowisol20};
            std::vector<xAOD::Iso::IsolationType> isoTypesLowPt{xAOD::Iso::ptvarcone30_Nonprompt_All_MaxWeightTTVA_pt500,
                                                                xAOD::Iso::neflowisol20};
            wp->addCut(std::make_unique<IsolationConditionCombined>("MuonPFlowLooseLowPt", isoTypesLowPt,
                                                                    std::make_unique<TF2>("pflowLFunctionLowPt", "fabs(x)+0.4*(y>0?y:0)"),
                                                                    "0.16*(x>50e3?1e9:x)"));
            wp->addCut(std::make_unique<IsolationConditionCombined>("MuonPFlowLooseHighPt", isoTypesHighPt,
                                                                    std::make_unique<TF2>("pflowLFunctionHighPt", "fabs(x)+0.4*(y>0?y:0)"),
                                                                    "0.16*(x>50e3?x:1e9)"));
        } else if (muWPname == "PflowLoose_VarRad") {
            std::vector<xAOD::Iso::IsolationType> isoTypes{xAOD::Iso::ptvarcone30_Nonprompt_All_MaxWeightTTVA_pt500,
                                                           xAOD::Iso::neflowisol20};
            wp->addCut(std::make_unique<IsolationConditionCombined>(
                "MuonPFlowLoose", isoTypes, std::make_unique<TF2>("pflowTFunction", "fabs(x)+0.4*(y>0?y:0)"), "0.16*x"));
        } else {
            ATH_MSG_ERROR("Unknown muon isolation WP: " << muWPname);
            return StatusCode::FAILURE;
        }
        m_muonAccept.addCut(wp->name(), wp->name());
        addDependencies(m_inMuonContainer, *wp);
        m_muWPs.push_back(std::move(wp));        
        return StatusCode::SUCCESS;
    }

    StatusCode IsolationSelectionTool::addPhotonWP(std::string phWPname) {
        std::unique_ptr<IsolationWP> wp = std::make_unique<IsolationWP>(phWPname);
        if (phWPname == "TightCaloOnly") {
            wp->addCut(std::make_unique<IsolationConditionFormula>("PhFixedCut_calo40", xAOD::Iso::topoetcone40, "0.022*x+2450"));
        } else if (phWPname == "FixedCutTight") {
            wp->addCut(std::make_unique<IsolationConditionFormula>("PhFixedCut_calo40", xAOD::Iso::topoetcone40, "0.022*x+2450"));
            wp->addCut(std::make_unique<IsolationConditionFormula>("PhFixedCut_track20", xAOD::Iso::ptcone20, "0.05*x"));
        } else if (phWPname == "FixedCutLoose") {
            wp->addCut(std::make_unique<IsolationConditionFormula>("PhFixedCut_calo20", xAOD::Iso::topoetcone20, "0.065*x"));
            wp->addCut(std::make_unique<IsolationConditionFormula>("PhFixedCut_track20", xAOD::Iso::ptcone20, "0.05*x"));
        } else if (phWPname == "Tight") {
            wp->addCut(std::make_unique<IsolationConditionFormula>("PhFixedCut_calo40", xAOD::Iso::topoetcone40, "0.022*x+2450"));
            wp->addCut(std::make_unique<IsolationConditionFormula>("PhFixedCut_Tighttrack20",
                                                                   xAOD::Iso::ptcone20_Nonprompt_All_MaxWeightTTVA_pt1000, "0.05*x"));
        } else if (phWPname == "Loose") {
            wp->addCut(std::make_unique<IsolationConditionFormula>("PhFixedCut_calo20", xAOD::Iso::topoetcone20, "0.065*x"));
            wp->addCut(std::make_unique<IsolationConditionFormula>("PhFixedCut_Tighttrack20",
                                                                   xAOD::Iso::ptcone20_Nonprompt_All_MaxWeightTTVA_pt1000, "0.05*x"));
        } else {
            ATH_MSG_ERROR("Unknown photon isolation WP: " << phWPname);
            return StatusCode::FAILURE;
        }

        m_photonAccept.addCut(wp->name(), wp->name());
        addDependencies(m_inPhotContainer, *wp);
        m_phWPs.push_back(std::move(wp));
       
        // Return gracefully:
        return StatusCode::SUCCESS;
    }

    StatusCode IsolationSelectionTool::addElectronWP(std::string elWPname) {
        std::unique_ptr<IsolationWP> wp = std::make_unique<IsolationWP>(elWPname);

        if (elWPname == "HighPtCaloOnly") {
            wp->addCut(std::make_unique<IsolationConditionFormula>("FCHighPtCaloOnly_calo", xAOD::Iso::topoetcone20,
                                                                   "std::max(0.015*x,3.5E3)"));  // units are MeV!
        } else if (elWPname == "Tight_VarRad") {
            wp->addCut(std::make_unique<IsolationConditionFormula>(
                "ElecTight_track", xAOD::Iso::ptvarcone30_Nonprompt_All_MaxWeightTTVALooseCone_pt1000, "0.06*x"));
            wp->addCut(std::make_unique<IsolationConditionFormula>("ElecTight_calo", xAOD::Iso::topoetcone20, "0.06*x"));
        } else if (elWPname == "Loose_VarRad") {
            wp->addCut(std::make_unique<IsolationConditionFormula>(
                "ElecLoose_track", xAOD::Iso::ptvarcone30_Nonprompt_All_MaxWeightTTVALooseCone_pt1000, "0.15*x"));
            wp->addCut(std::make_unique<IsolationConditionFormula>("ElecLoose_calo", xAOD::Iso::topoetcone20, "0.20*x"));
        } else if (elWPname == "TightTrackOnly_VarRad") {
            wp->addCut(std::make_unique<IsolationConditionFormula>(
                "ElecTightTrackOnly", xAOD::Iso::ptvarcone30_Nonprompt_All_MaxWeightTTVALooseCone_pt1000, "0.06*x"));
        } else if (elWPname == "TightTrackOnly_FixedRad") {
            wp->addCut(std::make_unique<IsolationConditionFormula>(
                "ElecTightTrackOnly_lowPt", xAOD::Iso::ptvarcone30_Nonprompt_All_MaxWeightTTVALooseCone_pt1000, "0.06*(x>50e3?1e9:x)"));
            wp->addCut(std::make_unique<IsolationConditionFormula>(
                "ElecTightTrackOnly_highPt", xAOD::Iso::ptcone20_Nonprompt_All_MaxWeightTTVALooseCone_pt1000, "0.06*(x>50e3?x:1e9)"));
        } else if (elWPname == "PflowTight_FixedRad") {
            std::vector<xAOD::Iso::IsolationType> isoTypesHighPt{xAOD::Iso::ptcone20_Nonprompt_All_MaxWeightTTVALooseCone_pt500,
                                                                 xAOD::Iso::neflowisol20};
            std::vector<xAOD::Iso::IsolationType> isoTypesLowPt{xAOD::Iso::ptvarcone30_Nonprompt_All_MaxWeightTTVALooseCone_pt500,
                                                                xAOD::Iso::neflowisol20};
            wp->addCut(std::make_unique<IsolationConditionCombined>("ElecPFlowTightLowPt", isoTypesLowPt,
                                                                    std::make_unique<TF2>("pflowTFunctionLowPt", "fabs(x)+0.4*(y>0?y:0)"),
                                                                    "0.045*(x>50e3?1e9:x)"));
            wp->addCut(std::make_unique<IsolationConditionCombined>("ElecPFlowTightHighPt", isoTypesHighPt,
                                                                    std::make_unique<TF2>("pflowTFunctionHighPt", "fabs(x)+0.4*(y>0?y:0)"),
                                                                    "0.045*(x>50e3?x:1e9)"));
        } else if (elWPname == "PflowTight") {
            std::vector<xAOD::Iso::IsolationType> isoTypes{xAOD::Iso::ptvarcone30_Nonprompt_All_MaxWeightTTVALooseCone_pt500,
                                                           xAOD::Iso::neflowisol20};
            wp->addCut(std::make_unique<IsolationConditionCombined>(
                "ElecPFlowTight", isoTypes, std::make_unique<TF2>("pflowLFunction", "fabs(x)+0.4*(y>0?y:0)"), "0.045*x"));
        } else if (elWPname == "PflowLoose_FixedRad") {
            std::vector<xAOD::Iso::IsolationType> isoTypesHighPt{xAOD::Iso::ptcone20_Nonprompt_All_MaxWeightTTVALooseCone_pt500,
                                                                 xAOD::Iso::neflowisol20};
            std::vector<xAOD::Iso::IsolationType> isoTypesLowPt{xAOD::Iso::ptvarcone30_Nonprompt_All_MaxWeightTTVALooseCone_pt500,
                                                                xAOD::Iso::neflowisol20};
            wp->addCut(std::make_unique<IsolationConditionCombined>("ElecPFlowLooseLowPt", isoTypesLowPt,
                                                                    std::make_unique<TF2>("pflowLFunctionLowPt", "fabs(x)+0.4*(y>0?y:0)"),
                                                                    "0.16*(x>50e3?1e9:x)"));
            wp->addCut(std::make_unique<IsolationConditionCombined>("ElecPFlowLooseHighPt", isoTypesHighPt,
                                                                    std::make_unique<TF2>("pflowLFunctionHighPt", "fabs(x)+0.4*(y>0?y:0)"),
                                                                    "0.16*(x>50e3?x:1e9)"));
        } else if (elWPname == "PflowLoose") {
            std::vector<xAOD::Iso::IsolationType> isoTypes{xAOD::Iso::ptvarcone30_Nonprompt_All_MaxWeightTTVALooseCone_pt500,
                                                           xAOD::Iso::neflowisol20};
            wp->addCut(std::make_unique<IsolationConditionCombined>(
                "ElecPFlowLoose", isoTypes, std::make_unique<TF2>("pflowLFunction", "fabs(x)+0.4*(y>0?y:0)"), "0.16*x"));
        } else {
            ATH_MSG_ERROR("Unknown electron isolation WP: " << elWPname);
            return StatusCode::FAILURE;
        }

        m_electronAccept.addCut(wp->name(), wp->name());
        addDependencies(m_inElecContainer, *wp);
        m_elWPs.push_back(std::move(wp));
        
        // Return gracefully:
        return StatusCode::SUCCESS;
    }

    StatusCode IsolationSelectionTool::addUserDefinedWP(std::string WPname, xAOD::Type::ObjectType ObjType,
                                                        std::vector<std::pair<xAOD::Iso::IsolationType, std::string>>& cuts,
                                                        std::string key, IsoWPType type) {
        std::vector<std::unique_ptr<IsolationWP>>* wps(nullptr);
        asg::AcceptInfo* ac = nullptr;
        if (ObjType == xAOD::Type::Electron) {
            if (key == "") key = m_elWPKey;
            wps = &m_elWPs;
            ac = &m_electronAccept;
        } else if (ObjType == xAOD::Type::Muon) {
            if (key == "") key = m_muWPKey;
            wps = &m_muWPs;
            ac = &m_muonAccept;
        } else if (ObjType == xAOD::Type::Photon) {
            if (key == "") key = m_phWPKey;
            wps = &m_phWPs;
            ac = &m_photonAccept;
        } else if (ObjType == xAOD::Type::Other) {
            if (key == "") return StatusCode::FAILURE;
            wps = &m_objWPs;
            ac = &m_objAccept;
        } else {
            return StatusCode::FAILURE;
        }

        std::unique_ptr<IsolationWP> wp = std::make_unique<IsolationWP>(WPname);
        if (type == Efficiency) {
            for (auto& c : cuts) ATH_CHECK(addCutToWP(wp.get(), key, c.first, c.second));
        } else if (type == Cut) {
            for (auto& c : cuts) wp->addCut(std::make_unique<IsolationConditionFormula>(xAOD::Iso::toCString(c.first), c.first, c.second));
        } else {
            ATH_MSG_ERROR("Unknown isolation WP type -- should not happen.");
            return StatusCode::FAILURE;
        }

        ac->addCut(wp->name(), wp->name());
        wps->push_back(std::move(wp));
        return StatusCode::SUCCESS;
    }

    StatusCode IsolationSelectionTool::addWP(std::string WP, xAOD::Type::ObjectType ObjType) {
        if (ObjType == xAOD::Type::Electron) {
            return addElectronWP(WP);
        } else if (ObjType == xAOD::Type::Muon) {
            return addMuonWP(WP);
        } else if (ObjType == xAOD::Type::Photon) {
            return addPhotonWP(WP);
        }

        return StatusCode::FAILURE;
    }
    StatusCode IsolationSelectionTool::addWP(std::unique_ptr<IsolationWP> wp, xAOD::Type::ObjectType ObjType) {
        if (ObjType == xAOD::Type::Electron) {
            m_electronAccept.addCut(wp->name(), wp->name());
            m_elWPs.push_back(std::move(wp));
        } else if (ObjType == xAOD::Type::Muon) {
            m_muonAccept.addCut(wp->name(), wp->name());
            m_muWPs.push_back(std::move(wp));
    
        } else if (ObjType == xAOD::Type::Photon) {
            m_photonAccept.addCut(wp->name(), wp->name());
            m_phWPs.push_back(std::move(wp));
    
        } else if (ObjType == xAOD::Type::Other) {
            m_objAccept.addCut(wp->name(), wp->name());
            m_objWPs.push_back(std::move(wp));    
        } else {
            return StatusCode::FAILURE;
        }

        return StatusCode::SUCCESS;
    }
    template <typename T>
    void IsolationSelectionTool::evaluateWP(const T& x, const std::vector<std::unique_ptr<IsolationWP>>& WP, asg::AcceptData& accept) const {
        accept.clear();
        for (const std::unique_ptr<IsolationWP>& i : WP) {
            if (i->accept(x)) accept.setCutResult(i->name(), true);
        }
    }
    asg::AcceptData IsolationSelectionTool::accept(const xAOD::Photon& x) const {
        asg::AcceptData accept(&m_photonAccept);
        evaluateWP(x, m_phWPs, accept);
        return accept;
    }

    asg::AcceptData IsolationSelectionTool::accept(const xAOD::Electron& x) const {
        asg::AcceptData accept(&m_electronAccept);
        evaluateWP(x, m_elWPs, accept);
        return accept;
    }

    asg::AcceptData IsolationSelectionTool::accept(const xAOD::Muon& x) const {
        asg::AcceptData accept(&m_muonAccept);
        evaluateWP(x, m_muWPs, accept);
        return accept;
    }

    asg::AcceptData IsolationSelectionTool::accept(const xAOD::IParticle& x) const {
        if (x.type() == xAOD::Type::Electron) {
            asg::AcceptData accept(&m_electronAccept);
            evaluateWP(x, m_elWPs, accept);
            return accept;
        } else if (x.type() == xAOD::Type::Muon) {
            asg::AcceptData accept(&m_muonAccept);
            evaluateWP(x, m_muWPs, accept);
            return accept;
        } else if (x.type() == xAOD::Type::Photon) {
            asg::AcceptData accept(&m_photonAccept);
            evaluateWP(x, m_phWPs, accept);
            return accept;
        }

        else if (m_iparAcceptInfo && m_iparWPs) {
            asg::AcceptData accept(m_iparAcceptInfo);
            evaluateWP(x, *m_iparWPs, accept);
            return accept;
        }
        ATH_MSG_ERROR("Someting here makes really no  sense");
        return asg::AcceptData(&m_objAccept);
    }

    asg::AcceptData IsolationSelectionTool::accept(const strObj& x) const {
        if (x.type == xAOD::Type::Electron) {
            asg::AcceptData accept(&m_electronAccept);
            evaluateWP(x, m_elWPs, accept);
            return accept;
        } else if (x.type == xAOD::Type::Muon) {
            asg::AcceptData accept(&m_muonAccept);
            evaluateWP(x, m_muWPs, accept);
            return accept;
        } else if (x.type == xAOD::Type::Photon) {
            asg::AcceptData accept(&m_photonAccept);
            evaluateWP(x, m_phWPs, accept);
            return accept;
        } else {
            asg::AcceptData accept(&m_objAccept);
            evaluateWP(x, m_objWPs, accept);
            return accept;
        }
        return asg::AcceptData(&m_objAccept);
    }

    const asg::AcceptInfo& IsolationSelectionTool::getPhotonAcceptInfo() const { return m_photonAccept; }

    const asg::AcceptInfo& IsolationSelectionTool::getElectronAcceptInfo() const { return m_electronAccept; }

    const asg::AcceptInfo& IsolationSelectionTool::getMuonAcceptInfo() const { return m_muonAccept; }
    const asg::AcceptInfo& IsolationSelectionTool::getObjAcceptInfo() const { return m_objAccept; }

}  // namespace CP
