/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
 */

#include "TopPartons/CalcTtbarJetsPartonHistory.h"
#include "TopConfiguration/TopConfig.h"
#include "TopPartons/PartonHistoryUtils.h"

namespace top {
  CalcTtbarJetsPartonHistory::CalcTtbarJetsPartonHistory(const std::string& name) : CalcTopPartonHistory(name) {}

  bool PartonAlreadyFound(const xAOD::TruthParticle* truthParticle, std::vector<std::pair<const xAOD::TruthParticle*, const xAOD::TruthParticle*>> vector) {

    auto Found = [truthParticle](std::pair<const xAOD::TruthParticle*, const xAOD::TruthParticle*> x) { return std::get<0>(x)->barcode() == PartonHistoryUtils::findAfterFSR(truthParticle)->barcode();};
    if (std::find_if(vector.begin(), vector.end(), Found) != vector.end()) return true;

    return false;
  }

  void CalcTtbarJetsPartonHistory::ttbarjetsTLorentzFill(xAOD::PartonHistory* ttbarjetsPartonHistory, TLorentzVector& p4, const std::string& name, FillBranchMethod FillMethod){

    if (FillMethod == FillBranchMethod::Regular) {

        ttbarjetsPartonHistory->auxdecor< float >(name+"_m") = p4.M();
        ttbarjetsPartonHistory->auxdecor< float >(name+"_pt") = p4.Pt();
        ttbarjetsPartonHistory->auxdecor< float >(name+"_phi") = p4.Phi();
        fillEtaBranch(ttbarjetsPartonHistory, name+"_eta", p4);
    }
    else{ // FillMethod == FillBranchMethod::PushBack
        ttbarjetsPartonHistory->auxdecor< std::vector<float> >(name+"_m").push_back(p4.M());
        ttbarjetsPartonHistory->auxdecor< std::vector<float> >(name+"_pt").push_back(p4.Pt());
        ttbarjetsPartonHistory->auxdecor< std::vector<float> >(name+"_phi").push_back(p4.Phi());
        fillEtaBranch(ttbarjetsPartonHistory, name+"_eta", p4, FillMethod);
    }
  }

