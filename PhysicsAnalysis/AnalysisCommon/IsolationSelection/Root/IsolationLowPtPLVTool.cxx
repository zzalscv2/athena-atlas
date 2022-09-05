/*
 Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
 */

#include <IsolationSelection/IsolationLowPtPLVTool.h>
#include <xAODBase/IParticleHelpers.h>
#include <xAODBase/ObjectType.h>

// Tools includes:
#include <cmath>

#include "PathResolver/PathResolver.h"
#include "TMVA/MethodBDT.h"
#include "TMVA/Reader.h"
#include "TMVA/Tools.h"

namespace CP {
    // Accessors for input variables
    static const ShortAccessor s_acc_TrackJetNTrack("PromptLeptonInput_TrackJetNTrack");
    static const FloatAccessor s_acc_DRlj("PromptLeptonInput_DRlj");
    static const FloatAccessor s_acc_PtRel("PromptLeptonInput_PtRel");
    static const FloatAccessor s_acc_PtFrac("PromptLeptonInput_PtFrac");
    static const FloatAccessor s_acc_topoetcone20("topoetcone20");
    static const FloatAccessor s_acc_ptvarcone20("ptvarcone20");
    static const FloatAccessor s_acc_ptvarcone30("ptvarcone30");
    static const FloatDecorator s_dec_iso_PLT("LowPtPLV");

    constexpr int N_VARIABLES = 6;

    IsolationLowPtPLVTool::IsolationLowPtPLVTool(const std::string& toolName) : asg::AsgTool(toolName) {}

    StatusCode IsolationLowPtPLVTool::initialize() {
        TMVA::Tools::Instance();

        std::string fullPathToFile_Muon = PathResolverFindCalibFile(m_muonCalibFile);
        std::string fullPathToFile_Elec = PathResolverFindCalibFile(m_elecCalibFile);

        std::vector<std::string> BDT_vars_Muon{"TrackJetNTrack", "PtRel", "PtFrac", "DRlj", "TopoEtCone20Rel", "PtVarCone30Rel"};

        std::vector<std::string> BDT_vars_Elec{"TrackJetNTrack", "PtRel", "PtFrac", "DRlj", "TopoEtCone20Rel", "PtVarCone20Rel"};
        m_TMVAReader_Muon = std::make_unique<TMVA::Reader>(BDT_vars_Muon, "!Silent:Color");
        m_TMVAReader_Elec = std::make_unique<TMVA::Reader>(BDT_vars_Elec, "!Silent:Color");

        if (fullPathToFile_Muon.empty()) {
            ATH_MSG_ERROR("Error! No xml file found for Muon LowPtPLV");
            return StatusCode::FAILURE;
        }
        m_TMVAReader_Muon->BookMVA(m_muonMethodName.value(), fullPathToFile_Muon);
        TMVA::MethodBDT* method_Muon_bdt = dynamic_cast<TMVA::MethodBDT*>(m_TMVAReader_Muon->FindMVA(m_muonMethodName.value()));
        if (!method_Muon_bdt) {
            ATH_MSG_ERROR("Error! No method found for Muon LowPtPLV");
            return StatusCode::FAILURE;
        }

        if (fullPathToFile_Elec.empty()) {
            ATH_MSG_ERROR("Error! No xml file found for Electron LowPtPLV");
            return StatusCode::FAILURE;
        }
        m_TMVAReader_Elec->BookMVA(m_elecMethodName.value(), fullPathToFile_Elec);
        TMVA::MethodBDT* method_Elec_bdt = dynamic_cast<TMVA::MethodBDT*>(m_TMVAReader_Elec->FindMVA(m_elecMethodName.value()));
        if (!method_Elec_bdt) {
            ATH_MSG_ERROR("Error! No method found for Electron LowPtPLV");
            return StatusCode::FAILURE;
        }

        ATH_MSG_INFO("Initialized IsolationLowPtPLVTool");
        return StatusCode::SUCCESS;
    }

    StatusCode IsolationLowPtPLVTool::augmentPLV(const xAOD::IParticle& Particle) {
        // Check if input variables exist
        bool inputvar_missing = false;
        if (!s_acc_TrackJetNTrack.isAvailable(Particle)) {
            ATH_MSG_ERROR("TrackJetNTrack not available");
            inputvar_missing = true;
        }

        if (!s_acc_DRlj.isAvailable(Particle)) {
            ATH_MSG_ERROR("DRlj not available");
            inputvar_missing = true;
        }

        if (!s_acc_PtRel.isAvailable(Particle)) {
            ATH_MSG_ERROR("PtRel not available");
            inputvar_missing = true;
        }

        if (!s_acc_PtFrac.isAvailable(Particle)) {
            ATH_MSG_ERROR("PtFrac not available");
            inputvar_missing = true;
        }

        if (!s_acc_topoetcone20.isAvailable(Particle)) {
            ATH_MSG_ERROR("topoetcone20 not available");
            inputvar_missing = true;
        }

        if (Particle.type() == xAOD::Type::ObjectType::Electron && !s_acc_ptvarcone20.isAvailable(Particle)) {
            ATH_MSG_ERROR("ptvarcone20 not available");
            inputvar_missing = true;
        }

        if (Particle.type() == xAOD::Type::ObjectType::Muon && !s_acc_ptvarcone30.isAvailable(Particle)) {
            ATH_MSG_ERROR("ptvarcone30 not available");
            inputvar_missing = true;
        }

        if (inputvar_missing) {
            ATH_MSG_ERROR("input variable(s) missing, augmenting fixed value 1.1");
            s_dec_iso_PLT(Particle) = 1.1;
            return StatusCode::FAILURE;
        }

        short TrackJetNTrack = s_acc_TrackJetNTrack(Particle);
        float DRlj = s_acc_DRlj(Particle);
        float PtRel = s_acc_PtRel(Particle);
        float PtFrac = s_acc_PtFrac(Particle);
        float topoetcone20 = s_acc_topoetcone20(Particle);
        float ptvarcone30 = 0;
        float ptvarcone20 = 0;

        float pt = Particle.pt();
        float score = 1.1;
        std::vector<double> var_vector(N_VARIABLES, 0);
        if (Particle.type() == xAOD::Type::ObjectType::Muon) {
            ptvarcone30 = s_acc_ptvarcone30(Particle);
            var_vector[0] = TrackJetNTrack;
            var_vector[1] = PtRel;
            var_vector[2] = PtFrac;
            var_vector[3] = DRlj;
            var_vector[4] = topoetcone20 / pt;
            var_vector[5] = ptvarcone30 / pt;
            score = m_TMVAReader_Muon->EvaluateMVA(var_vector, m_muonMethodName.value());
        } else if (Particle.type() == xAOD::Type::ObjectType::Electron) {
            ptvarcone20 = s_acc_ptvarcone20(Particle);
            var_vector[0] = TrackJetNTrack;
            var_vector[1] = PtRel;
            var_vector[2] = PtFrac;
            var_vector[3] = DRlj;
            var_vector[4] = topoetcone20 / pt;
            var_vector[5] = ptvarcone20 / pt;
            score = m_TMVAReader_Elec->EvaluateMVA(var_vector, m_elecMethodName.value());
        } else {
            ATH_MSG_ERROR("The function needs either a muon or an electron!");
            return StatusCode::FAILURE;
        }
        s_dec_iso_PLT(Particle) = score;

        return StatusCode::SUCCESS;
    }
}  // namespace CP
