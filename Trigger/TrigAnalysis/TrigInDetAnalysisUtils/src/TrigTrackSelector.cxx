/**
 **     @file    TrigTrackSelector.cxx
 **
 **     @author  mark sutton
 **     @date    Sun  2 Nov 2014 11:10:06 CET 
 **
 **     Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
 **/


#include "TrigInDetAnalysisUtils/TrigTrackSelector.h"
#include "TruthUtils/HepMCHelpers.h"

#include "xAODTruth/TruthVertexContainer.h"

#include <stdexcept>

/// NB: This was 47 for Run 2, but wit the addition of the IBL it should be 32 
///     It was kept at 47 for all Run 2 and mogration to MT, but for Run 3 we 
///     really want it changed to be 32  
const double TrigTrackSelector::s_default_radius = 47;


TrigTrackSelector::TrigTrackSelector( TrackFilter* selector, double radius, int selectPdgId, int selectParentPdgId ) : 
    TrackSelector(selector), m_id(0), m_xBeam(0), m_yBeam(0), m_zBeam(0),
    m_correctTrkTracks(false), 
    m_radius(radius), m_selectPdgId(selectPdgId), m_selectParentPdgId(selectParentPdgId) { 
} 



/// recursive function to identify whether a particle comes from some other 
/// ancestor particle at any point in it's history - checks it's parents and 
/// then function calls itself to check their parents etc, until it either 
/// finds the pdgid it is looking for, or it has no more ancestors
/// if it finds an appropriate ancestor it returns the pointer to it, 
/// otherwise it returns a nullptr 

const xAOD::TruthParticle* TrigTrackSelector::fromAncestor(const int pdg_id,  const xAOD::TruthParticle *p) const { 
  if ( p==nullptr ) return nullptr;
  if (p->absPdgId()==11 || p->absPdgId()==13 ) return nullptr; //don't want light leptons from eg tau decays - they are found directly
  if ( p->absPdgId()==pdg_id ) {
    return p;   // recursive stopping conditions
  }
  auto vertex = p->prodVtx();
  if ( vertex == nullptr ) {
    return nullptr; // has no production vertex !!!
  }
  if ( vertex->nIncomingParticles() < 1 ) {
    return nullptr;  // recursive stopping conditions
  }
  for( unsigned ip = 0; ip < vertex->nIncomingParticles(); ip++ ) {
    auto* in = vertex->incomingParticle(ip);
    auto parent = fromAncestor( pdg_id, in);
    if ( parent!=nullptr ) { 
      if (parent->absPdgId()==pdg_id) return parent;
    }
  }
  
  return nullptr;
}


/// recursive function to identify whether a particle comes from some other 
/// number of ancestor particles, with the pdgids passed in as a vector.
/// Any such ancestors at point in it's history will do - checks it's parents 
/// and then calls the function calls itself to check the parent's parents etc, 
/// until it either finds one of the pdgids it is looking for, or it has no 
/// more ancestors 
/// if it finds an appropriate ancestor it returns the pointer to it, otherwise 
/// it returns a nullptr 

const xAOD::TruthParticle* TrigTrackSelector::fromAncestor( const std::vector<int>& ids,  const xAOD::TruthParticle *p ) const { 
  if ( p==nullptr ) return nullptr;
  if (p->absPdgId()==11 || p->absPdgId()==13 ) return nullptr; //don't want light leptons from eg tau decays - they are found directly
  for ( size_t i=ids.size() ; i-- ; ) { 
    if ( p->absPdgId()==ids[i] ) return p; // recursive stopping conditions
  }

  auto vertex = p->prodVtx();
  if ( vertex == nullptr ) return nullptr; // has no production vertex !!!
 
  if ( vertex->nIncomingParticles()<1 ) return nullptr;  // recursive stopping conditions
 
  for( unsigned ip = 0; ip < vertex->nIncomingParticles(); ip++ ) {
    auto* in = vertex->incomingParticle(ip);
    auto parent = fromAncestor( ids, in);
    if ( parent!=nullptr ) { 
      for ( size_t i=ids.size() ; i-- ; ) { 
	if ( parent->absPdgId()==ids[i] ) return parent; 
      }
    }
  }  

  return nullptr;
}



/// neater code to make use of vector function also for a single ancestor pdgid, instead of 
/// the full code duplication, but less efficienct as it then needs to create a single element 
/// vector for each particle, to avoid the code duplication.
/// Perhaps something can be done for the compiler optimisation to realise this and somehow  
/// reduce the opverhead of creating the single int vector - need to investiogate this more
/// thoroghly before including this, as it would be a much neater solution

// const xAOD::TruthParticle* TrigTrackSelector::fromAncestor(const int pdg_id,  const xAOD::TruthParticle *p) const { 
//   return fromAncestor( std::vector<int>(1,pdg_id), p );
// }
  

  



bool TrigTrackSelector::selectTrack( const TrigInDetTrack* track, const TrigInDetTrackTruthMap* truthMap ) {     
    // do the track extraction stuff here....
    if ( track ) { 

	double eta    = track->param()->eta();
	double phi    = track->param()->phi0();
	double z0     = track->param()->z0(); 
	double pT     = track->param()->pT(); 
	double d0     = track->param()->a0();

	double deta    = track->param()->eeta();
	double dphi    = track->param()->ephi0();
	double dz0     = track->param()->ez0(); 
	double dpT     = track->param()->epT(); 
	double dd0     = track->param()->ea0();

	double theta = 2*std::atan( exp( (-1)*eta ) );
	correctToBeamline( z0, dz0, d0, dd0, theta, phi );
	
	int   algoid  = track->algorithmId(); 	      

	int nBlayerHits = (track->HitPattern() & 0x1);
	int nPixelHits  = 2 * track->NPixelSpacePoints();  // NB: for comparison with offline 
	int nSctHits    = 2 * track->NSCT_SpacePoints();   //     a spacepoint is 2 "hits"
	int nStrawHits  =  track->NStrawHits();
	int nTrHits     =  track->NTRHits();

	int nSiHits     = nPixelHits + nSctHits;

	bool expectBL   = false;                           //not filled in 

	unsigned long id = (unsigned long)track;

	unsigned hitPattern = track->HitPattern();
	unsigned multiPattern = 0;

	double chi2    = track->chi2();
	double dof     = 0; /// not definied ofr TrigInDetTracks

	bool truth = false;
	int match_barcode = -1;
	
	if ( truthMap ) { 
	  const TrigInDetTrackTruth* trackTruth = truthMap->truth(track);
	  if (trackTruth!=0 && trackTruth->nrMatches() > 0) {
	    match_barcode = trackTruth->bestSiMatch()->barcode();
	    truth = true;
	  }
	}
	
	
	TIDA::Track* t = new TIDA::Track(  eta,  phi,  z0,  d0,  pT, chi2, dof, 
								     deta, dphi, dz0, dd0, dpT, 
								     nBlayerHits, nPixelHits, nSctHits, nSiHits, 
								     nStrawHits, nTrHits, 
								     hitPattern, multiPattern, 
								     algoid, truth, -1, match_barcode,
								     expectBL, id) ; 
	
	//	std::cout << "SUTT ID track " << *t << "\t0x" << std::hex << track->HitPattern() << std::dec << std::endl;
	
	if ( !addTrack( t ) ){
	  delete t;
	  return false;
	}	
	return true;
    }
    return false;
}


