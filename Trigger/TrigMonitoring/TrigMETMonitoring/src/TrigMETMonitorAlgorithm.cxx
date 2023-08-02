/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#include "LArRecEvent/LArEventBitInfo.h"
#include "TrigMETMonitorAlgorithm.h"
#include <TVector3.h>
#include <cmath>

TrigMETMonitorAlgorithm::TrigMETMonitorAlgorithm( const std::string& name, ISvcLocator* pSvcLocator )
  : AthMonitorAlgorithm(name,pSvcLocator)
{
}


TrigMETMonitorAlgorithm::~TrigMETMonitorAlgorithm() {}


StatusCode TrigMETMonitorAlgorithm::initialize() {
    ATH_CHECK( m_EventInfoKey.initialize() );
    ATH_CHECK( m_offline_met_key.initialize() );
    ATH_CHECK( m_hlt_electron_key.initialize() );
    ATH_CHECK( m_hlt_muon_key.initialize() );
    ATH_CHECK( m_topoclusters_key.initialize() );
    ATH_CHECK( m_tracks_key.initialize() );
    ATH_CHECK( m_vertex_key.initialize() );
    ATH_CHECK( m_offline_vertex_key.initialize() );
    ATH_CHECK( m_lvl1_roi_key.initialize() );
    ATH_CHECK( m_l1_jFexMet_key.initialize() );
    ATH_CHECK( m_l1_jFexSumEt_key.initialize() );
    ATH_CHECK( m_l1_gFexJwojScalar_key.initialize() );
    ATH_CHECK( m_l1_gFexJwojMETComponents_key.initialize() );
    ATH_CHECK( m_l1_gFexJwojMHTComponents_key.initialize() );
    ATH_CHECK( m_l1_gFexJwojMSTComponents_key.initialize() );
    ATH_CHECK( m_l1_gFexNCMETScalar_key.initialize() );
    ATH_CHECK( m_l1_gFexNCMETComponents_key.initialize() );
    ATH_CHECK( m_l1_gFexRhoMETScalar_key.initialize() );
    ATH_CHECK( m_l1_gFexRhoMETComponents_key.initialize() );
    ATH_CHECK( m_hlt_cell_met_key.initialize() );
    ATH_CHECK( m_hlt_mht_met_key.initialize() );
    ATH_CHECK( m_hlt_tc_met_key.initialize() );
    ATH_CHECK( m_hlt_tc_em_met_key.initialize() );
    ATH_CHECK( m_hlt_tcpufit_met_key.initialize() );
    ATH_CHECK( m_hlt_tcpufit_sig30_met_key.initialize() );
    ATH_CHECK( m_hlt_trkmht_met_key.initialize() );
    ATH_CHECK( m_hlt_pfsum_met_key.initialize() );
    ATH_CHECK( m_hlt_pfopufit_met_key.initialize() );
    ATH_CHECK( m_hlt_pfopufit_sig30_met_key.initialize() );
    ATH_CHECK( m_hlt_cvfpufit_met_key.initialize() );
    ATH_CHECK( m_hlt_mhtpufit_pf_met_key.initialize() );
    ATH_CHECK( m_hlt_mhtpufit_em_met_key.initialize() );
    ATH_CHECK( m_hlt_met_nn_key.initialize() );
    ATH_CHECK( m_hlt_pfsum_cssk_met_key.initialize() );
    ATH_CHECK( m_hlt_pfsum_vssk_met_key.initialize() );

    return AthMonitorAlgorithm::initialize();
}


