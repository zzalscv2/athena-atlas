/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// Tool to decorate the EventInfo object with truth categories informations
// Authors: T. Guillemin, J. Lacey, D. Gillberg

#ifndef DerivationFrameworkHiggs_TruthCategoriesDecorator_H
#define DerivationFrameworkHiggs_TruthCategoriesDecorator_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKeyArray.h"

// EDM: typedefs, so must be included and not forward-referenced
#include "xAODEventInfo/EventInfo.h"
#include "xAODTruth/TruthEventContainer.h"

// Note: must include TLorentzVector before the next one

#include "GenInterfaces/IHiggsTruthCategoryTool.h"
#include "GenInterfaces/IxAODtoHepMCTool.h"
#include "TruthRivetTools/HiggsTemplateCrossSectionsDefs.h"

namespace DerivationFramework {

    class TruthCategoriesDecorator : public AthReentrantAlgorithm {
    public:
        TruthCategoriesDecorator(const std::string& n, ISvcLocator* p);
        virtual ~TruthCategoriesDecorator() = default;
        StatusCode initialize();
        StatusCode execute(const EventContext& ctx) const;

    private:
        ToolHandle<IxAODtoHepMCTool> m_xAODtoHepMCTool{this, "HepMCTool", "xAODtoHepMCTool"};
        ToolHandle<IHiggsTruthCategoryTool> m_higgsTruthCatTool{this, "CategoryTool", "HiggsTruthCategoryTool"};

        // Path to TEnv file containing MC-channel-numbre <-> HiggsProdMode map
        Gaudi::Property<std::string> m_configPath{this, "ConfigPath", "DerivationFrameworkHiggs/HiggsMCsamples.cfg"};
        struct HTXSSample {
            /// Higgs production modes, corresponding to input sample
            HTXS::HiggsProdMode prod{HTXS::HiggsProdMode::UNKNOWN};
            /// Additional identifier flag for TH production modes
            HTXS::tH_type th_type{HTXS::tH_type::noTH};
            // DSIDs satisfying this production mode
            std::set<int> dsids{};
        };

        std::vector<HTXSSample> m_htxs_samples{};

        // Detail level. Steers amount of decoration.
        //  0: basic information. Categoization ints, Higgs prod mode, Njets, Higgs pT
        //  1: the above + Higgs boson 4-vec + associated V 4-vec
        //  2: the above + truth jets built excluding the Higgs boson decay
        //  3: the above + 4-vector sum of all decay products from Higgs boson and V-boson
        Gaudi::Property<int> m_detailLevel{this, "DetailLevel", 3};

        // Methods for decoration of four vectors
        StatusCode decorateFourVec(const EventContext& ctx, const std::string& prefix, const TLorentzVector& p4) const;
        StatusCode decorateFourVecs(const EventContext& ctx, const std::string& prefix, const std::vector<TLorentzVector>& p4s) const;

        SG::ReadHandleKey<xAOD::TruthEventContainer> m_truthEvtKey{this, "TruthEvtKey", "TruthEvents"};
        SG::ReadHandleKey<xAOD::EventInfo> m_evtInfoKey{this, "EvtKey", "EventInfo"};

        using EvtInfoDecorKey = SG::WriteDecorHandleKey<xAOD::EventInfo>;
        SG::WriteDecorHandleKeyArray<xAOD::EventInfo> m_STXSDecors{this, "ToolDecorations", {}, "List of all keys decorated by the alg"};

        /// Set of DecorHandleKeys to write the four momenta needed for the HTXS categorization.
        struct FourMomDecoration {
            FourMomDecoration(const SG::ReadHandleKey<xAOD::EventInfo>& ev_key, const std::string& prefix) :
                pt{ev_key.key() + "." + prefix + "_pt"},
                eta{ev_key.key() + "." + prefix + "_eta"},
                phi{ev_key.key() + "." + prefix + "_phi"},
                m{ev_key.key() + "." + prefix + "_m"} {}
            EvtInfoDecorKey pt;
            EvtInfoDecorKey eta;
            EvtInfoDecorKey phi;
            EvtInfoDecorKey m;
            std::vector<EvtInfoDecorKey> vect() const { return {pt, eta, phi, m}; }
            StatusCode initialize(){
                if (!pt.initialize().isSuccess() || !eta.initialize().isSuccess() || !phi.initialize().isSuccess() || !m.initialize().isSuccess()){
                    return StatusCode::FAILURE;
                }
                return StatusCode::SUCCESS;
            }
        };
        using P4DecorMap = std::map<std::string, FourMomDecoration>;
        P4DecorMap m_p4_decors{};
    };  /// class

}  // namespace DerivationFramework

#endif
