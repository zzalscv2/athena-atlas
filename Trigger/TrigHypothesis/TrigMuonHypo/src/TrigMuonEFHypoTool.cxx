/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "GaudiKernel/SystemOfUnits.h"

#include "TrigCompositeUtils/Combinators.h"
#include "TrigMuonEFHypoTool.h"
#include "AthenaMonitoringKernel/Monitored.h"
class ISvcLocator;
TrigMuonEFHypoTool::TrigMuonEFHypoTool(const std::string & type, const std::string & name, const IInterface* parent):
  AthAlgTool(type, name, parent),
  m_decisionId(HLT::Identifier::fromToolName(name)){
}
TrigMuonEFHypoTool::~TrigMuonEFHypoTool(){
}
StatusCode TrigMuonEFHypoTool::initialize(){

  if(m_muonqualityCut) {
    if(m_muonSelTool.retrieve().isFailure()) {
      ATH_MSG_ERROR("Unable to retrieve " << m_muonSelTool);
      return StatusCode::FAILURE;
    }
  } else m_muonSelTool.disable();

  if(m_acceptAll) {
    ATH_MSG_INFO("Accepting all the events!");
  } else {
    if(m_ptBins.size()<=0){ 
      ATH_MSG_ERROR("Trying to configure hypo with no pT bins. This is probably a configuration mistake.");
      return StatusCode::FAILURE;
    }
    m_bins.resize(m_ptBins.size());
    for(size_t j=0; j<m_ptBins.size(); j++){
      m_bins[j] = m_ptBins[j].size() - 1;
      if (m_bins[j] != m_ptThresholds[j].size()) {
        ATH_MSG_ERROR("bad thresholds setup .... exiting!");
        return StatusCode::FAILURE;
      }
      if (msgLvl(MSG::DEBUG)) {
        for (std::vector<float>::size_type i=0; i<m_bins[j];++i) {
          ATH_MSG_DEBUG( "bin " << m_ptBins[j][i] << " - " <<  m_ptBins[j][i+1]<<" with Pt Threshold of " << (m_ptThresholds[j][i])/Gaudi::Units::GeV<< " GeV");
        }
      }
    }
  }
  // minimum d0 cut for displaced muon triggers
  if (m_d0min>0.) ATH_MSG_DEBUG( " Rejecting muons with abs(d0) < "<<m_d0min<<" mm");
  
  if (m_runCommissioningChain) ATH_MSG_INFO("m_runCommissioningChain set to true (for absence of NSW)");
  
  if ( not m_monTool.name().empty() ) {
    ATH_CHECK( m_monTool.retrieve() );
    ATH_MSG_DEBUG("MonTool name: " << m_monTool);
  }

  if(m_doSA) m_type = xAOD::Muon::TrackParticleType::ExtrapolatedMuonSpectrometerTrackParticle;
  else m_type = xAOD::Muon::TrackParticleType::CombinedTrackParticle;

  return StatusCode::SUCCESS;
}
bool TrigMuonEFHypoTool::decideOnSingleObject(TrigMuonEFHypoTool::MuonEFInfo& input, size_t cutIndex) const{
  ATH_MSG_DEBUG( "deciding...");
  //Monitored Variables
  std::vector<float> fexPt, fexEta, fexPhi, selPt, selEta, selPhi;
  auto muonPtMon = Monitored::Collection("Pt", fexPt);
  auto muonEtaMon = Monitored::Collection("Eta", fexEta);
  auto muonPhiMon = Monitored::Collection("Phi", fexPhi);
  auto muonPtSelMon = Monitored::Collection("Pt_sel", selPt);
  auto muonEtaSelMon = Monitored::Collection("Eta_sel", selEta);
  auto muonPhiSelMon = Monitored::Collection("Phi_sel", selPhi);
  auto monitorIt	= Monitored::Group(m_monTool, muonPtMon, muonEtaMon, muonPhiMon, muonPtSelMon, muonEtaSelMon, muonPhiSelMon); 
  bool result = false;
  //for pass through mode
  if(m_acceptAll) {
    result = true;
    ATH_MSG_DEBUG("Accept property is set: taking all the events");
    return result;
  }
  //  decision making
  //Get xAOD::MuonContainer from hypotool
  const xAOD::Muon* muon = input.muon;
  if( !muon ){
    ATH_MSG_DEBUG("Retrieval of xAOD::MuonContainer failed");
    return false;
  }

  if (muon->primaryTrackParticle()) { // was there a muon in this RoI ?
    const xAOD::TrackParticle* tr = muon->trackParticle(m_type);
    if (!tr) {
      ATH_MSG_DEBUG("No TrackParticle found.");
    } else {
      ATH_MSG_DEBUG("Retrieved Track track with abs pt "<< (*tr).pt()/Gaudi::Units::GeV << " GeV ");
      //fill monitored variables
      fexPt.push_back(tr->pt()/Gaudi::Units::GeV);
      fexEta.push_back(tr->eta());
      fexPhi.push_back(tr->phi());
      //Apply hypo cuts
      float absEta = std::abs(tr->eta());
      float threshold = 0;
      for (std::vector<float>::size_type k=0; k<m_bins[cutIndex]; ++k) {
        if (absEta > m_ptBins[cutIndex][k] && absEta <= m_ptBins[cutIndex][k+1]) threshold = m_ptThresholds[cutIndex][k];
      }
      if (std::abs(tr->pt())/Gaudi::Units::GeV > (threshold/Gaudi::Units::GeV)){
        result = true;
        // If trigger path name includes "muonqual", check whether the muon passes those criteria   
        if(m_muonqualityCut == true) result = passedQualityCuts(muon);
	//cut on Nprecision layers (for 3layerEC msonly triggers)
	if(m_threeStationCut){
	  uint8_t nGoodPrcLayers=0;
	  if (!muon->summaryValue(nGoodPrcLayers, xAOD::numberOfGoodPrecisionLayers)){
	    ATH_MSG_DEBUG("No numberOfGoodPrecisionLayers variable found; not passing hypo");
	    result=false;
	  }
	  if(std::abs(muon->eta()) > 1.3) {
	    if (m_runCommissioningChain) {
	      if(nGoodPrcLayers < 2){
		ATH_MSG_DEBUG("Muon has less than two GoodPrecisionLayers; not passing hypo (requrement loosend according to absence of NSW)");
		result=false;
	      }
	    } else {
	      if(nGoodPrcLayers < 3){
		ATH_MSG_DEBUG("Muon has less than three GoodPrecisionLayers; not passing hypo");
		result=false;
	      }
	    }
	  } else if (std::abs(muon->eta()) > 1.05) {
	    if(nGoodPrcLayers < 3){
	      ATH_MSG_DEBUG("Muon has less than three GoodPrecisionLayers; not passing hypo");
	      result=false;
	    }
	  }
	}
	if (m_d0min>0.) {
	  ATH_MSG_DEBUG("Muon has d0 less than "<<m_d0min<<"mm; not passing hypo");
	  if (std::abs(tr->d0())<m_d0min) result = false;
	}
      }
      if(result == true){
        selPt.push_back(tr->pt()/Gaudi::Units::GeV);
        selEta.push_back(tr->eta());
        selPhi.push_back(tr->phi());
      }
      if (m_d0min>0.) {
	ATH_MSG_DEBUG(" REGTEST muon pt is " << tr->pt()/Gaudi::Units::GeV << " GeV "
		      << " with Charge " << tr->charge()
		      << " and threshold cut is " << threshold/Gaudi::Units::GeV << " GeV"
		      << " so hypothesis is " << (result?"true":"false"));
      } else {
	ATH_MSG_DEBUG(" REGTEST muon pt is " << tr->pt()/Gaudi::Units::GeV << " GeV "
		      << " with Charge " << tr->charge()
		      << " and with d0 " << tr->d0()
		      << " the threshold cut is " << threshold/Gaudi::Units::GeV << " GeV"
		      << " and d0min cut is " << m_d0min<<" mm"
		      << " so hypothesis is " << (result?"true":"false"));
      }
    }
  }
  return result;	
}