// extract all the tracks from a TrigInDetTrack collection and associated TruthMap and convert them
void TrigTrackSelector::selectTracks( const TrigInDetTrackCollection* trigtracks, const TrigInDetTrackTruthMap* truthMap ) {     
    // do the track extraction stuff here....
    TrigInDetTrackCollection::const_iterator  trackitr = trigtracks->begin();
    TrigInDetTrackCollection::const_iterator  trackend = trigtracks->end();
    while ( trackitr!=trackend ) { 
      selectTrack( *trackitr, truthMap );
      ++trackitr;
    }
}


// add a TrackParticle 
bool TrigTrackSelector::selectTrack( const Rec::TrackParticle* track ) { 
        
    // do the track extraction stuff here....

    static const int hpmap[20] = { 0, 1, 2,  7, 8, 9,  3, 4, 5, 6, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19 };

    if ( track ) { 
	    
#ifdef TRKPARAMETERS_MEASUREDPERIGEE_H
      const Trk::MeasuredPerigee* measPer = track->measuredPerigee();
#else
      const Trk::Perigee* measPer = track->measuredPerigee();
#endif
      //     CLHEP::HepVector perigeeParams = measPer->parameters();
      
      double pT    = measPer->pT(); 
      double eta   = measPer->eta();
      double phi   = measPer->parameters()[Trk::phi0];
      double z0    = measPer->parameters()[Trk::z0] + m_zBeam; /// temporarily remove interaction mean z position
      double d0    = measPer->parameters()[Trk::d0];

      double theta = measPer->parameters()[Trk::theta];
      double  p    = 1/measPer->parameters()[Trk::qOverP];
  
      // AAARCH!!!!! the TrackParticle pT is NOT SIGNED!!!! ( I ask you! ) 
      if ( measPer->parameters()[Trk::qOverP]<0 && pT>0 ) pT *= -1;

#ifdef TRKPARAMETERS_MEASUREDPERIGEE_H
      const Trk::ErrorMatrix err = measPer->localErrorMatrix();
      double dtheta = err.error(Trk::theta);
      double dqovp  = err.error(Trk::qOverP); 
      double covthetaOvP  = err.covValue(Trk::qOverP,Trk::theta);  
#else
      double dtheta      = std::sqrt((*measPer->covariance())(Trk::theta,Trk::theta));
      double dqovp       = std::sqrt((*measPer->covariance())(Trk::qOverP,Trk::qOverP)); 
      double covthetaOvP = (*measPer->covariance())(Trk::qOverP,Trk::theta);  
#endif


      double deta = 0.5*dtheta/(std::cos(0.5*theta)*std::cos(0.5*theta)*std::tan(0.5*theta));   /// << check this <<--

#ifdef TRKPARAMETERS_MEASUREDPERIGEE_H
      double dphi = err.error(Trk::phi0);
      double dz0  = err.error(Trk::z0);
      double dd0  = err.error(Trk::d0);
#else
      double dphi = std::sqrt((*measPer->covariance())(Trk::phi0,Trk::phi0));
      double dz0  = std::sqrt((*measPer->covariance())(Trk::z0,Trk::z0));
      double dd0  = std::sqrt((*measPer->covariance())(Trk::d0,Trk::d0));
#endif

      double dpT  = 0; 


      double sintheta = std::sin(theta);
      double costheta = std::cos(theta);
      double dpt2 = (p*p*sintheta)*(p*p*sintheta)*dqovp*dqovp + (p*costheta)*(p*costheta)*dtheta*dtheta - 2*(p*p*sintheta)*(p*costheta)*covthetaOvP;   /// check this !!!

      if ( dpt2>0 ) dpT = std::sqrt( dpt2 );

      // Check number of hits
      // NB: a spacepoint is two offline "hits", so a pixel spacepoint is really 
      // 2 "hits" and an offline SCT "hit" is really a 1D cluster, so two intersetcting
      // stereo clusters making a spacepoint are two "hits"
      const Trk::TrackSummary *summary = track->trackSummary();
      int nBlayerHits = 2*summary->get(Trk::numberOfBLayerHits); 
      int nPixelHits  = 2*summary->get(Trk::numberOfPixelHits);  
      int nSctHits    = summary->get(Trk::numberOfSCTHits); 
      int nStrawHits  = summary->get(Trk::numberOfTRTHits);
      int nTrHits     = summary->get(Trk::numberOfTRTHighThresholdHits);

      int nSiHits     = nPixelHits + nSctHits;
      bool expectBL   = false; // Not stored for Rec::TrackParticle

      const Trk::FitQuality *quality   = track->fitQuality();
      double chi2 = quality->chiSquared();
      double dof  = quality->numberDoF();
      
      unsigned bitmap = 0;

      
      unsigned long id = (unsigned long)track;

      for ( int ih=0 ; ih<20 ; ih++ ) {
	if ( summary->isHit(Trk::DetectorType(ih)) ) bitmap |= ( 1<<hpmap[ih] ); 	
      }

      /// now some *ridiculous* code to get the author of the 
      /// TrackParticle (offline) tracks
      
      //      std::cout << "fetching author info :" << track->info().trackFitter() << ":" 
      //		<< track->info().dumpInfo() << ": bm 0x" << std::hex << bitmap << std::dec << std::endl;
      
      int fitter = track->info().trackFitter();
      std::string dumpinfo = track->info().dumpInfo();
      
      int trackAuthor = -1; 
      if ( fitter>0 && fitter<Trk::TrackInfo::NumberOfTrackFitters ) {
	if      ( dumpinfo.find("TRTStandalone")!=std::string::npos)        trackAuthor = 2;
	else if ( dumpinfo.find("TRTSeededTrackFinder")!=std::string::npos) trackAuthor = 1;
	else                                                                trackAuthor = 0;
      }
      
#if 0
      std::cout << "\t\t\tSUTT TP track" 
		<< "\teta=" << eta  // << " +- " << (*trackitr)->params()->deta()
		<< "\tphi=" << phi  // << " +- " << (*trackitr)->params()->dphi()
		<< "\tz0="  << z0 
		<< "\tpT="  << pT // << "\t( " << 1/qoverp << ")"
		<< "\td0="  << d0
		<< "\tNsi=" << nSiHits 
		<< "\tNtrt=" << nTrHits 
		<< "\tNstr=" << nStrawHits
		<< "\tauthor=" << trackAuthor
		<< std::endl;
#endif	

      // Create and save Track
      
      TIDA::Track* t = new TIDA::Track(eta, phi, z0, d0, pT, chi2, dof,
                                       deta, dphi, dz0, dd0, dpT,
                                       nBlayerHits, nPixelHits, nSctHits, nSiHits,
                                       nStrawHits, nTrHits, bitmap, 0,
                                       trackAuthor, false, -1, -1,  
                                       expectBL, id) ;  

      //      std::cout << "SUTT TP track " << *t << "\t0x" << std::hex << bitmap << std::dec << std::endl; 
      
      if ( !addTrack( t ) ){
	delete t;
	return false;
      }
      return true;
      
    }
    return false;
}
  

