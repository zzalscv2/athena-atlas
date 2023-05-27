/*
 Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
 */

#ifndef ISOLATIONSELECTION_TESTMARCOHELPERS_H
#define ISOLATIONSELECTION_TESTMARCOHELPERS_H

#ifndef XAOD_STANDALONE
#include <IsolationSelection/Defs.h>
#include <IsolationSelection/IsoVariableHelper.h>
#include <IsolationSelection/IsolationCloseByCorrectionTool.h>
#include <MuonTesterTree/MuonTesterTreeDict.h>
#include <StoreGate/ReadDecorHandleKey.h>
#include <xAODBase/IParticleContainer.h>
namespace CP {

    class IsolationWP;
    class IsoCorrectionTestHelper : public MuonVal::MuonTesterBranch {
    public:
        IsoCorrectionTestHelper(MuonVal::MuonTesterTree& outTree, const std::string& ContainerName, const std::vector<std::unique_ptr<IsolationWP>>& WP);
        StatusCode Fill(const xAOD::IParticleContainer* Particles);

        /// Pipe the name of the decorator selecting the objects dumped to the TTree
        void SetSelectionDecorator(const std::string& acc);
        /// Pipe the name of te decorator encoding whether the object already passed the vanilla isolation
        void SetIsolationDecorator(const std::string& acc);
        /// Pipe the name of te decorator encoding whether the object already passed the corrected isolation
        void SetUpdatedIsoDecorator(const std::string& acc);

        /// Specify whether the vanilla isolation variable is backuped to another set of decorators
        void SetBackupPreFix(const std::string& prefix);

        /// set the list of all clusters from the Cluster container matched
        /// by the IsolationCloseByCorrectionTool... A bit awkward path,
        /// but otherwise we do not get
        /// consistency in the validation plots
        void SetClusters(const ClusterSet& clusters);
        void SetFlowElements(const PflowSet& flows);

        bool init() override final;
        bool fill(const EventContext& ctx) override final;

    private:
        float Charge(const xAOD::IParticle* P) const;
        StatusCode FillIsolationBranches(const xAOD::IParticle* P, const IsoHelperPtr& Acc, MuonVal::VectorBranch<float>& Original,
                                         MuonVal::VectorBranch<float>& Corrected);

        std::string m_cont_name{};

        MuonVal::VectorBranch<float>& m_pt{parent().newVector<float>(m_cont_name + "_pt")};
        MuonVal::VectorBranch<float>& m_eta{parent().newVector<float>(m_cont_name + "_eta")};
        MuonVal::VectorBranch<float>& m_phi{parent().newVector<float>(m_cont_name + "_ phi")};
        MuonVal::VectorBranch<float>& m_e{parent().newVector<float>(m_cont_name + "_e")};
        MuonVal::VectorBranch<int>& m_Q{parent().newVector<int>(m_cont_name + "_Q")};

        MuonVal::VectorBranch<bool>& m_orig_passIso{parent().newVector<bool>(m_cont_name + "_OrigPassIso")};
        MuonVal::VectorBranch<bool>& m_corr_passIso{parent().newVector<bool>(m_cont_name + "_CorrPassIso")};

        MuonVal::VectorBranch<float>& m_assoc_track_pt{parent().newVector<float>(m_cont_name + "_trackPt")};

        MuonVal::VectorBranch<float>& m_assoc_cluster_et{parent().newVector<float>(m_cont_name + "_clusterEt")};
        MuonVal::VectorBranch<float>& m_assoc_cluster_eta{parent().newVector<float>(m_cont_name + "_clusterEta")};
        MuonVal::VectorBranch<float>& m_assoc_cluster_phi{parent().newVector<float>(m_cont_name + "_clusterPhi")};

        MuonVal::VectorBranch<float>& m_assoc_pflow_et{parent().newVector<float>(m_cont_name + "_pflowEt")};
        MuonVal::VectorBranch<float>& m_assoc_pflow_eta{parent().newVector<float>(m_cont_name + "_pflowEta")};
        MuonVal::VectorBranch<float>& m_assoc_pflow_phi{parent().newVector<float>(m_cont_name + "_pflowPhi")};
        struct IsolationBranches {
            IsolationBranches(IsoCorrectionTestHelper& parent, IsoType T, const std::string& prefix) :
                Accessor{std::make_unique<IsoVariableHelper>(T, prefix)},
                original_cones{parent.parent().newVector<float>(parent.name() + "_Orig_" + Accessor->name())},
                corrected_cones{parent.parent().newVector<float>(parent.name() + "_Corr_" + Accessor->name())} {}
            IsoHelperPtr Accessor;
            MuonVal::VectorBranch<float>& original_cones;
            MuonVal::VectorBranch<float>& corrected_cones;
        };
        std::vector<IsolationBranches> m_iso_branches;

        std::optional<CharAccessor> m_acc_used_for_corr{std::nullopt};
        std::optional<CharAccessor> m_acc_passDefault{std::nullopt};
        std::optional<CharAccessor> m_acc_passCorrected{std::nullopt};

        ClusterSet m_clusters{};
        PflowSet m_flows{};
       
    };
}  // namespace CP
#endif
#endif
