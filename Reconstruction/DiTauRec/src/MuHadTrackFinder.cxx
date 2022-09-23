/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAOD_ANALYSIS
#include "TrkToolInterfaces/ITrackSelectorTool.h"
#include "TrkParametersIdentificationHelpers/TrackParametersIdHelper.h"
#include "RecoToolInterfaces/IParticleCaloExtensionTool.h"
#include "xAODTau/TauJet.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTau/TauJetContainer.h"
#include "xAODTau/TauTrackContainer.h"
#include "xAODTau/TauTrackAuxContainer.h"

#include "DiTauRec/MuHadTrackFinder.h"
#include "tauRecTools/KineUtils.h"
#include "tauRecTools/TrackSort.h"

MuHadTrackFinder::MuHadTrackFinder(const std::string& name ) :
        TauRecToolBase(name),
        m_caloExtensionTool("Trk::ParticleCaloExtensionTool/ParticleCaloExtensionTool"),
        m_trackSelectorTool_tau(""),
        m_trackToVertexTool("Reco::TrackToVertex"),
        m_z0maxDelta(1000),
        m_applyZ0cut(false),
        m_storeInOtherTrks(true),
        m_bypassSelector(false),
        m_bypassExtrapolator(false),
        m_muIPwp1P(1),
        m_muIPwp3P(2),
        m_muIPwpWide(1) 
{
    declareProperty("MaxJetDrTau", m_maxJetDr_tau = 0.2);
    declareProperty("MaxJetDrWide", m_maxJetDr_wide = 0.4);
    declareProperty("TrackSelectorToolTau", m_trackSelectorTool_tau);
    declareProperty("TrackParticleContainer", m_inputTrackParticleContainerName = "InDetTrackParticles");
    declareProperty("TauTrackParticleContainer", m_inputTauTrackContainerName = "MuRmTauTracks");
    declareProperty("ParticleCaloExtensionTool",   m_caloExtensionTool );
    declareProperty("TrackToVertexTool",m_trackToVertexTool);
    declareProperty("maxDeltaZ0wrtLeadTrk", m_z0maxDelta);
    declareProperty("removeTracksOutsideZ0wrtLeadTrk", m_applyZ0cut);
    declareProperty("StoreRemovedCoreWideTracksInOtherTracks", m_storeInOtherTrks = true);
    declareProperty("removeDuplicateCoreTracks", m_removeDuplicateCoreTracks = true);
    declareProperty("BypassSelector", m_bypassSelector = false);
    declareProperty("BypassExtrapolator", m_bypassExtrapolator = false);
    declareProperty("MuonIDqualityCut1P", m_muIPwp1P = 1 );
    declareProperty("MuonIDqualityCut3P", m_muIPwp3P = 2 );
    declareProperty("MuonIDqualityCutRing", m_muIPwpWide = 1 );

    // initialize samplings
    m_EMSamplings = {CaloSampling::EME1, CaloSampling::EMB1};
    m_HadSamplings = {CaloSampling::TileBar1, CaloSampling::HEC1, CaloSampling::TileExt1};

}