// extract all the tracks from a TrackParticle collection and add them
void TrigTrackSelector::selectTracks( const Rec::TrackParticleContainer* trigtracks ) { 
    
    //    std::cout << "\t\t\tSUTT \tTrackParticleContainer->size() = " << trigtracks->size() << std::endl;
    
    Rec::TrackParticleContainer::const_iterator  trackitr = trigtracks->begin();
    Rec::TrackParticleContainer::const_iterator  trackend = trigtracks->end();
   
    while ( trackitr!=trackend ) { 
    
      selectTrack( *trackitr );
      
      ++trackitr;

    } // loop over tracks
    
}



// extract all the tracks from a TruthParticle collection and add them
void TrigTrackSelector::selectTracks( const TruthParticleContainer* truthtracks ) { 
    //    std::cout << "\t\t\tSUTT \tTrackParticleContainer->size() = " << trigtracks->size() << std::endl;
    
    TruthParticleContainer::const_iterator  trackitr = truthtracks->begin();
    TruthParticleContainer::const_iterator  trackend = truthtracks->end();
   
    while ( trackitr!=trackend ) { 
    
      selectTrack( *trackitr );
      
      ++trackitr;

    } // loop over tracks
    
}


/// extract all the tracks from a xAOD::TruthParticle collection and 
/// histogram the x and y production coordinates to determine the 
/// event "beamline" position

void TrigTrackSelector::truthBeamline( const xAOD::TruthParticleContainer* truthtracks, double& x0, double& y0 ) { 

  x0 = 0;
  y0 = 0;
    
  /// histograms and book keeoing
 
  int Nx = 300;
  int Ny = 300;

  /// positions
  std::vector<double> xpos(Nx,0);
  std::vector<double> ypos(Ny,0);

  /// numbers of tracks
  std::vector<int> xn(Nx,0);
  std::vector<int> yn(Ny,0);

  int xoffset = Nx/2;; 
  int yoffset = Ny/2; 

  double deltax = 3.0/Nx;
  double deltay = 3.0/Ny;

  /// fill histograms ...

  xAOD::TruthParticleContainer::const_iterator  trackitr = truthtracks->begin();
  xAOD::TruthParticleContainer::const_iterator  trackend = truthtracks->end();
    
  for ( ; trackitr!=trackend ; ++trackitr ) { 
    
    const xAOD::TruthParticle* track = (*trackitr);
    
    if ( track->status() != 1 || !track->hasProdVtx() ) continue; 

    /// get track production vertex

    double xp[3] = { track->prodVtx()->x(), track->prodVtx()->y(), track->prodVtx()->z() };

    /// add to the correct bins

    int ix = xp[0]/deltax + xoffset;
    int iy = xp[1]/deltay + yoffset;

    if ( ix<0 || ix>=Nx || iy<0 || iy>=Nx ) continue;

    xpos[ix] += xp[0];
    ypos[iy] += xp[1];

    xn[ix]++;
    yn[iy]++;

  } // loop over tracks

  /// calculate the most populous bin in x and y 

  int imx = 0; 
  int imy = 0; 

  for ( size_t i=0 ; i<xpos.size() ; i++ ) {
    if ( xn[i]>xn[imx] ) imx = i;
    if ( yn[i]>yn[imy] ) imy = i;
  }

  /// require more than 1 particle for it to be classed a "vertex"
  /// therefore, for single particle Monte Carlo, this will not 
  /// be updated and (0,0) will still be correctly used 
  if ( xn[imx]>1 ) x0 = xpos[imx]/xn[imx];
  if ( yn[imy]>1 ) y0 = ypos[imy]/yn[imy];

}


// extract all the tracks from a xAOD::TruthParticle collection and add them
void TrigTrackSelector::selectTracks( const xAOD::TruthParticleContainer* truthtracks) { 

  xAOD::TruthParticleContainer::const_iterator  trackitr = truthtracks->begin();
  xAOD::TruthParticleContainer::const_iterator  trackend = truthtracks->end();
  
  /// get truth beamline 
  double x0 = 0;
  double y0 = 0;

  truthBeamline( truthtracks, x0, y0 );


  for ( ; trackitr!=trackend; ++trackitr) {


    // Only select charged final state particles
    double q = (*trackitr)->charge();

    /// fix default (unset) TruthParticle charge
    static const particleType ptype;
    if ( q==-999 ) q = ptype.charge( (*trackitr)->pdgId() );

    if (q == 0 || (*trackitr)->status() !=1) continue;

    // If looking for tau parents, don't select mu or e children

    // select based on the pdg of final state particle 
    bool gotPdgId = true;
    if (m_selectPdgId!=0) gotPdgId = (*trackitr)->absPdgId()==m_selectPdgId;

    // select based on the pdg of the parent or ancestor
    bool gotParentPdgId = true;
    if   ( gotPdgId && m_selectParentPdgId!=0 )         gotParentPdgId = fromAncestor(m_selectParentPdgId, (*trackitr))!=nullptr;
    /// not just yet - save for later ...
    /// else ( gotPdgId && m_selectParentPdgIds.size()!=0 ) gotParentPdgId = fromAncestor(m_selectParentPdgIds, (*trackitr))!=nullptr;

    if ( gotParentPdgId && gotPdgId ) selectTrack( *trackitr, x0, y0);

  } // loop over tracks
    
}