StatusCode TrigMETMonitorAlgorithm::fillHistograms( const EventContext& ctx ) const {
    using namespace Monitored;

    // access event info container
    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_EventInfoKey, ctx);
    if (! eventInfo.isValid() ){
        ATH_MSG_DEBUG("Container "<< eventInfo << " does not exist");
    }

    // access lepton containers
    SG::ReadHandle<xAOD::ElectronContainer> hlt_electron_cont(m_hlt_electron_key, ctx);
    if (! hlt_electron_cont.isValid() ) {
        ATH_MSG_DEBUG("Container "<< m_hlt_electron_key << " does not exist");
    }

    SG::ReadHandle<xAOD::MuonContainer> hlt_muon_cont(m_hlt_muon_key, ctx);
    if (! hlt_muon_cont.isValid() ) {
        ATH_MSG_DEBUG("Container "<< m_hlt_muon_key << " does not exist");
    }

    // access topoclusters container
    SG::ReadHandle<xAOD::CaloClusterContainer> hlt_topoclusters_cont(m_topoclusters_key, ctx);
    if (! hlt_topoclusters_cont.isValid() ) {
        ATH_MSG_DEBUG("Container "<< m_topoclusters_key << " does not exist");
    }

    // access tracks container
    SG::ReadHandle<xAOD::TrackParticleContainer> hlt_tracks_cont(m_tracks_key, ctx);
    if (! hlt_tracks_cont.isValid() ) {
        ATH_MSG_DEBUG("Container "<< m_tracks_key << " does not exist");
    }

    // access vertex container
    SG::ReadHandle<xAOD::VertexContainer> hlt_vertex_cont(m_vertex_key, ctx);
    if (! hlt_vertex_cont.isValid() ) {
        ATH_MSG_DEBUG("Container "<< m_vertex_key << " does not exist");
    }
    
    SG::ReadHandle<xAOD::VertexContainer> offline_vertex_cont(m_offline_vertex_key, ctx);
    if (! offline_vertex_cont.isValid() ) {
        ATH_MSG_DEBUG("Container "<< m_offline_vertex_key << " does not exist");
    }

    // access offline met containers
    SG::ReadHandle<xAOD::MissingETContainer> offline_met_cont(m_offline_met_key, ctx);
    if (! offline_met_cont.isValid() ) {
        ATH_MSG_DEBUG("Container "<< m_offline_met_key << " does not exist");
    }

    // access L1 met containers
    SG::ReadHandle<xAOD::EnergySumRoI> l1_roi_cont(m_lvl1_roi_key, ctx);
    if (! l1_roi_cont.isValid() ) {
        ATH_MSG_DEBUG("Container "<< m_lvl1_roi_key << " does not exist");
    }

    // access L1 Fex met containers
    SG::ReadHandle<xAOD::jFexMETRoIContainer> l1_jFexMet_cont(m_l1_jFexMet_key, ctx);
    if (! l1_jFexMet_cont.isValid() ) {
        ATH_MSG_DEBUG("Container "<< m_l1_jFexMet_key << " does not exist");
    }
    SG::ReadHandle<xAOD::jFexSumETRoIContainer> l1_jFexSumEt_cont(m_l1_jFexSumEt_key, ctx);
    if (! l1_jFexSumEt_cont.isValid() ) {
        ATH_MSG_DEBUG("Container "<< m_l1_jFexSumEt_key << " does not exist");
    }
    SG::ReadHandle<xAOD::gFexGlobalRoIContainer> l1_gFexJwojScalar_cont(m_l1_gFexJwojScalar_key, ctx);
    if (! l1_gFexJwojScalar_cont.isValid() ) {
        ATH_MSG_DEBUG("Container "<< m_l1_gFexJwojScalar_key << " does not exist");
    }
    SG::ReadHandle<xAOD::gFexGlobalRoIContainer> l1_gFexJwojMETComponents_cont(m_l1_gFexJwojMETComponents_key, ctx);
    if (! l1_gFexJwojMETComponents_cont.isValid() ) {
        ATH_MSG_DEBUG("Container "<< m_l1_gFexJwojMETComponents_key << " does not exist");
    }
    SG::ReadHandle<xAOD::gFexGlobalRoIContainer> l1_gFexJwojMHTComponents_cont(m_l1_gFexJwojMHTComponents_key, ctx);
    if (! l1_gFexJwojMHTComponents_cont.isValid() ) {
        ATH_MSG_DEBUG("Container "<< m_l1_gFexJwojMHTComponents_key << " does not exist");
    }
    SG::ReadHandle<xAOD::gFexGlobalRoIContainer> l1_gFexJwojMSTComponents_cont(m_l1_gFexJwojMSTComponents_key, ctx);
    if (! l1_gFexJwojMSTComponents_cont.isValid() ) {
        ATH_MSG_DEBUG("Container "<< m_l1_gFexJwojMSTComponents_key << " does not exist");
    }
    SG::ReadHandle<xAOD::gFexGlobalRoIContainer> l1_gFexNCMETScalar_cont(m_l1_gFexNCMETScalar_key, ctx);
    if (! l1_gFexNCMETScalar_cont.isValid() ) {
        ATH_MSG_DEBUG("Container "<< m_l1_gFexNCMETScalar_key << " does not exist");
    }
    SG::ReadHandle<xAOD::gFexGlobalRoIContainer> l1_gFexNCMETComponents_cont(m_l1_gFexNCMETComponents_key, ctx);
    if (! l1_gFexNCMETComponents_cont.isValid() ) {
        ATH_MSG_DEBUG("Container "<< m_l1_gFexNCMETComponents_key << " does not exist");
    }
    SG::ReadHandle<xAOD::gFexGlobalRoIContainer> l1_gFexRhoMETScalar_cont(m_l1_gFexRhoMETScalar_key, ctx);
    if (! l1_gFexRhoMETScalar_cont.isValid() ) {
        ATH_MSG_DEBUG("Container "<< m_l1_gFexRhoMETScalar_key << " does not exist");
    }
    SG::ReadHandle<xAOD::gFexGlobalRoIContainer> l1_gFexRhoMETComponents_cont(m_l1_gFexRhoMETComponents_key, ctx);
    if (! l1_gFexRhoMETComponents_cont.isValid() ) {
        ATH_MSG_DEBUG("Container "<< m_l1_gFexRhoMETComponents_key << " does not exist");
    }

    // access HLT met containers
    SG::ReadHandle<xAOD::TrigMissingETContainer> hlt_cell_met_cont(m_hlt_cell_met_key, ctx);
    if (! hlt_cell_met_cont.isValid() ) {
        ATH_MSG_DEBUG("Container "<< m_hlt_cell_met_key << " does not exist");
    }

    SG::ReadHandle<xAOD::TrigMissingETContainer> hlt_mht_met_cont(m_hlt_mht_met_key, ctx);
    if (! hlt_mht_met_cont.isValid() ) {
        ATH_MSG_DEBUG("Container "<< m_hlt_mht_met_key << " does not exist");
    }

    SG::ReadHandle<xAOD::TrigMissingETContainer> hlt_tc_met_cont(m_hlt_tc_met_key, ctx);
    if (! hlt_tc_met_cont.isValid() ) {
        ATH_MSG_DEBUG("Container "<< m_hlt_tc_met_key << " does not exist");
    }

    SG::ReadHandle<xAOD::TrigMissingETContainer> hlt_tc_em_met_cont(m_hlt_tc_em_met_key, ctx);
    if (! hlt_tc_em_met_cont.isValid() ) {
        ATH_MSG_DEBUG("Container "<< m_hlt_tc_em_met_key << " does not exist");
    }

    SG::ReadHandle<xAOD::TrigMissingETContainer> hlt_tcpufit_met_cont(m_hlt_tcpufit_met_key, ctx);
    if (! hlt_tcpufit_met_cont.isValid() ) {
        ATH_MSG_DEBUG("Container "<< m_hlt_tcpufit_met_key << " does not exist");
    }
    
    SG::ReadHandle<xAOD::TrigMissingETContainer> hlt_tcpufit_sig30_met_cont(m_hlt_tcpufit_sig30_met_key, ctx);
    if (! hlt_tcpufit_sig30_met_cont.isValid() ) {
        ATH_MSG_DEBUG("Container "<< m_hlt_tcpufit_sig30_met_key << " does not exist");
    }

    SG::ReadHandle<xAOD::TrigMissingETContainer> hlt_trkmht_met_cont(m_hlt_trkmht_met_key, ctx);
    if (! hlt_trkmht_met_cont.isValid() ) {
        ATH_MSG_DEBUG("Container "<< m_hlt_trkmht_met_key << " does not exist");
    }

    SG::ReadHandle<xAOD::TrigMissingETContainer> hlt_pfsum_met_cont(m_hlt_pfsum_met_key, ctx);
    if (! hlt_pfsum_met_cont.isValid() ) {
        ATH_MSG_DEBUG("Container "<< m_hlt_pfsum_met_key << " does not exist");
    }

    SG::ReadHandle<xAOD::TrigMissingETContainer> hlt_pfsum_cssk_met_cont(m_hlt_pfsum_cssk_met_key, ctx);
    if (! hlt_pfsum_cssk_met_cont.isValid() ) {
        ATH_MSG_DEBUG("Container "<< m_hlt_pfsum_cssk_met_key << " does not exist");
    }

    SG::ReadHandle<xAOD::TrigMissingETContainer> hlt_pfsum_vssk_met_cont(m_hlt_pfsum_vssk_met_key, ctx);
    if (! hlt_pfsum_vssk_met_cont.isValid() ) {
        ATH_MSG_DEBUG("Container "<< m_hlt_pfsum_vssk_met_key << " does not exist");
    }

    SG::ReadHandle<xAOD::TrigMissingETContainer> hlt_pfopufit_met_cont(m_hlt_pfopufit_met_key, ctx);
    if (! hlt_pfopufit_met_cont.isValid() ) {
        ATH_MSG_DEBUG("Container "<< m_hlt_pfopufit_met_key << " does not exist");
    }
    
    SG::ReadHandle<xAOD::TrigMissingETContainer> hlt_pfopufit_sig30_met_cont(m_hlt_pfopufit_sig30_met_key, ctx);
    if (! hlt_pfopufit_sig30_met_cont.isValid() ) {
        ATH_MSG_DEBUG("Container "<< m_hlt_pfopufit_sig30_met_key << " does not exist");
    }

    SG::ReadHandle<xAOD::TrigMissingETContainer> hlt_cvfpufit_met_cont(m_hlt_cvfpufit_met_key, ctx);
    if (! hlt_cvfpufit_met_cont.isValid() ) {
        ATH_MSG_DEBUG("Container "<< m_hlt_cvfpufit_met_key << " does not exist");
    }

    SG::ReadHandle<xAOD::TrigMissingETContainer> hlt_mhtpufit_pf_met_cont(m_hlt_mhtpufit_pf_met_key, ctx);
    if (! hlt_mhtpufit_pf_met_cont.isValid() ) {
        ATH_MSG_DEBUG("Container "<< m_hlt_mhtpufit_pf_met_key << " does not exist");
    }

    SG::ReadHandle<xAOD::TrigMissingETContainer> hlt_mhtpufit_em_met_cont(m_hlt_mhtpufit_em_met_key, ctx);
    if (! hlt_mhtpufit_em_met_cont.isValid() ) {
        ATH_MSG_DEBUG("Container "<< m_hlt_mhtpufit_em_met_key << " does not exist");
    }
    
    SG::ReadHandle<xAOD::TrigMissingETContainer> hlt_met_nn_cont(m_hlt_met_nn_key, ctx);
    if (! hlt_met_nn_cont.isValid() ) {
        ATH_MSG_DEBUG("Container "<< m_hlt_met_nn_key << " does not exist");
    }

    // define variables
    auto act_IPBC = Monitored::Scalar<float>("act_IPBC",0.0);
    auto hlt_el_mult = Monitored::Scalar<int>("hlt_el_mult",0.0);
    auto hlt_el_pt = Monitored::Scalar<float>("hlt_el_pt",0.0);
    auto hlt_mu_mult = Monitored::Scalar<int>("hlt_mu_mult",0.0);
    auto hlt_mu_pt = Monitored::Scalar<float>("hlt_mu_pt",0.0);

    auto hlt_topoclusters_mult = Monitored::Scalar<int>("hlt_topoclusters_mult",0.0);
    auto hlt_topoclusters_pt = Monitored::Scalar<float>("hlt_topoclusters_pt",0.0);
    auto hlt_tracks_mult = Monitored::Scalar<int>("hlt_tracks_mult",0.0);
    auto hlt_tracks_pt = Monitored::Scalar<float>("hlt_tracks_pt",0.0);
    auto hlt_tracks_phi = Monitored::Scalar<float>("hlt_tracks_phi",0.0);
    auto hlt_tracks_eta = Monitored::Scalar<float>("hlt_tracks_eta",0.0);
    auto hlt_tracks_leading_pt = Monitored::Scalar<float>("hlt_tracks_leading_pt",0.0);
    auto hlt_tracks_vec_sumPt = Monitored::Scalar<float>("hlt_tracks_vec_sumPt",0.0);
    auto hlt_tracks_sca_sumPt = Monitored::Scalar<float>("hlt_tracks_sca_sumPt",0.0);
    auto hlt_vertex_mult = Monitored::Scalar<int>("hlt_vertex_mult",0.0);
    auto hlt_vertex_mult_mu = Monitored::Scalar<int>("hlt_vertex_mult_mu",0.0);
    auto hlt_vertex_z = Monitored::Scalar<float>("hlt_vertex_z",0.0);
    auto hlt_vertex_z_diff = Monitored::Scalar<float>("hlt_vertex_z_diff",0.0);

    auto offline_Ex = Monitored::Scalar<float>("offline_Ex",0.0);
    auto offline_Ey = Monitored::Scalar<float>("offline_Ey",0.0);
    auto offline_Et = Monitored::Scalar<float>("offline_Et",0.0);
    auto offline_sumEt = Monitored::Scalar<float>("offline_sumEt",0.0);
    auto offline_Et_eff = Monitored::Scalar<float>("offline_Et_eff",0.0); 
    auto offline_NoMu_Ex = Monitored::Scalar<float>("offline_NoMu_Ex",0.0);
    auto offline_NoMu_Ey = Monitored::Scalar<float>("offline_NoMu_Ey",0.0);
    auto offline_NoMu_Et = Monitored::Scalar<float>("offline_NoMu_Et",0.0);
    auto offline_NoMu_sumEt = Monitored::Scalar<float>("offline_NoMu_sumEt",0.0);
    auto offline_NoMu_Et_eff = Monitored::Scalar<float>("offline_NoMu_Et_eff",0.0);

    auto HLT_MET_status = Monitored::Scalar<int>("HLT_MET_status",0.0);
    auto MET_status = Monitored::Scalar<float>("MET_status",0.0);
    auto HLT_MET_component = Monitored::Scalar<int>("HLT_MET_component",0.0);
    auto component_Et = Monitored::Scalar<float>("component_Et",0.0);
    auto component = Monitored::Scalar<int>("component",0.0);
    auto component_status = Monitored::Scalar<int>("component_status",0.0);
    auto component_status_weight = Monitored::Scalar<int>("component_status_weight",0.0);

    // constant floor for log plots
    double epsilon = 1.189;

    // for histogram filling
    // Fill. First argument is the tool (GMT) name as defined in the py file,
    // all others are the variables to be saved.
    auto tool = getGroup("TrigMETMonitor");

    // access pileup <mu>
    act_IPBC = eventInfo->actualInteractionsPerCrossing();

    // access lepton values
    // access events with electron passing primary single electron chain
    bool passedPrimaryEl = false;
    for (const std::string& chain : m_hltChainEl){ 
      if(getTrigDecisionTool()->isPassed(chain)){
        passedPrimaryEl = true;
        break;
      }
    }
    if ( hlt_electron_cont.isValid() && passedPrimaryEl ){
      hlt_el_mult = hlt_electron_cont->size();
      fill(tool,hlt_el_mult);
      if( hlt_electron_cont->size() > 0 ) {
        for (auto Electron: *hlt_electron_cont) {
          hlt_el_pt = Electron->pt()/Gaudi::Units::GeV;
          fill(tool, hlt_el_pt);
        }
      }
    }

    // access events with muon passing primary single muon chain
    bool passedPrimaryMu = false;
    for (const std::string& chain : m_hltChainMu){ 
      if(getTrigDecisionTool()->isPassed(chain)){
        passedPrimaryMu = true;
        break;
      }
    }
    if( hlt_muon_cont.isValid() &&passedPrimaryMu ){
      hlt_mu_mult = hlt_muon_cont->size();
      fill(tool,hlt_mu_mult);
      if ( hlt_muon_cont->size() > 0 ){
        for(auto Muon : *hlt_muon_cont){
          hlt_mu_pt = Muon->pt()/Gaudi::Units::GeV;
          fill(tool, hlt_mu_pt);
        }
      }
    }

    // access topoclusters container
    if(hlt_topoclusters_cont.isValid() && hlt_topoclusters_cont->size() > 0){
      hlt_topoclusters_mult = hlt_topoclusters_cont->size();
      for(auto topoclusters : *hlt_topoclusters_cont){
        hlt_topoclusters_pt = topoclusters->pt()/Gaudi::Units::GeV;
        if(hlt_topoclusters_pt > 0){
          fill(tool, hlt_topoclusters_pt);
        }
      }

      if(hlt_topoclusters_mult > 0){
        fill(tool, hlt_topoclusters_mult);
      }
    }

    // access tracks container
    if( hlt_tracks_cont.isValid() && hlt_tracks_cont->size() > 0){
      hlt_tracks_mult = hlt_tracks_cont->size();
      float scalarSumPt = 0.0;
      float scalarSumPx = 0.0;
      float scalarSumPy = 0.0;
      for(auto tracks : *hlt_tracks_cont){
        float i_track_pt = tracks->pt()/Gaudi::Units::GeV;
        hlt_tracks_pt = i_track_pt;
        scalarSumPt += i_track_pt;
        scalarSumPx += (tracks->p4().Px());
        scalarSumPy += (tracks->p4().Py());

        if(hlt_tracks_pt > hlt_tracks_leading_pt){
          hlt_tracks_leading_pt = i_track_pt;
        }

        if(hlt_tracks_pt > 0){
          fill(tool, hlt_tracks_pt);
        }

        if(hlt_tracks_pt > 3){
          hlt_tracks_eta = (tracks->eta());
          hlt_tracks_phi = (tracks->phi());	  
          fill(tool, hlt_tracks_eta, hlt_tracks_phi);
        }
      }
      
      hlt_tracks_vec_sumPt = std::sqrt(scalarSumPx*scalarSumPx + scalarSumPy*scalarSumPy)/Gaudi::Units::GeV;
      hlt_tracks_sca_sumPt = scalarSumPt;

      fill(tool, hlt_tracks_mult, hlt_tracks_leading_pt, hlt_tracks_vec_sumPt, hlt_tracks_sca_sumPt);
    }

    // access vertex container
    if(hlt_vertex_cont.isValid() && hlt_vertex_cont->size() > 0){
      hlt_vertex_mult = hlt_vertex_cont->size();
      hlt_vertex_mult_mu = hlt_vertex_cont->size();
      const xAOD::Vertex_v1* hlt_vertex = nullptr;
      for(auto vertex : *hlt_vertex_cont){
        if(vertex->vertexType() == xAOD::VxType::VertexType::PriVtx){
          hlt_vertex = vertex; 
          break;
        } 
      }
 
      if(hlt_vertex){
        hlt_vertex_z = hlt_vertex->z();
        fill(tool, hlt_vertex_z); 
  
        if(offline_vertex_cont.isValid() && offline_vertex_cont->size() > 0){
          const xAOD::Vertex_v1* offline_vertex = nullptr;
          for(auto vertex : *offline_vertex_cont){
            if(vertex->vertexType() == xAOD::VxType::VertexType::PriVtx){
              offline_vertex = vertex;
              break;
            } 
          }
 
          if(offline_vertex){
            hlt_vertex_z_diff = hlt_vertex_z - offline_vertex->z();
            fill(tool, hlt_vertex_z_diff);
          }
        }
      }
    }else{
      hlt_vertex_mult = -1;
      hlt_vertex_mult_mu = 1;
      act_IPBC = -1.;
    }
    fill(tool, hlt_vertex_mult);
    fill(tool, act_IPBC, hlt_vertex_mult_mu);

    // access offline MET values
    const xAOD::MissingET *finalTrkMET = 0;
    const xAOD::MissingET *muonsMET = 0;
    if ( offline_met_cont.isValid() && offline_met_cont->size() > 0 ) {
      finalTrkMET = ((*offline_met_cont)["FinalTrk"]);
      muonsMET = ((*offline_met_cont)["Muons"]);

      if(finalTrkMET) {
        offline_Ex = - finalTrkMET->mpx()/Gaudi::Units::GeV;
        offline_Ey = - finalTrkMET->mpy()/Gaudi::Units::GeV;
        offline_sumEt = finalTrkMET->sumet()/Gaudi::Units::GeV;
        offline_Et = std::sqrt(offline_Ex*offline_Ex + offline_Ey*offline_Ey);
        offline_Et_eff = std::sqrt(offline_Ex*offline_Ex + offline_Ey*offline_Ey);
        fill(tool,offline_Ex,offline_Ey,offline_Et,offline_sumEt);

        if(muonsMET){
          xAOD::MissingET finalTrkNoMuMET = *finalTrkMET - *muonsMET;
          offline_NoMu_Ex = - finalTrkNoMuMET.mpx()/Gaudi::Units::GeV;
          offline_NoMu_Ey = - finalTrkNoMuMET.mpy()/Gaudi::Units::GeV;
          offline_NoMu_sumEt = finalTrkNoMuMET.sumet()/Gaudi::Units::GeV;
          offline_NoMu_Et = std::sqrt(offline_NoMu_Ex*offline_NoMu_Ex + offline_NoMu_Ey*offline_NoMu_Ey);
          offline_NoMu_Et_eff = std::sqrt(offline_NoMu_Ex*offline_NoMu_Ex + offline_NoMu_Ey*offline_NoMu_Ey);
          fill(tool,offline_NoMu_Ex,offline_NoMu_Ey,offline_NoMu_Et,offline_NoMu_sumEt);
        }
      }
    }

    // access L1 MET values
    for (const std::string& alg : m_algsL1) {
      SG::ReadHandle<xAOD::EnergySumRoI> l1_met_cont;
      if (alg == "roi" && l1_roi_cont.isValid()) {
        l1_met_cont = l1_roi_cont;
      }

      if ( l1_met_cont.isValid() ) {
        if ((l1_met_cont->energyX())>-9e12 && (l1_met_cont->energyX())<9e12 && (l1_met_cont->energyY())>-9e12 && (l1_met_cont->energyY())<9e12) {
          float L1_met_Ex = - l1_met_cont->energyX()/Gaudi::Units::GeV;
          float L1_met_Ey = - l1_met_cont->energyY()/Gaudi::Units::GeV;
          float L1_met_Et = std::sqrt(L1_met_Ex*L1_met_Ex + L1_met_Ey*L1_met_Ey);
          float L1_met_sumEt = l1_met_cont->energyT()/Gaudi::Units::GeV;
          float L1_met_Ex_log = signed_log(L1_met_Ex, epsilon);
          float L1_met_Ey_log = signed_log(L1_met_Ey, epsilon);
          float L1_met_Et_log = signed_log(L1_met_Et, epsilon);
          float L1_met_sumEt_log = signed_log(L1_met_sumEt, epsilon);
          TVector3 v(L1_met_Ex, L1_met_Ey, 0.0);
          float L1_met_phi = v.Phi();

          auto L1_Ex = Monitored::Scalar<float>("L1_"+alg+"_Ex", static_cast<float>(L1_met_Ex));
          auto L1_Ey = Monitored::Scalar<float>("L1_"+alg+"_Ey", static_cast<float>(L1_met_Ey));
          auto L1_Et = Monitored::Scalar<float>("L1_"+alg+"_Et", static_cast<float>(L1_met_Et));
          auto L1_sumEt = Monitored::Scalar<float>("L1_"+alg+"_sumEt", static_cast<float>(L1_met_sumEt));
          auto L1_Ex_log = Monitored::Scalar<float>("L1_"+alg+"_Ex_log", static_cast<float>(L1_met_Ex_log));
          auto L1_Ey_log = Monitored::Scalar<float>("L1_"+alg+"_Ey_log", static_cast<float>(L1_met_Ey_log));
          auto L1_Et_log = Monitored::Scalar<float>("L1_"+alg+"_Et_log", static_cast<float>(L1_met_Et_log));
          auto L1_sumEt_log = Monitored::Scalar<float>("L1_"+alg+"_sumEt_log", static_cast<float>(L1_met_sumEt_log));
          auto L1_phi = Monitored::Scalar<float>("L1_"+alg+"_phi", static_cast<float>(L1_met_phi));
          fill(tool, L1_Ex, L1_Ey, L1_Et, L1_sumEt,
               L1_Ex_log, L1_Ey_log, L1_Et_log, L1_sumEt_log, L1_phi);
        }
      }
    }

    // access L1 jFex MET values
    if (l1_jFexMet_cont.isValid() && l1_jFexMet_cont->size() > 0) {
      float L1_met_Ex = 0;
      float L1_met_Ey = 0;
      for (const auto l1_jmet: *l1_jFexMet_cont) {
        L1_met_Ex += l1_jmet->Ex()/Gaudi::Units::GeV;
        L1_met_Ey += l1_jmet->Ey()/Gaudi::Units::GeV;
      }
      float L1_met_Et = std::sqrt(L1_met_Ex*L1_met_Ex + L1_met_Ey*L1_met_Ey);
      float L1_met_Ex_log = signed_log(L1_met_Ex, epsilon);
      float L1_met_Ey_log = signed_log(L1_met_Ey, epsilon);
      float L1_met_Et_log = signed_log(L1_met_Et, epsilon);
      TVector3 v(L1_met_Ex, L1_met_Ey, 0.0);
      float L1_met_phi = v.Phi();
      auto L1_Ex = Monitored::Scalar<float>("L1_jFex_Ex", static_cast<float>(L1_met_Ex));
      auto L1_Ey = Monitored::Scalar<float>("L1_jFex_Ey", static_cast<float>(L1_met_Ey));
      auto L1_Et = Monitored::Scalar<float>("L1_jFex_Et", static_cast<float>(L1_met_Et));
      auto L1_Ex_log = Monitored::Scalar<float>("L1_jFex_Ex_log", static_cast<float>(L1_met_Ex_log));
      auto L1_Ey_log = Monitored::Scalar<float>("L1_jFex_Ey_log", static_cast<float>(L1_met_Ey_log));
      auto L1_Et_log = Monitored::Scalar<float>("L1_jFex_Et_log", static_cast<float>(L1_met_Et_log));
      auto L1_phi = Monitored::Scalar<float>("L1_jFex_phi", static_cast<float>(L1_met_phi));
     fill(tool, L1_Ex, L1_Ey, L1_Et, L1_Ex_log, L1_Ey_log, L1_Et_log, L1_phi);
    }
    if (l1_jFexSumEt_cont.isValid() && l1_jFexSumEt_cont->size() > 0) {
      float L1_met_sumEt = 0;
      for (const auto l1_jsumEt: *l1_jFexSumEt_cont) {
        L1_met_sumEt += l1_jsumEt->Et_lower()/Gaudi::Units::GeV + l1_jsumEt->Et_upper()/Gaudi::Units::GeV;
      }
      float L1_met_sumEt_log = signed_log(L1_met_sumEt, epsilon);
      auto L1_sumEt = Monitored::Scalar<float>("L1_jFex_sumEt", static_cast<float>(L1_met_sumEt));
      auto L1_sumEt_log = Monitored::Scalar<float>("L1_jFex_sumEt_log", static_cast<float>(L1_met_sumEt_log));
      fill(tool, L1_sumEt, L1_sumEt_log);
    }

    // define L1 gFex MET object
    const xAOD::gFexGlobalRoI *l1_gmet;

    // access L1 gFex MET values
    // This will be properly implemented when it's ready
    if (l1_gFexJwojScalar_cont.isValid() && l1_gFexJwojScalar_cont->size() > 0) {
      l1_gmet = l1_gFexJwojScalar_cont->at(0);
      float L1_met_Et = l1_gmet->METquantityOne()/Gaudi::Units::GeV;
      float L1_met_sumEt = l1_gmet->METquantityTwo()/Gaudi::Units::GeV;
      float L1_met_Et_log = signed_log(L1_met_Et, epsilon);
      float L1_met_sumEt_log = signed_log(L1_met_sumEt, epsilon);
      auto L1_Et = Monitored::Scalar<float>("L1_gFexJwoj_Et", static_cast<float>(L1_met_Et));
      auto L1_Et_log = Monitored::Scalar<float>("L1_gFexJwoj_Et_log", static_cast<float>(L1_met_Et_log));
      auto L1_sumEt = Monitored::Scalar<float>("L1_gFexJwoj_sumEt", static_cast<float>(L1_met_sumEt));
      auto L1_sumEt_log = Monitored::Scalar<float>("L1_gFexJwoj_sumEt_log", static_cast<float>(L1_met_sumEt_log));
      fill(tool, L1_Et, L1_Et_log, L1_sumEt, L1_sumEt_log);
    }

    if (l1_gFexJwojMETComponents_cont.isValid() && l1_gFexJwojMETComponents_cont->size() > 0) {
      l1_gmet = l1_gFexJwojMETComponents_cont->at(0);
      float L1_met_Ex = l1_gmet->METquantityOne()/Gaudi::Units::GeV;
      float L1_met_Ex_log = signed_log(L1_met_Ex, epsilon);
      float L1_met_Ey = l1_gmet->METquantityTwo()/Gaudi::Units::GeV;
      float L1_met_Ey_log = signed_log(L1_met_Ey, epsilon);
      TVector3 v(L1_met_Ex, L1_met_Ey, 0.0);
      float L1_met_phi = v.Phi();
      auto L1_Ex = Monitored::Scalar<float>("L1_gFexJwoj_Ex", static_cast<float>(L1_met_Ex));
      auto L1_Ey = Monitored::Scalar<float>("L1_gFexJwoj_Ey", static_cast<float>(L1_met_Ey));
      auto L1_Ex_log = Monitored::Scalar<float>("L1_gFexJwoj_Ex_log", static_cast<float>(L1_met_Ex_log));
      auto L1_Ey_log = Monitored::Scalar<float>("L1_gFexJwoj_Ey_log", static_cast<float>(L1_met_Ey_log));
      auto L1_phi = Monitored::Scalar<float>("L1_gFexJwoj_phi", static_cast<float>(L1_met_phi));
      fill(tool, L1_Ex, L1_Ey, L1_Ex_log, L1_Ey_log, L1_phi);
    }

    if (l1_gFexJwojMHTComponents_cont.isValid() && l1_gFexJwojMHTComponents_cont->size() > 0) {
      l1_gmet = l1_gFexJwojMHTComponents_cont->at(0);
      float L1_met_HT_Ex = l1_gmet->METquantityOne()/Gaudi::Units::GeV;
      float L1_met_HT_Ex_log = signed_log(L1_met_HT_Ex, epsilon);
      float L1_met_HT_Ey = l1_gmet->METquantityTwo()/Gaudi::Units::GeV;
      float L1_met_HT_Ey_log = signed_log(L1_met_HT_Ey, epsilon);
      TVector3 v(L1_met_HT_Ex, L1_met_HT_Ey, 0.0);
      float L1_met_HT_phi = v.Phi();
      auto L1_HT_Ex = Monitored::Scalar<float>("L1_gFexJwoj_HT_Ex", static_cast<float>(L1_met_HT_Ex));
      auto L1_HT_Ey = Monitored::Scalar<float>("L1_gFexJwoj_HT_Ey", static_cast<float>(L1_met_HT_Ey));
      auto L1_HT_Ex_log = Monitored::Scalar<float>("L1_gFexJwoj_HT_Ex_log", static_cast<float>(L1_met_HT_Ex_log));
      auto L1_HT_Ey_log = Monitored::Scalar<float>("L1_gFexJwoj_HT_Ey_log", static_cast<float>(L1_met_HT_Ey_log));
      auto L1_HT_phi = Monitored::Scalar<float>("L1_gFexJwoj_HT_phi", static_cast<float>(L1_met_HT_phi));
      fill(tool, L1_HT_Ex, L1_HT_Ey, L1_HT_Ex_log, L1_HT_Ey_log, L1_HT_phi);
    }
	
    if (l1_gFexJwojMSTComponents_cont.isValid() && l1_gFexJwojMSTComponents_cont->size() > 0) {
      l1_gmet = l1_gFexJwojMSTComponents_cont->at(0);
      float L1_met_ST_Ex = l1_gmet->METquantityOne()/Gaudi::Units::GeV;
      float L1_met_ST_Ex_log = signed_log(L1_met_ST_Ex, epsilon);
      float L1_met_ST_Ey = l1_gmet->METquantityTwo()/Gaudi::Units::GeV;
      float L1_met_ST_Ey_log = signed_log(L1_met_ST_Ey, epsilon);
      TVector3 v(L1_met_ST_Ex, L1_met_ST_Ey, 0.0);
      float L1_met_ST_phi = v.Phi();
      auto L1_ST_Ex = Monitored::Scalar<float>("L1_gFexJwoj_ST_Ex", static_cast<float>(L1_met_ST_Ex));
      auto L1_ST_Ey = Monitored::Scalar<float>("L1_gFexJwoj_ST_Ey", static_cast<float>(L1_met_ST_Ey));
      auto L1_ST_Ex_log = Monitored::Scalar<float>("L1_gFexJwoj_ST_Ex_log", static_cast<float>(L1_met_ST_Ex_log));
      auto L1_ST_Ey_log = Monitored::Scalar<float>("L1_gFexJwoj_ST_Ey_log", static_cast<float>(L1_met_ST_Ey_log));
      auto L1_ST_phi = Monitored::Scalar<float>("L1_gFexJwoj_ST_phi", static_cast<float>(L1_met_ST_phi));
      fill(tool, L1_ST_Ex, L1_ST_Ey, L1_ST_Ex_log, L1_ST_Ey_log, L1_ST_phi);
    }
    
    if (l1_gFexNCMETScalar_cont.isValid() && l1_gFexNCMETScalar_cont->size() > 0) {
      l1_gmet = l1_gFexNCMETScalar_cont->at(0);
      float L1_met_Et = l1_gmet->METquantityOne()/Gaudi::Units::GeV;
      float L1_met_sumEt = l1_gmet->METquantityTwo()/Gaudi::Units::GeV;
      float L1_met_Et_log = signed_log(L1_met_Et, epsilon);
      float L1_met_sumEt_log = signed_log(L1_met_sumEt, epsilon);
      auto L1_Et = Monitored::Scalar<float>("L1_gFexNC_Et", static_cast<float>(L1_met_Et));
      auto L1_Et_log = Monitored::Scalar<float>("L1_gFexNC_Et_log", static_cast<float>(L1_met_Et_log));
      auto L1_sumEt = Monitored::Scalar<float>("L1_gFexNC_sumEt", static_cast<float>(L1_met_sumEt));
      auto L1_sumEt_log = Monitored::Scalar<float>("L1_gFexNC_sumEt_log", static_cast<float>(L1_met_sumEt_log));
      fill(tool, L1_Et, L1_Et_log, L1_sumEt, L1_sumEt_log);
    }
    
    if (l1_gFexNCMETComponents_cont.isValid() && l1_gFexNCMETComponents_cont->size() > 0) {
      l1_gmet = l1_gFexNCMETComponents_cont->at(0);
      float L1_met_Ex = l1_gmet->METquantityOne()/Gaudi::Units::GeV;
      float L1_met_Ex_log = signed_log(L1_met_Ex, epsilon);
      float L1_met_Ey = l1_gmet->METquantityTwo()/Gaudi::Units::GeV;
      float L1_met_Ey_log = signed_log(L1_met_Ey, epsilon);
      TVector3 v(L1_met_Ex, L1_met_Ey, 0.0);
      float L1_met_phi = v.Phi();
      auto L1_Ex = Monitored::Scalar<float>("L1_gFexNC_Ex", static_cast<float>(L1_met_Ex));
      auto L1_Ey = Monitored::Scalar<float>("L1_gFexNC_Ey", static_cast<float>(L1_met_Ey));
      auto L1_Ex_log = Monitored::Scalar<float>("L1_gFexNC_Ex_log", static_cast<float>(L1_met_Ex_log));
      auto L1_Ey_log = Monitored::Scalar<float>("L1_gFexNC_Ey_log", static_cast<float>(L1_met_Ey_log));
      auto L1_phi = Monitored::Scalar<float>("L1_gFexNC_phi", static_cast<float>(L1_met_phi));
      fill(tool, L1_Ex, L1_Ey, L1_Ex_log, L1_Ey_log, L1_phi);
    }
	
    if (l1_gFexRhoMETScalar_cont.isValid() && l1_gFexRhoMETScalar_cont->size() > 0) {
      l1_gmet = l1_gFexRhoMETScalar_cont->at(0);
      float L1_met_Et = l1_gmet->METquantityOne()/Gaudi::Units::GeV;
      float L1_met_sumEt = l1_gmet->METquantityTwo()/Gaudi::Units::GeV;
      float L1_met_Et_log = signed_log(L1_met_Et, epsilon);
      float L1_met_sumEt_log = signed_log(L1_met_sumEt, epsilon);
      auto L1_Et = Monitored::Scalar<float>("L1_gFexRho_Et", static_cast<float>(L1_met_Et));
      auto L1_Et_log = Monitored::Scalar<float>("L1_gFexRho_Et_log", static_cast<float>(L1_met_Et_log));
      auto L1_sumEt = Monitored::Scalar<float>("L1_gFexRho_sumEt", static_cast<float>(L1_met_sumEt));
      auto L1_sumEt_log = Monitored::Scalar<float>("L1_gFexRho_sumEt_log", static_cast<float>(L1_met_sumEt_log));
      fill(tool, L1_Et, L1_Et_log, L1_sumEt, L1_sumEt_log);
    }
    
    if (l1_gFexRhoMETComponents_cont.isValid() && l1_gFexRhoMETComponents_cont->size() > 0) {
      l1_gmet = l1_gFexRhoMETComponents_cont->at(0);
      float L1_met_Ex = l1_gmet->METquantityOne()/Gaudi::Units::GeV;
      float L1_met_Ex_log = signed_log(L1_met_Ex, epsilon);
      float L1_met_Ey = l1_gmet->METquantityTwo()/Gaudi::Units::GeV;
      float L1_met_Ey_log = signed_log(L1_met_Ey, epsilon);
      TVector3 v(L1_met_Ex, L1_met_Ey, 0.0);
      float L1_met_phi = v.Phi();
      auto L1_Ex = Monitored::Scalar<float>("L1_gFexRho_Ex", static_cast<float>(L1_met_Ex));
      auto L1_Ey = Monitored::Scalar<float>("L1_gFexRho_Ey", static_cast<float>(L1_met_Ey));
      auto L1_Ex_log = Monitored::Scalar<float>("L1_gFexRho_Ex_log", static_cast<float>(L1_met_Ex_log));
      auto L1_Ey_log = Monitored::Scalar<float>("L1_gFexRho_Ey_log", static_cast<float>(L1_met_Ey_log));
      auto L1_phi = Monitored::Scalar<float>("L1_gFexRho_phi", static_cast<float>(L1_met_phi));
      fill(tool, L1_Ex, L1_Ey, L1_Ex_log, L1_Ey_log, L1_phi);
    }
    
    // define TrigMissingET object
    const xAOD::TrigMissingET *hlt_met = 0;

    // status, component from HLT cell
    int nComponent = m_compNames.size();
    int nStatus = m_bitNames.size();
    if ( hlt_cell_met_cont.isValid() && hlt_cell_met_cont->size() > 0 ) {
      hlt_met = hlt_cell_met_cont->at(0);
      for (int j=0; j<nStatus; ++j) { //status loop
        unsigned mask = (1u<<j);
        if (hlt_met->flag() & mask) {
          MET_status = 1.;
        } else {
          MET_status = 0;
        }
        auto mon1 = Monitored::Scalar<std::string>( "HLT_MET_status",m_bitNames[j]);
        fill(tool,mon1,MET_status);
      }

      for (int i=0; i<nComponent; ++i) { //component loop
        float ex = hlt_met->exComponent(i)/Gaudi::Units::GeV;
        float ey = hlt_met->eyComponent(i)/Gaudi::Units::GeV;
        component_Et = sqrt(ex*ex+ey*ey);
        auto mon2 = Monitored::Scalar<std::string>( "HLT_MET_component",m_compNames[i]);
        fill(tool,mon2,component_Et);
      }

      for (int i=0; i<nComponent; ++i) { //component loop
        for (int j=0; j<nStatus; ++j) { //status loop
          unsigned mask = (1u<<j);
          if (hlt_met->statusComponent(i) & mask) {
            component_status_weight = 1.;
          } else {
            component_status_weight = 0;
          }
          auto mon_bit = Monitored::Scalar<std::string>( "component_status",m_bitNames[j]);
          auto mon_comp = Monitored::Scalar<std::string>( "component",m_compNames[i]);
          fill(tool,mon_comp,mon_bit,component_status_weight);
        }
      }
    }

    // get L1_roiMet_Et for pre-selection
    float L1_roiMet_Et = 0;
    if ( l1_roi_cont.isValid() ) {
      if ((l1_roi_cont->energyX())>-9e12 && (l1_roi_cont->energyX())<9e12 && (l1_roi_cont->energyY())>-9e12 && (l1_roi_cont->energyY())<9e12) {
	      float Ex = - l1_roi_cont->energyX()/Gaudi::Units::GeV;
	      float Ey = - l1_roi_cont->energyY()/Gaudi::Units::GeV;
           L1_roiMet_Et = std::sqrt(Ex*Ex + Ey*Ey);
      }
    }

    // access HLT MET values
    for (const std::string& alg : m_algsHLT) {
      if (alg == "cell" && hlt_cell_met_cont.isValid() && hlt_cell_met_cont->size() > 0) {
        hlt_met = hlt_cell_met_cont->at(0);
      } else if (alg == "mht" && hlt_mht_met_cont.isValid() && hlt_mht_met_cont->size() > 0) {
        hlt_met = hlt_mht_met_cont->at(0);
      } else if (alg == "tc" && hlt_tc_met_cont.isValid() && hlt_tc_met_cont->size() > 0) {
        hlt_met = hlt_tc_met_cont->at(0);
      } else if (alg == "tc_em" && hlt_tc_em_met_cont.isValid() && hlt_tc_em_met_cont->size() > 0) {
        hlt_met = hlt_tc_em_met_cont->at(0);
      } else if (alg == "tcpufit" && hlt_tcpufit_met_cont.isValid() && hlt_tcpufit_met_cont->size() > 0) {
        hlt_met = hlt_tcpufit_met_cont->at(0);
      } else if (alg == "tcpufit_sig30" && hlt_tcpufit_sig30_met_cont.isValid() && hlt_tcpufit_sig30_met_cont->size() > 0) {
        hlt_met = hlt_tcpufit_sig30_met_cont->at(0);
      } else if (alg == "trkmht" && hlt_trkmht_met_cont.isValid() && hlt_trkmht_met_cont->size() > 0) {
        hlt_met = hlt_trkmht_met_cont->at(0);
      } else if (alg == "pfsum" && hlt_pfsum_met_cont.isValid() && hlt_pfsum_met_cont->size() > 0) {
        hlt_met = hlt_pfsum_met_cont->at(0);
      } else if (alg == "pfsum_cssk" && hlt_pfsum_cssk_met_cont.isValid() && hlt_pfsum_cssk_met_cont->size() > 0) {
        hlt_met = hlt_pfsum_cssk_met_cont->at(0);
      } else if (alg == "pfsum_vssk" && hlt_pfsum_vssk_met_cont.isValid() && hlt_pfsum_vssk_met_cont->size() > 0) {
        hlt_met = hlt_pfsum_vssk_met_cont->at(0);
      } else if (alg == "pfopufit" && hlt_pfopufit_met_cont.isValid() && hlt_pfopufit_met_cont->size() > 0) {
        hlt_met = hlt_pfopufit_met_cont->at(0);
      } else if (alg == "pfopufit_sig30" && hlt_pfopufit_sig30_met_cont.isValid() && hlt_pfopufit_sig30_met_cont->size() > 0) {
        hlt_met = hlt_pfopufit_sig30_met_cont->at(0);
      } else if (alg == "cvfpufit" && hlt_cvfpufit_met_cont.isValid() && hlt_cvfpufit_met_cont->size() > 0) {
        hlt_met = hlt_cvfpufit_met_cont->at(0);
      } else if (alg == "mhtpufit_pf" && hlt_mhtpufit_pf_met_cont.isValid() && hlt_mhtpufit_pf_met_cont->size() > 0) {
        hlt_met = hlt_mhtpufit_pf_met_cont->at(0);
      } else if (alg == "mhtpufit_em" && hlt_mhtpufit_em_met_cont.isValid() && hlt_mhtpufit_em_met_cont->size() > 0) {
        hlt_met = hlt_mhtpufit_em_met_cont->at(0);
      } else if (alg == "met_nn" && hlt_met_nn_cont.isValid() && hlt_met_nn_cont->size() > 0) {
        hlt_met = hlt_met_nn_cont->at(0);
      } else {
        hlt_met = 0;
      }

      if ( hlt_met ) {
        float hlt_Ex = hlt_met->ex()/Gaudi::Units::GeV;
        float hlt_Ey = hlt_met->ey()/Gaudi::Units::GeV;
        float hlt_Ez = hlt_met->ez()/Gaudi::Units::GeV;
        float hlt_Et = std::sqrt(hlt_Ex*hlt_Ex + hlt_Ey*hlt_Ey);
        float hlt_sumEt = hlt_met->sumEt()/Gaudi::Units::GeV;
        float hlt_Ex_log = signed_log(hlt_Ex, epsilon);
        float hlt_Ey_log = signed_log(hlt_Ey, epsilon);
        float hlt_Et_log = signed_log(hlt_Et, epsilon);
        float hlt_sumEt_log = signed_log(hlt_sumEt, epsilon); 
        TVector3 v(hlt_Ex, hlt_Ey, hlt_Ez);
        float hlt_eta = v.Eta();
        float hlt_phi = v.Phi();

        auto met_Ex = Monitored::Scalar<float>(alg+"_Ex", static_cast<float>(hlt_Ex));
        auto met_Ey = Monitored::Scalar<float>(alg+"_Ey", static_cast<float>(hlt_Ey));
        auto met_Et = Monitored::Scalar<float>(alg+"_Et", static_cast<float>(hlt_Et));
        auto met_sumEt = Monitored::Scalar<float>(alg+"_sumEt", static_cast<float>(hlt_sumEt));
        auto met_Ex_log = Monitored::Scalar<float>(alg+"_Ex_log", static_cast<float>(hlt_Ex_log));
        auto met_Ey_log = Monitored::Scalar<float>(alg+"_Ey_log", static_cast<float>(hlt_Ey_log));
        auto met_Et_log = Monitored::Scalar<float>(alg+"_Et_log", static_cast<float>(hlt_Et_log));
        auto met_sumEt_log = Monitored::Scalar<float>(alg+"_sumEt_log", static_cast<float>(hlt_sumEt_log));
        auto met_eta = Monitored::Scalar<float>(alg+"_eta", static_cast<float>(hlt_eta));
        auto met_phi = Monitored::Scalar<float>(alg+"_phi", static_cast<float>(hlt_phi));
        fill(tool,met_Ex,met_Ey,met_Et,met_sumEt,
             met_Ex_log,met_Ey_log,met_Et_log,met_sumEt_log,
             met_eta,met_phi);

      }
    }
    
    // access HLT pre-selection MET values
    for (const std::string& alg : m_algsHLTPreSel) {
      if (alg == "cell" && hlt_cell_met_cont.isValid() && hlt_cell_met_cont->size() > 0) {
        hlt_met = hlt_cell_met_cont->at(0);
      } else if (alg == "mht" && hlt_mht_met_cont.isValid() && hlt_mht_met_cont->size() > 0) {
        hlt_met = hlt_mht_met_cont->at(0);
      } else if (alg == "tc" && hlt_tc_met_cont.isValid() && hlt_tc_met_cont->size() > 0) {
        hlt_met = hlt_tc_met_cont->at(0);
      } else if (alg == "tc_em" && hlt_tc_em_met_cont.isValid() && hlt_tc_em_met_cont->size() > 0) {
        hlt_met = hlt_tc_em_met_cont->at(0);
      } else if (alg == "tcpufit" && hlt_tcpufit_met_cont.isValid() && hlt_tcpufit_met_cont->size() > 0) {
        hlt_met = hlt_tcpufit_met_cont->at(0);
      } else if (alg == "tcpufit_sig30" && hlt_tcpufit_sig30_met_cont.isValid() && hlt_tcpufit_sig30_met_cont->size() > 0) {
        hlt_met = hlt_tcpufit_sig30_met_cont->at(0);
      } else if (alg == "trkmht" && hlt_trkmht_met_cont.isValid() && hlt_trkmht_met_cont->size() > 0) {
        hlt_met = hlt_trkmht_met_cont->at(0);
      } else if (alg == "pfsum" && hlt_pfsum_met_cont.isValid() && hlt_pfsum_met_cont->size() > 0) {
        hlt_met = hlt_pfsum_met_cont->at(0);
      } else if (alg == "pfsum_cssk" && hlt_pfsum_cssk_met_cont.isValid() && hlt_pfsum_cssk_met_cont->size() > 0) {
        hlt_met = hlt_pfsum_cssk_met_cont->at(0);
      } else if (alg == "pfsum_vssk" && hlt_pfsum_vssk_met_cont.isValid() && hlt_pfsum_vssk_met_cont->size() > 0) {
        hlt_met = hlt_pfsum_vssk_met_cont->at(0);
      } else if (alg == "pfopufit" && hlt_pfopufit_met_cont.isValid() && hlt_pfopufit_met_cont->size() > 0) {
        hlt_met = hlt_pfopufit_met_cont->at(0);
      } else if (alg == "pfopufit_sig30" && hlt_pfopufit_sig30_met_cont.isValid() && hlt_pfopufit_sig30_met_cont->size() > 0) {
        hlt_met = hlt_pfopufit_sig30_met_cont->at(0);
      } else if (alg == "cvfpufit" && hlt_cvfpufit_met_cont.isValid() && hlt_cvfpufit_met_cont->size() > 0) {
        hlt_met = hlt_cvfpufit_met_cont->at(0);
      } else if (alg == "mhtpufit_pf" && hlt_mhtpufit_pf_met_cont.isValid() && hlt_mhtpufit_pf_met_cont->size() > 0) {
        hlt_met = hlt_mhtpufit_pf_met_cont->at(0);
      } else if (alg == "mhtpufit_em" && hlt_mhtpufit_em_met_cont.isValid() && hlt_mhtpufit_em_met_cont->size() > 0) {
        hlt_met = hlt_mhtpufit_em_met_cont->at(0);
      } else if (alg == "met_nn" && hlt_met_nn_cont.isValid() && hlt_met_nn_cont->size() > 0) {
        hlt_met = hlt_met_nn_cont->at(0);
      } else {
        hlt_met = 0;
      }

      if ( hlt_met ) {
        float hlt_Ex = hlt_met->ex()/Gaudi::Units::GeV;
        float hlt_Ey = hlt_met->ey()/Gaudi::Units::GeV;
        float hlt_Et = std::sqrt(hlt_Ex*hlt_Ex + hlt_Ey*hlt_Ey);
        if (L1_roiMet_Et > 50. && !std::isnan(hlt_Et)) {
          auto met_presel_Et = Monitored::Scalar<float>(alg+"_presel_Et", hlt_Et);
          fill(tool,met_presel_Et);
        }
      }
    }

    for (const std::string& alg : m_LArNoiseBurstVetoAlgs) {
      if (alg == "cell" && hlt_cell_met_cont.isValid() && hlt_cell_met_cont->size() > 0) {
      hlt_met = hlt_cell_met_cont->at(0);
      } else if (alg == "mht" && hlt_mht_met_cont.isValid() && hlt_mht_met_cont->size() > 0) {
      hlt_met = hlt_mht_met_cont->at(0);
      } else if (alg == "tc" && hlt_tc_met_cont.isValid() && hlt_tc_met_cont->size() > 0) {
      hlt_met = hlt_tc_met_cont->at(0);
      } else if (alg == "tc_em" && hlt_tc_em_met_cont.isValid() && hlt_tc_em_met_cont->size() > 0) {
      hlt_met = hlt_tc_em_met_cont->at(0);
      } else if (alg == "tcpufit" && hlt_tcpufit_met_cont.isValid() && hlt_tcpufit_met_cont->size() > 0) {
      hlt_met = hlt_tcpufit_met_cont->at(0);
      } else if (alg == "trkmht" && hlt_trkmht_met_cont.isValid() && hlt_trkmht_met_cont->size() > 0) {
      hlt_met = hlt_trkmht_met_cont->at(0);
      } else if (alg == "pfsum" && hlt_pfsum_met_cont.isValid() && hlt_pfsum_met_cont->size() > 0) {
      hlt_met = hlt_pfsum_met_cont->at(0);
      } else if (alg == "pfsum_cssk" && hlt_pfsum_cssk_met_cont.isValid() && hlt_pfsum_cssk_met_cont->size() > 0) {
      hlt_met = hlt_pfsum_cssk_met_cont->at(0);
      } else if (alg == "pfsum_vssk" && hlt_pfsum_vssk_met_cont.isValid() && hlt_pfsum_vssk_met_cont->size() > 0) {
      hlt_met = hlt_pfsum_vssk_met_cont->at(0);
      } else if (alg == "pfopufit" && hlt_pfopufit_met_cont.isValid() && hlt_pfopufit_met_cont->size() > 0) {
      hlt_met = hlt_pfopufit_met_cont->at(0);
      } else if (alg == "cvfpufit" && hlt_cvfpufit_met_cont.isValid() && hlt_cvfpufit_met_cont->size() > 0) {
      hlt_met = hlt_cvfpufit_met_cont->at(0);
      } else if (alg == "mhtpufit_pf" && hlt_mhtpufit_pf_met_cont.isValid() && hlt_mhtpufit_pf_met_cont->size() > 0) {
      hlt_met = hlt_mhtpufit_pf_met_cont->at(0);
      } else if (alg == "mhtpufit_em" && hlt_mhtpufit_em_met_cont.isValid() && hlt_mhtpufit_em_met_cont->size() > 0) {
      hlt_met = hlt_mhtpufit_em_met_cont->at(0);
      } else {
      hlt_met = 0;
      }

      if ( hlt_met ) {
        float hlt_Ex = hlt_met->ex()/Gaudi::Units::GeV;
        float hlt_Ey = hlt_met->ey()/Gaudi::Units::GeV;
        float hlt_Ez = hlt_met->ez()/Gaudi::Units::GeV;
        float hlt_Et = std::sqrt(hlt_Ex*hlt_Ex + hlt_Ey*hlt_Ey);
        float hlt_sumEt = hlt_met->sumEt()/Gaudi::Units::GeV;
        float hlt_Ex_log = signed_log(hlt_Ex, epsilon);
        float hlt_Ey_log = signed_log(hlt_Ey, epsilon);
        float hlt_Et_log = signed_log(hlt_Et, epsilon);
        float hlt_sumEt_log = signed_log(hlt_sumEt, epsilon); 
        TVector3 v(hlt_Ex, hlt_Ey, hlt_Ez);
        float hlt_eta = v.Eta();
        float hlt_phi = v.Phi();


        // LAr noiseburst Veto
        bool LArNoiseBurst = eventInfo->isEventFlagBitSet(xAOD::EventInfo::LAr,LArEventBitInfo::NOISEBURSTVETO);
        if (!LArNoiseBurst){
          auto met_Ex = Monitored::Scalar<float>(alg+"_LArNoiseBurstVeto_Ex", static_cast<float>(hlt_Ex));
          auto met_Ey = Monitored::Scalar<float>(alg+"_LArNoiseBurstVeto_Ey", static_cast<float>(hlt_Ey));
          auto met_Et = Monitored::Scalar<float>(alg+"_LArNoiseBurstVeto_Et", static_cast<float>(hlt_Et));
          auto met_sumEt = Monitored::Scalar<float>(alg+"_LArNoiseBurstVeto_sumEt", static_cast<float>(hlt_sumEt));
          auto met_Ex_log = Monitored::Scalar<float>(alg+"_LArNoiseBurstVeto_Ex_log", static_cast<float>(hlt_Ex_log));
          auto met_Ey_log = Monitored::Scalar<float>(alg+"_LArNoiseBurstVeto_Ey_log", static_cast<float>(hlt_Ey_log));
          auto met_Et_log = Monitored::Scalar<float>(alg+"_LArNoiseBurstVeto_Et_log", static_cast<float>(hlt_Et_log));
          auto met_sumEt_log = Monitored::Scalar<float>(alg+"_LArNoiseBurstVeto_sumEt_log", static_cast<float>(hlt_sumEt_log));
          auto met_phi = Monitored::Scalar<float>(alg+"_LArNoiseBurstVeto_phi", static_cast<float>(hlt_phi));
          auto met_eta = Monitored::Scalar<float>(alg+"_LArNoiseBurstVeto_eta", static_cast<float>(hlt_eta));
          fill(tool,met_Ex,met_Ey,met_Et,met_sumEt,
               met_Ex_log,met_Ey_log,met_Et_log,met_sumEt_log,
               met_eta,met_phi);
        }
      }
    }


    for (const std::string& alg : m_signalLepAlgs) {
      if (alg == "cell" && hlt_cell_met_cont.isValid() && hlt_cell_met_cont->size() > 0) {
      hlt_met = hlt_cell_met_cont->at(0);
      } else if (alg == "mht" && hlt_mht_met_cont.isValid() && hlt_mht_met_cont->size() > 0) {
      hlt_met = hlt_mht_met_cont->at(0);
      } else if (alg == "tc" && hlt_tc_met_cont.isValid() && hlt_tc_met_cont->size() > 0) {
      hlt_met = hlt_tc_met_cont->at(0);
      } else if (alg == "tc_em" && hlt_tc_em_met_cont.isValid() && hlt_tc_em_met_cont->size() > 0) {
      hlt_met = hlt_tc_em_met_cont->at(0);
      } else if (alg == "tcpufit" && hlt_tcpufit_met_cont.isValid() && hlt_tcpufit_met_cont->size() > 0) {
      hlt_met = hlt_tcpufit_met_cont->at(0);
      } else if (alg == "trkmht" && hlt_trkmht_met_cont.isValid() && hlt_trkmht_met_cont->size() > 0) {
      hlt_met = hlt_trkmht_met_cont->at(0);
      } else if (alg == "pfsum" && hlt_pfsum_met_cont.isValid() && hlt_pfsum_met_cont->size() > 0) {
      hlt_met = hlt_pfsum_met_cont->at(0);
      } else if (alg == "pfsum_cssk" && hlt_pfsum_cssk_met_cont.isValid() && hlt_pfsum_cssk_met_cont->size() > 0) {
      hlt_met = hlt_pfsum_cssk_met_cont->at(0);
      } else if (alg == "pfsum_vssk" && hlt_pfsum_vssk_met_cont.isValid() && hlt_pfsum_vssk_met_cont->size() > 0) {
      hlt_met = hlt_pfsum_vssk_met_cont->at(0);
      } else if (alg == "pfopufit" && hlt_pfopufit_met_cont.isValid() && hlt_pfopufit_met_cont->size() > 0) {
      hlt_met = hlt_pfopufit_met_cont->at(0);
      } else if (alg == "cvfpufit" && hlt_cvfpufit_met_cont.isValid() && hlt_cvfpufit_met_cont->size() > 0) {
      hlt_met = hlt_cvfpufit_met_cont->at(0);
      } else if (alg == "mhtpufit_pf" && hlt_mhtpufit_pf_met_cont.isValid() && hlt_mhtpufit_pf_met_cont->size() > 0) {
      hlt_met = hlt_mhtpufit_pf_met_cont->at(0);
      } else if (alg == "mhtpufit_em" && hlt_mhtpufit_em_met_cont.isValid() && hlt_mhtpufit_em_met_cont->size() > 0) {
      hlt_met = hlt_mhtpufit_em_met_cont->at(0);
      } else {
      hlt_met = 0;
      }

      if ( hlt_met ) {
        float hlt_Ex = hlt_met->ex()/Gaudi::Units::GeV;
        float hlt_Ey = hlt_met->ey()/Gaudi::Units::GeV;
        float hlt_Ez = hlt_met->ez()/Gaudi::Units::GeV;
        float hlt_Et = std::sqrt(hlt_Ex*hlt_Ex + hlt_Ey*hlt_Ey);
        float hlt_sumEt = hlt_met->sumEt()/Gaudi::Units::GeV;
        float hlt_Ex_log = signed_log(hlt_Ex, epsilon);
        float hlt_Ey_log = signed_log(hlt_Ey, epsilon);
        float hlt_Et_log = signed_log(hlt_Et, epsilon);
        float hlt_sumEt_log = signed_log(hlt_sumEt, epsilon); 
        TVector3 v(hlt_Ex, hlt_Ey, hlt_Ez);
        float hlt_eta = v.Eta();
        float hlt_phi = v.Phi();

//      access events with electron passing primary single electron chain
        if(passedPrimaryEl){
          auto met_Ex = Monitored::Scalar<float>(alg+"_SigEl_Ex", static_cast<float>(hlt_Ex));
          auto met_Ey = Monitored::Scalar<float>(alg+"_SigEl_Ey", static_cast<float>(hlt_Ey));
          auto met_Et = Monitored::Scalar<float>(alg+"_SigEl_Et", static_cast<float>(hlt_Et));
          auto met_sumEt = Monitored::Scalar<float>(alg+"_SigEl_sumEt", static_cast<float>(hlt_sumEt));
          auto met_Ex_log = Monitored::Scalar<float>(alg+"_SigEl_Ex_log", static_cast<float>(hlt_Ex_log));
          auto met_Ey_log = Monitored::Scalar<float>(alg+"_SigEl_Ey_log", static_cast<float>(hlt_Ey_log));
          auto met_Et_log = Monitored::Scalar<float>(alg+"_SigEl_Et_log", static_cast<float>(hlt_Et_log));
          auto met_sumEt_log = Monitored::Scalar<float>(alg+"_SigEl_sumEt_log", static_cast<float>(hlt_sumEt_log));
          auto met_eta = Monitored::Scalar<float>(alg+"_SigEl_eta", static_cast<float>(hlt_eta));
          auto met_phi = Monitored::Scalar<float>(alg+"_SigEl_phi", static_cast<float>(hlt_phi));
          fill(tool,met_Ex,met_Ey,met_Et,met_sumEt,
            met_Ex_log,met_Ey_log,met_Et_log,met_sumEt_log,
            met_eta,met_phi);
        }

//      access events with muon passing primary single muon chain
        if(passedPrimaryMu){
          auto met_Ex = Monitored::Scalar<float>(alg+"_SigMu_Ex", static_cast<float>(hlt_Ex));
          auto met_Ey = Monitored::Scalar<float>(alg+"_SigMu_Ey", static_cast<float>(hlt_Ey));
          auto met_Et = Monitored::Scalar<float>(alg+"_SigMu_Et", static_cast<float>(hlt_Et));
          auto met_sumEt = Monitored::Scalar<float>(alg+"_SigMu_sumEt", static_cast<float>(hlt_sumEt));
          auto met_Ex_log = Monitored::Scalar<float>(alg+"_SigMu_Ex_log", static_cast<float>(hlt_Ex_log));
          auto met_Ey_log = Monitored::Scalar<float>(alg+"_SigMu_Ey_log", static_cast<float>(hlt_Ey_log));
          auto met_Et_log = Monitored::Scalar<float>(alg+"_SigMu_Et_log", static_cast<float>(hlt_Et_log));
          auto met_sumEt_log = Monitored::Scalar<float>(alg+"_SigMu_sumEt_log", static_cast<float>(hlt_sumEt_log));
          auto met_eta = Monitored::Scalar<float>(alg+"_SigMu_eta", static_cast<float>(hlt_eta));
          auto met_phi = Monitored::Scalar<float>(alg+"_SigMu_phi", static_cast<float>(hlt_phi));
          fill(tool,met_Ex,met_Ey,met_Et,met_sumEt,
            met_Ex_log,met_Ey_log,met_Et_log,met_sumEt_log,
            met_eta,met_phi);  
        }
      }
    }

    // access HLT  MET Expert values
    for (const std::string& alg : m_algsHLTExpert) {
      if (alg == "cell" && hlt_cell_met_cont.isValid() && hlt_cell_met_cont->size() > 0) {
        hlt_met = hlt_cell_met_cont->at(0);
      } else if (alg == "mht" && hlt_mht_met_cont.isValid() && hlt_mht_met_cont->size() > 0) {
        hlt_met = hlt_mht_met_cont->at(0);
      } else if (alg == "tc" && hlt_tc_met_cont.isValid() && hlt_tc_met_cont->size() > 0) {
        hlt_met = hlt_tc_met_cont->at(0);
      } else if (alg == "tc_em" && hlt_tc_em_met_cont.isValid() && hlt_tc_em_met_cont->size() > 0) {
        hlt_met = hlt_tc_em_met_cont->at(0);
      } else if (alg == "tcpufit" && hlt_tcpufit_met_cont.isValid() && hlt_tcpufit_met_cont->size() > 0) {
        hlt_met = hlt_tcpufit_met_cont->at(0);
      } else if (alg == "tcpufit_sig30" && hlt_tcpufit_sig30_met_cont.isValid() && hlt_tcpufit_sig30_met_cont->size() > 0) {
        hlt_met = hlt_tcpufit_sig30_met_cont->at(0);
      } else if (alg == "trkmht" && hlt_trkmht_met_cont.isValid() && hlt_trkmht_met_cont->size() > 0) {
        hlt_met = hlt_trkmht_met_cont->at(0);
      } else if (alg == "pfsum" && hlt_pfsum_met_cont.isValid() && hlt_pfsum_met_cont->size() > 0) {
        hlt_met = hlt_pfsum_met_cont->at(0);
      } else if (alg == "pfsum_cssk" && hlt_pfsum_cssk_met_cont.isValid() && hlt_pfsum_cssk_met_cont->size() > 0) {
        hlt_met = hlt_pfsum_cssk_met_cont->at(0);
      } else if (alg == "pfsum_vssk" && hlt_pfsum_vssk_met_cont.isValid() && hlt_pfsum_vssk_met_cont->size() > 0) {
        hlt_met = hlt_pfsum_vssk_met_cont->at(0);
      } else if (alg == "pfopufit" && hlt_pfopufit_met_cont.isValid() && hlt_pfopufit_met_cont->size() > 0) {
        hlt_met = hlt_pfopufit_met_cont->at(0);
      } else if (alg == "pfopufit_sig30" && hlt_pfopufit_sig30_met_cont.isValid() && hlt_pfopufit_sig30_met_cont->size() > 0) {
        hlt_met = hlt_pfopufit_sig30_met_cont->at(0);
      } else if (alg == "cvfpufit" && hlt_cvfpufit_met_cont.isValid() && hlt_cvfpufit_met_cont->size() > 0) {
        hlt_met = hlt_cvfpufit_met_cont->at(0);
      } else if (alg == "mhtpufit_pf" && hlt_mhtpufit_pf_met_cont.isValid() && hlt_mhtpufit_pf_met_cont->size() > 0) {
        hlt_met = hlt_mhtpufit_pf_met_cont->at(0);
      } else if (alg == "mhtpufit_em" && hlt_mhtpufit_em_met_cont.isValid() && hlt_mhtpufit_em_met_cont->size() > 0) {
        hlt_met = hlt_mhtpufit_em_met_cont->at(0);
      } else if (alg == "met_nn" && hlt_met_nn_cont.isValid() && hlt_met_nn_cont->size() > 0) {
        hlt_met = hlt_met_nn_cont->at(0);
      } else {
        hlt_met = 0;
      }

      if ( hlt_met ) {
        float hlt_Ex = hlt_met->ex()/Gaudi::Units::GeV;
        float hlt_Ey = hlt_met->ey()/Gaudi::Units::GeV;
        float hlt_Et = std::sqrt(hlt_Ex*hlt_Ex + hlt_Ey*hlt_Ey);
        float hlt_sumEt = hlt_met->sumEt()/Gaudi::Units::GeV;
        if (!std::isnan(hlt_Et))  {
          auto met_Ex = Monitored::Scalar<float>(alg+"_Ex", static_cast<float>(hlt_Ex));
          auto met_Ey = Monitored::Scalar<float>(alg+"_Ey", static_cast<float>(hlt_Ey));
          auto met_Et = Monitored::Scalar<float>(alg+"_Et", static_cast<float>(hlt_Et));
          auto met_sumEt = Monitored::Scalar<float>(alg+"_sumEt", static_cast<float>(hlt_sumEt));
          fill(tool,met_Ex,met_Ey,met_Et,met_sumEt);
          ATH_MSG_DEBUG(alg << ": hlt_Et = " << hlt_Et);
          if (L1_roiMet_Et > 50.) {
            auto met_presel_Et = Monitored::Scalar<float>(alg+"_presel_Et", static_cast<float>(hlt_Et));
            fill(tool,met_presel_Et);
          }
        }
      }
    }
    
    // Make 2D tcpufit MET distributions wrt track-based MET
    const xAOD::TrigMissingET *hlt_tcpufit_met = 0;
    for (const std::string& alg : m_algsMET2d_tcpufit) {
      if (alg == "cell" && hlt_cell_met_cont.isValid() && hlt_cell_met_cont->size() > 0) {
        hlt_met = hlt_cell_met_cont->at(0);
      } else if (alg == "mht" && hlt_mht_met_cont.isValid() && hlt_mht_met_cont->size() > 0) {
        hlt_met = hlt_mht_met_cont->at(0);
      } else if (alg == "tc" && hlt_tc_met_cont.isValid() && hlt_tc_met_cont->size() > 0) {
        hlt_met = hlt_tc_met_cont->at(0);
      } else if (alg == "tc_em" && hlt_tc_em_met_cont.isValid() && hlt_tc_em_met_cont->size() > 0) {
        hlt_met = hlt_tc_em_met_cont->at(0);
      } else if (alg == "trkmht_pf" && hlt_trkmht_met_cont.isValid() && hlt_trkmht_met_cont->size() > 0) {
        hlt_met = hlt_trkmht_met_cont->at(0);
      } else if (alg == "pfsum" && hlt_pfsum_met_cont.isValid() && hlt_pfsum_met_cont->size() > 0) {
        hlt_met = hlt_pfsum_met_cont->at(0);
      } else if (alg == "pfsum_cssk" && hlt_pfsum_cssk_met_cont.isValid() && hlt_pfsum_cssk_met_cont->size() > 0) {
        hlt_met = hlt_pfsum_cssk_met_cont->at(0);
      } else if (alg == "pfsum_vssk" && hlt_pfsum_vssk_met_cont.isValid() && hlt_pfsum_vssk_met_cont->size() > 0) {
        hlt_met = hlt_pfsum_vssk_met_cont->at(0);
      } else if (alg == "pfopufit" && hlt_pfopufit_met_cont.isValid() && hlt_pfopufit_met_cont->size() > 0) {
        hlt_met = hlt_pfopufit_met_cont->at(0);
      } else if (alg == "cvfpufit" && hlt_cvfpufit_met_cont.isValid() && hlt_cvfpufit_met_cont->size() > 0) {
        hlt_met = hlt_cvfpufit_met_cont->at(0);
      } else if (alg == "mhtpufit_pf" && hlt_mhtpufit_pf_met_cont.isValid() && hlt_mhtpufit_pf_met_cont->size() > 0) {
        hlt_met = hlt_mhtpufit_pf_met_cont->at(0);
      } else if (alg == "mhtpufit_em" && hlt_mhtpufit_em_met_cont.isValid() && hlt_mhtpufit_em_met_cont->size() > 0) {
        hlt_met = hlt_mhtpufit_em_met_cont->at(0);
      } else {
        hlt_met = 0;
      }

      if(hlt_met && hlt_tcpufit_met_cont.isValid() && hlt_tcpufit_met_cont->size() > 0){
        hlt_tcpufit_met = hlt_tcpufit_met_cont->at(0); 
        float hlt_Ex = hlt_met->ex()/Gaudi::Units::GeV;
        float hlt_Ey = hlt_met->ey()/Gaudi::Units::GeV;
        float hlt_Et = std::sqrt(hlt_Ex*hlt_Ex + hlt_Ey*hlt_Ey);
        auto met_Et = Monitored::Scalar<float>(alg+"_2D_Et", static_cast<float>(hlt_Et));
        
        float hlt_tcpufit_Ex = hlt_tcpufit_met->ex()/Gaudi::Units::GeV;
        float hlt_tcpufit_Ey = hlt_tcpufit_met->ey()/Gaudi::Units::GeV;
        float hlt_tcpufit_Et = std::sqrt(hlt_tcpufit_Ex*hlt_tcpufit_Ex + hlt_tcpufit_Ey*hlt_tcpufit_Ey);
        auto tcpufit_met_Et = Monitored::Scalar<float>("tcpufit_2D_Et", static_cast<float>(hlt_tcpufit_Et));
        fill(tool, met_Et, tcpufit_met_Et);
      }
    } 

    // efficiency plots
    for (const std::string& chain : m_l1Chains) {
      auto pass_chain = Monitored::Scalar<float>("pass_" + chain, static_cast<float>(getTrigDecisionTool()->isPassed(chain)));
      fill(tool, pass_chain,offline_NoMu_Et_eff);
    }
    for (const std::string& chain : m_hltChains) {
      auto pass_chain = Monitored::Scalar<float>("pass_" + chain, static_cast<float>(getTrigDecisionTool()->isPassed(chain)));
      fill(tool, pass_chain,offline_NoMu_Et_eff);
    }
    for (const std::string& chain : m_hltChainsVal) {
      auto pass_chain = Monitored::Scalar<float>("pass_" + chain, static_cast<float>(getTrigDecisionTool()->isPassed(chain)));
      fill(tool, pass_chain,offline_NoMu_Et_eff);
    }
    for (const std::string& chain : m_hltChainsT0) {
      auto pass_chain = Monitored::Scalar<float>("pass_" + chain, static_cast<float>(getTrigDecisionTool()->isPassed(chain)));
      fill(tool, pass_chain,offline_NoMu_Et_eff);
    }

    return StatusCode::SUCCESS;
}


double TrigMETMonitorAlgorithm::signed_log(double e, double epsilon) const {


  double e_log = -9e9;
  if (std::abs(e) > epsilon)
    e_log = std::copysign(std::log10(std::abs(e)), e);
  else
    e_log = 0.01;

  return e_log;
}
