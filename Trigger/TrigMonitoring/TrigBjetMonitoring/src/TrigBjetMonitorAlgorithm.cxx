/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigBjetMonitorAlgorithm.h"

#include "EventPrimitives/EventPrimitivesHelpers.h"
#include "xAODJet/JetContainer.h"
#include "xAODBTagging/BTaggingContainer.h"

TrigBjetMonitorAlgorithm::TrigBjetMonitorAlgorithm( const std::string& name, ISvcLocator* pSvcLocator )
  : AthMonitorAlgorithm(name,pSvcLocator)
{}

TrigBjetMonitorAlgorithm::~TrigBjetMonitorAlgorithm() {}


StatusCode TrigBjetMonitorAlgorithm::initialize() {
  ATH_CHECK( m_muonContainerKey.initialize() );

  ATH_CHECK( m_offlineVertexContainerKey.initialize(m_collisionRun) );
  ATH_CHECK( m_onlineVertexContainerKey.initialize(m_collisionRun) );
  ATH_CHECK( m_onlineTrackContainerKey.initialize() );
  ATH_CHECK( m_trigDecTool.retrieve() );

  return AthMonitorAlgorithm::initialize();
}

bool LLR(double pu, double pc, double pb, double &w)  {
  w = -100.;
  bool ll = false;
  double denom;
  float cfrac(0.018); // DG 2022/07/28
  if (pb > 0.) {
    denom = pu*(1.-cfrac)+pc*cfrac;
    if (denom > 0.) {
      w = log(pb/denom);
      ll = true;
    }
  }
  return ll; 
}


bool CalcRelPt (float muonPt, float muonEta, float muonPhi, float jetPt, float jetEta, float jetPhi, float &RelPt) {

  bool r = false;
  RelPt = -20.;

  float muonT, muonX, muonY, muonZ, muon, jetT, jetX, jetY, jetZ, jet, scprod;

  muonT = 2.*atan( exp(-muonEta) );
  jetT = 2.*atan( exp(-jetEta) );
  if ( (std::abs(muonT) > 0.) && (std::abs(jetT) > 0.) ) { 
    muon = muonPt/std::abs( sin(muonT) );
    muonX = muonPt*cos(muonPhi); 
    muonY = muonPt*sin(muonPhi);
    muonZ = muon*cos(muonT);
    jet = jetPt/std::abs( sin(jetT) );
    jetX = jetPt*cos(jetPhi); 
    jetY = jetPt*sin(jetPhi);
    jetZ = jet*cos(jetT);
    scprod = (muonX*jetX + muonY*jetY + muonZ*jetZ)/(muon*jet);
    scprod *= scprod;
    if ( (1. - scprod) > 0. ) { 
      RelPt = muon * sqrt(1. - scprod);
      r = true;
    }
  }

  return r;

}

float phiCorr(float phi) {
  if (phi < -M_PI) phi += 2*M_PI;
  if (phi >  M_PI) phi -= 2*M_PI;
  return phi;
}
 