// add a TruthParticle from a GenParticle - easy, bet it doesn't work 
bool TrigTrackSelector::selectTrack( HepMC::ConstGenParticlePtr track ) {
  
    /// not a "final state" particle
    if ( track->status() != 1 ) return false;

    /// set this so can use it as the identifier - don't forget to reset!!
//AV Using memory to get some value is not a good idea. This is not a repruducible/portable way, but I leave it as is.
#ifdef HEPMC3
    m_id = (unsigned long)(track.get());
#else
    m_id = (unsigned long)track;
#endif
    bool sel;
    sel = selectTrack( TruthParticle(track) );
    m_id = 0;
    
    return sel; 
    
}


// add a TruthParticle 
bool TrigTrackSelector::selectTrack( const TruthParticle& track ) {

  return selectTrack( &track );
  
}


// add a TruthParticle 
bool TrigTrackSelector::selectTrack( const TruthParticle* track ) { 
    TIDA::Track* t = makeTrack( track, m_id );
    if ( t == 0 ) return false;
    if ( !addTrack(t) ) {
      delete t;
      return false;
    }
    return true;
}




// add an xAOD::TruthParticle 
bool TrigTrackSelector::selectTrack( const xAOD::TruthParticle* track, double x0, double y0) { 
  if ( track ) { 
        
    if (!MC::isStable(track) ) return false;

    /// lazy just to avoid a find-replace of measPer to track
    const xAOD::TruthParticle* measPer = track;

    double pT    = measPer->pt(); 
    double eta   = measPer->eta();
    double phi   = measPer->phi();
  
    // AAARCH!!!!! the TrackParticle pT is NOT SIGNED!!!! ( I ask you! ) 
    if ( measPer->charge()<0 && pT>0 ) pT *= -1;
    double q = track->charge();

    static const particleType ptype;

    /// avoid default (unset) TruthParticle charge
    if ( q==-999 ) q = ptype.charge( track->pdgId() );
    
    ///  only use charged tracks
    if ( q==0 ) return 0;

    /// need to calculate the origin, and the beamline, and the d0 and z0 
    /// with respect to the beamline
    /// leave this in until we have checked whether everything is implemented correctly
    //      double xbeam = getBeamX(); // track->vx(); 
    //      double ybeam = getBeamY(); // track->vy(); 
    //      double zbeam = getBeamZ(); // track->vz(); 
      
    if ( !track->hasProdVtx() ) return false; 

    /// need to calculate d0 and z0 correctly.

    double xp[3] = { measPer->prodVtx()->x(), measPer->prodVtx()->y(), measPer->prodVtx()->z() };
    double xb[3] = { xp[0]-x0, xp[1]-y0, measPer->prodVtx()->z() };
    double xd[3] = { 0, 0, 0 };
      
    if ( track->hasDecayVtx() ) { 
      xd[0] = track->decayVtx()->x();
      xd[1] = track->decayVtx()->y();
      xd[2] = track->decayVtx()->z();
    }
      
    double rp = std::sqrt( xp[0]*xp[0] + xp[1]*xp[1] ); 
    double rd = std::sqrt( xd[0]*xd[0] + xd[1]*xd[1] ); 
      
    /// these are the d0 and z at the point of closest approach to x0, y0, the event 
    /// "beamline", which is the best that we can do for the moment
    double theta = 2*std::atan( std::exp( -eta ) );
    double z0 = xb[2] - (xb[0]*std::cos(phi) + xb[1]*std::sin(phi))/std::tan(theta);
    double d0 = xb[1]*std::cos(phi) -  xb[0]*std::sin(phi);

    bool final_state = false; 
      
    /// the is our new "final state" requirement
    /// the inner and outer radii are in some sense 
    /// arbitrary - these correspond to an envelope
    /// around the pixel detector, so the track must 
    /// pass through the entire pixel detector 
    /// NB: In order to ensure you don't miss any 
    ///     tracks they really need to be the same
    ///     ie if a track brems in your "volume"
    ///     then you will miss that track, and
    ///     also the resulting track, even if it is
    ///     a high et track  
    const double inner_radius = m_radius; /// was hardcoded as 47 - now is set from the constructor
    const double outer_radius = m_radius;

    if ( (  track->hasProdVtx() && rp<=inner_radius ) && 
	 ( !track->hasDecayVtx() || rd>outer_radius ) ) final_state = true; 
      
    if ( !final_state ) return false; 
    
    double deta = 0;
    double dphi = 0;
    double dz0  = 0;
    double dd0  = 0;

    double dpT  = 0; 

    int nBlayerHits = 0;
    int nPixelHits  = 0;
    int nSctHits    = 0;
    int nStrawHits  = 0;
    int nTrtHits    = 0;
      
    double chi2 = 0;
    double dof  = 0;

    bool expectBL = false;

    nSctHits   += 0;
    nPixelHits += 0;

    /// get the total number of holes as well
    int nSiHits     = 0;

    unsigned long id = (unsigned long)track;

    unsigned bitmap = 0;

    int trackAuthor = track->pdgId();
    int barcode     = track->barcode();

#if 0
    std::cout << "\t\t\tSUTT TP track" 
	      << "\teta=" << eta  
	      << "\tphi=" << phi  
	      << "\tz0="  << z0 
	      << "\tpT="  << pT 
	      << "\td0="  << d0
	      << "\tauthor=" << trackAuthor
	      << "\tVTX x " << xp[0]<< "\ty " << xp[1] << "\tz " << xp[2] 
	      << std::endl;
#endif

    // Create and save Track
      
    TIDA::Track* t = new TIDA::Track( eta,   phi,  z0,  d0,  pT, chi2, dof,
				      deta,  dphi, dz0, dd0, dpT,
				      nBlayerHits, nPixelHits, nSctHits, nSiHits,
				      nStrawHits,  nTrtHits,   bitmap, 0,
				      trackAuthor,  false, barcode, -1,  
				      expectBL, id) ;  

    /// useful debug info - leave in
    //      std::cout << "SUTT TP track " << *t << "\t0x" << std::hex << bitmap << std::dec << std::endl; 
      
    // addTrack applies additional cuts using the Filter
    if ( !addTrack( t ) ){
      delete t;
      return false;
    }
  }
  return false;
     
}




// make a TIDA::Track from a GenParticle 
TIDA::Track* TrigTrackSelector::makeTrack(HepMC::ConstGenParticlePtr track ) { 
//AV Using memory to get some value is not a good idea. This is not a repruducible/portable way, but I leave it as is.
#ifdef HEPMC3
    unsigned long id = (unsigned long)(track.get());
#else
    unsigned long id = (unsigned long)track;
#endif
    TruthParticle t = TruthParticle(track); 
    return  makeTrack( &t, id );
}