  void CalcTtbarJetsPartonHistory::ttbarjetsHistorySaver(const xAOD::TruthParticleContainer* truthParticles,
                                                 xAOD::PartonHistory* ttbarjetsPartonHistory) {

    ttbarjetsPartonHistory->IniVarTtbarJets();

    TLorentzVector t_before, t_after, t_after_SC;
    TLorentzVector Wp;
    TLorentzVector b;
    int b_barcode;
    TLorentzVector WpDecay1;
    TLorentzVector WpDecay2;
    int WpDecay1_pdgId;
    int WpDecay1_barcode;
    int WpDecay2_pdgId;
    int WpDecay2_barcode;

    bool event_top = CalcTopPartonHistory::topWb(truthParticles, 6, t_before, t_after, Wp,
                                                 b, b_barcode,
                                                 WpDecay1, WpDecay1_pdgId, WpDecay1_barcode,
                                                 WpDecay2, WpDecay2_pdgId, WpDecay2_barcode);
    bool event_top_SC = CalcTopPartonHistory::topAfterFSR_SC(truthParticles, 6, t_after_SC);

    TLorentzVector tbar_before, tbar_after, tbar_after_SC;
    TLorentzVector Wm;
    TLorentzVector bbar;
    int bbar_barcode;
    TLorentzVector WmDecay1;
    TLorentzVector WmDecay2;
    int WmDecay1_pdgId;
    int WmDecay1_barcode;
    int WmDecay2_pdgId;
    int WmDecay2_barcode;

    bool event_topbar    = CalcTopPartonHistory::topWb(truthParticles, -6, tbar_before, tbar_after, Wm,
                                                        bbar, bbar_barcode,
                                                        WmDecay1, WmDecay1_pdgId, WmDecay1_barcode,
                                                        WmDecay2, WmDecay2_pdgId, WmDecay2_barcode);
    bool event_topbar_SC = CalcTopPartonHistory::topAfterFSR_SC(truthParticles, -6, tbar_after_SC);


    if (event_top && event_topbar) {

        std::vector<std::pair<const xAOD::TruthParticle*,const xAOD::TruthParticle*>> fsr_isr_quarks;
        std::vector<std::pair<const xAOD::TruthParticle*,const xAOD::TruthParticle*>> hard_non_ttbar_out_quarks;

        // Need to catch back-evolution ISR -- this wil not be caught by looking at children of hard quarks, but by looking at mothers...
        //  Get all any legs of the hard process that are not part of the ttbar system
        for (const xAOD::TruthParticle* truthParticle : *truthParticles){

            // Since we save partons after FSR, we may run into the same parton more than once
            // at different radiation versions. Make sure we only keep one.
            if (PartonAlreadyFound(truthParticle, fsr_isr_quarks)) continue;
            if (PartonAlreadyFound(truthParticle, hard_non_ttbar_out_quarks)) continue;

            // Check the particle is a quark (u,d,s,c,b) or a gluon
            // Keep gluons to construct all legs of the 2->N hard process
            if ((std::abs(truthParticle->pdgId()) > 5) && (truthParticle->pdgId() != 21) ) continue;
            // Check the particle is not the b from top
            if (truthParticle->barcode() == b_barcode || truthParticle->barcode() == bbar_barcode) continue;
            // Check the particle is not a quark from the W_from_top decay
            if (truthParticle->barcode() == WpDecay1_barcode || truthParticle->barcode() == WpDecay2_barcode) continue;
            if (truthParticle->barcode() == WmDecay1_barcode || truthParticle->barcode() == WmDecay2_barcode) continue;

            // If we found an outgoing quark that is part of the hard process
            if ((truthParticle->status() == 23) && (truthParticle->pdgId() != 21)){
                // any b from ttbar system will not have 20s status -- since the top will first undergo FSR then decay.
                const xAOD::TruthParticle* truthParticle_parent = truthParticle->parent(0);
                // Do we want it or do we want the post-FSR version of it?
                hard_non_ttbar_out_quarks.push_back({PartonHistoryUtils::findAfterFSR(truthParticle), truthParticle_parent});
            }

            // If the particle is a gluon, we want to get its children
            if (truthParticle->pdgId() == 21){
                int nchildren = truthParticle->nChildren();
                if (nchildren == 0) {
                    // No children found, skip
                    continue;
                }

                if (nchildren == 1){
                    // This is often momentum shifts due to ISR/FSR in other parts of the event
                    continue;
                }
                if ( nchildren == 2 ){
                    // If the gluon has = 2 children we hit a splitting
                    const xAOD::TruthParticle* child1 = truthParticle->child(0);
                    const xAOD::TruthParticle* child2 = truthParticle->child(1);
                    if (!(child1 && child2)) continue;
                    // We will get plenty of splittings into pairs of gluons, skip those
                    if (child1->pdgId() == 21 || child2->pdgId() == 21) continue;

                    for (int ichild = 0; ichild < nchildren; ichild++){
                        const xAOD::TruthParticle* child = truthParticle->child(ichild);
                        // We now have gluons splitting into pairs of quarks left
                        // Only keep quarks that are part of ISR/FSR
                        if (child->status() > 40 && child->status() < 60){
                            if (PartonAlreadyFound(child, fsr_isr_quarks)) continue;
                            if (PartonAlreadyFound(child, hard_non_ttbar_out_quarks)) continue;

                            fsr_isr_quarks.push_back({ PartonHistoryUtils::findAfterFSR(child), truthParticle});
                        }
                        if (child->status() == 31){
                            // This is a quark branching from a gluon and entering a 2->2 MPI (qq->qq, qg->qg, gg->gg)
                            // It should have a child with the same PDG ID

                            // There should always be 2 children
                            int n_mpi_children = child->nChildren();
                            if (n_mpi_children == 1){

                                const xAOD::TruthParticle* mpi_child = child->child(0);
                                if (!mpi_child) continue;
                                // If that child is an intermediate in MPI (subprocess)
                                if (mpi_child->status() == 32){
                                    int n_mpi_interm_children = mpi_child->nChildren();
                                    // MPI interemediate should have 2 children
                                    if (n_mpi_interm_children != 2) continue;
                                    // Go through the children of the intermediate
                                    for (int jchild = 0; jchild < n_mpi_interm_children; jchild++){
                                        const xAOD::TruthParticle* mpi_interm_child = mpi_child->child(jchild);
                                        // If child is a gluon, we will get to it eventually by the main loop, so don't worry about it now
                                        if (mpi_interm_child->pdgId() == 21) continue;
                                        // else if we have a quark, we want to keep it as an extra quark
                                        if (PartonAlreadyFound(mpi_interm_child, fsr_isr_quarks)) continue;
                                        if (PartonAlreadyFound(mpi_interm_child, hard_non_ttbar_out_quarks)) continue;
                                        fsr_isr_quarks.push_back({PartonHistoryUtils::findAfterFSR(mpi_interm_child), child});
                                    }
                                }
                            }
                            else if (n_mpi_children == 2){
                                for (int jchild = 0; jchild < n_mpi_children; jchild++){
                                    const xAOD::TruthParticle* mpi_child = child->child(jchild);
                                    // If child is a gluon, we will get to it evantually by the main loop, so don't worry about it now
                                    if (mpi_child->pdgId() == 21) continue;
                                    // else if we have a quark, we want to keep it as an extra quark
                                    if (PartonAlreadyFound(mpi_child, fsr_isr_quarks)) continue;
                                    if (PartonAlreadyFound(mpi_child, hard_non_ttbar_out_quarks)) continue;
                                    fsr_isr_quarks.push_back({PartonHistoryUtils::findAfterFSR(mpi_child), child});
                                }
                            }
                            else{
                                continue;
                            }
                        }
                    }
                }
                if ( nchildren > 2 ){
                    // This is often during hadornisation, you get a lot of mesons, etc..
                    continue;
                }
            }
        }

        // Add all the extra quarks in one vector of parton-parent pairs
        std::vector<std::pair<const xAOD::TruthParticle*,const xAOD::TruthParticle*>> all_extra_quarks = hard_non_ttbar_out_quarks;
        all_extra_quarks.insert( all_extra_quarks.end(), fsr_isr_quarks.begin(), fsr_isr_quarks.end() );

        // Gather the kinematics/information of the extra quarks and their parents
        std::vector<PartonParentPair> extra_quarks;
        for (std::pair<const xAOD::TruthParticle*, const xAOD::TruthParticle*> extra_quark_and_parent: all_extra_quarks){
            // Get the extra quark
            const xAOD::TruthParticle* extra_quark = std::get<0>(extra_quark_and_parent);
            PartonInfo extra_q(extra_quark->p4(), extra_quark->pdgId(), extra_quark->status(), extra_quark->barcode());
            // Get the parent of te extra quark
            const xAOD::TruthParticle* extra_quark_parent = std::get<1>(extra_quark_and_parent);

            // In case the pointer to parent is null, we create a dummy parent then update it
            PartonInfo extra_q_parent(TLorentzVector(0,0,0,0), 0, 0, 0);
            if (extra_quark_parent){
                extra_q_parent = {extra_quark_parent->p4(), extra_quark_parent->pdgId(), extra_quark_parent->status(), extra_quark_parent->barcode()};
            }

            // Create the info on extra quark and its parent
            PartonParentPair extra_quark_info(extra_q, extra_q_parent);
            extra_quarks.push_back(extra_quark_info);
        }


        // Order extra quarks by pT
        std::sort(extra_quarks.begin(), extra_quarks.end(), [](const PartonParentPair& lhs, const PartonParentPair& rhs) { return std::get<0>(std::get<0>(lhs)).Pt() > std::get<0>(std::get<0>(rhs)).Pt(); });


        // Save the 12 hardest extra quarks and their respective parents
        size_t max_quarks = 12;
        for (size_t i = 0; i < std::min(max_quarks, extra_quarks.size()); i++) {

            // Extra quark/parent pair
            PartonParentPair extra_quark_info = extra_quarks[i];

            // Extra quark
            PartonInfo extra_quark        = std::get<0>(extra_quark_info);
            TLorentzVector extra_quark_p4 = std::get<0>(extra_quark);
            int extra_quark_pdgId         = std::get<1>(extra_quark);
            int extra_quark_status        = std::get<2>(extra_quark);

            ttbarjetsTLorentzFill(ttbarjetsPartonHistory, extra_quark_p4, "MC_extra_quarks", FillBranchMethod::PushBack);
            ttbarjetsPartonHistory->auxdecor< std::vector<int> >("MC_extra_quarks_pdgId").push_back(extra_quark_pdgId);
            ttbarjetsPartonHistory->auxdecor< std::vector<int> >("MC_extra_quarks_status").push_back(extra_quark_status);

            // Extra quark's parent
            PartonInfo extra_quark_parent      = std::get<1>(extra_quark_info);
            TLorentzVector extra_quark_parent_p4  = std::get<0>(extra_quark_parent);
            int extra_quark_parent_pdgId       = std::get<1>(extra_quark_parent);
            int extra_quark_parent_status      = std::get<2>(extra_quark_parent);
            int extra_quark_parent_barcode     = std::get<3>(extra_quark_parent);

            ttbarjetsTLorentzFill(ttbarjetsPartonHistory, extra_quark_parent_p4, "MC_extra_quarks_parent", FillBranchMethod::PushBack);
            ttbarjetsPartonHistory->auxdecor< std::vector<int> >("MC_extra_quarks_parent_pdgId").push_back(extra_quark_parent_pdgId);
            ttbarjetsPartonHistory->auxdecor< std::vector<int> >("MC_extra_quarks_parent_status").push_back(extra_quark_parent_status);
            ttbarjetsPartonHistory->auxdecor< std::vector<int> >("MC_extra_quarks_parent_barcode").push_back(extra_quark_parent_barcode);

        }

        TLorentzVector temp = t_before + tbar_before;
        ttbarjetsTLorentzFill(ttbarjetsPartonHistory, temp, "MC_ttbar_beforeFSR");

        temp = t_after + tbar_after;
        ttbarjetsTLorentzFill(ttbarjetsPartonHistory, temp, "MC_ttbar_afterFSR_beforeDecay");

        temp = WmDecay1 + WmDecay2 + b + WpDecay1 + WpDecay2 + bbar;
        ttbarjetsTLorentzFill(ttbarjetsPartonHistory, temp, "MC_ttbar_afterFSR");

    }//if
    if (event_top) {

        ttbarjetsTLorentzFill(ttbarjetsPartonHistory, t_before, "MC_t_beforeFSR");

        ttbarjetsTLorentzFill(ttbarjetsPartonHistory, t_after, "MC_t_afterFSR");

        if (event_top_SC) {
            ttbarjetsTLorentzFill(ttbarjetsPartonHistory, t_after_SC, "MC_t_afterFSR_SC");
        }

        ttbarjetsTLorentzFill(ttbarjetsPartonHistory, Wp, "MC_W_from_t");

        ttbarjetsTLorentzFill(ttbarjetsPartonHistory, b, "MC_b_from_t");

        ttbarjetsTLorentzFill(ttbarjetsPartonHistory, WpDecay1, "MC_Wdecay1_from_t");
        ttbarjetsPartonHistory->auxdecor< int >("MC_Wdecay1_from_t_pdgId") = WpDecay1_pdgId;

        ttbarjetsTLorentzFill(ttbarjetsPartonHistory, WpDecay2, "MC_Wdecay2_from_t");
        ttbarjetsPartonHistory->auxdecor< int >("MC_Wdecay2_from_t_pdgId") = WpDecay2_pdgId;

    }//if

    if (event_topbar) {

        ttbarjetsTLorentzFill(ttbarjetsPartonHistory, tbar_before, "MC_tbar_beforeFSR");

        ttbarjetsTLorentzFill(ttbarjetsPartonHistory, tbar_after, "MC_tbar_afterFSR");

        if (event_topbar_SC) {

            ttbarjetsTLorentzFill(ttbarjetsPartonHistory, tbar_after_SC, "MC_tbar_afterFSR_SC");
        }

        ttbarjetsTLorentzFill(ttbarjetsPartonHistory, Wm, "MC_W_from_tbar");

        ttbarjetsTLorentzFill(ttbarjetsPartonHistory, bbar, "MC_b_from_tbar");


        ttbarjetsTLorentzFill(ttbarjetsPartonHistory, WmDecay1, "MC_Wdecay1_from_tbar");
        ttbarjetsPartonHistory->auxdecor< int >("MC_Wdecay1_from_tbar_pdgId") = WmDecay1_pdgId;


        ttbarjetsTLorentzFill(ttbarjetsPartonHistory, WmDecay2, "MC_Wdecay2_from_tbar");
        ttbarjetsPartonHistory->auxdecor< int >("MC_Wdecay2_from_tbar_pdgId") = WmDecay2_pdgId;

        }//if
    }

