/*
   Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
 */

#include "TopPartons/CalcTwhPartonHistory.h"
#include "TopPartons/CalcTopPartonHistory.h"
#include "TopPartons/PartonHistoryUtils.h"
#include "TopConfiguration/TopConfig.h"

namespace top {
  CalcTwhPartonHistory::CalcTwhPartonHistory(const std::string& name) : CalcTopPartonHistory(name) {}

  bool CalcTwhPartonHistory::HiggsAndDecay(const xAOD::TruthParticleContainer* truthParticles) {

    for (const xAOD::TruthParticle* particle : *truthParticles) {
      if (particle->pdgId() != 25 || particle->nChildren() != 2) {
        continue;
      }
      tWH.Higgs_p4 = particle->p4();

      const top::PartonHistoryUtils::HiggsDecay& higgs = top::PartonHistoryUtils::AnalyzeHiggsDecay(particle);
      
      tWH.decay1_p4 = higgs.decay1_vector;
      tWH.decay2_p4 = higgs.decay2_vector;
      tWH.decay1_pdgId = higgs.decay1_pdgId;
      tWH.decay2_pdgId = higgs.decay2_pdgId;
      tWH.tau_decay1_isHadronic = higgs.tau_decay1_isHadronic;
      tWH.tau_decay2_isHadronic = higgs.tau_decay2_isHadronic;
      tWH.tau_decay1_p4 = higgs.tau_decay1_vector;
      tWH.tau_decay2_p4 = higgs.tau_decay2_vector;
      tWH.tauvis_decay1_p4 = higgs.tauvis_decay1_vector;
      tWH.tauvis_decay2_p4 = higgs.tauvis_decay2_vector;
      tWH.decay1_from_decay1_p4 = higgs.decay1_from_decay1_vector;
      tWH.decay2_from_decay1_p4 = higgs.decay2_from_decay1_vector;
      tWH.decay1_from_decay1_pdgId = higgs.decay1_from_decay1_pdgId;
      tWH.decay2_from_decay1_pdgId = higgs.decay2_from_decay1_pdgId;
      tWH.decay1_from_decay2_p4 = higgs.decay1_from_decay2_vector;
      tWH.decay2_from_decay2_p4 = higgs.decay2_from_decay2_vector;
      tWH.decay1_from_decay2_pdgId = higgs.decay1_from_decay2_pdgId;
      tWH.decay2_from_decay2_pdgId = higgs.decay2_from_decay2_pdgId;
      tWH.tau_decay1_from_decay1_isHadronic = higgs.tau_decay1_from_decay1_isHadronic;
      tWH.tau_decay2_from_decay1_isHadronic = higgs.tau_decay2_from_decay1_isHadronic;
      tWH.tau_decay1_from_decay2_isHadronic = higgs.tau_decay1_from_decay2_isHadronic;
      tWH.tau_decay2_from_decay2_isHadronic = higgs.tau_decay2_from_decay2_isHadronic;
      tWH.tau_decay1_from_decay1_p4 = higgs.tau_decay1_from_decay1_vector;
      tWH.tau_decay2_from_decay1_p4 = higgs.tau_decay2_from_decay1_vector;
      tWH.tau_decay1_from_decay2_p4 = higgs.tau_decay1_from_decay2_vector;
      tWH.tau_decay2_from_decay2_p4 = higgs.tau_decay2_from_decay2_vector;
      tWH.tauvis_decay1_from_decay1_p4 = higgs.tauvis_decay1_from_decay1_vector;
      tWH.tauvis_decay2_from_decay1_p4 = higgs.tauvis_decay2_from_decay1_vector;
      tWH.tauvis_decay1_from_decay2_p4 = higgs.tauvis_decay1_from_decay2_vector;
      tWH.tauvis_decay2_from_decay2_p4 = higgs.tauvis_decay2_from_decay2_vector;
      

      return true;
    }
    return false;
  }