// make a TIDA::Track from a TruthParticle 
TIDA::Track* TrigTrackSelector::makeTrack( const TruthParticle* track, unsigned long tid ) { 
  
    if ( track==0 ) return 0; 
    if ( track->status() != 1 ) return 0;   /// check for final state


    double phi = track->phi();
    double eta = track->eta();

    ////  ABSOLUTELY STUPID!!! to get the production vertex, you need to navigate to 
    ////  the genparticle, the production vertex, etc, at each stage they could be a 
    ////  null pointer, so you would have to check all of them to be robust 
    double xp[3] = { 0, 0, 0 };

    if ( track->genParticle()->production_vertex() ) { 
      xp[0] = track->genParticle()->production_vertex()->position().x();
      xp[1] = track->genParticle()->production_vertex()->position().y();
      xp[2] = track->genParticle()->production_vertex()->position().z();
    }

    // CHANGED BY JK - z0 with respect to (0,0)
    //    double z0 = xp[2];
    double theta = 2*std::atan( exp( (-1)*eta ) );
    double z0 = xp[2] - (xp[0]*std::cos(phi) + xp[1]*std::sin(phi))/std::tan(theta);
    
    double xd[3] = { 0, 0, 0 };

    if ( track->genParticle()->end_vertex() ) { 
      xd[0] = track->genParticle()->end_vertex()->position().x();
      xd[1] = track->genParticle()->end_vertex()->position().y();
      xd[2] = track->genParticle()->end_vertex()->position().z();
    }

    double rp = std::sqrt( xp[0]*xp[0] + xp[1]*xp[1] ); 
    double rd = std::sqrt( xd[0]*xd[0] + xd[1]*xd[1] ); 

    
    bool final_state = false; 

    /// the is our new "final state" requirement
    /// the inner and outer radii are in some sense 
    /// arbitrary - these correspond to an envelope
    /// around the pixel detector, so the track must 
    /// pass through the entire pixel detector 
    /// NB: In order to ensure you don't miss any 
    ///     tracks they really need to be the same
    ///     ie if a track brems in your "volume"
    ///     then you will miss that track, and
    ///     also the resulting track, even if it is
    ///     a high et track  
    const double inner_radius = m_radius;
    const double outer_radius = m_radius;
    if ( ( track->genParticle()->production_vertex() && rp<=inner_radius ) && 
	 ( track->genParticle()->end_vertex()==0 || rd>outer_radius ) ) final_state = true; 
     
    
    if ( !final_state ) return 0; /// keep anything over 10 GeV with the old requirement
    
    //// AAAARGHHH!!!! For some *stupid* reason, when converting from a 
    //// GenParticle to a TruthParticle, the charge is not filled (or in
    //// fact it *is* filled, but with a default value of -999) and so 
    //// we would need to access the PDT in order to find it out
    //// *surely* this lookup should be done in the TruthParticle 
    //// constructor, by a helper class, and not be dependent on the 
    //// subsequent invocation of some external helper class at some 
    //// later stage? I mean, who leaves it up to some external class
    //// to provide values inside a class based on information already 
    //// in the class? It's perverse! 

    double q = track->charge();
    
    static const particleType ptype;

    /// avoid default (unset) TruthParticle charge
    if ( q==-999 ) q = ptype.charge( track->pdgId() );

    ///  only use charged tracks
    if ( q==0 ) return 0;
    
    double pT  = q*track->pt();


    double d0  = 0;

    /// what a faff - refuse to mess about with the classes to swim tracks etc - why can't 
    /// they just encode this sort of information in the class!! It's not as if it doesn't 
    /// actually have members for anything else useless!! Classes should be designed for 
    /// ease of use !!!!

    /// is there a sign issue here ?


    // CHANGED BY JK - d0 with respect to (0,0)
    // d0 = q*rp*std::sin(phi);
    d0  =  xp[1]*std::cos(phi) -  xp[0]*std::sin(phi);


    /// correct back to the beamline 

    double dz0 = 0;
    double dd0 = 0;
   
    correctToBeamline( z0, dz0, d0, dd0, theta, phi );



    //// AAAARGHHH!!!! For some *stupid* reason, when converting from a 
 
    /// static HepPDT::ParticleDataTable* m_pdt = new ParticleDataTable();
    ///
    /// // Get the Particle Properties Service
    /// if ( m_pdt==0 ) {  
    ///   ServiceHandle<IPartPropSvc> partPropSvc("PartPropSvc", "TrigTestMonToolAC"); // , name());
    ///   if ( !partPropSvc.retrieve().isSuccess() ) {
    ///      m_pdt = partPropSvc->PDT();
    ///   }
    ///   else { 
    ///      std::cerr << " Could not initialize Particle Properties Service" << std::endl;
    ///      return; // StatusCode::FAILURE;
    ///   }
    /// }    

    /// how about storing barcode/status/pidg info?
    int author  = track->pdgId();   /// this isn't good!! but it will do for testing 
    int barcode = track->barcode(); /// probably won't work either


    unsigned long id = (unsigned long)track;
    if ( tid!=0 ) id = tid;

    /// get the production vertex for the z0 and d0 but should we store the z0 of 
    /// the production vertex? or swim the track to the perigee point with respect 
    /// to 0,0 and use the parameters there? 


    /// what to do with these???
		
    //    std::cout << "\t\t\tSUTT Truth track" 
    //	            << "\teta=" << eta  // << " +- " << (*trackitr)->params()->deta()
    //	            << "\tphi=" << phi  // << " +- " << (*trackitr)->params()->dphi()
    //              << "\tz0="  << z0 
    //              << "\tpT="  << pT // << "\t( " << 1/qoverp << ")"
    //              << "\td0="  << d0
    //              << "\tauthor=" << author
    //              << std::endl;
    


    TIDA::Track* t = new TIDA::Track(eta, phi, z0, d0, pT, 0, 0,
                                     0, 0, 0, 0, 0,
                                     0, 0, 0, 0,
                                     0, 0, 0, 0,
                                     author, false, barcode, -1,
                                     false, 
                                     id ) ;  

    return t;

}



