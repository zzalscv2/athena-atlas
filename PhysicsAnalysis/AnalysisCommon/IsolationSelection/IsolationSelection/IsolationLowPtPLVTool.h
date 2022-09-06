/*
 Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
 */

#ifndef IsolationSelection_IsolationLowPtPLVTool_H
#define IsolationSelection_IsolationLowPtPLVTool_H

#include <AsgTools/AsgTool.h>
#include <AsgTools/PropertyWrapper.h>
#include <IsolationSelection/Defs.h>
#include <IsolationSelection/IIsolationLowPtPLVTool.h>

// TMVA
#include "TMVA/Reader.h"

namespace CP {
    class IsolationLowPtPLVTool : public asg::AsgTool, public virtual IIsolationLowPtPLVTool {
    public:
        IsolationLowPtPLVTool(const std::string& name);
        ASG_TOOL_CLASS(IsolationLowPtPLVTool, IIsolationLowPtPLVTool)
        virtual StatusCode initialize() override;
        virtual StatusCode augmentPLV(const xAOD::IParticle& particle) override;

    private:
        Gaudi::Property<std::string> m_muonCalibFile{
            this, "MuonCalibFile", "IsolationCorrections/v5/TMVAClassification_BDT_Muon_LowPtPromptLeptonTagger_20191107.weights.xml",
            "XML file holding the TMVA configuration for muons"};
        Gaudi::Property<std::string> m_elecCalibFile{
            this, "ElecCalibFile", "IsolationCorrections/v5/TMVAClassification_BDT_Electron_LowPtPromptLeptonTagger_20191107.weights.xml",
            "XML file holding the TMVA configuration for electrons"};
        Gaudi::Property<std::string> m_muonMethodName{this, "MuonMethodName", "LowPtPLT_Muon", "Method name for electron LowPtPLV"};
        Gaudi::Property<std::string> m_elecMethodName{this, "ElecMethodName", "LowPtPLT_Elec", "Method name for muon LowPtPLV"};
        std::unique_ptr<TMVA::Reader> m_TMVAReader_Muon{nullptr};
        std::unique_ptr<TMVA::Reader> m_TMVAReader_Elec{nullptr};
    };

}  // namespace CP
#endif