  StatusCode CalcTtbarJetsPartonHistory::execute() {
    // Get the Truth Particles

    const xAOD::TruthParticleContainer* truthParticles(nullptr);

    if(m_config->getDerivationStream() == "PHYS") //in DAOD_PHYS we don't have the truth particles container
    {
      //the functions ued in this class always start from the top, so it's enough to do the following
      std::vector<std::string> collections = {"TruthTop"};
      ATH_CHECK(buildContainerFromMultipleCollections(collections,"AT_ttbarjetsPartonHistory_TruthParticles"));
      ATH_CHECK(evtStore()->retrieve(truthParticles, "AT_ttbarjetsPartonHistory_TruthParticles"));

      //we need to be able to navigate from the Ws to their decayProducts, see CalcTopPartonHistory.h for details
      ATH_CHECK(linkBosonCollections());

    }
    else  //otherwise we retrieve the container as usual
    {
      ATH_CHECK(evtStore()->retrieve(truthParticles, m_config->sgKeyMCParticle()));
    }

    // Create the partonHistory xAOD object
    xAOD::PartonHistoryAuxContainer* partonAuxCont = new xAOD::PartonHistoryAuxContainer {};
    xAOD::PartonHistoryContainer* partonCont = new xAOD::PartonHistoryContainer {};
    partonCont->setStore(partonAuxCont);

    xAOD::PartonHistory* ttbarjetsPartonHistory = new xAOD::PartonHistory {};
    partonCont->push_back(ttbarjetsPartonHistory);

    // Recover the parton history for ttbar events
    ttbarjetsHistorySaver(truthParticles, ttbarjetsPartonHistory);

    // Save to StoreGate / TStore
    std::string outputSGKey = m_config->sgKeyTopPartonHistory();
    std::string outputSGKeyAux = outputSGKey + "Aux.";

    xAOD::TReturnCode save = evtStore()->tds()->record(partonCont, outputSGKey);
    xAOD::TReturnCode saveAux = evtStore()->tds()->record(partonAuxCont, outputSGKeyAux);
    if (!save || !saveAux) {
      return StatusCode::FAILURE;
    }

    return StatusCode::SUCCESS;
  }
}
