/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////
// RCJetSubstructureAug.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
// Author: G. Albouy (galbouy@lpsc.in2p3.fr)
// This tool computes substructure variables for ReClustered jets
// from LCTopo clusters ghost associated to RC jets 
// by constructing cluster jets 

#ifndef DERIVATIONFRAMEWORK_RCJetSubstructureAug_H
#define DERIVATIONFRAMEWORK_RCJetSubstructureAug_H

#include <string>

#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "xAODJet/JetContainer.h"
#include "StoreGate/ReadHandleKey.h"

#include "ExpressionEvaluation/ExpressionParserUser.h"

namespace DerivationFramework {
    enum EJetParser { kRCJetSelection , kNumJetParser };
    class RCJetSubstructureAug :  public extends<ExpressionParserUser<AthAlgTool, kNumJetParser>, IAugmentationTool> {
        public: 
        RCJetSubstructureAug(const std::string& t, const std::string& n, const IInterface* p);
        virtual ~RCJetSubstructureAug();
        virtual StatusCode initialize() override;
        virtual StatusCode finalize() override;
        virtual StatusCode addBranches() const override;

        private:
        
        StringProperty m_streamName
            { this, "StreamName", "", "Name of the stream" };
        StringProperty m_ghostName
            { this, "GhostClusterName", "GhostLCTopo", "Name of the ghost clusters"};
        StringProperty m_selectionString 
            { this, "SelectionString", "", "Selection to apply to jet"};
        StringProperty m_suffix
            { this, "Suffix", "", "Suffix for variables"};
        
        SG::ReadHandleKey< xAOD::JetContainer > m_jetKey{ this, "JetContainerKey", ""};

        // Struct to hold all decorators
        struct moments_t;

        moments_t* m_moments;

    };

    struct RCJetSubstructureAug::moments_t {
        /// Qw decorator
        SG::AuxElement::Decorator<float> dec_Qw;

        /// Nsubjetiness decorators
        SG::AuxElement::Decorator<float> dec_Tau1;
        SG::AuxElement::Decorator<float> dec_Tau2;
        SG::AuxElement::Decorator<float> dec_Tau3;
        SG::AuxElement::Decorator<float> dec_Tau4;
        SG::AuxElement::Decorator<float> dec_Tau21;
        SG::AuxElement::Decorator<float> dec_Tau32;

        /// KtSplittingScale decorators
        SG::AuxElement::Decorator<float> dec_Split12;
        SG::AuxElement::Decorator<float> dec_Split23;
        SG::AuxElement::Decorator<float> dec_Split34;

        /// Energy correlation factors decorators
        SG::AuxElement::Decorator<float> dec_ECF1;
        SG::AuxElement::Decorator<float> dec_ECF2;
        SG::AuxElement::Decorator<float> dec_ECF3;
        SG::AuxElement::Decorator<float> dec_ECF4;
        SG::AuxElement::Decorator<float> dec_C2;
        SG::AuxElement::Decorator<float> dec_D2;

        /// Cluster jets informations decorators 
        SG::AuxElement::Decorator<float> dec_pT;
        SG::AuxElement::Decorator<float> dec_m;
        SG::AuxElement::Decorator<float> dec_NClusts;
        SG::AuxElement::Decorator<float> dec_eta;
        SG::AuxElement::Decorator<float> dec_phi;

        /// Timing information 
        SG::AuxElement::Decorator<float> dec_timing;
        
        moments_t (const std::string& suffix):
        
            dec_Qw("Qw_" + suffix),

            dec_Tau1("Tau1_" + suffix), 
            dec_Tau2("Tau2_" + suffix),
            dec_Tau3("Tau3_" + suffix),
            dec_Tau4("Tau4_" + suffix),
            dec_Tau21("Tau21_" + suffix),
            dec_Tau32("Tau32_" + suffix),

            dec_Split12("Split12_" + suffix),
            dec_Split23("Split23_" + suffix),
            dec_Split34("Split34_" + suffix),

            dec_ECF1("ECF1_" + suffix),
            dec_ECF2("ECF2_" + suffix),
            dec_ECF3("ECF3_" + suffix),
            dec_ECF4("ECF4_" + suffix),
            dec_C2("C2_" + suffix),
            dec_D2("D2_" + suffix),

            dec_pT("pT_" + suffix),
            dec_m("m_" + suffix),
            dec_NClusts("NClusts"),
            dec_eta("eta_" + suffix),
            dec_phi("phi_" + suffix),

            dec_timing("timing_" + suffix){}

    };

}

#endif // DERIVATIONFRAMEWORK_RCJetSubstructureAug_H