  void CalcTwhPartonHistory::THHistorySaver(const xAOD::TruthParticleContainer* truthParticles,
                                            xAOD::PartonHistory* TwhPartonHistory) {
    TwhPartonHistory->IniVarThq();
    TLorentzVector t_before, t_after, t_after_SC;
    TLorentzVector Wp;
    TLorentzVector b;
    TLorentzVector WpDecay1;
    TLorentzVector WpDecay2;
    int WpDecay1_pdgId;
    int WpDecay2_pdgId;
    TLorentzVector tau_decay_from_W_p4;
    int tau_decay_from_W_isHadronic;
    TLorentzVector tauvis_decay_from_W_p4;

    const bool event_top = CalcTopPartonHistory::topWb(truthParticles, 6, t_before, t_after,
						       Wp, b, WpDecay1, WpDecay1_pdgId, WpDecay2, 
						       WpDecay2_pdgId, tau_decay_from_W_p4, tau_decay_from_W_isHadronic, tauvis_decay_from_W_p4);
    const bool event_top_SC = CalcTopPartonHistory::topAfterFSR_SC(truthParticles, 6, t_after_SC);
    const bool event_topbar = CalcTopPartonHistory::topWb(truthParticles, -6, t_before, t_after,
							  Wp, b, WpDecay1, WpDecay1_pdgId, WpDecay2, 
							  WpDecay2_pdgId, tau_decay_from_W_p4, tau_decay_from_W_isHadronic, tauvis_decay_from_W_p4);
    const bool event_topbar_SC = CalcTopPartonHistory::topAfterFSR_SC(truthParticles, -6, t_after_SC);
    const bool event_Higgs =  CalcTwhPartonHistory::HiggsAndDecay(truthParticles);
   
    TLorentzVector WnotFromTop;
    TLorentzVector WnotFromTopDecay1;
    TLorentzVector WnotFromTopDecay2;
    int WnotFromTop_pdgId;
    int WnotFromTopDecay1_pdgId;
    int WnotFromTopDecay2_pdgId;

    bool event_WnotTop = CalcTopPartonHistory::Wt_W(truthParticles, 
                                                    WnotFromTop, WnotFromTop_pdgId, 
                                                    WnotFromTopDecay1, WnotFromTopDecay1_pdgId, 
                                                    WnotFromTopDecay2, WnotFromTopDecay2_pdgId);


    // second b-quark
    bool event_secondb = false;
    bool event_secondbbar = false;
    TLorentzVector secondb_beforeFSR_p4;
    int secondb_beforeFSR_pdgId;
    TLorentzVector secondb_afterFSR_p4;
    int secondb_afterFSR_pdgId;

    if (event_topbar) {
      event_secondb =     CalcTwhPartonHistory::secondb(truthParticles, 5, secondb_beforeFSR_p4, secondb_beforeFSR_pdgId, secondb_afterFSR_p4, secondb_afterFSR_pdgId);
    }
    if (event_top) {
      event_secondbbar =  CalcTwhPartonHistory::secondb(truthParticles, -5, secondb_beforeFSR_p4, secondb_beforeFSR_pdgId, secondb_afterFSR_p4, secondb_afterFSR_pdgId);
    }


    if (event_Higgs) {
      if ((event_top && event_secondbbar) || (event_topbar && event_secondb)) {
            //second b-quark
            TwhPartonHistory->auxdecor< float >("MC_secondb_beforeFSR_m") = secondb_beforeFSR_p4.M();
            TwhPartonHistory->auxdecor< float >("MC_secondb_beforeFSR_pt") = secondb_beforeFSR_p4.Pt();
            TwhPartonHistory->auxdecor< float >("MC_secondb_beforeFSR_phi") = secondb_beforeFSR_p4.Phi();
            TwhPartonHistory->auxdecor< int >("MC_secondb_beforeFSR_pdgId") = secondb_beforeFSR_pdgId;
            fillEtaBranch(TwhPartonHistory, "MC_secondb_beforeFSR_eta", secondb_beforeFSR_p4);
            
            TwhPartonHistory->auxdecor< float >("MC_secondb_afterFSR_m") = secondb_afterFSR_p4.M();
            TwhPartonHistory->auxdecor< float >("MC_secondb_afterFSR_pt") = secondb_afterFSR_p4.Pt();
            TwhPartonHistory->auxdecor< float >("MC_secondb_afterFSR_phi") = secondb_afterFSR_p4.Phi();
            TwhPartonHistory->auxdecor< int >("MC_secondb_afterFSR_pdgId") = secondb_afterFSR_pdgId;
            fillEtaBranch(TwhPartonHistory, "MC_secondb_afterFSR_eta", secondb_afterFSR_p4);      

            // top quark
            TwhPartonHistory->auxdecor< float >("MC_t_beforeFSR_m") = t_before.M();
            TwhPartonHistory->auxdecor< float >("MC_t_beforeFSR_pt") = t_before.Pt();
            TwhPartonHistory->auxdecor< float >("MC_t_beforeFSR_phi") = t_before.Phi();
            fillEtaBranch(TwhPartonHistory, "MC_t_beforeFSR_eta", t_before);

            TwhPartonHistory->auxdecor< float >("MC_t_afterFSR_m") = t_after.M();
            TwhPartonHistory->auxdecor< float >("MC_t_afterFSR_pt") = t_after.Pt();
            TwhPartonHistory->auxdecor< float >("MC_t_afterFSR_phi") = t_after.Phi();
            fillEtaBranch(TwhPartonHistory, "MC_t_afterFSR_eta", t_after);

            if (event_top_SC || event_topbar_SC) {
                TwhPartonHistory->auxdecor< float >("MC_t_afterFSR_SC_m") = t_after_SC.M();
                TwhPartonHistory->auxdecor< float >("MC_t_afterFSR_SC_pt") = t_after_SC.Pt();
                TwhPartonHistory->auxdecor< float >("MC_t_afterFSR_SC_phi") = t_after_SC.Phi();
                fillEtaBranch(TwhPartonHistory, "MC_t_afterFSR_SC_eta", t_after_SC);
            }

            // W Boson
            if (((event_top && !event_topbar) || (!event_top && event_topbar)) && event_WnotTop) {
                TwhPartonHistory->auxdecor< float >("MC_W_not_from_t_pt") = WnotFromTop.Pt();
                TwhPartonHistory->auxdecor< float >("MC_W_not_from_t_eta") = WnotFromTop.Eta();
                TwhPartonHistory->auxdecor< float >("MC_W_not_from_t_phi") = WnotFromTop.Phi();
                TwhPartonHistory->auxdecor< float >("MC_W_not_from_t_m") = WnotFromTop.M();

                TwhPartonHistory->auxdecor< float >("MC_Wdecay1_not_from_t_pt") = WnotFromTopDecay1.Pt();
                TwhPartonHistory->auxdecor< float >("MC_Wdecay1_not_from_t_eta") = WnotFromTopDecay1.Eta();
                TwhPartonHistory->auxdecor< float >("MC_Wdecay1_not_from_t_phi") = WnotFromTopDecay1.Phi();
                TwhPartonHistory->auxdecor< float >("MC_Wdecay1_not_from_t_m") = WnotFromTopDecay1.M();
                TwhPartonHistory->auxdecor< int >("MC_Wdecay1_not_from_t_pdgId") = WnotFromTopDecay1_pdgId;

                TwhPartonHistory->auxdecor< float >("MC_Wdecay2_not_from_t_pt") = WnotFromTopDecay2.Pt();
                TwhPartonHistory->auxdecor< float >("MC_Wdecay2_not_from_t_eta") = WnotFromTopDecay2.Eta();
                TwhPartonHistory->auxdecor< float >("MC_Wdecay2_not_from_t_phi") = WnotFromTopDecay2.Phi();
                TwhPartonHistory->auxdecor< float >("MC_Wdecay2_not_from_t_m") = WnotFromTopDecay2.M();
                TwhPartonHistory->auxdecor< int >("MC_Wdecay2_not_from_t_pdgId") = WnotFromTopDecay2_pdgId;
            }//if


            // tau decay system
            TwhPartonHistory->auxdecor< float >("MC_W_from_t_m") = Wp.M();
            TwhPartonHistory->auxdecor< float >("MC_W_from_t_pt") = Wp.Pt();
            TwhPartonHistory->auxdecor< float >("MC_W_from_t_phi") = Wp.Phi();
            fillEtaBranch(TwhPartonHistory, "MC_W_from_t_eta", Wp);

            TwhPartonHistory->auxdecor< float >("MC_b_from_t_m") = b.M();
            TwhPartonHistory->auxdecor< float >("MC_b_from_t_pt") = b.Pt();
            TwhPartonHistory->auxdecor< float >("MC_b_from_t_phi") = b.Phi();
            fillEtaBranch(TwhPartonHistory, "MC_b_from_t_eta", b);

            TwhPartonHistory->auxdecor< float >("MC_Wdecay1_from_t_m") = WpDecay1.M();
            TwhPartonHistory->auxdecor< float >("MC_Wdecay1_from_t_pt") = WpDecay1.Pt();
            TwhPartonHistory->auxdecor< float >("MC_Wdecay1_from_t_phi") = WpDecay1.Phi();
            TwhPartonHistory->auxdecor< int >("MC_Wdecay1_from_t_pdgId") = WpDecay1_pdgId;
            fillEtaBranch(TwhPartonHistory, "MC_Wdecay1_from_t_eta", WpDecay1);

            TwhPartonHistory->auxdecor< float >("MC_Wdecay2_from_t_m") = WpDecay2.M();
            TwhPartonHistory->auxdecor< float >("MC_Wdecay2_from_t_pt") = WpDecay2.Pt();
            TwhPartonHistory->auxdecor< float >("MC_Wdecay2_from_t_phi") = WpDecay2.Phi();
            TwhPartonHistory->auxdecor< int >("MC_Wdecay2_from_t_pdgId") = WpDecay2_pdgId;
            fillEtaBranch(TwhPartonHistory, "MC_Wdecay2_from_t_eta", WpDecay2);

            TwhPartonHistory->auxdecor< float >("MC_tau_from_W_from_t_m") = tau_decay_from_W_p4.M();
            TwhPartonHistory->auxdecor< float >("MC_tau_from_W_from_t_pt") = tau_decay_from_W_p4.Pt();
            TwhPartonHistory->auxdecor< float >("MC_tau_from_W_from_t_phi") = tau_decay_from_W_p4.Phi();
            TwhPartonHistory->auxdecor< int >("MC_tau_from_W_from_t_isHadronic") = tau_decay_from_W_isHadronic;
            fillEtaBranch(TwhPartonHistory, "MC_tau_from_W_from_t_eta", tau_decay_from_W_p4);

            TwhPartonHistory->auxdecor< float >("MC_tauvis_from_W_from_t_m") = tauvis_decay_from_W_p4.M();
            TwhPartonHistory->auxdecor< float >("MC_tauvis_from_W_from_t_pt") = tauvis_decay_from_W_p4.Pt();
            TwhPartonHistory->auxdecor< float >("MC_tauvis_from_W_from_t_phi") = tauvis_decay_from_W_p4.Phi();
            fillEtaBranch(TwhPartonHistory, "MC_tauvis_from_W_from_t_eta", tauvis_decay_from_W_p4);

            //Higgs-Variables
            TwhPartonHistory->auxdecor< float >("MC_Higgs_m") = tWH.Higgs_p4.M();
            TwhPartonHistory->auxdecor< float >("MC_Higgs_pt") = tWH.Higgs_p4.Pt();
            TwhPartonHistory->auxdecor< float >("MC_Higgs_phi") = tWH.Higgs_p4.Phi();
            fillEtaBranch(TwhPartonHistory, "MC_Higgs_eta", tWH.Higgs_p4);

            //Higgs-decay1-Variables
            TwhPartonHistory->auxdecor< float >("MC_Higgs_decay1_m") = tWH.decay1_p4.M();
            TwhPartonHistory->auxdecor< float >("MC_Higgs_decay1_pt") = tWH.decay1_p4.Pt();
            TwhPartonHistory->auxdecor< float >("MC_Higgs_decay1_phi") = tWH.decay1_p4.Phi();
            TwhPartonHistory->auxdecor< int >("MC_Higgs_decay1_pdgId") = tWH.decay1_pdgId;
            fillEtaBranch(TwhPartonHistory, "MC_Higgs_decay1_eta", tWH.decay1_p4);

            TwhPartonHistory->auxdecor< int >("MC_Higgs_tau_decay1_isHadronic") = tWH.tau_decay1_isHadronic;
            TwhPartonHistory->auxdecor< float >("MC_Higgs_tau_decay1_m") = tWH.tau_decay1_p4.M();
            TwhPartonHistory->auxdecor< float >("MC_Higgs_tau_decay1_pt") = tWH.tau_decay1_p4.Pt();
            TwhPartonHistory->auxdecor< float >("MC_Higgs_tau_decay1_phi") = tWH.tau_decay1_p4.Phi();
            fillEtaBranch(TwhPartonHistory, "MC_Higgs_tau_decay1_eta", tWH.tau_decay1_p4);

            TwhPartonHistory->auxdecor< float >("MC_Higgs_tauvis_decay1_m") = tWH.tauvis_decay1_p4.M();
            TwhPartonHistory->auxdecor< float >("MC_Higgs_tauvis_decay1_pt") = tWH.tauvis_decay1_p4.Pt();
            TwhPartonHistory->auxdecor< float >("MC_Higgs_tauvis_decay1_phi") = tWH.tauvis_decay1_p4.Phi();
            fillEtaBranch(TwhPartonHistory, "MC_Higgs_tauvis_decay1_eta", tWH.tauvis_decay1_p4);
                
            //Higgs-decay2-Variables
            TwhPartonHistory->auxdecor< float >("MC_Higgs_decay2_m") = tWH.decay2_p4.M();
            TwhPartonHistory->auxdecor< float >("MC_Higgs_decay2_pt") = tWH.decay2_p4.Pt();
            TwhPartonHistory->auxdecor< float >("MC_Higgs_decay2_phi") = tWH.decay2_p4.Phi();
            TwhPartonHistory->auxdecor< int >("MC_Higgs_decay2_pdgId") = tWH.decay2_pdgId;
            fillEtaBranch(TwhPartonHistory, "MC_Higgs_decay2_eta", tWH.decay2_p4);

            TwhPartonHistory->auxdecor< int >("MC_Higgs_tau_decay2_isHadronic") = tWH.tau_decay2_isHadronic;
            TwhPartonHistory->auxdecor< float >("MC_Higgs_tau_decay2_m") = tWH.tau_decay2_p4.M();
            TwhPartonHistory->auxdecor< float >("MC_Higgs_tau_decay2_pt") = tWH.tau_decay2_p4.Pt();
            TwhPartonHistory->auxdecor< float >("MC_Higgs_tau_decay2_phi") = tWH.tau_decay2_p4.Phi();
            fillEtaBranch(TwhPartonHistory, "MC_Higgs_tau_decay2_eta", tWH.tau_decay2_p4);	

            TwhPartonHistory->auxdecor< float >("MC_Higgs_tauvis_decay2_m") = tWH.tauvis_decay2_p4.M();
            TwhPartonHistory->auxdecor< float >("MC_Higgs_tauvis_decay2_pt") = tWH.tauvis_decay2_p4.Pt();
            TwhPartonHistory->auxdecor< float >("MC_Higgs_tauvis_decay2_phi") = tWH.tauvis_decay2_p4.Phi();
            fillEtaBranch(TwhPartonHistory, "MC_Higgs_tauvis_decay2_eta", tWH.tauvis_decay2_p4);
            
            //Higgs-decay1- from decay1-Variables
            TwhPartonHistory->auxdecor< float >("MC_Higgs_decay1_from_decay1_m") = tWH.decay1_from_decay1_p4.M();
            TwhPartonHistory->auxdecor< float >("MC_Higgs_decay1_from_decay1_pt") = tWH.decay1_from_decay1_p4.Pt();
            TwhPartonHistory->auxdecor< float >("MC_Higgs_decay1_from_decay1_phi") = tWH.decay1_from_decay1_p4.Phi();
            TwhPartonHistory->auxdecor< int >("MC_Higgs_decay1_from_decay1_pdgId") = tWH.decay1_from_decay1_pdgId;
            fillEtaBranch(TwhPartonHistory, "MC_Higgs_decay1_from_decay1_eta", tWH.decay1_from_decay1_p4);

            TwhPartonHistory->auxdecor< int >("MC_Higgs_tau_decay1_from_decay1_isHadronic") = tWH.tau_decay1_from_decay1_isHadronic;
            TwhPartonHistory->auxdecor< float >("MC_Higgs_tau_decay1_from_decay1_m") = tWH.tau_decay1_from_decay1_p4.M();
            TwhPartonHistory->auxdecor< float >("MC_Higgs_tau_decay1_from_decay1_pt") = tWH.tau_decay1_from_decay1_p4.Pt();
            TwhPartonHistory->auxdecor< float >("MC_Higgs_tau_decay1_from_decay1_phi") = tWH.tau_decay1_from_decay1_p4.Phi();
            fillEtaBranch(TwhPartonHistory, "MC_Higgs_tau_decay1_from_decay1_eta", tWH.tau_decay1_from_decay1_p4);

            TwhPartonHistory->auxdecor< float >("MC_Higgs_tauvis_decay1_from_decay1_m") = tWH.tauvis_decay1_from_decay1_p4.M();
            TwhPartonHistory->auxdecor< float >("MC_Higgs_tauvis_decay1_from_decay1_pt") = tWH.tauvis_decay1_from_decay1_p4.Pt();
            TwhPartonHistory->auxdecor< float >("MC_Higgs_tauvis_decay1_from_decay1_phi") = tWH.tauvis_decay1_from_decay1_p4.Phi();
            fillEtaBranch(TwhPartonHistory, "MC_Higgs_tauvis_decay1_from_decay1_eta", tWH.tauvis_decay1_from_decay1_p4);
            
            //Higgs-decay2- from decay1-Variables
            TwhPartonHistory->auxdecor< float >("MC_Higgs_decay2_from_decay1_m") = tWH.decay2_from_decay1_p4.M();
            TwhPartonHistory->auxdecor< float >("MC_Higgs_decay2_from_decay1_pt") = tWH.decay2_from_decay1_p4.Pt();
            TwhPartonHistory->auxdecor< float >("MC_Higgs_decay2_from_decay1_phi") = tWH.decay2_from_decay1_p4.Phi();
            TwhPartonHistory->auxdecor< int >("MC_Higgs_decay2_from_decay1_pdgId") = tWH.decay2_from_decay1_pdgId;
            fillEtaBranch(TwhPartonHistory, "MC_Higgs_decay2_from_decay1_eta", tWH.decay2_from_decay1_p4);



            TwhPartonHistory->auxdecor< int >("MC_Higgs_tau_decay2_from_decay1_isHadronic") = tWH.tau_decay2_from_decay1_isHadronic;
            TwhPartonHistory->auxdecor< float >("MC_Higgs_tau_decay2_from_decay1_m") = tWH.tau_decay2_from_decay1_p4.M();
            TwhPartonHistory->auxdecor< float >("MC_Higgs_tau_decay2_from_decay1_pt") = tWH.tau_decay2_from_decay1_p4.Pt();
            TwhPartonHistory->auxdecor< float >("MC_Higgs_tau_decay2_from_decay1_phi") = tWH.tau_decay2_from_decay1_p4.Phi();
            fillEtaBranch(TwhPartonHistory, "MC_Higgs_tau_decay2_from_decay1_eta", tWH.tau_decay2_from_decay1_p4);	

            TwhPartonHistory->auxdecor< float >("MC_Higgs_tauvis_decay2_from_decay1_m") = tWH.tauvis_decay2_from_decay1_p4.M();
            TwhPartonHistory->auxdecor< float >("MC_Higgs_tauvis_decay2_from_decay1_pt") = tWH.tauvis_decay2_from_decay1_p4.Pt();
            TwhPartonHistory->auxdecor< float >("MC_Higgs_tauvis_decay2_from_decay1_phi") = tWH.tauvis_decay2_from_decay1_p4.Phi();
            fillEtaBranch(TwhPartonHistory, "MC_Higgs_tauvis_decay2_from_decay1_eta", tWH.tauvis_decay2_from_decay1_p4);


            //Higgs-decay1- from decay2-Variables
            TwhPartonHistory->auxdecor< float >("MC_Higgs_decay1_from_decay2_m") = tWH.decay1_from_decay2_p4.M();
            TwhPartonHistory->auxdecor< float >("MC_Higgs_decay1_from_decay2_pt") = tWH.decay1_from_decay2_p4.Pt();
            TwhPartonHistory->auxdecor< float >("MC_Higgs_decay1_from_decay2_phi") = tWH.decay1_from_decay2_p4.Phi();
            TwhPartonHistory->auxdecor< int >("MC_Higgs_decay1_from_decay2_pdgId") = tWH.decay1_from_decay2_pdgId;
            fillEtaBranch(TwhPartonHistory, "MC_Higgs_decay1_from_decay2_eta", tWH.decay1_from_decay2_p4);

            TwhPartonHistory->auxdecor< int >("MC_Higgs_tau_decay1_from_decay2_isHadronic") = tWH.tau_decay1_from_decay2_isHadronic;
            TwhPartonHistory->auxdecor< float >("MC_Higgs_tau_decay1_from_decay2_m") = tWH.tau_decay1_from_decay2_p4.M();
            TwhPartonHistory->auxdecor< float >("MC_Higgs_tau_decay1_from_decay2_pt") = tWH.tau_decay1_from_decay2_p4.Pt();
            TwhPartonHistory->auxdecor< float >("MC_Higgs_tau_decay1_from_decay2_phi") = tWH.tau_decay1_from_decay2_p4.Phi();
            fillEtaBranch(TwhPartonHistory, "MC_Higgs_tau_decay1_from_decay2_eta", tWH.tau_decay1_from_decay2_p4);

            TwhPartonHistory->auxdecor< float >("MC_Higgs_tauvis_decay1_from_decay2_m") = tWH.tauvis_decay1_from_decay2_p4.M();
            TwhPartonHistory->auxdecor< float >("MC_Higgs_tauvis_decay1_from_decay2_pt") = tWH.tauvis_decay1_from_decay2_p4.Pt();
            TwhPartonHistory->auxdecor< float >("MC_Higgs_tauvis_decay1_from_decay2_phi") = tWH.tauvis_decay1_from_decay2_p4.Phi();
            fillEtaBranch(TwhPartonHistory, "MC_Higgs_tauvis_decay1_from_decay2_eta", tWH.tauvis_decay1_from_decay2_p4);

            //Higgs-decay2- from decay2-Variables
            TwhPartonHistory->auxdecor< float >("MC_Higgs_decay2_from_decay2_m") = tWH.decay2_from_decay2_p4.M();
            TwhPartonHistory->auxdecor< float >("MC_Higgs_decay2_from_decay2_pt") = tWH.decay2_from_decay2_p4.Pt();
            TwhPartonHistory->auxdecor< float >("MC_Higgs_decay2_from_decay2_phi") = tWH.decay2_from_decay2_p4.Phi();
            TwhPartonHistory->auxdecor< int >("MC_Higgs_decay2_from_decay2_pdgId") = tWH.decay2_from_decay2_pdgId;
            fillEtaBranch(TwhPartonHistory, "MC_Higgs_decay2_from_decay2_eta", tWH.decay2_from_decay2_p4);

            TwhPartonHistory->auxdecor< int >("MC_Higgs_tau_decay2_from_decay2_isHadronic") = tWH.tau_decay2_from_decay2_isHadronic;
            TwhPartonHistory->auxdecor< float >("MC_Higgs_tau_decay2_from_decay2_m") = tWH.tau_decay2_from_decay2_p4.M();
            TwhPartonHistory->auxdecor< float >("MC_Higgs_tau_decay2_from_decay2_pt") = tWH.tau_decay2_from_decay2_p4.Pt();
            TwhPartonHistory->auxdecor< float >("MC_Higgs_tau_decay2_from_decay2_phi") = tWH.tau_decay2_from_decay2_p4.Phi();
            fillEtaBranch(TwhPartonHistory, "MC_Higgs_tau_decay2_from_decay2_eta", tWH.tau_decay2_from_decay2_p4);

            TwhPartonHistory->auxdecor< float >("MC_Higgs_tauvis_decay2_from_decay2_m") = tWH.tauvis_decay2_from_decay2_p4.M();
            TwhPartonHistory->auxdecor< float >("MC_Higgs_tauvis_decay2_from_decay2_pt") = tWH.tauvis_decay2_from_decay2_p4.Pt();
            TwhPartonHistory->auxdecor< float >("MC_Higgs_tauvis_decay2_from_decay2_phi") = tWH.tauvis_decay2_from_decay2_p4.Phi();
            fillEtaBranch(TwhPartonHistory, "MC_Higgs_tauvis_decay2_from_decay2_eta", tWH.tauvis_decay2_from_decay2_p4);
      } // a top and a second b  req
    } // a higgs req
  } // end of function