// add a Trk::Track
bool TrigTrackSelector::selectTrack( const Trk::Track* track ) { 
        
    // do the track extraction stuff here....

    static const int hpmap[20] = { 0, 1, 2,  7, 8, 9,  3, 4, 5, 6, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19 };
    //    std::cout << __FILE__<< " " <<__LINE__<<std::endl;

    if ( track ) { 
	    
      // const Trk::Perigee* startPerigee = track->perigeeParameters();

#ifdef TRKPARAMETERS_MEASUREDPERIGEE_H
      const Trk::MeasuredPerigee* startPerigee = dynamic_cast<const Trk::MeasuredPerigee*>(track->perigeeParameters());
      //      const Trk::MeasuredPerigee* measPer = startPerigee; // just out of laziness
#else
      const Trk::Perigee* startPerigee = track->perigeeParameters();
      const Trk::Perigee* measPer = startPerigee; // just out of laziness
#endif


      //      CLHEP::HepVector perigeeParams = measPer->parameters();      
      //      double pT    = measPer->pT(); 
      //      double eta   = measPer->eta();
      //      double phi   = perigeeParams[Trk::phi0];
      //      double z0    = perigeeParams[Trk::z0];
      //      double d0    = perigeeParams[Trk::d0];
      //      // AAARCH!!!!! the TrackParticle pT is NOT SIGNED!!!! ( I ask you! ) 
      //      if ( perigeeParams[Trk::qOverP]<0 ) pT *= -1;
      //      std::cout <<pT1<<" pt1vspT "<<pT<<std::endl;
      
      if (startPerigee){
     
	double theta   = startPerigee->parameters()[Trk::theta];  
	double p       = 1/startPerigee->parameters()[Trk::qOverP];
	double qOverPt = startPerigee->parameters()[Trk::qOverP]/std::sin(theta);
	double charge  = startPerigee->charge();
	double eta     = startPerigee->eta();
	double phi     = startPerigee->parameters()[Trk::phi0];
	double z0      = startPerigee->parameters()[Trk::z0];
	double d0      = startPerigee->parameters()[Trk::d0];
	//	double pT = (1./qOverPt)*(charge);	 
	double pT      = (1./qOverPt); // always use signed PT
	
	if ( charge<0 && pT>0 ) pT *= -1;
	if ( charge<0 && p>0 )  p  *= -1;
	


#ifdef TRKPARAMETERS_MEASUREDPERIGEE_H
	const Trk::ErrorMatrix err = startPerigee->localErrorMatrix();
	double dtheta = err.error(Trk::theta);
	double dqovp  = err.error(Trk::qOverP); 
	double covthetaOvP  = err.covValue(Trk::qOverP,Trk::theta);  
	
	double dphi = err.error(Trk::phi0);
	double dz0  = err.error(Trk::z0);
	double dd0  = err.error(Trk::d0);
#else
	double dtheta      = std::sqrt((*measPer->covariance())(Trk::theta,Trk::theta));
	double dqovp       = std::sqrt((*measPer->covariance())(Trk::qOverP,Trk::qOverP)); 
	double covthetaOvP = (*measPer->covariance())(Trk::qOverP,Trk::theta);    /// a covariance! 

	double dphi = std::sqrt((*measPer->covariance())(Trk::phi0,Trk::phi0));
	double dz0  = std::sqrt((*measPer->covariance())(Trk::z0,Trk::z0));
	double dd0  = std::sqrt((*measPer->covariance())(Trk::d0,Trk::d0));
#endif	
	
	double deta = 0.5*dtheta/(std::cos(0.5*theta)*std::cos(0.5*theta)*std::tan(0.5*theta));  /// check this <<--


	if ( m_correctTrkTracks ) correctToBeamline( z0, dz0, d0, dd0, theta, phi );

	double dpT  = 0; 
	
	
	double sintheta = std::sin(theta);
	double costheta = std::cos(theta);
	double dpT2 = (p*p*sintheta)*(p*p*sintheta)*dqovp*dqovp + (p*costheta)*(p*costheta)*dtheta*dtheta - 2*(p*p*sintheta)*(p*costheta)*covthetaOvP;
	
	if ( dpT2>0 ) dpT = std::sqrt( dpT2 );

	// Check number of hits
	// NB: a spacepoint is two offline "hits", so a pixel spacepoint is really 
	// 2 "hits" and an offline SCT "hit" is really a 1D cluster, so two intersetcting
	// stereo clusters making a spacepoint are two "hits"
	// const Trk::TrackSummary *summary = dynamic_cast<const Trk::TrackSummary*>(track->trackSummary());
	//ToolHandle< Trk::ITrackSummaryTool > m_trackSumTool;
	//m_trackSumTool  = ToolHandle<Trk::ITrackSummaryTool>("Trk::TrackSummaryTool/InDetTrackSummaryTool");
	//const Trk::TrackSummary* summary = NULL;       
	//summary = m_trackSumTool->createSummary(*track); 

	const Trk::TrackSummary * summary = track->trackSummary();
	int nBlayerHits = 0;
	int nPixelHits  = 0;
	int nSctHits    = 0;
	int nStrawHits  = 0;
	int nTrHits     = 0;
	int nSiHits     = 0;
	bool expectBL = false; // Not stored for Trk::Track
	unsigned bitmap = 0;

	if(summary==0){
            std::cerr << "Could not create TrackSummary  - Track will likely fail hits requirements" << std::endl;
	}    
	else{      
            nBlayerHits = 2*summary->get(Trk::numberOfBLayerHits); 
            nPixelHits  = 2*summary->get(Trk::numberOfPixelHits);  
	    nSctHits    = summary->get(Trk::numberOfSCTHits); 
            nStrawHits  = summary->get(Trk::numberOfTRTHits);
            nTrHits     = summary->get(Trk::numberOfTRTHighThresholdHits);
	    nSiHits     = nPixelHits + nSctHits;

	    for ( int ih=0 ; ih<20 ; ih++ ) {
	      if ( summary->isHit(Trk::DetectorType(ih)) ) bitmap |= ( 1<<hpmap[ih] ); 	
	    }
	}

	unsigned long id = (unsigned long)track;
	double chi2 = 0;
	double dof  = 0; 
	//const Trk::FitQuality *quality   = dynamic_cast<const Trk::FitQuality*>(track->fitQuality());
	const Trk::FitQuality *quality   = (track->fitQuality());
	if(quality==0) std::cerr << "Could not create FitQuality  - Track will likely fail hits requirements" << std::endl;
	else{
            chi2 = quality->chiSquared();          
	    dof  = quality->numberDoF();
	}

	int trackAuthor = -1;

	/// now some *ridiculous* code to get the author of the 
	/// TrackParticle (offline) tracks

	//      std::cout << "fetching author info :" << track->info().trackFitter() << ":" 
	//		<< track->info().dumpInfo() << ": bm 0x" << std::hex << bitmap << std::dec << std::endl;

	int fitter = track->info().trackFitter();
	//      std::string dumpinfo = track->info().dumpInfo();

	if ( fitter>0 && fitter<Trk::TrackInfo::NumberOfTrackFitters ) {
	  if	 ((track->info().dumpInfo()).find("TRTStandalone")        != std::string::npos) trackAuthor = 2;
	  else if	((track->info().dumpInfo()).find("TRTSeededTrackFinder") != std::string::npos) trackAuthor = 1;
	  else                                                                                    trackAuthor = 0;
	}

  #if 0
	std::cout << "\t\t\tSUTT TP track" 
		  << "\teta=" << eta  // << " +- " << (*trackitr)->params()->deta()
		  << "\tphi=" << phi  // << " +- " << (*trackitr)->params()->dphi()
		  << "\tz0="  << z0 
		  << "\tpT="  << pT // << "\t( " << 1/qoverp << ")"
		  << "\td0="  << d0
		  << "\tNsi=" << nSiHits 
		  << "\tNtrt=" << nTrHits 
		  << "\tNstr=" << nStrawHits
		  << "\tauthor=" << trackAuthor
		  << std::endl;
  #endif	
	// Create and save Track      
	TIDA::Track* t = new TIDA::Track(eta, phi, z0, d0, pT, chi2, dof,
                                         deta, dphi, dz0, dd0, dpT,
                                         nBlayerHits, nPixelHits, nSctHits, nSiHits,
                                         nStrawHits, nTrHits, bitmap, 0,
                                         trackAuthor,  false, -1, -1,  
                                         expectBL, id) ;  

	if ( !addTrack( t ) ){
	  delete t;
	  return false;
	}
	return true;
	
	//std::cout << "SUTT TP track " << *t << "\t0x" << std::hex << bitmap << std::dec << std::endl; 
      }
    }
    
    return false;
}

