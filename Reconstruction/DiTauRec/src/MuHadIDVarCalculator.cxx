/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/**
 * Tau ID input variable calculator tool
 *
 * Author: Lorenz Hauswald
 */

#include "DiTauRec/MuHadIDVarCalculator.h"
#include "xAODTracking/VertexContainer.h"  
#include "xAODEventInfo/EventInfo.h"
#include "CaloGeoHelpers/CaloSampling.h"
#include "FourMomUtils/xAODP4Helpers.h"
#include "TLorentzVector.h"

const float MuHadIDVarCalculator::DEFAULT = -1111.;

MuHadIDVarCalculator::MuHadIDVarCalculator(const std::string& name):
  TauRecToolBase(name),
  m_vertexContainerKey("PrimaryVertices"),
  m_nVtx(1),
  m_mu(0.)
{
  declareProperty("vertexContainerKey", m_vertexContainerKey, "collection of vertices for PileUp estimation" );
}

StatusCode MuHadIDVarCalculator::eventInitialize()
{
  if(!inTrigger()){
    m_nVtx = int(DEFAULT);
    const xAOD::VertexContainer* vertexContainer = nullptr;
    if( evtStore()->retrieve( vertexContainer, m_vertexContainerKey ).isFailure() ){
      ATH_MSG_ERROR("VertexContainer with key " << m_vertexContainerKey << " could not be retrieved.");
      return StatusCode::FAILURE;
    }
    if(vertexContainer){
      m_nVtx = 0;
      for( auto vertex : *vertexContainer ){
	if(!vertex) continue;
	int nTrackParticles = vertex->nTrackParticles();
	if( (nTrackParticles >= 4 && vertex->vertexType() == xAOD::VxType::PriVtx) ||
	    (nTrackParticles >= 2 && vertex->vertexType() == xAOD::VxType::PileUp)){
	  m_nVtx++;
	}
      }
    }
  }

  // accessing mu via EventInfo can also be done for trigger (LumiBlock/LumiBlockComps/src/LumiBlockMuWriter.cxx)
  // that avoids the use of tauEventData in TrigTauDiscriBuilder.cxx which would be needed just for retrieving mu
  const xAOD::EventInfo* m_xEventInfo = nullptr ;
  ATH_CHECK( evtStore()->retrieve(m_xEventInfo,"EventInfo") );
  m_mu = m_xEventInfo->averageInteractionsPerCrossing();

  return StatusCode::SUCCESS;
}