bool TrigMuonEFHypoTool::passedQualityCuts(const xAOD::Muon* muon) const {
    bool passCut = false;
    const xAOD::TrackParticle* idtrack = muon->trackParticle( xAOD::Muon::InnerDetectorTrackParticle );
    const xAOD::TrackParticle* metrack = muon->trackParticle( xAOD::Muon::ExtrapolatedMuonSpectrometerTrackParticle );
    float mePt = -999999., idPt = -999999.;

    float reducedChi2 = -10, qOverPsignif = -10;

    if(idtrack && metrack) {
        mePt = metrack->pt();
        idPt = idtrack->pt();
        float meP  = 1.0 / ( sin(metrack->theta()) / mePt); 
        float idP  = 1.0 / ( sin(idtrack->theta()) / idPt); 
        qOverPsignif  = std::abs( (metrack->charge() / meP) - (idtrack->charge() / idP) ) / sqrt( idtrack->definingParametersCovMatrix()(4,4) + metrack->definingParametersCovMatrix()(4,4) ); 
        reducedChi2 = muon->primaryTrackParticle()->chiSquared()/muon->primaryTrackParticle()->numberDoF(); 
        // Selection criteria based on the requirements that are part of the muon quality working points (offline)
        if(std::abs(reducedChi2) < 8.0 && !m_muonSelTool->isBadMuon(*muon) && qOverPsignif<7.0 && muon->author()==xAOD::Muon::MuidCo) passCut = true;
   }
        
   return passCut;
}
  
