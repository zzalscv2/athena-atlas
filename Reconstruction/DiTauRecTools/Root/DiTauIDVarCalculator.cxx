/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "DiTauRecTools/DiTauIDVarCalculator.h"

// Core include(s):
#include "AthLinks/ElementLink.h"

// EDM include(s):
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/TrackParticle.h"

#include "xAODJet/Jet.h"
#include "xAODJet/JetContainer.h"
#include "xAODJet/JetConstituentVector.h"

#include "xAODTau/DiTauJet.h"
#include "xAODTau/DiTauJetContainer.h"
#include "xAODTau/DiTauJetAuxContainer.h"
#include "xAODTau/TauJetContainer.h"

#include "xAODEventInfo/EventInfo.h"

// fastjet
#include "fastjet/PseudoJet.hh"
#include "fastjet/JetDefinition.hh"
#include "fastjet/AreaDefinition.hh"
#include "fastjet/ClusterSequenceArea.hh"
#include "fastjet/tools/Filter.hh"
#include "fastjet/tools/MassDropTagger.hh"

using namespace DiTauRecTools;
using namespace fastjet;

typedef std::vector< ElementLink< xAOD::TrackParticleContainer > >  TrackParticleLinks_t;
typedef ElementLink< xAOD::JetContainer > JetLink_t;

//=================================PUBLIC-PART==================================
//______________________________________________________________________________
DiTauIDVarCalculator::DiTauIDVarCalculator( const std::string& name )
  : AsgTool(name)
  , m_sDiTauContainerName("DiTauJets")
  , m_eDecayChannel(DecayChannel::Default)
{
  declareProperty( "DefaultValue", m_dDefault = 0);
  declareProperty( "DiTauContainerName", m_sDiTauContainerName = "DiTauJets");
  declareProperty( "DiTauDecayChannel", m_sDecayChannel = "HadHad");
}

//______________________________________________________________________________
DiTauIDVarCalculator::~DiTauIDVarCalculator( )
{
}