// extract all the tracks from a TrackCollection and add them
void  TrigTrackSelector::selectTracks( const TrackCollection* trigtracks ) { 
    
    //    std::cout << "\t\t\tSUTT \tTrackContainer->size() = " << trigtracks->size() << std::endl;
    
    TrackCollection::const_iterator  trackitr = trigtracks->begin();
    TrackCollection::const_iterator  trackend = trigtracks->end();
   
    while ( trackitr!=trackend ) { 
      selectTrack( *trackitr );
      ++trackitr;
    } // loop over tracks
    
}



bool TrigTrackSelector::selectTrack( const xAOD::TrackParticle* track, void* ) {
    // do the track extraction stuff here....

    //    static int hpmap[20] = { 0, 1, 2,  7, 8, 9,  3, 4, 5, 6, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19 };

    if ( track ) { 
	    
      /// lazy just to avoid a find-replace of measPer to track
      const xAOD::TrackParticle* measPer = track;

      //     CLHEP::HepVector perigeeParams = measPer->parameters();
      
      double pT    = measPer->pt(); 
      double eta   = measPer->eta();
      double phi   = measPer->phi0();
      double z0    = measPer->z0() + measPer->vz();  /// Grrrrr remove interaction mean z position!!!
      double d0    = measPer->d0();

      double theta = measPer->theta();

      /// protect against spurious qOverP = 0 BUT !!!
      /// this should NEVER happen !!! so shouldn't really have this failing silently, but 
      /// not allowed to put in std::cerr output
      /// leave the original code in as this is mostly for a test
      // if ( measPer->qOverP()==0 ) return false;
      if ( measPer->qOverP()==0 ) throw std::runtime_error( "probable corrupted track - this should never happen" );
      double  p    = 1/measPer->qOverP();
  
      // AAARCH!!!!! the TrackParticle pT is NOT SIGNED!!!! ( I ask you! ) 
      if ( measPer->qOverP()<0 && pT>0 ) pT *= -1;

      double dtheta      = std::sqrt(measPer->definingParametersCovMatrix()(Trk::theta,Trk::theta));
      double dqovp       = std::sqrt(measPer->definingParametersCovMatrix()(Trk::qOverP,Trk::qOverP)); 
      double covthetaOvP = measPer->definingParametersCovMatrix()(Trk::qOverP,Trk::theta);  /// a covariance *not* an error

      double deta = 0.5*dtheta/(std::cos(0.5*theta)*std::cos(0.5*theta)*std::tan(0.5*theta));  // ???? CHECK THIS <<--

      double dphi = std::sqrt(measPer->definingParametersCovMatrix()(Trk::phi0,Trk::phi0));
      double dz0  = std::sqrt(measPer->definingParametersCovMatrix()(Trk::z0,Trk::z0));
      double dd0  = std::sqrt(measPer->definingParametersCovMatrix()(Trk::d0,Trk::d0));

      double dpT  = 0; 


      /// don't correct xaod tracks to the beamline
      //      if ( m_xBeam!=0 || m_yBeam!=0 ) correctToBeamline( z0, dz0, d0, dd0, theta, phi );
      

      double sintheta = std::sin(theta);
      double costheta = std::cos(theta);
      double dpt2 = (p*p*sintheta)*(p*p*sintheta)*dqovp*dqovp + (p*costheta)*(p*costheta)*dtheta*dtheta - 2*(p*p*sintheta)*(p*costheta)*covthetaOvP;   /// ??? <<-- check this

      if ( dpt2>0 ) dpT = std::sqrt( dpt2 );

      // Check number of hits
      // NB: a spacepoint is two offline "hits", so a pixel spacepoint is really 
      // 2 "hits" and an offline SCT "hit" is really a 1D cluster, so two intersetcting
      // stereo clusters making a spacepoint are two "hits"
      
      uint8_t sum_nBlayerHits = 0;
      track->summaryValue( sum_nBlayerHits, xAOD::numberOfInnermostPixelLayerHits);
      int nBlayerHits = 2*sum_nBlayerHits;

      uint8_t  sum_nPixelHits = 0;
      track->summaryValue( sum_nPixelHits, xAOD::numberOfPixelHits);
      int nPixelHits = 2*sum_nPixelHits;

      uint8_t  sum_nSctHits = 0;
      track->summaryValue( sum_nSctHits, xAOD::numberOfSCTHits);
      int nSctHits = sum_nSctHits;

      uint8_t  sum_nStrawHits = 0;
      track->summaryValue( sum_nStrawHits, xAOD::numberOfTRTHits);
      int nStrawHits  = sum_nStrawHits;

      uint8_t  sum_nTrtHits = 0;
      track->summaryValue( sum_nTrtHits, xAOD::numberOfTRTHighThresholdHits);
      int nTrtHits  = sum_nTrtHits;
      

      uint8_t sum_expectBL  = 0;
      track->summaryValue( sum_expectBL, xAOD::expectInnermostPixelLayerHit);
      bool expectBL = ( sum_expectBL ? true : false );

      /// holes 

      uint8_t sum_sctholes  = 0;
      track->summaryValue( sum_sctholes, xAOD::numberOfSCTHoles);

      uint8_t sum_pixholes  = 0;
      track->summaryValue( sum_pixholes, xAOD::numberOfPixelHoles);

      /// cheat !! pack the holes into the hits
      /// so that eg int pixelholes() { return npix/1000; } 
      ///            int  pixelhits() { return npix%1000; }

      nSctHits   += 1000*sum_sctholes;
      nPixelHits += 1000*sum_pixholes;

      /// get the total number of holes as well
      int nSiHits     = nPixelHits + nSctHits;

      /// fit quality

      double chi2 = track->chiSquared();
      double dof  = track->numberDoF();
      
      unsigned long id = (unsigned long)track;

      unsigned bitmap = track->hitPattern();



      double xbeam = track->vx(); 
      double ybeam = track->vy(); 
      double zbeam = track->vz(); 
      
      if ( xbeam!=getBeamX() || ybeam!=getBeamY() || zbeam!=getBeamZ() ) setBeamline( xbeam, ybeam, zbeam );
	
      int trackAuthor = 0;

      int fitter = track->trackFitter();
      std::bitset<xAOD::NumberOfTrackRecoInfo>  patternrec = track->patternRecoInfo();

      //int icount = 0;<- never used if section below is commented
      for ( unsigned ipr=patternrec.size() ; ipr-- ; ) { 
	if ( patternrec[ipr] ) {
	  //icount++; <- never used if section below is commented
	  trackAuthor |= (ipr >> 16);
	  // static bool first = true;
	  // if ( first && icount>1 ) { 
	  //   std::cerr << "more than one pattern rec strategy " << ipr << "\t(suppressing further output)" << std::endl;
	  //   first = false;
	  // }
	}
      }
      
      trackAuthor |= fitter;

      //   if ( fitter>0 && fitter<Trk::TrackInfo::NumberOfTrackFitters ) {
      //	if      ( dumpinfo.find("TRTStandalone")!=std::string::npos)        trackAuthor = 2;
      //	else if ( dumpinfo.find("TRTSeededTrackFinder")!=std::string::npos) trackAuthor = 1;
      //	else                                                                trackAuthor = 0;
      //   }
      
#if 0
      std::cout << "\t\t\tSUTT TP track" 
		<< "\teta=" << eta  // << " +- " << (*trackitr)->params()->deta()
		<< "\tphi=" << phi  // << " +- " << (*trackitr)->params()->dphi()
		<< "\tz0="  << z0 
		<< "\tpT="  << pT // << "\t( " << 1/qoverp << ")"
		<< "\td0="  << d0
		<< "\tNsi=" << nSiHits 
	//		<< "\tNtrt=" << nTrtHits 
	//		<< "\tNstr=" << nStrawHits
		<< "\tfitter=" << fitter
		<< "\tauthor=" << trackAuthor
		<< "\tVTX x " << track->vx() << "\ty " << track->vy() << "\tz " << track->vz() 
		<< std::endl;
#endif	

      // Create and save Track
      
      TIDA::Track* t = new TIDA::Track( eta,   phi,  z0,  d0,  pT, chi2, dof,
		 		        deta,  dphi, dz0, dd0, dpT,
				        nBlayerHits, nPixelHits, nSctHits, nSiHits,
				        nStrawHits,  nTrtHits,   bitmap, 0,
				        trackAuthor,  false, -1, -1,  
				        expectBL, id) ;  

      //      std::cout << "SUTT TP track " << *t << "\t0x" << std::hex << bitmap << std::dec << std::endl; 
      
      if ( !addTrack( t ) ){
	delete t;
	return false;
      }
      return true;
      
    }
    return false;
     

}