StatusCode TrigBjetMonitorAlgorithm::fillHistograms( const EventContext& ctx ) const {
  using namespace Monitored;


  // Read off-line PV's  and fill histograms

  bool Eofflinepv(false);
  float offlinepvz(-1.e6);
  float offlinepvx(-1.e6);
  float offlinepvy(-1.e6);
 
  if (m_collisionRun) {
    auto OffNVtx = Monitored::Scalar<int>("Off_NVtx",0);
    auto OffxVtx = Monitored::Scalar<float>("Off_xVtx",0.0);
    auto OffyVtx = Monitored::Scalar<float>("Off_yVtx",0.0);
    auto OffzVtx = Monitored::Scalar<float>("Off_zVtx",0.0);

    SG::ReadHandle<xAOD::VertexContainer> offlinepv = SG::makeHandle( m_offlineVertexContainerKey, ctx );
    if (! offlinepv.isValid() ) {
      ATH_MSG_ERROR("evtStore() does not contain VertexContainer Collection with name "<< m_offlineVertexContainerKey);
      return StatusCode::FAILURE;
    }
    ATH_MSG_DEBUG(" Size of the Off-line PV container: " << offlinepv->size() );
    if ( offlinepv->size() ) {
      Eofflinepv = true;
      offlinepvz = offlinepv->front()->z();
      offlinepvx = offlinepv->front()->x();
      offlinepvy = offlinepv->front()->y();
      OffNVtx = offlinepv->size() ;
      for (unsigned int j = 0; j<offlinepv->size(); j++){
	OffxVtx = (*(offlinepv))[j]->x();
	OffyVtx = (*(offlinepv))[j]->y();
	OffzVtx = (*(offlinepv))[j]->z();
	fill("TrigBjetMonitor",OffxVtx,OffyVtx,OffzVtx);
      }
      fill("TrigBjetMonitor",OffNVtx);
    } // if size
  } // if m_collisionRun

  // print the trigger chain names 

  std::string chainName;
  
  int size_AllChains = m_allChains.size();
  ATH_MSG_DEBUG(" Size of the AllChains trigger container: " << size_AllChains );
  for (int i =0; i<size_AllChains; i++){
    chainName = m_allChains[i];
    ATH_MSG_DEBUG("  Chain number: " << i << " AllChains Chain Name: " << chainName );
  }
  
  // Verifiy if the trigger chain was fired and if yes, fill the corresponding histogram
  
  bool mujetChain(false);
  bool bjetChain(true);
  
  

  for ( auto& trigName : m_allChains ) {

    
    if ( m_trigDecTool->isPassed(trigName) ) {
      ATH_MSG_DEBUG(" Trigger chain from AllChains list: " << trigName << " has fired !!! " );
      
      // Verify if the chain was in the Express Stream if the job was an express job
      
      const unsigned int passBits = m_trigDecTool->isPassedBits(trigName);
      const bool expressPass = passBits & TrigDefs::Express_passed;

      ATH_MSG_DEBUG( " Express Stream Test: Chain: " << trigName<< " m_expressStreamFlag: " << m_expressStreamFlag << " expressPass: " << expressPass );

      if ( !m_expressStreamFlag || (m_expressStreamFlag && expressPass) ) {
	
	
	// bjet vs mujet
	mujetChain = false;
	bjetChain = true;
	std::size_t found = trigName.find("HLT_mu");
	if (found!=std::string::npos) {
	  mujetChain = true;
	  bjetChain = false;
	}// found
	
      
	ATH_MSG_DEBUG("  ===> Run 3 access to Trigger Item: " << trigName);
	
	// online track container 
	SG::ReadHandle<xAOD::TrackParticleContainer> theTracks(m_onlineTrackContainerKey, ctx);
	// verify the content 
	for ( const xAOD::TrackParticle* track : *theTracks ) {
	  ATH_MSG_DEBUG( " Pt of track in TrackParticleContainer: " << track->pt() );
	}
	
	float zPrmVtx = 0.; // used for muon-jets
	
	// Online Primary Vertex from SG

	if (m_collisionRun) { 
	  SG::ReadHandle<xAOD::VertexContainer> vtxContainer = SG::makeHandle( m_onlineVertexContainerKey, ctx );
	  int nPV = 0;
	  for (const xAOD::Vertex* vtx : *vtxContainer) {
	    if (vtx->vertexType() == xAOD::VxType::PriVtx) {
	      nPV++;
	      std::string NameH = "PVz_tr_"+trigName;
	      ATH_MSG_DEBUG( " NameH: " << NameH  );
	      auto PVz_tr = Monitored::Scalar<float>(NameH,0.0);
	      PVz_tr = vtx->z();
	      zPrmVtx = PVz_tr;
	      ATH_MSG_DEBUG("        PVz_tr: " << PVz_tr);
	      fill("TrigBjetMonitor",PVz_tr);
	      if (Eofflinepv) {
		NameH = "DiffOnOffPVz_tr_"+trigName;
		ATH_MSG_DEBUG( " NameH: " << NameH  );
		auto DiffOnOffPVz_tr = Monitored::Scalar<float>(NameH,0.0);
		DiffOnOffPVz_tr = vtx->z()-offlinepvz;
		ATH_MSG_DEBUG("        DiffOnOffPVz_tr: " << DiffOnOffPVz_tr);
		fill("TrigBjetMonitor",DiffOnOffPVz_tr);
	      } // if Eofflinepv
	      NameH = "PVx_tr_"+trigName;
	      ATH_MSG_DEBUG( " NameH: " << NameH  );
	      auto PVx_tr = Monitored::Scalar<float>(NameH,0.0);
	      PVx_tr = vtx->x();
	      ATH_MSG_DEBUG("        PVx_tr: " << PVx_tr);
	      fill("TrigBjetMonitor",PVx_tr);
	      if (Eofflinepv) {
		NameH = "DiffOnOffPVx_tr_"+trigName;
		ATH_MSG_DEBUG( " NameH: " << NameH  );
		auto DiffOnOffPVx_tr = Monitored::Scalar<float>(NameH,0.0);
		DiffOnOffPVx_tr = vtx->x()-offlinepvx;
		ATH_MSG_DEBUG("        DiffOnOffPVx_tr: " << DiffOnOffPVx_tr);
		fill("TrigBjetMonitor",DiffOnOffPVx_tr);
	      } // if Eofflinepv
	      NameH = "PVy_tr_"+trigName;
	      ATH_MSG_DEBUG( " NameH: " << NameH  );
	      auto PVy_tr = Monitored::Scalar<float>(NameH,0.0);
	      PVy_tr = vtx->y();
	      ATH_MSG_DEBUG("        PVy_tr: " << PVy_tr);
	      fill("TrigBjetMonitor",PVy_tr);
	      if (Eofflinepv) {
		NameH = "DiffOnOffPVy_tr_"+trigName;
		ATH_MSG_DEBUG( " NameH: " << NameH  );
		auto DiffOnOffPVy_tr = Monitored::Scalar<float>(NameH,0.0);
		DiffOnOffPVy_tr = vtx->y()-offlinepvy;
		ATH_MSG_DEBUG("        DiffOnOffPVy_tr: " << DiffOnOffPVy_tr);
		fill("TrigBjetMonitor",DiffOnOffPVy_tr);
	      } // if Eofflinepv
	    } // if vtx type
	  } // loop on vtxContainer
	  std::string NpvH = "nPV_tr_"+trigName;
	  ATH_MSG_DEBUG( " NpvH: " << NpvH  );
	  auto nPV_tr = Monitored::Scalar<int>(NpvH,0.0);
	  nPV_tr = nPV;
	  fill("TrigBjetMonitor",nPV_tr);
	} // if m_collisionRun
	
	if (mujetChain) {
	  std::vector< TrigCompositeUtils::LinkInfo<xAOD::MuonContainer> > onlinemuons = m_trigDecTool->features<xAOD::MuonContainer>(trigName, TrigDefs::Physics); // TM 2022-05-16
	  int imuon = 0;
	  std::string nMuonH = "nMuon_"+trigName;
	  auto nMuon = Monitored::Scalar<int>(nMuonH,0.0);
	  nMuon = onlinemuons.size();
	  fill("TrigBjetMonitor",nMuon);
	  
	  std::vector< TrigCompositeUtils::LinkInfo<xAOD::JetContainer> > onlinejets = m_trigDecTool->features<xAOD::JetContainer>(trigName, TrigDefs::Physics); // TM 2021-10-30
	  int ijet = 0;
	  std::string nJetH = "nJet_"+trigName;
	  auto nJet = Monitored::Scalar<int>(nJetH,0.0);
	  nJet = onlinejets.size();
	  fill("TrigBjetMonitor",nJet);
	  
	  float muonPt1(0.), muonEta1(0.), muonPhi1(0.), muonZ1(0.), jetPt1(0.), jetEta1(0.), jetPhi1(0.), jetZ1(0.), muonZ(0.);
	  double GN1_mv(0.);
	  double DL1d_mv(0.);
	  bool theLLR(false), theLLR_DL1d(false), theLLR_GN1(false);
	  bool plotDeltaZ(false);
	  
	  for(const auto& muonLinkInfo : onlinemuons) {
	    const xAOD::Muon* muon = *(muonLinkInfo.link);
	    // muonPt
	    std::string NameH = "muonPt_"+trigName;
	    ATH_MSG_DEBUG( " NameH: " << NameH  );
	    auto muonPt = Monitored::Scalar<float>(NameH,0.0);
	    muonPt = (muon->pt())*1.e-3;
	    ATH_MSG_DEBUG("        muonPt: " << muonPt);
	    fill("TrigBjetMonitor",muonPt);
	    // muonEta
	    NameH = "muonEta_"+trigName;
	    ATH_MSG_DEBUG( " NameH: " << NameH  );
	    auto muonEta = Monitored::Scalar<float>(NameH,0.0);
	    muonEta = muon->eta();
	    ATH_MSG_DEBUG("        muonEta: " << muonEta);
	    fill("TrigBjetMonitor",muonEta);
	    // muonPhi
	    NameH = "muonPhi_"+trigName;
	    ATH_MSG_DEBUG( " NameH: " << NameH  );
	    auto muonPhi = Monitored::Scalar<float>(NameH,0.0);
	    muonPhi = muon->phi();
	    ATH_MSG_DEBUG("        muonPhi : " << muonPhi);
	    // muonZ
	    auto link = muon->combinedTrackParticleLink();    // TM and DG 18/06/22
	    if (link.isValid()) {
	      plotDeltaZ = true;
	      const xAOD::TrackParticle* track = *link;
	      muonZ = track->z0() + track->vz(); 
	    } else {
	      plotDeltaZ = false;
	      muonZ = 0.;
	    }
	    
	    if (imuon == 0) {
	      //store the parameter for the 1st muon
	      muonPt1 = muonPt;
	      muonEta1 = muonEta;
	      muonPhi1 = muonPhi;
	      muonZ1 = muonZ;
	    }// if imuon==0
	    
	    // The associated jet loop 
	    for(const auto& jetLinkInfo : onlinejets) {
	      const xAOD::Jet* jet = *(jetLinkInfo.link);
	      // jetPt
	      NameH = "jetPt_"+trigName;
	      ATH_MSG_DEBUG( " NameH: " << NameH  );
	      auto jetPt = Monitored::Scalar<float>(NameH,0.0);
	      jetPt = (jet->pt())*1.e-3;
	      ATH_MSG_DEBUG("        jetPt: " << jetPt);
	      fill("TrigBjetMonitor",jetPt);
	      // jetEta
	      NameH = "jetEta_"+trigName;
	      ATH_MSG_DEBUG( " NameH: " << NameH  );
	      auto jetEta = Monitored::Scalar<float>(NameH,0.0);
	      jetEta = jet->eta();
	      ATH_MSG_DEBUG("        jetEta : " << jetEta);
	      fill("TrigBjetMonitor",jetEta);
	      // jetPhi
	      NameH = "jetPhi_"+trigName;
	      ATH_MSG_DEBUG( " NameH: " << NameH  );
	      auto jetPhi = Monitored::Scalar<float>(NameH,0.0);
	      jetPhi = jet->phi();
	      ATH_MSG_DEBUG("        jetPhi : " << jetPhi);
	      
	      // Take the b-tagging info from the first jet
	      if (ijet == 0) {
		//store the parameter for the 1st jet
		jetPt1 = jetPt;
		jetEta1 = jetEta;
		jetPhi1 = jetPhi;
		jetZ1 = zPrmVtx;
		
		auto btaggingLinkInfo = TrigCompositeUtils::findLink<xAOD::BTaggingContainer>(jetLinkInfo.source, m_btaggingLinkName); // TM 2021-10-30 

		if ( btaggingLinkInfo.isValid() ) {

		  const xAOD::BTagging* btag = *(btaggingLinkInfo.link);
		  
		  double GN1_pu(0.), GN1_pc(0.), GN1_pb(0.);
		  btag->pu("GN120220813",GN1_pu);
		  ATH_MSG_DEBUG("        GN1_pu: " << GN1_pu);
		  btag->pc("GN120220813",GN1_pc);
		  ATH_MSG_DEBUG("        GN1_pc: " << GN1_pc);
		  btag->pb("GN120220813",GN1_pb);
		  ATH_MSG_DEBUG("        GN1_pb: " << GN1_pb);
		  theLLR = LLR (GN1_pu, GN1_pc, GN1_pb, GN1_mv);
		  theLLR_GN1 = theLLR;
		  if ( !theLLR ) GN1_mv=-100.;
		  ATH_MSG_DEBUG("        GN1_mv: " << GN1_mv << " LLR: " << theLLR); 
		  
		  double DL1d_pu(0.), DL1d_pc(0.), DL1d_pb(0.);
		  btag->pu("DL1d20211216",DL1d_pu);
		  ATH_MSG_DEBUG("        DL1d_pu: " << DL1d_pu);
		  btag->pc("DL1d20211216",DL1d_pc);
		  ATH_MSG_DEBUG("        DL1d_pc: " << DL1d_pc);
		  btag->pb("DL1d20211216",DL1d_pb);
		  ATH_MSG_DEBUG("        DL1d_pb: " << DL1d_pb);
		  theLLR = LLR (DL1d_pu, DL1d_pc, DL1d_pb, DL1d_mv);
		  theLLR_DL1d = theLLR;
		  if ( !theLLR ) DL1d_mv=-100.;
		  ATH_MSG_DEBUG("        DL1d_mv: " << DL1d_mv << " LLR: " << theLLR);
		  
		}

	      }// if ijet==0
	      
	      ijet++;
	      
	    }// for onlinejets
	    
	    imuon++;
	    
	  }// for onlinemuons
	  
	  // muon vs jet histograms
	  
	  // Delta R(muon,jet)
	  std::string DeltaRH = "DeltaR_"+trigName;
	  ATH_MSG_DEBUG( " DeltaRH: " << DeltaRH  );
	  auto DeltaR = Monitored::Scalar<float>(DeltaRH,0.0);
	  float DeltaEta = muonEta1 - jetEta1;
	  float DeltaPhi = phiCorr( phiCorr(muonPhi1) - phiCorr(jetPhi1) );
	  DeltaR = sqrt( DeltaEta*DeltaEta + DeltaPhi*DeltaPhi );
	  ATH_MSG_DEBUG("       Delta R : " << DeltaR);
	  fill("TrigBjetMonitor",DeltaR);
	  
	  // Delta Z(muon,jet)
	  std::string DeltaZH = "DeltaZ_"+trigName;
	  ATH_MSG_DEBUG( " DeltaZH: " << DeltaZH  );
	  auto DeltaZ = Monitored::Scalar<float>(DeltaZH,0.0);
	  DeltaZ = std::abs(muonZ1-jetZ1);
	  ATH_MSG_DEBUG("       Delta Z : " << DeltaZ);
	  if (plotDeltaZ) fill("TrigBjetMonitor",DeltaZ);
	  
	  // muonPt/jetPt
	  std::string RatioPtH = "RatioPt_"+trigName;
	  ATH_MSG_DEBUG( " RatioPtH: " << RatioPtH  );
	  auto RatioPt = Monitored::Scalar<float>(RatioPtH,0.0);
	  RatioPt = -100.;
	  if (jetPt1 > 0.) RatioPt = muonPt1/jetPt1;
	  ATH_MSG_DEBUG("        RatioPt : " << RatioPt);
	  if (RatioPt > 0.) fill("TrigBjetMonitor",RatioPt);
	  
	  // muonPt relative to jet direction
	  std::string RelPtH = "RelPt_"+trigName;
	  ATH_MSG_DEBUG( " RelPtH: " << RelPtH  );
	  auto RelPt = Monitored::Scalar<float>(RelPtH,0.0);
	  RelPt = 1.e10;
	  bool calc_relpt = CalcRelPt (muonPt1, muonEta1, muonPhi1, jetPt1, jetEta1, jetPhi1, RelPt);
	  ATH_MSG_DEBUG("        RelPt : " << RelPt);	

	  // wGN1
	  std::string wGN1H = "wGN1_"+trigName;
	  ATH_MSG_DEBUG( " NameH: " << wGN1H  );
	  auto wGN1 = Monitored::Scalar<float>(wGN1H,0.0);
	  wGN1 = float(GN1_mv);
	  ATH_MSG_DEBUG("        wGN1: " << wGN1 << " RelPt : " << RelPt);
	  if (calc_relpt && theLLR_GN1) fill("TrigBjetMonitor",wGN1,RelPt);
	  
	  // wDL1d
	  std::string wDL1dH = "wDL1d_"+trigName;
	  ATH_MSG_DEBUG( " NameH: " << wDL1dH  );
	  auto wDL1d = Monitored::Scalar<float>(wDL1dH,0.0);
	  wDL1d = float(DL1d_mv);
	  ATH_MSG_DEBUG("        wDL1d: " << wDL1d << " RelPt : " << RelPt);
	  if (calc_relpt && theLLR_DL1d) fill("TrigBjetMonitor",wDL1d,RelPt);
	  
	  
	  
	}// if mujetChain
	
	// bjet chains
	if (bjetChain) {
	  
	  // Jets and PV and tracks through jet link
	  
	  std::vector< TrigCompositeUtils::LinkInfo<xAOD::JetContainer> > onlinejets = m_trigDecTool->features<xAOD::JetContainer>(trigName, TrigDefs::Physics); // TM 2021-10-30
	  
	  int ijet = 0;
	  int itrack = 0;
	  std::string nJetH = "nJet_"+trigName;
	  auto nJet = Monitored::Scalar<int>(nJetH,0.0);
	  nJet = onlinejets.size();
	  fill("TrigBjetMonitor",nJet);
	  for(const auto& jetLinkInfo : onlinejets) {
	    const xAOD::Jet* jet = *(jetLinkInfo.link);
	    // jetPt
	    std::string NameH = "jetPt_"+trigName;
	    ATH_MSG_DEBUG( " NameH: " << NameH  );
	    auto jetPt = Monitored::Scalar<float>(NameH,0.0);
	    jetPt = (jet->pt())*1.e-3;
	    ATH_MSG_DEBUG("        jetPt: " << jetPt);
	    fill("TrigBjetMonitor",jetPt);
	    // jetEta
	    NameH = "jetEta_"+trigName;
	    ATH_MSG_DEBUG( " NameH: " << NameH  );
	    auto jetEta = Monitored::Scalar<float>(NameH,0.0);
	    jetEta = jet->eta();
	    // jetPhi
	    NameH = "jetPhi_"+trigName;
	    ATH_MSG_DEBUG( " NameH: " << NameH  );
	    auto jetPhi = Monitored::Scalar<float>(NameH,0.0);
	    jetPhi = jet->phi();
	    ATH_MSG_DEBUG("        jetEta: " << jetEta << " jetPhi : " << jetPhi);
	    fill("TrigBjetMonitor",jetEta,jetPhi);
	    
	    // zPV associated to the jets in the same event: they are the same for every jet in the same event so only the first zPV should be plotted
	    if (ijet == 0) {
	      
	      // Fetch and plot PV
	      
	      std::string vtxname = m_onlineVertexContainerKey.key();
	      if ( vtxname.compare(0, 4, "HLT_")==0 ) vtxname.erase(0,4);
	      auto vertexLinkInfo = TrigCompositeUtils::findLink<xAOD::VertexContainer>(jetLinkInfo.source, vtxname ); // CV 200120 & MS 290620
	      ATH_CHECK( vertexLinkInfo.isValid() ) ; // TM 200120
	      const xAOD::Vertex* vtx = *(vertexLinkInfo.link);
	      NameH = "PVz_jet_"+trigName;
	      ATH_MSG_DEBUG( " NameH: " << NameH  );
	      auto PVz_jet = Monitored::Scalar<float>(NameH,0.0);
	      PVz_jet = vtx->z();
	      ATH_MSG_DEBUG("        PVz_jet: " << PVz_jet);
	      fill("TrigBjetMonitor",PVz_jet);
	      NameH = "PVx_jet_"+trigName;
	      ATH_MSG_DEBUG( " NameH: " << NameH  );
	      auto PVx_jet = Monitored::Scalar<float>(NameH,0.0);
	      PVx_jet = vtx->x();
	      ATH_MSG_DEBUG("        PVx_jet: " << PVx_jet);
	      fill("TrigBjetMonitor",PVx_jet);
	      NameH = "PVy_jet_"+trigName;
	      ATH_MSG_DEBUG( " NameH: " << NameH  );
	      auto PVy_jet = Monitored::Scalar<float>(NameH,0.0);
	      PVy_jet = vtx->y();
	      ATH_MSG_DEBUG("        PVy_jet: " << PVy_jet);
	      fill("TrigBjetMonitor",PVy_jet);
	      
	      
	    } // if (ijet == 0)
	    
	    ijet++;
	    
	    // Fetch and plot BTagging information
	    
	    auto btaggingLinkInfo = TrigCompositeUtils::findLink<xAOD::BTaggingContainer>(jetLinkInfo.source, m_btaggingLinkName); // TM 2021-10-30 
	    ATH_CHECK( btaggingLinkInfo.isValid() ) ;
	    const xAOD::BTagging* btag = *(btaggingLinkInfo.link);
	    
	    
	    // SV1 variables (credit LZ)
	    NameH = "xNVtx_tr_"+trigName;
	    ATH_MSG_DEBUG( " NameH: " << NameH  );
	    auto svp_n2t = Monitored::Scalar<int>(NameH,0.0);
	    btag->variable<int>("SV1", "N2Tpair", svp_n2t);
	    ATH_MSG_DEBUG("        svp_n2t: " << svp_n2t);
	    fill("TrigBjetMonitor",svp_n2t);
	    
	    NameH = "xMVtx_tr_"+trigName;
	    ATH_MSG_DEBUG( " NameH: " << NameH  );
	    auto svp_mass = Monitored::Scalar<float>(NameH,0.0);
	    btag->variable<float>("SV1", "masssvx", svp_mass);
	    svp_mass *= 1.e-3;
	    ATH_MSG_DEBUG("        svp_mass in GeV: " << svp_mass );
	    fill("TrigBjetMonitor",svp_mass);
	    
	    if (svp_mass > 0) {
	      NameH = "xEVtx_tr_"+trigName;
	      ATH_MSG_DEBUG( " NameH: " << NameH  );
	      auto svp_efrc = Monitored::Scalar<float>(NameH,0.0);
	      btag->variable<float>("SV1", "efracsvx", svp_efrc);
	      ATH_MSG_DEBUG("        svp_efrc: " << svp_efrc);
	      fill("TrigBjetMonitor",svp_efrc);
	    }
	    
	    // JF variables (a la LZ)
	    NameH = "JFxNVtx_tr_"+trigName;
	    ATH_MSG_DEBUG( " NameH: " << NameH  );
	    auto jf_n2t = Monitored::Scalar<int>(NameH,0.0);
	    btag->variable<int>("JetFitter", "N2Tpair", jf_n2t);
	    ATH_MSG_DEBUG("        jf_n2t: " << jf_n2t);
	    fill("TrigBjetMonitor",jf_n2t);
	    
	    NameH = "JFxSig_tr_"+trigName;
	    ATH_MSG_DEBUG( " NameH: " << NameH  );
	    auto jf_sig3 = Monitored::Scalar<float>(NameH,0.0);
	    btag->variable<float>("JetFitter", "significance3d", jf_sig3);
	    ATH_MSG_DEBUG("        jf_sig3: " << jf_sig3);
	    fill("TrigBjetMonitor",jf_sig3);
	    
	    NameH = "JFxMVtx_tr_"+trigName;
	    ATH_MSG_DEBUG( " NameH: " << NameH  );
	    auto jf_mass = Monitored::Scalar<float>(NameH,0.0);
	    btag->variable<float>("JetFitter", "mass", jf_mass);
	    jf_mass *= 1.e-3;
	    ATH_MSG_DEBUG("        jf_mass in GeV: " << jf_mass );
	    fill("TrigBjetMonitor",jf_mass);
	    
	    NameH = "JFxEVtx_tr_"+trigName;
	    ATH_MSG_DEBUG( " NameH: " << NameH  );
	    auto jf_efrc = Monitored::Scalar<float>(NameH,0.0);
	    btag->variable<float>("JetFitter", "energyFraction", jf_efrc);
	    ATH_MSG_DEBUG("        jf_efrc: " << jf_efrc);
	    fill("TrigBjetMonitor",jf_efrc);
	    
	    
	    bool theLLR(false);	    
	    NameH = "GN1_pu_tr_"+trigName;
	    ATH_MSG_DEBUG( " NameH: " << NameH  );
	    auto GN1_pu = Monitored::Scalar<double>(NameH,0.0);
	    btag->pu("GN120220813",GN1_pu);
	    ATH_MSG_DEBUG("        GN1_pu: " << GN1_pu);
	    fill("TrigBjetMonitor",GN1_pu);
	    
	    NameH = "GN1_pc_tr_"+trigName;
	    ATH_MSG_DEBUG( " NameH: " << NameH  );
	    auto GN1_pc = Monitored::Scalar<double>(NameH,0.0);
	    btag->pc("GN120220813",GN1_pc);
	    ATH_MSG_DEBUG("        GN1_pc: " << GN1_pc);
	    fill("TrigBjetMonitor",GN1_pc);
	    
	    NameH = "GN1_pb_tr_"+trigName;
	    ATH_MSG_DEBUG( " NameH: " << NameH  );
	    auto GN1_pb = Monitored::Scalar<double>(NameH,0.0);
	    btag->pb("GN120220813",GN1_pb);
	    ATH_MSG_DEBUG("        GN1_pb: " << GN1_pb);
	    fill("TrigBjetMonitor",GN1_pb);
	    
	    NameH = "GN1_mv_tr_"+trigName;
	    ATH_MSG_DEBUG( " NameH: " << NameH  );
	    auto GN1_mv = Monitored::Scalar<double>(NameH,0.0);
	    theLLR = LLR (GN1_pu, GN1_pc, GN1_pb, GN1_mv);
	    if ( theLLR ) fill("TrigBjetMonitor",GN1_mv);
	    ATH_MSG_DEBUG("        GN1_mv: " << GN1_mv << " LLR: " << theLLR); 
	    
	    

	    NameH = "DL1d_pu_tr_"+trigName;
	    ATH_MSG_DEBUG( " NameH: " << NameH  );
	    auto DL1d_pu = Monitored::Scalar<double>(NameH,0.0);
	    btag->pu("DL1d20211216",DL1d_pu);
	    ATH_MSG_DEBUG("        DL1d_pu: " << DL1d_pu);
	    fill("TrigBjetMonitor",DL1d_pu);
	    
	    NameH = "DL1d_pc_tr_"+trigName;
	    ATH_MSG_DEBUG( " NameH: " << NameH  );
	    auto DL1d_pc = Monitored::Scalar<double>(NameH,0.0);
	    btag->pc("DL1d20211216",DL1d_pc);
	    ATH_MSG_DEBUG("        DL1d_pc: " << DL1d_pc);
	    fill("TrigBjetMonitor",DL1d_pc);
	    
	    NameH = "DL1d_pb_tr_"+trigName;
	    ATH_MSG_DEBUG( " NameH: " << NameH  );
	    auto DL1d_pb = Monitored::Scalar<double>(NameH,0.0);
	    btag->pb("DL1d20211216",DL1d_pb);
	    ATH_MSG_DEBUG("        DL1d_pb: " << DL1d_pb);
	    fill("TrigBjetMonitor",DL1d_pb);
	    
	    NameH = "DL1d_mv_tr_"+trigName;
	    ATH_MSG_DEBUG( " NameH: " << NameH  );
	    auto DL1d_mv = Monitored::Scalar<double>(NameH,0.0);
	    theLLR = LLR (DL1d_pu, DL1d_pc, DL1d_pb, DL1d_mv);
	    if ( theLLR ) fill("TrigBjetMonitor",DL1d_mv);
	    ATH_MSG_DEBUG("        DL1d_mv: " << DL1d_mv << " LLR: " << theLLR); 
	    
	    
	    
	    NameH = "DIPSL_pu_tr_"+trigName;
	    ATH_MSG_DEBUG( " NameH: " << NameH  );
	    auto DIPSL_pu = Monitored::Scalar<double>(NameH,0.0);
	    btag->pu("dips20211116",DIPSL_pu);
	    ATH_MSG_DEBUG("        DIPSL_pu: " << DIPSL_pu);
	    fill("TrigBjetMonitor",DIPSL_pu);
	    
	    NameH = "DIPSL_pc_tr_"+trigName;
	    ATH_MSG_DEBUG( " NameH: " << NameH  );
	    auto DIPSL_pc = Monitored::Scalar<double>(NameH,0.0);
	    btag->pc("dips20211116",DIPSL_pc);
	    ATH_MSG_DEBUG("        DIPSL_pc: " << DIPSL_pc);
	    fill("TrigBjetMonitor",DIPSL_pc);
	    
	    NameH = "DIPSL_pb_tr_"+trigName;
	    ATH_MSG_DEBUG( " NameH: " << NameH  );
	    auto DIPSL_pb = Monitored::Scalar<double>(NameH,0.0);
	    btag->pb("dips20211116",DIPSL_pb);
	    ATH_MSG_DEBUG("        DIPSL_pb: " << DIPSL_pb);
	    fill("TrigBjetMonitor",DIPSL_pb);
	    
	    // Tracks associated to triggered jets ( featurs = onlinejets ) courtesy of Tim Martin on 12/05/2020 
	    const auto track_it_pair = m_trigDecTool->associateToEventView(theTracks, jetLinkInfo.source, "roi");
	    const xAOD::TrackParticleContainer::const_iterator start_it = track_it_pair.first;
	    const xAOD::TrackParticleContainer::const_iterator end_it = track_it_pair.second;
	    
	    int count = 0;
	    for ( xAOD::TrackParticleContainer::const_iterator it = start_it; it != end_it; ++it) {
	      count++;
	      ATH_MSG_DEBUG( " Track " << count << " with pT " << (*it)->pt() <<" from BJet with pT " << (*jetLinkInfo.link)->pt() );
	      ATH_MSG_DEBUG( " Track " << count << " with pT/eta/phi " << (*it)->pt() << "/" << (*it)->eta() << "/" << (*it)->phi() );
	      ATH_MSG_DEBUG( " Track " << count << " with d0/sigd0 " << (*it)->d0() << "/" << Amg::error((*it)->definingParametersCovMatrix(), 0) );
	      ATH_MSG_DEBUG( " Track " << count << " with z0/sigz0 " << (*it)->z0() << "/" << Amg::error((*it)->definingParametersCovMatrix(), 1) );
	      std::string NameH = "trkPt_"+trigName;
	      ATH_MSG_DEBUG( " NameH: " << NameH  );
	      auto trkPt = Monitored::Scalar<float>(NameH,0.0);
	      trkPt = ((*it)->pt())*1.e-3;
	      ATH_MSG_DEBUG("        trkPt: " << trkPt);
	      fill("TrigBjetMonitor",trkPt);
	      NameH = "trkEta_"+trigName;
	      ATH_MSG_DEBUG( " NameH: " << NameH  );
	      auto trkEta = Monitored::Scalar<float>(NameH,0.0);
	      trkEta = (*it)->eta();
	      NameH = "trkPhi_"+trigName;
	      ATH_MSG_DEBUG( " NameH: " << NameH  );
	      auto trkPhi = Monitored::Scalar<float>(NameH,0.0);
	      trkPhi = (*it)->phi();
	      ATH_MSG_DEBUG("        trkEta: " << trkEta << " trkPhi : " << trkPhi);
	      fill("TrigBjetMonitor",trkEta,trkPhi);
	      NameH = "d0_"+trigName;
	      ATH_MSG_DEBUG( " NameH: " << NameH  );
	      auto d0 = Monitored::Scalar<float>(NameH,0.0);
	      d0 = (*it)->d0();
	      ATH_MSG_DEBUG("        d0: " << d0);
	      fill("TrigBjetMonitor",d0);
	      NameH = "z0_"+trigName;
	      ATH_MSG_DEBUG( " NameH: " << NameH  );
	      auto z0 = Monitored::Scalar<float>(NameH,0.0);
	      z0 = (*it)->z0();
	      ATH_MSG_DEBUG("        z0: " << z0);
	      fill("TrigBjetMonitor",z0);
	      NameH = "ed0_"+trigName;
	      ATH_MSG_DEBUG( " NameH: " << NameH  );
	      auto ed0 = Monitored::Scalar<float>(NameH,0.0);
	      ed0 = Amg::error((*it)->definingParametersCovMatrix(), 0);
	      ATH_MSG_DEBUG("        ed0: " << ed0);
	      fill("TrigBjetMonitor",ed0);
	      NameH = "sd0_"+trigName;
	      ATH_MSG_DEBUG( " NameH: " << NameH  );
	      auto sd0 = Monitored::Scalar<float>(NameH,0.0);
	      sd0 = -10.;
	      if (ed0 > 0.) sd0 = std::abs(d0)/ed0;
	      ATH_MSG_DEBUG("        sd0: " << sd0);
	      fill("TrigBjetMonitor",sd0);
	      NameH = "ez0_"+trigName;
	      ATH_MSG_DEBUG( " NameH: " << NameH  );
	      auto ez0 = Monitored::Scalar<float>(NameH,0.0);
	      ez0 = Amg::error((*it)->definingParametersCovMatrix(), 1);
	      ATH_MSG_DEBUG("        ez0: " << ez0);
	      fill("TrigBjetMonitor",ez0);
	    } // it on tracks
	    ATH_MSG_DEBUG( "  Number of tracks: " << count );
	    itrack += count;
	    
	  } // jetLinkInfo from onlinejets
	  
	  ATH_MSG_DEBUG("  Total number of triggered b-jets: " << ijet << " nJet : " << nJet);
	  ATH_MSG_DEBUG(" Total number of triggered tracks associated to the b-jets: " << itrack);
	  std::string nTrackH = "nTrack_"+trigName;
	  auto nTrack = Monitored::Scalar<int>(nTrackH,0.0);
	  nTrack = itrack;
	  fill("TrigBjetMonitor",nTrack);
	  
	} //if bjetChain
	
      } else {
	ATH_MSG_DEBUG("  Chain " << trigName << " is declared for the Express Stream but it is NOT in the Express Stream in an Express Job");
      } // if m_expressStreamFlag
      
    } else {
      ATH_MSG_DEBUG( " Trigger chain from AllChains list: " << trigName << " has not fired "  );
    } // trigger not fired
    
    
  } // for AllChains
  
  return StatusCode::SUCCESS;
}
