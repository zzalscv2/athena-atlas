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

    class IsoCorrectionTestHelper : public IMuonTesterBranch {
    public:
        IsoCorrectionTestHelper(MuonTesterTree& outTree, const std::string& ContainerName, const std::vector<std::unique_ptr<IsolationWP>>& WP);
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


        MuonTesterTree& tree();
        /// Interface methods from IMuonTesterTree
        std::string name() const override;
        bool init() override;
        bool fill(const EventContext& ctx) override;
        std::vector<DataDependency> data_dependencies() override;

    private:
        float Charge(const xAOD::IParticle* P) const;
        StatusCode FillIsolationBranches(const xAOD::IParticle* P, const IsoHelperPtr& Acc, VectorBranch<float>& Original,
                                         VectorBranch<float>& Corrected);

        MuonTesterTree& m_tree;
        std::string m_cont_name{};

        VectorBranch<float>& m_pt{m_tree.newVector<float>(m_cont_name + "_pt")};
        VectorBranch<float>& m_eta{m_tree.newVector<float>(m_cont_name + "_eta")};
        VectorBranch<float>& m_phi{m_tree.newVector<float>(m_cont_name + "_ phi")};
        VectorBranch<float>& m_e{m_tree.newVector<float>(m_cont_name + "_e")};
        VectorBranch<int>& m_Q{m_tree.newVector<int>(m_cont_name + "_Q")};

        VectorBranch<bool>& m_orig_passIso{m_tree.newVector<bool>(m_cont_name + "_OrigPassIso")};
        VectorBranch<bool>& m_corr_passIso{m_tree.newVector<bool>(m_cont_name + "_CorrPassIso")};

        VectorBranch<float>& m_assoc_track_pt{m_tree.newVector<float>(m_cont_name + "_trackPt")};

        VectorBranch<float>& m_assoc_cluster_et{m_tree.newVector<float>(m_cont_name + "_clusterEt")};
        VectorBranch<float>& m_assoc_cluster_eta{m_tree.newVector<float>(m_cont_name + "_clusterEta")};
        VectorBranch<float>& m_assoc_cluster_phi{m_tree.newVector<float>(m_cont_name + "_clusterPhi")};

        VectorBranch<float>& m_assoc_pflow_et{m_tree.newVector<float>(m_cont_name + "_pflowEt")};
        VectorBranch<float>& m_assoc_pflow_eta{m_tree.newVector<float>(m_cont_name + "_pflowEta")};
        VectorBranch<float>& m_assoc_pflow_phi{m_tree.newVector<float>(m_cont_name + "_pflowPhi")};
        struct IsolationBranches {
            IsolationBranches(IsoCorrectionTestHelper& parent, IsoType T, const std::string& prefix) :
                Accessor{std::make_unique<IsoVariableHelper>(T, prefix)},
                original_cones{parent.tree().newVector<float>(parent.name() + "_Orig_" + Accessor->name())},
                corrected_cones{parent.tree().newVector<float>(parent.name() + "_Corr_" + Accessor->name())} {}
            IsoHelperPtr Accessor;
            VectorBranch<float>& original_cones;
            VectorBranch<float>& corrected_cones;
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