StatusCode MuHadIDVarCalculator::execute(xAOD::TauJet& tau)
{
  //define accessors:
  static SG::AuxElement::Accessor<int> acc_numTrack("NUMTRACK");
  acc_numTrack(tau) = tau.nTracks();

  static SG::AuxElement::Accessor<float> acc_mu("MU");
  acc_mu(tau) = m_mu;

  if(!inTrigger()){
    static SG::AuxElement::Accessor<int> acc_nVertex("NUMVERTICES");
    acc_nVertex(tau) = m_nVtx >= 0 ? m_nVtx : 0.;
  }
  
  if(inTrigger()){
    //for old trigger BDT:
    static SG::AuxElement::Accessor<int> acc_numWideTrk("NUMWIDETRACK");
#ifdef XAODTAU_VERSIONS_TAUJET_V3_H
    acc_numWideTrk(tau) = tau.nTracks(xAOD::TauJetParameters::classifiedIsolation);//the ID should train on nIsolatedTracks which is static!
#else
    acc_numWideTrk(tau) = tau.nWideTracks();
#endif
  }


  static const SG::AuxElement::Accessor<float> acc_absipSigLeadTrk("absipSigLeadTrk");
  float ipSigLeadTrk=0;
  if(!tau.detail(xAOD::TauJetParameters::ipSigLeadTrk, ipSigLeadTrk))
    return StatusCode::FAILURE;
  acc_absipSigLeadTrk(tau) = std::abs(ipSigLeadTrk);
  
  //don't calculate EleBDT variables if run from TrigTauDiscriminant:
  if(inTrigger()) return StatusCode::SUCCESS;
  
  //everything below is just for EleBDT!
  static SG::AuxElement::Accessor<float> acc_absEtaLead("ABS_ETA_LEAD_TRACK"); 
  static SG::AuxElement::Accessor<float> acc_leadTrackProbHT("leadTrackProbHT");
  static SG::AuxElement::Accessor<float> acc_leadTrackEta("leadTrackEta");
  static SG::AuxElement::Accessor<float> acc_absDeltaEta("TAU_ABSDELTAETA");
  static SG::AuxElement::Accessor<float> acc_absDeltaPhi("TAU_ABSDELTAPHI");
  static SG::AuxElement::ConstAccessor<float> acc_sumEMCellEtOverLeadTrkPt("sumEMCellEtOverLeadTrkPt");
  static SG::AuxElement::ConstAccessor<float> acc_etHadAtEMScale("etHadAtEMScale");
  static SG::AuxElement::ConstAccessor<float> acc_etEMAtEMScale("etEMAtEMScale");
  static SG::AuxElement::Accessor<float> acc_EMFractionAtEMScaleMOVEE3("EMFRACTIONATEMSCALE_MOVEE3");
  static SG::AuxElement::Accessor<float> acc_seedTrkSecMaxStripEtOverPt("TAU_SEEDTRK_SECMAXSTRIPETOVERPT");
  static SG::AuxElement::ConstAccessor<float> acc_secMaxStripEt("secMaxStripEt");
  static SG::AuxElement::ConstAccessor<float> acc_centFrac("centFrac");
  static SG::AuxElement::ConstAccessor<float> acc_etOverPtLeadTrk("etOverPtLeadTrk");
  static SG::AuxElement::Accessor<float> acc_corrftrk("CORRFTRK");
  static SG::AuxElement::ConstAccessor<float> acc_hadLeakEt("hadLeakEt");
  static SG::AuxElement::Accessor<float> acc_newhadLeakEt("HADLEAKET");
  static SG::AuxElement::Accessor<float> acc_trtNhtOverNlt("TAU_TRT_NHT_OVER_NLT");
  static SG::AuxElement::Accessor<float> acc_centFracCorrected("CORRCENTFRAC");

  // Will: Fixed variables for R21
  static SG::AuxElement::Accessor<float> acc_EMFracFixed("EMFracFixed");
  static SG::AuxElement::Accessor<float> acc_hadLeakFracFixed("hadLeakFracFixed");
  static SG::AuxElement::Accessor<float> acc_etHotShotDR1("etHotShotDR1"); // replace secMaxStripEt
  static SG::AuxElement::Accessor<float> acc_etHotShotWin("etHotShotWin"); // replace secMaxStripEt
  static SG::AuxElement::Accessor<float> acc_etHotShotDR1OverPtLeadTrk("etHotShotDR1OverPtLeadTrk"); // replace TAU_SEEDTRK_SECMAXSTRIPETOVERPT
  static SG::AuxElement::Accessor<float> acc_etHotShotWinOverPtLeadTrk("etHotShotWinOverPtLeadTrk"); // replace TAU_SEEDTRK_SECMAXSTRIPETOVERPT


  // EMFracFixed and eHad1AtEMScaleFixed (for acc_hadLeakFracFixed)
  // --------------------------------------------------------------
  // ECAL Layers: 0, 1, 2. Don't include 3 because it is mismodelled!
  std::vector<CaloSampling::CaloSample> EMSamps = { 
        CaloSampling::PreSamplerB, CaloSampling::PreSamplerE, 
        CaloSampling::EMB1, CaloSampling::EME1, 
        CaloSampling::EMB2, CaloSampling::EME2 };
  // All HCAL Layers
  std::vector<CaloSampling::CaloSample> HadSamps = { 
        CaloSampling::HEC0, CaloSampling::HEC1, CaloSampling::HEC2, CaloSampling::HEC3, 
        CaloSampling::TileBar0, CaloSampling::TileBar1, CaloSampling::TileBar2,
        CaloSampling::TileGap1, CaloSampling::TileGap2, CaloSampling::TileGap3,
        CaloSampling::TileExt0, CaloSampling::TileExt1, CaloSampling::TileExt2};
  // First HCal Layer
  std::vector<CaloSampling::CaloSample> Had1Samps = { 
        CaloSampling::HEC0, CaloSampling::TileBar0, CaloSampling::TileGap1, CaloSampling::TileExt0};

  // Get Clusters via Jet Seed 
  auto p4IntAxis = tau.p4(xAOD::TauJetParameters::IntermediateAxis);
  const xAOD::Jet *jetSeed = (*tau.jetLink());
  float eEMAtEMScaleFixed = 0;
  float eHadAtEMScaleFixed = 0;
  float eHad1AtEMScaleFixed = 0;

  static const SG::AuxElement::ConstAccessor< std::vector< double > > accMuonCluster( "overlapMuonCluster" );
  std::vector< double > muCluster_v4 = accMuonCluster( tau ) ;
  TLorentzVector muCluster ;
  muCluster.SetPtEtaPhiE( muCluster_v4[0], muCluster_v4[1], muCluster_v4[2], muCluster_v4[3] ) ;

  if (jetSeed) {
    for( auto it : jetSeed->getConstituents() ){
      auto *cl = dynamic_cast<const xAOD::CaloCluster *>((*it)->rawConstituent());
      if (!cl){
        ATH_MSG_WARNING("Found invalid cluster link from seed jet");
        continue;
      }
      // Only take clusters with dR<0.2 w.r.t IntermediateAxis
      if( p4IntAxis.DeltaR(cl->p4(xAOD::CaloCluster::UNCALIBRATED)) > 0.2 ) continue;

      TLorentzVector clusP4 = cl->p4() ;

      if (    muCluster.Pt() > 0
           && muCluster.DeltaR( clusP4 ) < 0.05 
           && std::abs( muCluster.Pt() - clusP4.Pt() )/clusP4.Pt() < 0.2  )
      {
        ATH_MSG_DEBUG( " overlapping preSelected muon cluster found in MRtauClusterSubStructVariables" )  ;
        continue ;
      }

      for( auto samp : EMSamps )
        eEMAtEMScaleFixed += cl->eSample(samp);
      for( auto samp : HadSamps )
        eHadAtEMScaleFixed += cl->eSample(samp);
      for( auto samp : Had1Samps )
        eHad1AtEMScaleFixed += cl->eSample(samp);  
    }

    acc_EMFracFixed(tau) = ( eEMAtEMScaleFixed + eHadAtEMScaleFixed ) != 0 ? 
        eEMAtEMScaleFixed / ( eEMAtEMScaleFixed + eHadAtEMScaleFixed ) : DEFAULT;
  } 
  else{
    ATH_MSG_WARNING("Tau got invalid xAOD::Jet link");
    acc_EMFracFixed(tau) = DEFAULT;
  }

 
  if(tau.nTracks() > 0){
    const xAOD::TrackParticle* track = nullptr ;
#ifdef XAODTAU_VERSIONS_TAUJET_V3_H
    track = tau.track(0)->track();
#else
    track = tau.track(0);
#endif
    acc_absEtaLead(tau) = std::abs( track->eta() );
    acc_leadTrackEta(tau) = std::abs( track->eta() );
    acc_absDeltaEta(tau) = std::abs( track->eta() - tau.eta() );
    acc_absDeltaPhi(tau) = std::abs( track->p4().DeltaPhi(tau.p4()) );
    //EMFRACTIONATEMSCALE_MOVEE3:
    float etEMScale1 = acc_etEMAtEMScale(tau);
    float etEMScale2 = acc_etHadAtEMScale(tau);
    float tau_sumETCellsLAr = acc_sumEMCellEtOverLeadTrkPt(tau) * track->pt();
    float tau_E3 = tau_sumETCellsLAr - etEMScale1;
    float tau_seedCalo_etHadAtEMScale_noE3 = etEMScale2 - tau_E3;
    float tau_seedCalo_etEMAtEMScale_yesE3 = etEMScale1 + tau_E3;
    acc_EMFractionAtEMScaleMOVEE3(tau) = ( tau_seedCalo_etEMAtEMScale_yesE3 + tau_seedCalo_etHadAtEMScale_noE3  != 0. ) ? 
             tau_seedCalo_etEMAtEMScale_yesE3 / (tau_seedCalo_etEMAtEMScale_yesE3 + tau_seedCalo_etHadAtEMScale_noE3 )  : DEFAULT ;
    //TAU_SEEDTRK_SECMAXSTRIPETOVERPT:
    acc_seedTrkSecMaxStripEtOverPt(tau) = (track->pt() != 0) ? acc_secMaxStripEt(tau) / track->pt() : DEFAULT;
    //TRT_NHT_OVER_NLT:
    uint8_t numberOfTRTHighThresholdHits;
    track->summaryValue(numberOfTRTHighThresholdHits, xAOD::numberOfTRTHighThresholdHits);
    uint8_t numberOfTRTHits;
    track->summaryValue(numberOfTRTHits, xAOD::numberOfTRTHits);
    uint8_t numberOfTRTHighThresholdOutliers;
    track->summaryValue(numberOfTRTHighThresholdOutliers, xAOD::numberOfTRTHighThresholdOutliers);
    uint8_t numberOfTRTOutliers;
    track->summaryValue(numberOfTRTOutliers, xAOD::numberOfTRTOutliers);
    acc_trtNhtOverNlt(tau) = (numberOfTRTHits + numberOfTRTOutliers) > 0 ?
      float( numberOfTRTHighThresholdHits + numberOfTRTHighThresholdOutliers) / float(numberOfTRTHits + numberOfTRTOutliers) : DEFAULT;
    acc_newhadLeakEt(tau) = acc_hadLeakEt(tau);

    float fTracksEProbabilityHT;
    track->summaryValue( fTracksEProbabilityHT, xAOD::eProbabilityHT);
    acc_leadTrackProbHT(tau) = fTracksEProbabilityHT;
    
    // hadLeakFracFixed
    acc_hadLeakFracFixed(tau) = (track->p4().P() != 0) ? eHad1AtEMScaleFixed / track->p4().P() : DEFAULT;

    // HOT SHOTS!!!!!
    // --------------
    // Get track position extrapolated to EM1
    const xAOD::TauTrack* tauTrack = tau.track(0);
    float etaCalo = -10.;
    float phiCalo = -10.;
    if( not tauTrack->detail(xAOD::TauJetParameters::CaloSamplingEtaEM, etaCalo))
        ATH_MSG_WARNING("Failed to retrieve extrapolated chargedPFO eta");
    if( not tauTrack->detail(xAOD::TauJetParameters::CaloSamplingPhiEM, phiCalo))
        ATH_MSG_WARNING("Failed to retrieve extrapolated chargedPFO phi");
    ATH_MSG_DEBUG("track EM " << ", eta: " << etaCalo << ", phi: " << phiCalo );
    
    // Get hottest shot in dR<0.1 and in 0.05 x 0.1 window
    float etHotShotDR1 = 0;
    float etHotShotWin = 0;
    for( auto shotLink : tau.shotPFOLinks() ){
        if( ! shotLink.isValid() ){
            ATH_MSG_WARNING("Invalid shotLink");
            continue;
        }
        const xAOD::PFO* shot = (*shotLink);
        float etShot = 0;
        shot->attribute(xAOD::PFODetails::tauShots_pt3, etShot);
       
        // In dR < 0.1
        if(xAOD::P4Helpers::deltaR(*shot, etaCalo, phiCalo, false) && etShot > etHotShotDR1){
          etHotShotDR1 = etShot;
        }
        // In 0.012 x 0.1 window
        if(std::abs(shot->eta() - etaCalo) > 0.012 ) continue;
        if(std::abs(xAOD::P4Helpers::deltaPhi(shot->phi(), phiCalo)) > 0.1 ) continue;
        if(etShot > etHotShotWin) etHotShotWin = etShot;
    }
    acc_etHotShotDR1(tau) = etHotShotDR1;
    acc_etHotShotWin(tau) = etHotShotWin;
    acc_etHotShotDR1OverPtLeadTrk(tau) = (track->pt() != 0) ? etHotShotDR1 / track->pt() : DEFAULT;
    acc_etHotShotWinOverPtLeadTrk(tau) = (track->pt() != 0) ? etHotShotWin / track->pt() : DEFAULT;

  }else{
    acc_absEtaLead(tau) = DEFAULT;
    acc_absDeltaEta(tau) = DEFAULT;
    acc_absDeltaPhi(tau) = DEFAULT;
    acc_newhadLeakEt(tau) = DEFAULT;
    acc_EMFractionAtEMScaleMOVEE3(tau) = DEFAULT;
    acc_seedTrkSecMaxStripEtOverPt(tau) = DEFAULT;
    acc_trtNhtOverNlt(tau) = DEFAULT;
    acc_hadLeakFracFixed(tau) = DEFAULT;
    acc_etHotShotDR1(tau) = DEFAULT; 
    acc_etHotShotWin(tau) = DEFAULT;
    acc_etHotShotDR1OverPtLeadTrk(tau) = DEFAULT; 
    acc_etHotShotWinOverPtLeadTrk(tau) = DEFAULT; 
  }
  //CORRFTRK
  float correction = m_nVtx != int(DEFAULT) ? 0.003 * m_nVtx : 0.;
  float etOverpTLeadTrk = acc_etOverPtLeadTrk(tau);
  float ptLeadTrkOverEt = etOverpTLeadTrk > 0 ? 1. / etOverpTLeadTrk : DEFAULT;
  acc_corrftrk(tau) = ptLeadTrkOverEt != DEFAULT ? ptLeadTrkOverEt + correction : ptLeadTrkOverEt;
  
  acc_centFracCorrected(tau) = tau.pt() < 80*1000. ? acc_centFrac(tau) + correction : acc_centFrac(tau);
 
  return StatusCode::SUCCESS;
}

