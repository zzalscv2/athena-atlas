/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAOD_ANALYSIS
#include "TrkToolInterfaces/ITrackSelectorTool.h"
#include "TrkParametersIdentificationHelpers/TrackParametersIdHelper.h"

#include "xAODTau/TauJet.h"
#include "xAODTau/TauTrackContainer.h"

#include "TauTrackFinder.h"
#include "tauRecTools/TrackSort.h"


TauTrackFinder::TauTrackFinder(const std::string& name) :
  TauRecToolBase(name),
  m_EMSamplings {CaloSampling::EME1, CaloSampling::EMB1},
  m_HadSamplings {CaloSampling::TileBar1, CaloSampling::HEC1, CaloSampling::TileExt1}
{  
}

TauTrackFinder::~TauTrackFinder() {
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
StatusCode TauTrackFinder::initialize() {

  // retrieve tools
  ATH_CHECK( m_trackSelectorTool_tau.retrieve() );
  ATH_CHECK( m_trackToVertexTool.retrieve() );
  ATH_CHECK( m_caloExtensionTool.retrieve() );
  ATH_CHECK( m_trackToVertexIPEstimator.retrieve() );

  // initialize ReadHandleKey
  ATH_CHECK( m_trackPartInputContainer.initialize() );
  // use CaloExtensionTool when key is empty 
  ATH_CHECK( m_ParticleCacheKey.initialize(SG::AllowEmpty) );
  // allow empty for LRT
  ATH_CHECK( m_largeD0TracksInputContainer.initialize(SG::AllowEmpty) );

  if(m_useGhostTracks) {
    if(inTrigger()) {
      ATH_MSG_ERROR ("Ghost matching is not a valid tau-track association scheme for trigger, use cone association. Aborting.");
      return StatusCode::FAILURE;
    }
    ATH_MSG_INFO ("Using ghost matching for tau-track association" << m_ghostTrackDR );
    // allow empty for trigger 
    ATH_CHECK( m_jetContainer.initialize(SG::AllowEmpty) );
  }

  ATH_CHECK( m_beamSpotKey.initialize(inTrigger()) );

  return StatusCode::SUCCESS;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
StatusCode TauTrackFinder::executeTrackFinder(xAOD::TauJet& pTau, xAOD::TauTrackContainer& tauTrackCon) const {
  
  std::vector<const xAOD::TrackParticle*> tauTracks;
  std::vector<const xAOD::TrackParticle*> wideTracks;
  std::vector<const xAOD::TrackParticle*> otherTracks;

  //Retrieve standard track container
  const xAOD::TrackParticleContainer* trackParticleCont = nullptr; 
  
  SG::ReadHandle<xAOD::TrackParticleContainer> trackPartInHandle( m_trackPartInputContainer );
  if (!trackPartInHandle.isValid()) {
    ATH_MSG_ERROR ("Could not retrieve HiveDataObj with key " << trackPartInHandle.key());
    return StatusCode::FAILURE;
  }
  trackParticleCont = trackPartInHandle.cptr();

  //Retrieve LRT container
  const xAOD::TrackParticleContainer* largeD0TracksParticleCont = nullptr; 
  std::vector<const xAOD::TrackParticle*> vecTrksLargeD0;
  if (! m_largeD0TracksInputContainer.empty()) { 
    SG::ReadHandle<xAOD::TrackParticleContainer> trackPartInHandle( m_largeD0TracksInputContainer );
    if (!trackPartInHandle.isValid()) {
      ATH_MSG_VERBOSE ("Could not retrieve HiveDataObj with key " << trackPartInHandle.key());
      ATH_MSG_VERBOSE ("LRT container " << trackPartInHandle.key()<<" is not being used for tau tracks");
    }
    else { 
      largeD0TracksParticleCont = trackPartInHandle.cptr();
      vecTrksLargeD0 = std::vector<const xAOD::TrackParticle*>(largeD0TracksParticleCont->begin(), largeD0TracksParticleCont->end());
    }  
  }

  

  // retrieve the seed jet container when using ghost-matching
  const xAOD::JetContainer* jetContainer = nullptr; 
  if (! m_jetContainer.empty()) {
    SG::ReadHandle<xAOD::JetContainer> jetContHandle( m_jetContainer );
    if (!jetContHandle.isValid()) {
      ATH_MSG_ERROR ("Could not retrieve HiveDataObj with key " << jetContHandle.key());
      return StatusCode::FAILURE;
    }
    jetContainer = jetContHandle.cptr();
  }
  // in EleRM reco, we need the original track particles
  std::vector<const xAOD::TrackParticle*> vecTrks;
  vecTrks.reserve( trackParticleCont->size() );
  for (auto trk : *trackParticleCont){
    if (!inEleRM()) { vecTrks.push_back(trk); }
    else{
      static const SG::AuxElement::ConstAccessor<ElementLink<xAOD::TrackParticleContainer>> acc_originalObject("ERMOriginalTrack");
      auto original_id_track_link = acc_originalObject(*trk);
      if (!original_id_track_link.isValid()) {
          ATH_MSG_ERROR("Original track link is not valid");
          continue;
      }
      vecTrks.push_back(*original_id_track_link);
    }
  }

  // get the primary vertex
  const xAOD::Vertex* pVertex = pTau.vertex();

  // retrieve tracks wrt a vertex                                                                                                                              
  // as a vertex is used: tau origin / PV / beamspot / 0,0,0 (in this order, depending on availability)                                                        

  getTauTracksFromPV(pTau, vecTrks, pVertex, m_useGhostTracks, jetContainer, tauTracks, wideTracks, otherTracks);

  bool foundLRTCont = bool (largeD0TracksParticleCont != nullptr);
  // additional LRT with vertex association added to tracks
  if (foundLRTCont){
    // for now, use cone association for LRTs, not ghost association
    getTauTracksFromPV(pTau, vecTrksLargeD0, pVertex, false, nullptr, tauTracks, wideTracks, otherTracks);
  }

  // remove core and wide tracks outside a maximal delta z0 wrt lead core track                                                                                
  if (m_applyZ0cut) {
    this->removeOffsideTracksWrtLeadTrk(tauTracks, wideTracks, otherTracks, pVertex, m_z0maxDelta);
  }

  if(m_removeDuplicateCoreTracks){
    bool alreadyUsed = false;
    for (std::vector<const xAOD::TrackParticle*>::iterator track_it = tauTracks.begin(); track_it != tauTracks.end() ;)
      {
	alreadyUsed = false;
	//loop over all up-to-now core tracks	
	for( const xAOD::TauTrack* tau_trk : tauTrackCon ) {
	  //originally it was coreTrack&passTrkSelector
	  if(! tau_trk->flagWithMask( (1<<xAOD::TauJetParameters::TauTrackFlag::coreTrack) | (1<<xAOD::TauJetParameters::TauTrackFlag::passTrkSelector))) continue; 
	  if( (*track_it) == tau_trk->track()) alreadyUsed = true;
	}
	//if this track has already been used by another tau, don't associate it to this new one                                                               
	if(alreadyUsed) ATH_MSG_INFO( "Found Already Used track new, now removing: " << *track_it );
	if (alreadyUsed) track_it = tauTracks.erase(track_it);
	else ++track_it;
      }
  }

  // associate track to tau candidate and calculate charge                                                                                                    
  float charge = 0.;  
  for (unsigned int i = 0; i < tauTracks.size(); ++i) {
    const xAOD::TrackParticle* trackParticle = tauTracks.at(i);

    ATH_MSG_VERBOSE(name() << " adding core track nr: " << i
		    << " eta " << trackParticle->eta()
		    << " phi " << trackParticle->phi());

    charge += trackParticle->charge();

    xAOD::TauTrack* track = new xAOD::TauTrack();
    tauTrackCon.push_back(track);

    ElementLink<xAOD::TrackParticleContainer> linkToTrackParticle;
    linkToTrackParticle.toContainedElement(*static_cast<const xAOD::TrackParticleContainer*>(trackParticle->container()), trackParticle);
    if (foundLRTCont && isLargeD0Track(trackParticle)){//Check LRT track and link to container
      track->setFlag(xAOD::TauJetParameters::TauTrackFlag::LargeRadiusTrack, true);
    }
    else {
      track->setFlag(xAOD::TauJetParameters::TauTrackFlag::LargeRadiusTrack, false);
    }
    track->addTrackLink(linkToTrackParticle);

    track->setP4(trackParticle->pt(), trackParticle->eta(), trackParticle->phi(), trackParticle->m());
    track->setFlag(xAOD::TauJetParameters::TauTrackFlag::coreTrack, true);
    track->setFlag(xAOD::TauJetParameters::TauTrackFlag::passTrkSelector, true);
    // in case TrackClassifier is not run, still get sensible results                                                                                        
    track->setFlag(xAOD::TauJetParameters::TauTrackFlag::classifiedCharged, true);
    track->setFlag(xAOD::TauJetParameters::TauTrackFlag::unclassified, true);
    
    ElementLink<xAOD::TauTrackContainer> linkToTauTrack;
    linkToTauTrack.toContainedElement(tauTrackCon, track);
    pTau.addTauTrackLink(linkToTauTrack);

    ATH_MSG_VERBOSE(name() << " added core track nr: " << i
		    << " eta " << pTau.track(i)->eta()
		    << " phi " << pTau.track(i)->phi());
  }
  // set the charge, which is defined by the core tau tracks only                                                                                              
  pTau.setCharge(charge);

  for (unsigned int i = 0; i < wideTracks.size(); ++i) {
    const xAOD::TrackParticle* trackParticle = wideTracks.at(i);

    ATH_MSG_VERBOSE(name() << " adding wide track nr: " << i
		    << " eta " << trackParticle->eta()
		    << " phi " << trackParticle->phi());

    xAOD::TauTrack* track = new xAOD::TauTrack();
    tauTrackCon.push_back(track);

    ElementLink<xAOD::TrackParticleContainer> linkToTrackParticle;
    linkToTrackParticle.toContainedElement(*static_cast<const xAOD::TrackParticleContainer*>(trackParticle->container()), trackParticle);
    if (foundLRTCont && isLargeD0Track(trackParticle)){//Check LRT track and link to container
      track->setFlag(xAOD::TauJetParameters::TauTrackFlag::LargeRadiusTrack, true);
    }
    else {
      track->setFlag(xAOD::TauJetParameters::TauTrackFlag::LargeRadiusTrack, false);
    }
    track->addTrackLink(linkToTrackParticle);

    track->setP4(trackParticle->pt(), trackParticle->eta(), trackParticle->phi(), trackParticle->m());
    track->setFlag(xAOD::TauJetParameters::TauTrackFlag::wideTrack, true);
    track->setFlag(xAOD::TauJetParameters::TauTrackFlag::passTrkSelector, true);
    // in case TrackClassifier is not run, still get sensible results                                                                                        
    track->setFlag(xAOD::TauJetParameters::TauTrackFlag::classifiedIsolation, true); // for sake of trigger, reset in TauTrackClassifier
    track->setFlag(xAOD::TauJetParameters::TauTrackFlag::modifiedIsolationTrack, true); // for sake of trigger, reset in TauTrackClassifier
    track->setFlag(xAOD::TauJetParameters::TauTrackFlag::unclassified, true);

    ElementLink<xAOD::TauTrackContainer> linkToTauTrack;
    linkToTauTrack.toContainedElement(tauTrackCon, track);
    pTau.addTauTrackLink(linkToTauTrack);
  }

  //These are set again in TauTrackClassifier                                                                                                                  
  pTau.setDetail(xAOD::TauJetParameters::nChargedTracks, (int) pTau.nTracks());
  pTau.setDetail(xAOD::TauJetParameters::nIsolatedTracks, (int) pTau.nTracks(xAOD::TauJetParameters::classifiedIsolation));

  for (unsigned int i = 0; i < otherTracks.size(); ++i) {
    const xAOD::TrackParticle* trackParticle = otherTracks.at(i);

    ATH_MSG_VERBOSE(name()     << " adding other track nr: " << i
		    << " eta " << trackParticle->eta()
		    << " phi " << trackParticle->phi());

    xAOD::TauTrack* track = new xAOD::TauTrack();
    tauTrackCon.push_back(track);

    ElementLink<xAOD::TrackParticleContainer> linkToTrackParticle;
    if (foundLRTCont && isLargeD0Track(trackParticle)){//Check LRT track and link to container
      linkToTrackParticle.toContainedElement(*static_cast<const xAOD::TrackParticleContainer*>(trackParticle->container()), trackParticle);
      track->setFlag(xAOD::TauJetParameters::TauTrackFlag::LargeRadiusTrack, true); 
    }
    else {
      linkToTrackParticle.toContainedElement(*static_cast<const xAOD::TrackParticleContainer*>(trackParticle->container()), trackParticle);
      track->setFlag(xAOD::TauJetParameters::TauTrackFlag::LargeRadiusTrack, false);
    }
    track->addTrackLink(linkToTrackParticle);

    track->setP4(trackParticle->pt(), trackParticle->eta(), trackParticle->phi(), trackParticle->m());
    float dR = track->p4().DeltaR(pTau.p4());
    if(dR<=0.2) track->setFlag(xAOD::TauJetParameters::TauTrackFlag::coreTrack, true);
    else track->setFlag(xAOD::TauJetParameters::TauTrackFlag::wideTrack, true);
    track->setFlag(xAOD::TauJetParameters::TauTrackFlag::unclassified, true);

    ElementLink<xAOD::TauTrackContainer> linkToTauTrack;
    linkToTauTrack.toContainedElement(tauTrackCon, track);
    pTau.addTauTrackLink(linkToTauTrack);
  }

  pTau.setDetail(xAOD::TauJetParameters::nLargeRadiusTracks, (int) pTau.nTracks(xAOD::TauJetParameters::LargeRadiusTrack));
  // keep track of total number of associated tracks, in case of tau track thinning
  pTau.setDetail(xAOD::TauJetParameters::nAllTracks, (int) pTau.nAllTracks());

  ATH_MSG_DEBUG("numTrack: " << "/" << pTau.nTracks());
  ATH_MSG_DEBUG("charge: " << "/" << pTau.charge());

  // impact parameter variables w.r.t. tau vertex 
  const xAOD::Vertex* vxcand = nullptr;

  xAOD::Vertex vxbkp;
  vxbkp.makePrivateStore();

  // FIXME: don't we want to use the beamspot in the offline reconstruction too, when there is no reconstructed primary vertex?
  if (pTau.vertex()!=nullptr && pTau.vertex()->vertexType() != xAOD::VxType::NoVtx) {
    vxcand = pTau.vertex();
  }
  else if (inTrigger()) { // online: use vertex with x-y coordinates from the beamspot and the z from the leading track
    vxbkp.setX(0); vxbkp.setY(0); vxbkp.setZ(0);

    SG::ReadCondHandle<InDet::BeamSpotData> beamSpotHandle { m_beamSpotKey };
    if(beamSpotHandle.isValid()) {
      vxbkp.setPosition(beamSpotHandle->beamPos());
      const auto& cov = beamSpotHandle->beamVtx().covariancePosition();
      vxbkp.setCovariancePosition(cov);

      if(!tauTracks.empty()) {
         vxbkp.setZ(tauTracks.at(0)->z0());
      }
    }
    else {
      ATH_MSG_DEBUG("No Beamspot object in tau candidate");
    }
    vxcand = & vxbkp;
  }


  // this could be replaced with TauTrack::setDetail
  static const SG::AuxElement::Accessor<float> dec_d0TJVA("d0TJVA");
  static const SG::AuxElement::Accessor<float> dec_z0sinthetaTJVA("z0sinthetaTJVA");
  static const SG::AuxElement::Accessor<float> dec_d0SigTJVA("d0SigTJVA");
  static const SG::AuxElement::Accessor<float> dec_z0sinthetaSigTJVA("z0sinthetaSigTJVA");

  for(const ElementLink<xAOD::TauTrackContainer>& trackLink : pTau.allTauTrackLinks())
  {
    assert (trackLink.getStorableObjectPointer() == &tauTrackCon);
    xAOD::TauTrack* track = tauTrackCon[trackLink.index()];
    dec_d0TJVA(*track) = track->track()->d0();
    dec_z0sinthetaTJVA(*track) = track->z0sinThetaTJVA(pTau);
    dec_d0SigTJVA(*track) = -999.;
    dec_z0sinthetaSigTJVA(*track) = -999.;

    // in the trigger, z0sintheta and corresponding significance are meaningless if we use the beamspot
    if(vxcand) {
      std::unique_ptr<const Trk::ImpactParametersAndSigma> myIPandSigma 
	= std::unique_ptr<const Trk::ImpactParametersAndSigma>(m_trackToVertexIPEstimator->estimate(track->track(), vxcand));
      
      if(myIPandSigma) {
	dec_d0TJVA(*track) = myIPandSigma->IPd0;
	dec_z0sinthetaTJVA(*track) = myIPandSigma->IPz0SinTheta;
	dec_d0SigTJVA(*track) = (myIPandSigma->sigmad0 != 0.) ? (float)( myIPandSigma->IPd0 / myIPandSigma->sigmad0 ) : -999.;
	dec_z0sinthetaSigTJVA(*track) = (myIPandSigma->sigmaz0SinTheta != 0.) ? (float)( myIPandSigma->IPz0SinTheta / myIPandSigma->sigmaz0SinTheta ) : -999.;
      }
    }
  }

  // extrapolate core tracks to calorimeter surface
  // store information only in ExtraDetailsContainer
  if(!m_bypassExtrapolator)
    {
      StatusCode sc = extrapolateToCaloSurface(pTau, tauTrackCon);
      if (sc.isFailure() && !sc.isRecoverable()) {
	ATH_MSG_ERROR("couldn't extrapolate tracks to calo surface");
	return StatusCode::FAILURE;
      }
    }
  
  return StatusCode::SUCCESS;
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
TauTrackFinder::TauTrackType TauTrackFinder::tauTrackType( const xAOD::TauJet& pTau,
							   const xAOD::TrackParticle& trackParticle,
							   const xAOD::Vertex* primaryVertex) const
{
  double dR = pTau.p4().DeltaR(trackParticle.p4());

  if (dR > m_maxJetDr_wide) return NotTauTrack;

  bool goodTrack = true;
  if(!m_bypassSelector)
    goodTrack = m_trackSelectorTool_tau->decision(trackParticle, primaryVertex);
    
  if (goodTrack) {
    if (dR > m_maxJetDr_tau)
      return TauTrackWide;
    else
      return TauTrackCore;
  } else
    return TauTrackOther;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
void TauTrackFinder::getTauTracksFromPV( const xAOD::TauJet& pTau,
					 const std::vector<const xAOD::TrackParticle*>& vecTrackParticles,
					 const xAOD::Vertex* primaryVertex,
					 const bool& useGhostTracks,
					 const xAOD::JetContainer* jetContainer,
					 std::vector<const xAOD::TrackParticle*> &tauTracks,
					 std::vector<const xAOD::TrackParticle*> &wideTracks,
					 std::vector<const xAOD::TrackParticle*> &otherTracks) const
{
  std::vector<const xAOD::TrackParticle*> ghostTracks;
  if(useGhostTracks) {
    const xAOD::Jet* seedJet = pTau.jet();
    if(seedJet) {
      if(!seedJet->getAssociatedObjects("GhostTrack", ghostTracks)) {
	ATH_MSG_WARNING("Could not retrieve GhostTrack from seed jet.");
      }
    }
  }
  // in EleRM reco, we need the original track particles
  if (inEleRM()){
    for (uint i = 0; i < ghostTracks.size(); i++){
      static const SG::AuxElement::ConstAccessor<ElementLink<xAOD::TrackParticleContainer>> acc_originalTrack("ERMOriginalTrack");
      auto original_id_track_link = acc_originalTrack(*(ghostTracks[i]));
      if (!original_id_track_link.isValid()) {
          ATH_MSG_ERROR("Original track link is not valid");
          continue;
      }
      ghostTracks[i] = *original_id_track_link;
    }
  }
  
  for (const xAOD::TrackParticle *trackParticle : vecTrackParticles) {
    TauTrackType type = tauTrackType(pTau, *trackParticle, primaryVertex);
    if(type == NotTauTrack) continue;

    if(useGhostTracks) {
      // require that tracks are ghost-matched with the seed jet at large dR(tau,track), to avoid using tracks from another tau
      double dR = pTau.p4().DeltaR(trackParticle->p4());
      if (dR > m_ghostTrackDR) {
	if (std::find(ghostTracks.begin(), ghostTracks.end(), trackParticle) == ghostTracks.end()) {
	  // check whether the jet closest to the track is the current seed jet
	  // if so, recover the track even if not ghost-matched, to improve tau-track association efficiency at low pt (esp. for 3p)
	  double dRmin = 999.;
	  bool isSeedClosest = false;
	  for (const xAOD::Jet* jet : *jetContainer) {
	    xAOD::JetFourMom_t jetP4 = jet->jetP4(xAOD::JetScale::JetConstitScaleMomentum);
	    TLorentzVector jetLV;
	    jetLV.SetPtEtaPhiM(jetP4.Pt(), jetP4.Eta(), jetP4.Phi(), jetP4.M());
	    double dRjet = trackParticle->p4().DeltaR(jetLV);
	    if(dRjet < dRmin) {
	      dRmin = dRjet;
	      isSeedClosest = (jet == pTau.jet());
	    }
	  }
	  if(!isSeedClosest) continue;
	}
      }
    }
    
    if (type == TauTrackCore)
      tauTracks.push_back(trackParticle);
    else if (type == TauTrackWide)
      wideTracks.push_back(trackParticle);
    else if (type == TauTrackOther)
      otherTracks.push_back(trackParticle);
  }
  std::sort(tauTracks.begin(), tauTracks.end(), TrackSort());
  std::sort(wideTracks.begin(), wideTracks.end(), TrackSort());
  std::sort(otherTracks.begin(), otherTracks.end(), TrackSort());
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
StatusCode TauTrackFinder::extrapolateToCaloSurface(xAOD::TauJet& pTau,
                                                    xAOD::TauTrackContainer& tauTrackCon) const
{
  Trk::TrackParametersIdHelper parsIdHelper;

  int trackIndex = -1;
  const Trk::CaloExtension * caloExtension = nullptr;
  std::unique_ptr<Trk::CaloExtension> uniqueExtension;
  for(const ElementLink<xAOD::TauTrackContainer>& trackLink : pTau.allTauTrackLinks())
  {
    assert (trackLink.getStorableObjectPointer() == &tauTrackCon);
    xAOD::TauTrack* tauTrack = tauTrackCon[trackLink.index()];
    const xAOD::TrackParticle *orgTrack = tauTrack->track();
    if( !orgTrack ) continue;
    trackIndex = orgTrack->index();   

    // set default values
    float etaEM = -10.0;
    float phiEM = -10.0;
    float etaHad = -10.0;
    float phiHad = -10.0;

    // get the extrapolation into the calo
    ATH_MSG_DEBUG( "Try extrapolation of track with pt = " << orgTrack->pt() 
		   << ", eta " << orgTrack->eta() 
		   << ", phi" << orgTrack->phi() );

    if(!m_ParticleCacheKey.key().empty()){
      /*get the CaloExtension object*/
      ATH_MSG_VERBOSE("Using the CaloExtensionBuilder Cache");
      SG::ReadHandle<CaloExtensionCollection>  particleCache {m_ParticleCacheKey};
      caloExtension = (*particleCache)[trackIndex];
      ATH_MSG_VERBOSE("Getting element " << trackIndex << " from the particleCache");
      if( not caloExtension ){
        ATH_MSG_VERBOSE("Cache does not contain a calo extension -> "
                        "Calculating with the a CaloExtensionTool");
        uniqueExtension = m_caloExtensionTool->caloExtension(
          Gaudi::Hive::currentContext(), *orgTrack);
        caloExtension = uniqueExtension.get();
      }
    }
    else {
      /* If CaloExtensionBuilder is unavailable, use the calo extension tool */
      ATH_MSG_VERBOSE("Using the CaloExtensionTool");
      uniqueExtension = m_caloExtensionTool->caloExtension(
        Gaudi::Hive::currentContext(), *orgTrack);
      caloExtension = uniqueExtension.get();
    }

    if (!caloExtension)
      { 
	ATH_MSG_DEBUG("Track extrapolation failed");
      }
    else {
      const std::vector<Trk::CurvilinearParameters>& clParametersVector = caloExtension->caloLayerIntersections();
      if (clParametersVector.empty()) {
	ATH_MSG_DEBUG("Track extrapolation failed");
      }

      ATH_MSG_DEBUG("Scanning samplings");
      bool validECal = false;
      bool validHCal = false;
      for( const Trk::CurvilinearParameters& cur : clParametersVector ){
	ATH_MSG_DEBUG("Sampling " << parsIdHelper.caloSample(cur.cIdentifier()) );
                
	// only use entry layer
	if( not parsIdHelper.isEntryToVolume(cur.cIdentifier()) ) continue;

	CaloSampling::CaloSample sample = parsIdHelper.caloSample(cur.cIdentifier());

	// ECal
	if( not validECal and m_EMSamplings.count(sample))
	  {
	    validECal = true;
	    etaEM = cur.position().eta();
	    phiEM = cur.position().phi();
	    ATH_MSG_DEBUG("Extrapolated to ECal layer " << sample);
	  }

	// HCal
	if( not validHCal and m_HadSamplings.count(sample))
	  {
	    validHCal = true;
	    etaHad = cur.position().eta();
	    phiHad = cur.position().phi();
	    ATH_MSG_DEBUG("Extrapolated to HCal layer " << sample);
	  }
	if( validECal and validHCal ) break;
      }
      // EM failure warn if within acceptance 
      if( not validECal and std::abs(orgTrack->pt()) < 2.48 ){
	ATH_MSG_DEBUG("Failed extrapolation to ECal");
      }
      // Had failure warn if enough pt to reach HCal
      if( not validHCal and orgTrack->pt() > 2000. ){
	ATH_MSG_DEBUG("Failed extrapolation to HCal");
      }

      ATH_MSG_DEBUG( "Extrapolated track with eta=" << orgTrack->eta()
		     << " phi="<<orgTrack->phi()
		     << " to ECal eta=" << etaEM 
		     << " phi="<< phiEM
		     << " HCal eta=" << etaHad 
		     << " phi="<< phiHad
		     );
    }
    tauTrack->setDetail(xAOD::TauJetParameters::CaloSamplingEtaEM, etaEM);
    tauTrack->setDetail(xAOD::TauJetParameters::CaloSamplingPhiEM, phiEM);
    tauTrack->setDetail(xAOD::TauJetParameters::CaloSamplingEtaHad, etaHad);
    tauTrack->setDetail(xAOD::TauJetParameters::CaloSamplingPhiHad, phiHad);
  }

  return StatusCode::SUCCESS;

}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
void TauTrackFinder::removeOffsideTracksWrtLeadTrk(std::vector<const xAOD::TrackParticle*> &tauTracks,
						   std::vector<const xAOD::TrackParticle*> &wideTracks,
						   std::vector<const xAOD::TrackParticle*> &otherTracks,
						   const xAOD::Vertex* tauOrigin,
						   double maxDeltaZ0) const
{
  float MAX=1e5;

  // need at least one core track to have a leading trk to compare with
  if (tauTracks.empty()) return;

  // get lead trk parameters
  const xAOD::TrackParticle *leadTrack = tauTracks.at(0);
  float z0_leadTrk = getZ0(leadTrack, tauOrigin);

  if (z0_leadTrk > MAX-1) return; // bad lead trk -> do nothing

  ATH_MSG_VERBOSE("before z0 cut: #coreTracks=" << tauTracks.size() << ", #wideTracks=" << wideTracks.size() << ", #otherTracks=" << otherTracks.size());

  std::vector<const xAOD::TrackParticle*>::iterator itr;
    
  // skip leading track, because it is the reference
  itr = tauTracks.begin()+1;
  while (itr!=tauTracks.end()) {
    float z0 = getZ0(*itr, tauOrigin);
    float deltaZ0=z0 - z0_leadTrk;
    ATH_MSG_VERBOSE("core Trks: deltaZ0= " << deltaZ0);

    if ( std::abs(deltaZ0) < maxDeltaZ0 ) {++itr;}
    else {
      if (m_storeInOtherTrks) otherTracks.push_back(*itr);
      itr = tauTracks.erase(itr); //remove from core track collection
    }
  }

  // check wide tracks
  itr = wideTracks.begin();
  while (itr!=wideTracks.end()) {
    float z0 = getZ0(*itr, tauOrigin);
    float deltaZ0=z0 - z0_leadTrk;
    ATH_MSG_VERBOSE("wide Trks: deltaZ0= " << deltaZ0);

    if ( std::abs(deltaZ0) < maxDeltaZ0 ) { ++itr; }
    else {
      if (m_storeInOtherTrks) otherTracks.push_back(*itr);
      itr = wideTracks.erase(itr); //remove from wide track collection
    }
  }

  ATH_MSG_VERBOSE("after z0 cut: #coreTracks=" << tauTracks.size() << ", #wideTracks=" << wideTracks.size() << ", #otherTracks=" << otherTracks.size());

  // sort again
  std::sort(tauTracks.begin(), tauTracks.end(), TrackSort());
  std::sort(wideTracks.begin(), wideTracks.end(), TrackSort());
  std::sort(otherTracks.begin(), otherTracks.end(), TrackSort());
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
float TauTrackFinder::getZ0(const xAOD::TrackParticle* track, const xAOD::Vertex* vertex) const
{
  float MAX=1e5;

  if (!track) return MAX;

  std::unique_ptr<Trk::Perigee> perigee;
  if (vertex) perigee = m_trackToVertexTool->perigeeAtVertex(Gaudi::Hive::currentContext(), *track, vertex->position());
  else        perigee = m_trackToVertexTool->perigeeAtVertex(Gaudi::Hive::currentContext(), *track); //will use beamspot or 0,0,0 instead

  if (!perigee) {
    ATH_MSG_WARNING("Bad track; can't find perigee at vertex.");
    return MAX;
  }

  float z0 = perigee->parameters()[Trk::z0];

  return z0;
}

bool TauTrackFinder::isLargeD0Track(const xAOD::TrackParticle* track) const
{
  const std::bitset<xAOD::NumberOfTrackRecoInfo> patternReco = track->patternRecoInfo();
  if (patternReco.test(xAOD::TrackPatternRecoInfo::SiSpacePointsSeedMaker_LargeD0)) {
    ATH_MSG_DEBUG("LargeD0Track found");
    return true;
  }

  return false;
}
#endif