StatusCode TrigMuonEFHypoTool::decide(std::vector<MuonEFInfo>& toolInput) const {
  size_t numTrigger = m_ptBins.size();
  size_t numMuon=toolInput.size();
  if(numTrigger==1){
    ATH_MSG_DEBUG("Applying selection of single << " << m_decisionId);
    return inclusiveSelection(toolInput);
  }
  else{
    ATH_MSG_DEBUG("Applying selection of multiplicity "<< m_decisionId<<" with nMuons"<<numMuon);
      return multiplicitySelection(toolInput);
  }
  return StatusCode::SUCCESS;
}
StatusCode TrigMuonEFHypoTool::inclusiveSelection(std::vector<MuonEFInfo>& toolInput) const{
  for (uint i=0; i<toolInput.size(); i++){
    auto& tool = toolInput.at(i);
    if(TrigCompositeUtils::passed(m_decisionId.numeric(), tool.previousDecisionIDs)){
      bool overlap = false;
      if(m_checkOvlp){
	for(uint j=i+1; j<toolInput.size();j++){
	  auto& tool2 = toolInput.at(j);
	  if(TrigCompositeUtils::passed(m_decisionId.numeric(), tool2.previousDecisionIDs))
	    if(tool2.muon->p4()==tool.muon->p4()) overlap=true;
	}
      }
      if(overlap) continue;

      if(decideOnSingleObject(tool, 0)==true){
        ATH_MSG_DEBUG("Passes selection");
        TrigCompositeUtils::addDecisionID(m_decisionId, tool.decision);
      }
      else ATH_MSG_DEBUG("Does not pass selection");
    }
  }
  return StatusCode::SUCCESS;
}
StatusCode TrigMuonEFHypoTool::multiplicitySelection(std::vector<MuonEFInfo>& toolInput) const{
  HLT::Index2DVec passingSelection(m_ptBins.size());
  bool passingNscan=false;
  for(size_t cutIndex=0; cutIndex < m_ptBins.size(); ++cutIndex) {
    size_t elementIndex{0};
    for(auto& tool : toolInput){
      if(TrigCompositeUtils::passed(m_decisionId.numeric(), tool.previousDecisionIDs)){
	if(decideOnSingleObject(tool, cutIndex)==true){
          if(m_nscan && cutIndex==0 && (!passingNscan)){
            ATH_MSG_DEBUG("Applying narrow-scan selection");
            float deta,dphi=10;
            unsigned int nInCone=0;
            float muonR = sqrt( pow(tool.muon->eta(),2) +pow(tool.muon->phi(),2));
      	    float coneCheck=m_conesize*muonR;
            for (auto& tooltmp : toolInput){
              ATH_MSG_DEBUG(">>Testing Muon with pt: "<<tooltmp.muon->pt()/Gaudi::Units::GeV << "GeV, eta: "
                         << tooltmp.muon->eta() << ", phi: " << tooltmp.muon->phi());
              if (tooltmp.muon->p4() == tool.muon->p4()) {
                ATH_MSG_DEBUG("<< same muon, skipping...");
              }else {
        	deta = std::abs(tooltmp.muon->eta()-tool.muon->eta());
                dphi = getdphi(tooltmp.muon->phi(),tool.muon->phi());
                if(deta<coneCheck && dphi<coneCheck){
                  nInCone++;
                }

                ATH_MSG_DEBUG(">> dPhi is: " << dphi);
                ATH_MSG_DEBUG(">> dEta is: " << deta);
                ATH_MSG_DEBUG(">> dR is: " <<sqrt( pow( deta, 2) + pow( dphi, 2) ));

              }  
            } 
            //end test nscan

            if (nInCone > 0) {
              ATH_MSG_DEBUG("Passes narrow-scan selection Index["<<elementIndex<<"]");
              passingNscan=true;
            }else ATH_MSG_DEBUG("Does not pass narrow-scan selection Index["<<elementIndex<<"]");
          }
          ATH_MSG_DEBUG("Passing selection "<<m_decisionId << " , Index["<<elementIndex<<"]");
          passingSelection[cutIndex].push_back(elementIndex); 
	}
	else ATH_MSG_DEBUG("Not passing selection "<<m_decisionId << " , Index["<<elementIndex<<"]");
      }
      else{
	ATH_MSG_DEBUG("No match for decisionId "<<m_decisionId);
      }
      elementIndex++;
    }
    if (m_nscan &&(!passingNscan)){
      ATH_MSG_DEBUG("Narrow-scan is required and no muons passed, all muons will be rejected ");
      return StatusCode::SUCCESS;
    }
    //If nothing passes, then we should stop
    if(passingSelection[cutIndex].empty()){
      ATH_MSG_DEBUG("No muons passed the selection "<<cutIndex<<" rejecting...");
      return StatusCode::SUCCESS;
    }
  }
  std::set<size_t> passingIndices;
  if(m_decisionPerRoI==true){
    auto notFromSameRoI = [&](const HLT::Index1DVec& comb){
      std::set<const xAOD::Muon*> setOfMuons;
      for (auto index : comb){
	setOfMuons.insert(toolInput[index].muon);
      }
      return setOfMuons.size()==comb.size();
    };
  
    HLT::elementsInUniqueCombinations(passingSelection, passingIndices, std::move(notFromSameRoI));
  }
  else{
    HLT::elementsInUniqueCombinations(passingSelection, passingIndices);
  }
  if(passingIndices.empty()){
    ATH_MSG_DEBUG("No muons passed selection "<<m_decisionId);
    return StatusCode::SUCCESS;
  }
  for(auto i : passingIndices){
    ATH_MSG_DEBUG("Muon["<<i<<"] passes "<<m_decisionId<<" with pT = "<<toolInput[i].muon->pt()/Gaudi::Units::GeV << "GeV");
    TrigCompositeUtils::addDecisionID(m_decisionId.numeric(), toolInput[i].decision);
  }
  return StatusCode::SUCCESS;
}
float TrigMuonEFHypoTool::getdphi(float phi1, float phi2) const{
  float dphi = phi1-phi2;
  if(dphi > TMath::Pi()) dphi -= TMath::TwoPi();
  if(dphi < -1*TMath::Pi()) dphi += TMath::TwoPi();
  return fabs(dphi);
}