//______________________________________________________________________________
StatusCode DiTauIDVarCalculator::initialize()
{
  ATH_MSG_INFO( "Initializing DiTauIDVarCalculator" );
  m_DiTauContainerNameAux = m_sDiTauContainerName + "Aux.";
  if(m_sDecayChannel == "HadHad")
    m_eDecayChannel = DecayChannel::HadHad;
  if(m_eDecayChannel == DecayChannel::Default){
    ATH_MSG_ERROR( "No Valid DecayChannel initialized. Valid options are: HadHad" );
    return StatusCode::FAILURE;
  }
  
  return StatusCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
//                              Wrapper functions                             //
////////////////////////////////////////////////////////////////////////////////

StatusCode DiTauIDVarCalculator::calculateIDVariables(const xAOD::DiTauJet& xDiTau){
  return execute(xDiTau);
}

StatusCode DiTauIDVarCalculator::execute(const xAOD::DiTauJet& xDiTau)
{
  switch(m_eDecayChannel) {
  case(DecayChannel::HadHad):
    return calculateHadHadIDVariables(xDiTau);
    break;
  default:
    return StatusCode::FAILURE;
  }
  return StatusCode::FAILURE;
}

StatusCode DiTauIDVarCalculator::calculateHadHadIDVariables(const xAOD::DiTauJet& xDiTau)
{
  ATH_MSG_DEBUG("Calculate DiTau ID variables");
  
  xDiTau.auxdecor< int >("n_subjets") = n_subjets(xDiTau);
  ATH_CHECK( decorNtracks(xDiTau) );
  xDiTau.auxdecor< float >( "ditau_pt") = ditau_pt(xDiTau);
  xDiTau.auxdecor< float >( "f_core_lead" ) = f_core(xDiTau, 0);
  xDiTau.auxdecor< float >( "f_core_subl" ) = f_core(xDiTau, 1);
  xDiTau.auxdecor< float >( "f_subjet_lead" ) = f_subjet(xDiTau, 0);
  xDiTau.auxdecor< float >( "f_subjet_subl" ) = f_subjet(xDiTau, 1);
  xDiTau.auxdecor< float >( "f_subjets") = f_subjets(xDiTau);
  xDiTau.auxdecor< float >( "f_track_lead") = f_track(xDiTau, 0);
  xDiTau.auxdecor< float >( "f_track_subl") = f_track(xDiTau, 1);
  xDiTau.auxdecor< float >( "R_max_lead") = R_max(xDiTau, 0);
  xDiTau.auxdecor< float >( "R_max_subl") = R_max(xDiTau, 1);
  xDiTau.auxdecor< int >( "n_track" ) = n_track(xDiTau);
  xDiTau.auxdecor< int >( "n_tracks_lead" ) = n_tracks(xDiTau, 0);
  xDiTau.auxdecor< int >( "n_tracks_subl" ) = n_tracks(xDiTau, 1);
  xDiTau.auxdecor< int >( "n_isotrack" ) = n_isotrack(xDiTau);
  xDiTau.auxdecor< float >( "R_track" ) = R_track(xDiTau);
  xDiTau.auxdecor< float >( "R_track_core" ) = R_track_core(xDiTau);
  xDiTau.auxdecor< float >( "R_track_all" ) = R_track_all(xDiTau);
  xDiTau.auxdecor< float >( "R_isotrack" ) = R_isotrack(xDiTau);
  xDiTau.auxdecor< float >( "R_core_lead" ) = R_core(xDiTau, 0);
  xDiTau.auxdecor< float >( "R_core_subl" ) = R_core(xDiTau, 1);
  xDiTau.auxdecor< float >( "R_tracks_lead" ) = R_tracks(xDiTau, 0);
  xDiTau.auxdecor< float >( "R_tracks_subl" ) = R_tracks(xDiTau, 1);
  xDiTau.auxdecor< float >( "m_track" ) = mass_track(xDiTau);
  xDiTau.auxdecor< float >( "m_track_core" ) = mass_track_core(xDiTau);
  xDiTau.auxdecor< float >( "m_core_lead" ) = mass_core(xDiTau, 0);
  xDiTau.auxdecor< float >( "m_core_subl" ) = mass_core(xDiTau, 1);
  xDiTau.auxdecor< float >( "m_track_all" ) = mass_track_all(xDiTau);
  xDiTau.auxdecor< float >( "m_tracks_lead" ) = mass_tracks(xDiTau, 0);
  xDiTau.auxdecor< float >( "m_tracks_subl" ) = mass_tracks(xDiTau, 1);
  xDiTau.auxdecor< float >( "E_frac_subl" ) = E_frac(xDiTau,1);
  xDiTau.auxdecor< float >( "E_frac_subsubl") = E_frac(xDiTau, 2);
  xDiTau.auxdecor< float >( "R_subjets_subl") = R_subjets(xDiTau, 1);
  xDiTau.auxdecor< float >( "R_subjets_subsubl") = R_subjets(xDiTau, 2);
  xDiTau.auxdecor< float >( "d0_leadtrack_lead") = d0_leadtrack(xDiTau, 0);
  xDiTau.auxdecor< float >( "d0_leadtrack_subl") = d0_leadtrack(xDiTau, 1);
  xDiTau.auxdecor< float >( "f_isotracks" ) = f_isotracks(xDiTau);

  return StatusCode::SUCCESS;
}

std::string DiTauIDVarCalculator::getDecayMode(){
  return m_sDecayChannel;
}

//=================================PRIVATE-PART=================================
//______________________________________________________________________________

float DiTauIDVarCalculator::n_subjets(const xAOD::DiTauJet& xDiTau) const
{
  int nSubjet = 0;
  while (xDiTau.subjetPt(nSubjet) > 0. )
  {
    nSubjet++;
  }

  return nSubjet;
}


float DiTauIDVarCalculator::ditau_pt(const xAOD::DiTauJet& xDiTau) const
{
  if (xDiTau.auxdata<int>("n_subjets") < 2 ) {
    return m_dDefault;
  }

  return xDiTau.subjetPt(0)+xDiTau.subjetPt(1);
}


//______________________________________________________________________________;
float DiTauIDVarCalculator::f_core(const xAOD::DiTauJet& xDiTau, int iSubjet) const 
{
  if (iSubjet < 0 || iSubjet >= xDiTau.auxdata<int>("n_subjets")) {
    return m_dDefault;
  }

  return xDiTau.fCore(iSubjet);
}


//______________________________________________________________________________;
float DiTauIDVarCalculator::f_subjet(const xAOD::DiTauJet& xDiTau, int iSubjet) const
{
  if (iSubjet < 0 || iSubjet >= xDiTau.auxdata<int>("n_subjets")) {
    return m_dDefault;
  }

  return xDiTau.subjetPt(iSubjet) / xDiTau.pt();
}


//______________________________________________________________________________;
float DiTauIDVarCalculator::f_subjets(const xAOD::DiTauJet& xDiTau) const
{
  if (xDiTau.auxdata<int>("n_subjets") < 2 ) {
    return m_dDefault;
  }

  return (xDiTau.subjetPt(0) + xDiTau.subjetPt(1))/ xDiTau.pt();
}


//______________________________________________________________________________;
float DiTauIDVarCalculator::f_track(const xAOD::DiTauJet& xDiTau, int iSubjet) const
{
  if (iSubjet < 0 || iSubjet >= xDiTau.auxdata<int>("n_subjets")) {
    return m_dDefault;
  }

  if (!xDiTau.isAvailable< TrackParticleLinks_t >("trackLinks") )
  {
    ATH_MSG_WARNING("Link not available");
  } 

  TrackParticleLinks_t xTracks = xDiTau.trackLinks();

  TLorentzVector tlvSubjet;
  tlvSubjet.SetPtEtaPhiE( xDiTau.subjetPt(iSubjet),
                          xDiTau.subjetEta(iSubjet),
                          xDiTau.subjetPhi(iSubjet),
                          xDiTau.subjetE(iSubjet) );
      
  TLorentzVector tlvTrack;
  TLorentzVector tlvLeadTrack;
  tlvLeadTrack.SetPtEtaPhiE( 0,0,0, 0);

  for (const auto &xTrack: xTracks) 
  { 
    if (!xTrack) 
    {
      ATH_MSG_ERROR("Could not read Track");
      continue;
    }
    tlvTrack.SetPtEtaPhiE( (*xTrack)->pt(),
                           (*xTrack)->eta(),
                           (*xTrack)->phi(),
                           (*xTrack)->e() );

    if ( tlvSubjet.DeltaR(tlvTrack) < 0.2 )
    {
      if (tlvLeadTrack.Pt() < tlvTrack.Pt()) 
      {
        tlvLeadTrack = tlvTrack;
      }
    }
  }

  return tlvLeadTrack.Pt() / tlvSubjet.Pt();
}


//______________________________________________________________________________;
float DiTauIDVarCalculator::R_max(const xAOD::DiTauJet& xDiTau, int iSubjet) const
{ 
  if (iSubjet < 0 || iSubjet >= xDiTau.auxdata<int>("n_subjets")) {
    return m_dDefault;
  }

  TrackParticleLinks_t xTracks = xDiTau.trackLinks();

  TLorentzVector tlvSubjet;
  tlvSubjet.SetPtEtaPhiE( xDiTau.subjetPt(iSubjet),
                          xDiTau.subjetEta(iSubjet),
                          xDiTau.subjetPhi(iSubjet),
                          xDiTau.subjetE(iSubjet) );
      
  TLorentzVector tlvTrack;
  TLorentzVector tlvRmaxTrack;
  double Rmax = 0;
  for (const auto &xTrack: xTracks) 
  {
    tlvTrack.SetPtEtaPhiE( (*xTrack)->pt(),
                           (*xTrack)->eta(),
                           (*xTrack)->phi(),
                           (*xTrack)->e() );

    if ( tlvSubjet.DeltaR(tlvTrack) < xDiTau.auxdata< float >("R_subjet") )
    {
      if (tlvTrack.DeltaR(tlvSubjet) > Rmax) 
      {
        Rmax = tlvTrack.DeltaR(tlvSubjet);
      }
    }
  }

  return Rmax;
}


//______________________________________________________________________________;
int DiTauIDVarCalculator::n_track(const xAOD::DiTauJet& xDiTau) const
{ 
  return xDiTau.nTracks();
}

//______________________________________________________________________________;
int DiTauIDVarCalculator::n_tracks(const xAOD::DiTauJet& xDiTau, int iSubjet) const
{
  if (iSubjet < 0 || iSubjet >= xDiTau.auxdata<int>("n_subjets")) {
    return m_dDefault;
  }

  if (!xDiTau.isAvailable<std::vector<int>>("n_tracks"))
  {
    ATH_MSG_DEBUG("n_tracks decoration not available. Try with track links.");

    if (!xDiTau.isAvailable< TrackParticleLinks_t >("trackLinks") )
    {
      ATH_MSG_WARNING("Track links not available. Return 0.");
      return (int)m_dDefault;
    } 

    TrackParticleLinks_t xTracks = xDiTau.trackLinks();

    TLorentzVector tlvSubjet;
    tlvSubjet.SetPtEtaPhiE( xDiTau.subjetPt(iSubjet),
                            xDiTau.subjetEta(iSubjet),
                            xDiTau.subjetPhi(iSubjet),
                            xDiTau.subjetE(iSubjet) );
        
    TLorentzVector tlvTrack;
    int nTracks = 0;
    for (const auto &xTrack: xTracks) 
    { 
      tlvTrack.SetPtEtaPhiE( (*xTrack)->pt(),
                             (*xTrack)->eta(),
                             (*xTrack)->phi(),
                             (*xTrack)->e() );
      if ( tlvSubjet.DeltaR(tlvTrack) < 0.2 ) nTracks++;
    }

    return nTracks;
  }

  return xDiTau.auxdata<std::vector<int>>("n_tracks").at(iSubjet);

}

//______________________________________________________________________________;
int DiTauIDVarCalculator::n_isotrack(const xAOD::DiTauJet& xDiTau) const
{ 
  return xDiTau.nIsoTracks();
}

//______________________________________________________________________________;
float DiTauIDVarCalculator::R_tracks(const xAOD::DiTauJet& xDiTau, int iSubjet) const
{
  double R_sum = 0;
  double pt = 0;

  if (!xDiTau.isAvailable< TrackParticleLinks_t >("trackLinks") )
  {
    ATH_MSG_WARNING("Link not available");
  }

  if (iSubjet < 0 || iSubjet >= xDiTau.auxdata<int>("n_subjets")) {
    return m_dDefault;
  }
 
  TrackParticleLinks_t xTracks = xDiTau.trackLinks();

  TLorentzVector tlvSubjet;
  tlvSubjet.SetPtEtaPhiE( xDiTau.subjetPt(iSubjet),
                          xDiTau.subjetEta(iSubjet),
                          xDiTau.subjetPhi(iSubjet),
                          xDiTau.subjetE(iSubjet) );
    
  TLorentzVector tlvTrack;

  for (auto xTrack: xTracks) 
  { 
    tlvTrack.SetPtEtaPhiE( (*xTrack)->pt(),
                           (*xTrack)->eta(),
                           (*xTrack)->phi(),
                           (*xTrack)->e() );

    if ( tlvSubjet.DeltaR(tlvTrack) < 0.2 )
    {
      //ATH_MSG_DEBUG("smaller");
      R_sum += tlvSubjet.DeltaR(tlvTrack)*tlvTrack.Pt();
      pt += tlvTrack.Pt();
    }
  }
  
  if (pt == 0)
  {
    return m_dDefault;
  }

  return R_sum / pt;
}

//______________________________________________________________________________;
float DiTauIDVarCalculator::R_core(const xAOD::DiTauJet& xDiTau, int iSubjet) const
{
  double R_sum = 0;
  double pt = 0;

  if (!xDiTau.isAvailable< TrackParticleLinks_t >("trackLinks") )
  {
    ATH_MSG_WARNING("Link not available");
  }
  if (iSubjet < 0 || iSubjet >= xDiTau.auxdata<int>("n_subjets")) {
    return m_dDefault;
  }
 
  TrackParticleLinks_t xTracks = xDiTau.trackLinks();

  TLorentzVector tlvSubjet;
  tlvSubjet.SetPtEtaPhiE( xDiTau.subjetPt(iSubjet),
                          xDiTau.subjetEta(iSubjet),
                          xDiTau.subjetPhi(iSubjet),
                          xDiTau.subjetE(iSubjet) );
    
  TLorentzVector tlvTrack;

  for (auto xTrack: xTracks) 
  { 
    tlvTrack.SetPtEtaPhiE( (*xTrack)->pt(),
                           (*xTrack)->eta(),
                           (*xTrack)->phi(),
                           (*xTrack)->e() );

    if ( tlvSubjet.DeltaR(tlvTrack) < xDiTau.auxdata< float >( "R_core" ) )
    {
      R_sum += tlvSubjet.DeltaR(tlvTrack)*tlvTrack.Pt();
      pt += tlvTrack.Pt();
    }
  }
  
  if (pt == 0)
  {
    return m_dDefault;
  }

  return R_sum / pt;
}

//______________________________________________________________________________;
float DiTauIDVarCalculator::R_track_core(const xAOD::DiTauJet& xDiTau) const
{
  double R_sum = 0;
  double pt = 0;

  if (!xDiTau.isAvailable< TrackParticleLinks_t >("trackLinks") )
  {
    ATH_MSG_WARNING("Link not available");
  }
  if (xDiTau.auxdata<int>("n_subjets") < 2) {
    return m_dDefault;
  }
 

  for (int i = 0; i<=1; i++)
  {
  
    TrackParticleLinks_t xTracks = xDiTau.trackLinks();

    TLorentzVector tlvSubjet;
    tlvSubjet.SetPtEtaPhiE( xDiTau.subjetPt(i),
                            xDiTau.subjetEta(i),
                            xDiTau.subjetPhi(i),
                            xDiTau.subjetE(i) );
      
    TLorentzVector tlvTrack;

    for (auto xTrack: xTracks) 
    { 
      tlvTrack.SetPtEtaPhiE( (*xTrack)->pt(),
                             (*xTrack)->eta(),
                             (*xTrack)->phi(),
                             (*xTrack)->e() );
      if ( tlvSubjet.DeltaR(tlvTrack) < xDiTau.auxdata< float >("R_core") )
      {
        //ATH_MSG_DEBUG("smaller");
        R_sum += tlvSubjet.DeltaR(tlvTrack)*tlvTrack.Pt();
        pt += tlvTrack.Pt();
      }
    }
  }
  if (pt == 0)
  {
    return m_dDefault;
  }

  return R_sum / pt;
}

//______________________________________________________________________________;
float DiTauIDVarCalculator::R_track(const xAOD::DiTauJet& xDiTau) const
{
  double R_sum = 0;
  double pt = 0;

  if (!xDiTau.isAvailable< TrackParticleLinks_t >("trackLinks") )
  {
    ATH_MSG_WARNING("Link not available");
  }
  if (xDiTau.auxdata<int>("n_subjets") < 2) {
    return m_dDefault;
  }
 
  for (int i = 0; i<=1; i++)
  {
  
    TrackParticleLinks_t xTracks = xDiTau.trackLinks();

    TLorentzVector tlvSubjet;
    tlvSubjet.SetPtEtaPhiE( xDiTau.subjetPt(i),
                            xDiTau.subjetEta(i),
                            xDiTau.subjetPhi(i),
                            xDiTau.subjetE(i) );
      
    TLorentzVector tlvTrack;

    for (auto xTrack: xTracks) 
    { 
      tlvTrack.SetPtEtaPhiE( (*xTrack)->pt(),
                             (*xTrack)->eta(),
                             (*xTrack)->phi(),
                             (*xTrack)->e() );

      if (tlvSubjet.DeltaR(tlvTrack) < 0.2)
      {
        R_sum += tlvSubjet.DeltaR(tlvTrack)*tlvTrack.Pt();
        pt += tlvTrack.Pt();
      }
    }
  }
  if (pt == 0)
  {
    return m_dDefault;
  }

  return R_sum / pt;
}
//______________________________________________________________________________;
float DiTauIDVarCalculator::R_track_all(const xAOD::DiTauJet& xDiTau) const
{
  double R_sum = 0;
  double pt = 0;

  if (!xDiTau.isAvailable< TrackParticleLinks_t >("trackLinks") )
  {
    ATH_MSG_WARNING("Link not available");
  }

  for (int i = 0; i<xDiTau.auxdata<int>("n_subjets"); i++)
  {
  
    TrackParticleLinks_t xTracks = xDiTau.trackLinks();

    TLorentzVector tlvSubjet;
    tlvSubjet.SetPtEtaPhiE( xDiTau.subjetPt(i),
                            xDiTau.subjetEta(i),
                            xDiTau.subjetPhi(i),
                            xDiTau.subjetE(i) );
      
    TLorentzVector tlvTrack;

    for (auto xTrack: xTracks) 
    { 
      tlvTrack.SetPtEtaPhiE( (*xTrack)->pt(),
                             (*xTrack)->eta(),
                             (*xTrack)->phi(),
                             (*xTrack)->e() );

      if (tlvSubjet.DeltaR(tlvTrack) <= 0.2)
      {
        R_sum += tlvSubjet.DeltaR(tlvTrack)*tlvTrack.Pt();
        pt += tlvTrack.Pt();
      }
    }
  }

  if (pt == 0)
  {
    return m_dDefault;
  }

  return R_sum / pt;
}

//______________________________________________________________________________;
float DiTauIDVarCalculator::R_isotrack(const xAOD::DiTauJet& xDiTau) const
{
  double R_sum = 0;
  double pt = 0;

  if (!xDiTau.isAvailable< TrackParticleLinks_t >("isoTrackLinks") )
  {
    ATH_MSG_WARNING("Link not available");
  }
  
  if (xDiTau.auxdata<int>("n_subjets") < 2) {
    return m_dDefault;
  }
 
  for (int i = 0; i<=1; i++)
  {
  
    TrackParticleLinks_t xIsoTracks = xDiTau.isoTrackLinks();

    TLorentzVector tlvSubjet;
    tlvSubjet.SetPtEtaPhiE( xDiTau.subjetPt(i),
                            xDiTau.subjetEta(i),
                            xDiTau.subjetPhi(i),
                            xDiTau.subjetE(i) );
      
    TLorentzVector tlvIsoTrack;

    for (auto xIsoTrack: xIsoTracks) 
    { 
      tlvIsoTrack.SetPtEtaPhiE( (*xIsoTrack)->pt(),
                                (*xIsoTrack)->eta(),
                                (*xIsoTrack)->phi(),
                                (*xIsoTrack)->e() );

      if (tlvSubjet.DeltaR(tlvIsoTrack) < 0.4)
      {
        R_sum += tlvSubjet.DeltaR(tlvIsoTrack)*tlvIsoTrack.Pt();
        pt += tlvIsoTrack.Pt();
      }
    }
  }

  if (pt == 0)
  {
    return m_dDefault;
  }

  return R_sum / pt;
}

//______________________________________________________________________________;
float DiTauIDVarCalculator::mass_track_core(const xAOD::DiTauJet& xDiTau) const
{

  if (!xDiTau.isAvailable< TrackParticleLinks_t >("trackLinks") )
  {
    ATH_MSG_WARNING("Link not available");
  } 
    
  if (xDiTau.auxdata<int>("n_subjets") < 2) {
    return m_dDefault;
  }

  TLorentzVector tlvallTracks;

  for (int i = 0; i<=1; i++)
  {

    TrackParticleLinks_t xTracks = xDiTau.trackLinks();

    TLorentzVector tlvSubjet;
    tlvSubjet.SetPtEtaPhiE( xDiTau.subjetPt(i),
                            xDiTau.subjetEta(i),
                            xDiTau.subjetPhi(i),
                            xDiTau.subjetE(i) );
    
    TLorentzVector tlvTrack;

    for (auto xTrack: xTracks) 
    { 
      tlvTrack.SetPtEtaPhiE( (*xTrack)->pt(),
                             (*xTrack)->eta(),
                             (*xTrack)->phi(),
                             (*xTrack)->e() );
      if ( tlvSubjet.DeltaR(tlvTrack) < xDiTau.auxdata< float >("R_core") )
      {
        //ATH_MSG_DEBUG("smaller");
        tlvallTracks += tlvTrack;
      }
    }
  }
  if (tlvallTracks.M() < 0)
  {
    return m_dDefault;
  }

  return tlvallTracks.M();
}

//______________________________________________________________________________;
float DiTauIDVarCalculator::mass_core(const xAOD::DiTauJet& xDiTau, int iSubjet) const
{

  if (!xDiTau.isAvailable< TrackParticleLinks_t >("trackLinks") )
  {
    ATH_MSG_WARNING("Link not available");
  } 
    
  if ( iSubjet < 0 || iSubjet >= xDiTau.auxdata<int>("n_subjets")) {
    return m_dDefault;
  }

  TLorentzVector tlvallTracks;


  TrackParticleLinks_t xTracks = xDiTau.trackLinks();

  TLorentzVector tlvSubjet;
  tlvSubjet.SetPtEtaPhiE( xDiTau.subjetPt(iSubjet),
                          xDiTau.subjetEta(iSubjet),
                          xDiTau.subjetPhi(iSubjet),
                          xDiTau.subjetE(iSubjet) );
  
  TLorentzVector tlvTrack;

  for (auto xTrack: xTracks) 
  { 
    tlvTrack.SetPtEtaPhiE( (*xTrack)->pt(),
                           (*xTrack)->eta(),
                           (*xTrack)->phi(),
                           (*xTrack)->e() );
    if ( tlvSubjet.DeltaR(tlvTrack) < xDiTau.auxdata< float >("R_core") )
    {
      //ATH_MSG_DEBUG("smaller");
      tlvallTracks += tlvTrack;
    }
  }
  
  if (tlvallTracks.M() < 0)
  {
    return m_dDefault;
  }

  return tlvallTracks.M();
}

//______________________________________________________________________________;
float DiTauIDVarCalculator::mass_tracks(const xAOD::DiTauJet& xDiTau, int iSubjet) const
{

  if (!xDiTau.isAvailable< TrackParticleLinks_t >("trackLinks") )
  {
    ATH_MSG_WARNING("Link not available");
  } 
    
  if ( iSubjet < 0 || iSubjet >= xDiTau.auxdata<int>("n_subjets")) {
    return m_dDefault;
  }

  TLorentzVector tlvallTracks;

  TrackParticleLinks_t xTracks = xDiTau.trackLinks();

  TLorentzVector tlvSubjet;
  tlvSubjet.SetPtEtaPhiE( xDiTau.subjetPt(iSubjet),
                          xDiTau.subjetEta(iSubjet),
                          xDiTau.subjetPhi(iSubjet),
                          xDiTau.subjetE(iSubjet) );
  
  TLorentzVector tlvTrack;

  for (auto xTrack: xTracks) 
  { 
    tlvTrack.SetPtEtaPhiE( (*xTrack)->pt(),
                           (*xTrack)->eta(),
                           (*xTrack)->phi(),
                           (*xTrack)->e() );
    if ( tlvSubjet.DeltaR(tlvTrack) < 0.2 )
    {
      tlvallTracks += tlvTrack;
    }
  }
  
  if (tlvallTracks.M() < 0)
  {
    return m_dDefault;
  }

  return tlvallTracks.M();
}
//______________________________________________________________________________;
float DiTauIDVarCalculator::mass_track(const xAOD::DiTauJet& xDiTau) const
{

  if (!xDiTau.isAvailable< TrackParticleLinks_t >("trackLinks") )
  {
    ATH_MSG_WARNING("Link not available");
  } 

  TLorentzVector tlvallTracks;

  TrackParticleLinks_t xTracks = xDiTau.trackLinks();
    
  TLorentzVector tlvTrack;

  for (auto xTrack: xTracks) 
  { 
    tlvTrack.SetPtEtaPhiE( (*xTrack)->pt(),
                           (*xTrack)->eta(),
                           (*xTrack)->phi(),
                           (*xTrack)->e() );

    tlvallTracks += tlvTrack;
  }
  
  if (tlvallTracks.M() < 0)
  {
    return m_dDefault;
  }
  return tlvallTracks.M();
}
//______________________________________________________________________________;
float DiTauIDVarCalculator::mass_track_all(const xAOD::DiTauJet& xDiTau) const
{

  if (!xDiTau.isAvailable< TrackParticleLinks_t >("trackLinks") )
  {
    ATH_MSG_WARNING("Link not available");
  } 

  TLorentzVector tlvallTracks;

  TrackParticleLinks_t xTracks = xDiTau.trackLinks();
    
  TLorentzVector tlvTrack;

  for (auto xTrack: xTracks) 
  { 
    tlvTrack.SetPtEtaPhiE( (*xTrack)->pt(),
                           (*xTrack)->eta(),
                           (*xTrack)->phi(),
                           (*xTrack)->e() );

    tlvallTracks += tlvTrack;
  }

  
  TrackParticleLinks_t xIsoTracks = xDiTau.isoTrackLinks();

  TLorentzVector tlvIsoTrack;

  for (auto xIsoTrack: xIsoTracks) 
  { 
    tlvIsoTrack.SetPtEtaPhiE( (*xIsoTrack)->pt(),
                             (*xIsoTrack)->eta(),
                             (*xIsoTrack)->phi(),
                             (*xIsoTrack)->e() );

    tlvallTracks += tlvIsoTrack;
  }

  if (tlvallTracks.M() < 0)
  {
    return m_dDefault;
  }

  return tlvallTracks.M();
}

//______________________________________________________________________________;
float DiTauIDVarCalculator::E_frac(const xAOD::DiTauJet& xDiTau, int iSubjet) const
{ 
  if ( iSubjet < 0 || iSubjet >= xDiTau.auxdata<int>("n_subjets")) {
    return m_dDefault;
  }

  return xDiTau.subjetE(iSubjet) / xDiTau.subjetE(0);
}

//______________________________________________________________________________;
float DiTauIDVarCalculator::R_subjets(const xAOD::DiTauJet& xDiTau, int iSubjet) const
{

  if ( iSubjet < 0 || iSubjet >= xDiTau.auxdata<int>("n_subjets")) {
    return m_dDefault;
  }
  
  if (!xDiTau.isAvailable< TrackParticleLinks_t >("trackLinks") )
  {
    ATH_MSG_WARNING("Track links not available");
  }
  
  TLorentzVector tlvLeadSubjet;
  tlvLeadSubjet.SetPtEtaPhiE( xDiTau.subjetPt(0),
                              xDiTau.subjetEta(0),
                              xDiTau.subjetPhi(0),
                              xDiTau.subjetE(0) );

  TLorentzVector tlvSubjet;
  tlvSubjet.SetPtEtaPhiE( xDiTau.subjetPt(iSubjet),
                          xDiTau.subjetEta(iSubjet),
                          xDiTau.subjetPhi(iSubjet),
                          xDiTau.subjetE(iSubjet) );
  return tlvLeadSubjet.DeltaR(tlvSubjet);
}

//______________________________________________________________________________;
float DiTauIDVarCalculator::d0_leadtrack(const xAOD::DiTauJet& xDiTau, int iSubjet) const
{
  double pt_leadtrk = 0;
  double d0 = m_dDefault;
  if (!xDiTau.isAvailable< TrackParticleLinks_t >("trackLinks") )
  {
    ATH_MSG_WARNING("Track links not available");
  } 

  if ( iSubjet < 0 || iSubjet >= xDiTau.auxdata<int>("n_subjets")) {
    return m_dDefault;
  }

  TLorentzVector tlvSubjet;
  tlvSubjet.SetPtEtaPhiE( xDiTau.subjetPt(iSubjet),
                          xDiTau.subjetEta(iSubjet),
                          xDiTau.subjetPhi(iSubjet),
                          xDiTau.subjetE(iSubjet) );
			  
  TrackParticleLinks_t xTracks = xDiTau.trackLinks();
    
  TLorentzVector tlvTrack;

  for (auto &xTrack: xTracks) 
  { 
    tlvTrack.SetPtEtaPhiE( (*xTrack)->pt(),
                           (*xTrack)->eta(),
                           (*xTrack)->phi(),
                           (*xTrack)->e() );

    if (tlvTrack.DeltaR(tlvSubjet) < xDiTau.auxdata< float >("R_core")) 
    {
      if (tlvTrack.Pt() > pt_leadtrk)
      {
        pt_leadtrk = tlvTrack.Pt();
        d0 = (*xTrack)->d0();
      }
    }
  }
  return d0;
}

//______________________________________________________________________________;
float DiTauIDVarCalculator::f_isotracks(const xAOD::DiTauJet& xDiTau) const
{
  double iso_pt = 0;
  if (!xDiTau.isAvailable< TrackParticleLinks_t >("isoTrackLinks") )
  {
    ATH_MSG_WARNING("Track links not available");
  }
  
  TrackParticleLinks_t xIsoTracks = xDiTau.isoTrackLinks();

  TLorentzVector tlvIsoTrack;

  for (auto xIsoTrack: xIsoTracks) 
  { 
    tlvIsoTrack.SetPtEtaPhiE( (*xIsoTrack)->pt(),
                              (*xIsoTrack)->eta(),
                              (*xIsoTrack)->phi(),
                              (*xIsoTrack)->e() );

    iso_pt += tlvIsoTrack.Pt();
  }

  return iso_pt / xDiTau.pt();
}

//______________________________________________________________________________;
StatusCode DiTauIDVarCalculator::decorNtracks (const xAOD::DiTauJet& xDiTau)
{
  if (!xDiTau.isAvailable< TrackParticleLinks_t >("trackLinks") )
  {
    Warning("decorNtracks()", "Track links not available.");
    return StatusCode::FAILURE;
  } 

  int nSubjets = xDiTau.auxdata<int>("n_subjets");

  float Rsubjet = xDiTau.auxdata<float>("R_subjet");
  std::vector<int> nTracks(nSubjets, 0);

  TrackParticleLinks_t xTracks = xDiTau.trackLinks();
  for (const auto &xTrack: xTracks)
  {
    double dRmin = 1111;
    double itrmin = -1;

    for (int i=0; i<nSubjets; ++i)
    {
      TLorentzVector tlvSubjet = TLorentzVector();
      tlvSubjet.SetPtEtaPhiE(xDiTau.subjetPt(i),
                             xDiTau.subjetEta(i),
                             xDiTau.subjetPhi(i),
                             xDiTau.subjetE(i));
      double dR = tlvSubjet.DeltaR((*xTrack)->p4());


      if ((dR < Rsubjet) && (dR < dRmin))
      {
        dRmin = dR;
        itrmin = i;
      }
    } // loop over subjets
    if (itrmin > -1) nTracks[itrmin]++;
  } // loop over tracks

  xDiTau.auxdecor< std::vector<int> >("n_tracks") = nTracks;

  return StatusCode::SUCCESS;
}