  bool CalcTwhPartonHistory::secondb(const xAOD::TruthParticleContainer* truthParticles, int start, 
				     TLorentzVector& secondb_beforeFSR_p4, int& secondb_beforeFSR_pdgId,
				     TLorentzVector& secondb_afterFSR_p4, int& secondb_afterFSR_pdgId) {
    for (const xAOD::TruthParticle* particle : *truthParticles) {
      if (particle->pdgId() != start) { continue; } // start = +- 6
      for (size_t i=0; i< particle->nParents(); i++) {
	const xAOD::TruthParticle* parent = particle->parent(i);
	if (parent->pdgId() == 21) { //second b is originated from gluon spliting
	  const xAOD::TruthParticle* secondb_beforeFSR = particle;
          secondb_beforeFSR_p4 = secondb_beforeFSR->p4();
	  secondb_beforeFSR_pdgId = secondb_beforeFSR->pdgId();

          const xAOD::TruthParticle* secondb_afterFSR = PartonHistoryUtils::findAfterFSR(particle);
          secondb_afterFSR_p4 = secondb_afterFSR->p4();
	  secondb_afterFSR_pdgId = secondb_afterFSR->pdgId();

	  return true;
	} // if parent is gluon
      } // for iparents
    }//for
    return false;
  }

  StatusCode CalcTwhPartonHistory::execute() {
    //Get the Truth Particles
    const xAOD::TruthParticleContainer* truthParticles(nullptr);

    ATH_CHECK(evtStore()->retrieve(truthParticles, m_config->sgKeyMCParticle()));

    // Create the partonHistory xAOD object
    xAOD::PartonHistoryAuxContainer* partonAuxCont = new xAOD::PartonHistoryAuxContainer {};
    xAOD::PartonHistoryContainer* partonCont = new xAOD::PartonHistoryContainer {};
    partonCont->setStore(partonAuxCont);

    xAOD::PartonHistory* TwhPartonHistory = new xAOD::PartonHistory {};
    partonCont->push_back(TwhPartonHistory);

    // Recover the parton history for TH events
    THHistorySaver(truthParticles, TwhPartonHistory);

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