MuHadTrackFinder::~MuHadTrackFinder() { } 

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
StatusCode MuHadTrackFinder::initialize() {

    // Get the TrackSelectorTool
    ATH_CHECK( m_trackSelectorTool_tau.retrieve() );

    // Get the TJVA
    ATH_CHECK( m_trackToVertexTool.retrieve() );
    ATH_CHECK( m_caloExtensionTool.retrieve() );

    return StatusCode::SUCCESS;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
StatusCode MuHadTrackFinder::eventInitialize() {

    m_tauTrackCont = nullptr ;
    ATH_CHECK( evtStore()->retrieve( m_tauTrackCont, m_inputTauTrackContainerName ));

    m_trackParticleCont = nullptr ;
    ATH_CHECK(  evtStore()->retrieve( m_trackParticleCont, m_inputTrackParticleContainerName ) ) ;

    return StatusCode::SUCCESS;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
StatusCode MuHadTrackFinder::execute(xAOD::TauJet& pTau) {

    ATH_MSG_VERBOSE("MuHadTrackFinder Executing");

    StatusCode sc = StatusCode::SUCCESS;

    pTau.clearTauTrackLinks();

    std::vector<const xAOD::TrackParticle*> tauTracks;
    std::vector<const xAOD::TrackParticle*> wideTracks;
    std::vector<const xAOD::TrackParticle*> otherTracks;

    const xAOD::Vertex* pVertex = pTau.vertexLink()!=0 ? (*pTau.vertexLink()) : nullptr ;
    // retrieve tracks wrt a vertex
    // as a vertex is used: tau origin / PV / beamspot / 0,0,0 (in this order, depending on availability)
    sc = getTauTracksFromPV( pTau, pVertex, tauTracks, wideTracks, otherTracks );
    if ( sc.isFailure() )
    {
      ATH_MSG_WARNING( "Failed to getTauTracksFromPV" ) ;
      return sc ;
    } 

    this->resetDeltaZ0Cache();
    // remove core and wide tracks outside a maximal delta z0 wrt lead core track
    if (m_applyZ0cut) {
        this->removeOffsideTracksWrtLeadTrk( tauTracks, wideTracks, otherTracks, pVertex, m_z0maxDelta );
    }

    static const SG::AuxElement::ConstAccessor< std::vector< double > > accMuonTrack( "overlapMuonTrack" );
    std::vector< double > muTrack_v4 = accMuonTrack( pTau ) ;
    TLorentzVector muTrack ;
    muTrack.SetPtEtaPhiE( muTrack_v4[0], muTrack_v4[1], muTrack_v4[2], muTrack_v4[3] ) ;

    static const SG::AuxElement::ConstAccessor< int > accMuonQlt( "overlapMuonQuality" ) ;
    int muQlt = accMuonQlt( pTau ) ;
  
    if( m_removeDuplicateCoreTracks )
    {
      bool alreadyUsed = false;
      for ( auto track_it = tauTracks.begin(); track_it != tauTracks.end() ;)
      {
        alreadyUsed = false;
          
          //loop over all up-to-now core tracks
        for( const xAOD::TauTrack* tau_trk : (*m_tauTrackCont) ) 
        {
          //  the two flagMask are in AND relation !!!
          if( ! tau_trk->flagWithMask( 
              (1<<xAOD::TauJetParameters::TauTrackFlag::coreTrack) | (1<<xAOD::TauJetParameters::TauTrackFlag::passTrkSelector)
                                     ) 
            ) continue; //originally it was coreTrack&passTrkSelector

          if( (*track_it) == tau_trk->track()) alreadyUsed = true;

        }
          //if this track has already been used by another tau, don't associate it to this new one
        if(alreadyUsed) 
        {
          ATH_MSG_DEBUG( "Found track already used in other tau, now removing: " << *track_it );
          track_it = tauTracks.erase(track_it);
        } 
        else 
          ++track_it;
      }
    }

    int muQltCut = 0 ;
    unsigned int numTrk = tauTracks.size() ;
    if ( numTrk == 1 || numTrk == 3 ) muQltCut = 0 ;
    else if ( numTrk == 2 )
      muQltCut = m_muIPwp1P ;
    else if ( numTrk == 4 )
      muQltCut = m_muIPwp3P ;
    else muQltCut = 2 ;

    if ( muQlt <= muQltCut )
    {
      for ( auto  track_it = tauTracks.begin(); track_it != tauTracks.end() ;)
      {
        if (      muTrack.Pt() > 0 && muTrack.DeltaR( (*track_it)->p4() ) < 0.05
               && std::abs( ( muTrack.Pt() - (*track_it)->pt() )/muTrack.Pt() ) < 0.05 
           ) 
        {
          ATH_MSG_DEBUG( " Tau core Track overlapped with muon " << (*track_it)->pt() <<" "<< (*track_it)->eta() ) ;
          track_it = tauTracks.erase(track_it);
        }
        else ++track_it;
      }
    }

    // associated track to tau candidate and calculate charge
    float charge = 0;
    for ( const xAOD::TrackParticle* trackParticle : tauTracks ) 
    {

        charge += trackParticle->charge();

        xAOD::TauTrack* track = new xAOD::TauTrack();
        m_tauTrackCont->push_back(track);

        ElementLink<xAOD::TrackParticleContainer> linkToTrackParticle;
        linkToTrackParticle.toContainedElement(*m_trackParticleCont, trackParticle);
        track->addTrackLink(linkToTrackParticle);
        track->setP4(trackParticle->pt(), trackParticle->eta(), trackParticle->phi(), trackParticle->m());
        track->setFlag(xAOD::TauJetParameters::TauTrackFlag::coreTrack, true);
        track->setFlag(xAOD::TauJetParameters::TauTrackFlag::passTrkSelector, true);
        // in case TrackClassifier is not run, still get sensible results
        track->setFlag(xAOD::TauJetParameters::TauTrackFlag::classifiedCharged, true); 
        track->setFlag(xAOD::TauJetParameters::TauTrackFlag::unclassified, true); 

        if( ! m_bypassExtrapolator ) extrapolateToCaloSurface( track, trackParticle ) ;

        ElementLink<xAOD::TauTrackContainer> linkToTauTrack;
        linkToTauTrack.toContainedElement( *m_tauTrackCont, track );
        pTau.addTauTrackLink( linkToTauTrack );

        ATH_MSG_VERBOSE( name() << " added core track eta " << track->eta() << " phi " << track->phi() );
    }
    // set the charge, which is defined by the core tau tracks only
    pTau.setCharge(charge);
    
    for ( auto  track_it =  wideTracks.begin(); track_it !=  wideTracks.end() ;)
    {
      ATH_MSG_VERBOSE(name()     << " adding wide track nr: " 
              << " eta " << (*track_it)->eta()
              << " phi " << (*track_it)->phi()
      );

      if (    muTrack.Pt() > 0 && muTrack.DeltaR( (*track_it)->p4() ) < 0.05
           && std::abs( ( muTrack.Pt() - (*track_it)->pt() )/muTrack.Pt() ) < 0.05 
           && muQlt <= m_muIPwpWide )
      {
        ATH_MSG_DEBUG( "overlapping preSelelcted muon in TrackFinder::wideTracks" ) ;
        track_it = wideTracks.erase(track_it);
      }
      else 
      {
        xAOD::TauTrack* track = new xAOD::TauTrack();
        m_tauTrackCont->push_back(track);

        const xAOD::TrackParticle* trackParticle = *track_it ;

        ElementLink<xAOD::TrackParticleContainer> linkToTrackParticle;
        linkToTrackParticle.toContainedElement(*m_trackParticleCont, trackParticle);
        track->addTrackLink(linkToTrackParticle);
        track->setP4(trackParticle->pt(), trackParticle->eta(), trackParticle->phi(), trackParticle->m());

        track->setFlag(xAOD::TauJetParameters::TauTrackFlag::wideTrack, true);
        track->setFlag(xAOD::TauJetParameters::TauTrackFlag::passTrkSelector, true);
        // in case TrackClassifier is not run, still get sensible results
        track->setFlag(xAOD::TauJetParameters::TauTrackFlag::classifiedIsolation, true); // for sake of trigger, reset in TauTrackClassifier
        track->setFlag(xAOD::TauJetParameters::TauTrackFlag::modifiedIsolationTrack, true); // for sake of trigger, reset in TauTrackClassifier
        track->setFlag(xAOD::TauJetParameters::TauTrackFlag::unclassified, true); 

        if(! m_bypassExtrapolator ) extrapolateToCaloSurface( track, trackParticle ) ;

        ElementLink<xAOD::TauTrackContainer> linkToTauTrack;
        linkToTauTrack.toContainedElement(*m_tauTrackCont, track);
        pTau.addTauTrackLink(linkToTauTrack);

        ++track_it;
      } 
    }

    //These are set again in TauTrackClassifier
    pTau.setDetail(xAOD::TauJetParameters::nChargedTracks, static_cast<int>( pTau.nTracks() ) );
    pTau.setDetail(xAOD::TauJetParameters::nIsolatedTracks, static_cast<int>( pTau.nTracks(xAOD::TauJetParameters::classifiedIsolation)) );

    for ( auto  track_it =  otherTracks.begin(); track_it !=  otherTracks.end() ;)
    {
      ATH_MSG_VERBOSE(name()     << " adding other track nr: " 
              << " eta " << (*track_it)->eta()
              << " phi " << (*track_it)->phi()
      );


      if (    muTrack.Pt() > 0 && muTrack.DeltaR( (*track_it)->p4() ) < 0.05
           && std::abs( ( muTrack.Pt() - (*track_it)->pt() )/muTrack.Pt() ) < 0.05  
           && muQlt <= 2 )  // tentatively LOOSE muon seem to provide better performance
      {
        //  tentatively LOOSE muon seem to provide better performance 
        ATH_MSG_DEBUG( "overlapping preSelelcted muon in TrackFinder::otherTracks " ) ;
        track_it = otherTracks.erase(track_it);
      }
      else
      {
        xAOD::TauTrack* track = new xAOD::TauTrack();
        m_tauTrackCont->push_back(track);

        const xAOD::TrackParticle* trackParticle = *track_it ;

        ElementLink<xAOD::TrackParticleContainer> linkToTrackParticle;
        linkToTrackParticle.toContainedElement(*m_trackParticleCont, trackParticle);
        track->addTrackLink(linkToTrackParticle);
        track->setP4(trackParticle->pt(), trackParticle->eta(), trackParticle->phi(), trackParticle->m());
        float dR = track->p4().DeltaR(pTau.p4());
        if(dR<=0.2) track->setFlag(xAOD::TauJetParameters::TauTrackFlag::coreTrack, true);
        else track->setFlag(xAOD::TauJetParameters::TauTrackFlag::wideTrack, true);
        track->setFlag(xAOD::TauJetParameters::TauTrackFlag::unclassified, true); 

        if(! m_bypassExtrapolator ) extrapolateToCaloSurface( track, trackParticle ) ;

        ElementLink<xAOD::TauTrackContainer> linkToTauTrack;
        linkToTauTrack.toContainedElement(*m_tauTrackCont, track);
        pTau.addTauTrackLink(linkToTauTrack);

        ++ track_it ;
      } 

    }

    ATH_MSG_DEBUG("numTrack: " << "/" << pTau.nTracks());
    ATH_MSG_DEBUG("charge: " << "/" << pTau.charge());

    return StatusCode::SUCCESS;
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
MuHadTrackFinder::TauTrackType MuHadTrackFinder::tauTrackType( const xAOD::TauJet& pTau,
        const xAOD::TrackParticle & trackParticle,
        const xAOD::Vertex* primaryVertex)
{
    double dR = Tau1P3PKineUtils::deltaR( pTau.eta(),pTau.phi(), trackParticle.eta(), trackParticle.phi() );

    if ( dR > m_maxJetDr_wide ) return NotTauTrack;

    ATH_MSG_DEBUG(" TauTrack : " << trackParticle.pt() <<" "<< trackParticle.eta() <<" "<< trackParticle.phi() <<" "<< dR );

    bool goodTrack = true;
    if(!m_bypassSelector)
      goodTrack = m_trackSelectorTool_tau->decision( trackParticle, primaryVertex );
    
    if (goodTrack) {
      if (dR > m_maxJetDr_tau )
        return TauTrackWide;
      else
        return TauTrackCore;
    } else
      return TauTrackOther;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
StatusCode  MuHadTrackFinder::getTauTracksFromPV( const xAOD::TauJet& pTau,
        const xAOD::Vertex* primaryVertex,
        std::vector<const xAOD::TrackParticle*> &tauTracks,
        std::vector<const xAOD::TrackParticle*> &wideTracks,
        std::vector<const xAOD::TrackParticle*> &otherTracks)
{
  
    for ( const xAOD::TrackParticle *trackParticle : * m_trackParticleCont ) 
    {

        TauTrackType type = tauTrackType( pTau, *trackParticle, primaryVertex );

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

    return StatusCode::SUCCESS ;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

bool MuHadTrackFinder::extrapolateToCaloSurface( xAOD::TauTrack * tauTrack , const xAOD::TrackParticle * orgTrack )
{
        // set default values
        float etaEM = -10.0;
        float phiEM = -10.0;
        float etaHad = -10.0;
        float phiHad = -10.0;
        Trk::TrackParametersIdHelper parsIdHelper;

        // get the extrapolation into the calo
        ATH_MSG_DEBUG( "Try extrapolation of track with pt = " << orgTrack->pt() 
                       << ", eta " << orgTrack->eta() 
                       << ", phi" << orgTrack->phi() );

        const Trk::CaloExtension* caloExtension = nullptr;
        if (not m_caloExtensionTool->caloExtension(*orgTrack,caloExtension)
            or caloExtension->caloLayerIntersections().empty() )
        { 
            ATH_MSG_DEBUG("Track extrapolation failed");
        }
        else {
            ATH_MSG_DEBUG("Scanning samplings");
            bool validECal = false;
            bool validHCal = false;
            for( auto cur : caloExtension->caloLayerIntersections() ){
                ATH_MSG_DEBUG("Sampling " << parsIdHelper.caloSample(cur->cIdentifier()) );
                
                // only use entry layer
                if( not parsIdHelper.isEntryToVolume(cur->cIdentifier()) ) continue;

                CaloSampling::CaloSample sample = parsIdHelper.caloSample(cur->cIdentifier());

                // ECal
                if( not validECal and m_EMSamplings.count(sample))
                {
                    validECal = true;
                    etaEM = cur->position().eta();
                    phiEM = cur->position().phi();
                    ATH_MSG_DEBUG("Extrapolated to ECal layer " << sample);
                }

                 // HCal
                if( not validHCal and m_HadSamplings.count(sample))
                {
                    validHCal = true;
                    etaHad = cur->position().eta();
                    phiHad = cur->position().phi();
                    ATH_MSG_DEBUG("Extrapolated to HCal layer " << sample);
                }
                if( validECal and validHCal ) break;
            }
            // EM failure warn if within acceptance 
            if( not validECal and abs(orgTrack->pt()) < 2.48 ){
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
        tauTrack->setDetail(xAOD::TauJetParameters::CaloSamplingEtaEM, static_cast<float>( etaEM) );
        tauTrack->setDetail(xAOD::TauJetParameters::CaloSamplingPhiEM, static_cast<float>( phiEM) );
        tauTrack->setDetail(xAOD::TauJetParameters::CaloSamplingEtaHad, static_cast<float>( etaHad) ) ;
        tauTrack->setDetail(xAOD::TauJetParameters::CaloSamplingPhiHad, static_cast<float>(phiHad) ) ;

    return true ;

}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
void MuHadTrackFinder::removeOffsideTracksWrtLeadTrk(std::vector<const xAOD::TrackParticle*> &tauTracks,
                           std::vector<const xAOD::TrackParticle*> &wideTracks,
                           std::vector<const xAOD::TrackParticle*> &otherTracks,
                           const xAOD::Vertex* tauOrigin,
                           double maxDeltaZ0)
{
    const float MAX=1e5;
    this->resetDeltaZ0Cache();

    // need at least one core track to have a leading trk to compare with
    if (tauTracks.size()<1) return;

    // get lead trk parameters
    const xAOD::TrackParticle *leadTrack = tauTracks.at(0);
    float z0_leadTrk = getZ0(leadTrack, tauOrigin);

    if (z0_leadTrk > MAX-1) return; // bad lead trk -> do nothing

    ATH_MSG_VERBOSE("before z0 cut: #coreTracks=" << tauTracks.size() << ", #wideTracks=" << wideTracks.size() << ", #otherTracks=" << otherTracks.size());

    // check core tracks
    // skip leading track, because it is the reference
    std::vector<const xAOD::TrackParticle*>::iterator itr = tauTracks.begin()+1;
    while (itr!=tauTracks.end()) {
        float z0 = getZ0(*itr, tauOrigin);
        float deltaZ0=z0 - z0_leadTrk;

        ATH_MSG_VERBOSE("core Trks: deltaZ0= " << deltaZ0);
        m_vDeltaZ0coreTrks.push_back(deltaZ0);

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
        m_vDeltaZ0wideTrks.push_back(deltaZ0);

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
float MuHadTrackFinder::getZ0(const xAOD::TrackParticle* track, const xAOD::Vertex* vertex)
{
    const float MAX=1e5;

    if (!track) return MAX;

    std::unique_ptr< const Trk::Perigee > perigee ;
    if (  vertex != nullptr ) 
      perigee =std::move(std::unique_ptr< const Trk::Perigee>(  
                           m_trackToVertexTool->perigeeAtVertex(*track, vertex->position() )  )  ) ;
    else 
      perigee =std::move( std::unique_ptr< const Trk::Perigee >(  
                          m_trackToVertexTool->perigeeAtVertex(*track )  )   ) ; 

    if (!perigee) {
        ATH_MSG_WARNING("Bad track; can't find perigee at vertex.");
        return MAX;
    }

    float z0 = perigee->parameters()[Trk::z0];

    return z0;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
void MuHadTrackFinder::getDeltaZ0Values(std::vector<float>& vDeltaZ0coreTrks, std::vector<float>& vDeltaZ0wideTrks)
{
  vDeltaZ0coreTrks.clear();
  vDeltaZ0coreTrks = m_vDeltaZ0coreTrks;

  vDeltaZ0wideTrks.clear();
  vDeltaZ0wideTrks = m_vDeltaZ0wideTrks;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
void MuHadTrackFinder::resetDeltaZ0Cache()
{
    m_vDeltaZ0coreTrks.clear();
    m_vDeltaZ0wideTrks.clear();
}

#endif