void TrigTrackSelector::selectTracks( const xAOD::TrackParticleContainer* tracks, void* ) {
    //    std::cout << "\t\t\tSUTT \tTrackContainer->size() = " << trigtracks->size() << std::endl;
    xAOD::TrackParticleContainer::const_iterator  trackitr = tracks->begin();
    xAOD::TrackParticleContainer::const_iterator  trackend = tracks->end();
    while ( trackitr!=trackend ) { 
      selectTrack( *trackitr );
      ++trackitr;
    } // loop over tracks     
}



void TrigTrackSelector::selectTracks( xAOD::TrackParticleContainer::const_iterator trackitr, 
				      xAOD::TrackParticleContainer::const_iterator trackend, void* ) {
    /// will need this printout during debugging, so leave commented 
    /// until all the feature access has been properly debugged
    //    std::cout << "\t\t\tSUTT \tTrackContainer->size() = " << trigtracks->size() << std::endl;
    while ( trackitr!=trackend ) { 
      selectTrack( *trackitr );
      ++trackitr;
    } // loop over tracks     
}


void TrigTrackSelector::selectTracks( const TrackParticleLinks_t& tracks ) {
  for( const auto& track : tracks ) selectTrack( *track );
}





void TrigTrackSelector::correctToBeamline( double& z0,    double& dz0, 
					   double& d0,    double& dd0, 
					   double  theta, double  phi ) {
    
    /// make sure that users have set the beamline parameters
    
    //   if ( m_first ) { 
    //     if ( m_xBeam==0 && m_yBeam==0 ) { 
    //	    std::cerr << "TrigTrackSelector::correctToBeamline() WARNING -- Beamline set to (0,0) -- WARNING" << std::endl;  
    //     }
    //     else { 
    //	   std::cout << "TrigTrackSelector::correctToBeamline() Beamline set to " << m_xBeam << " " << m_yBeam << std::endl;  
    //     }
    //     m_first = false;
    //   }
    
    
    //      double theta = 2*std::atan( exp( (-1)*eta ) );
    double z0t = z0 + ((std::cos(phi)*m_xBeam + std::sin(phi)*m_yBeam)/std::tan(theta));    
    double a0t = d0 +   std::sin(phi)*m_xBeam - std::cos(phi)*m_yBeam; 
    
    /// error estimates
    double dz0t = dz0 + ((std::cos(phi)*m_xBeam + std::sin(phi)*m_yBeam)/std::tan(theta));
    double da0t = dd0 +   std::sin(phi)*m_xBeam - std::cos(phi)*m_yBeam;
    
    z0  = z0t;
    d0  = a0t;
    
    dz0 = dz0t;
    dd0 = da0t;
}

