/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// DiTauProngDecorator.cxx
// Author: Nadav Tamir (nadavtamir@mail.tau.ac.il)
///////////////////////////////////////////////////////////////////

#include "DerivationFrameworkTau/DiTauProngDecorator.h"

#include "xAODTracking/Vertex.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/TrackParticle.h"

#include "xAODTau/DiTauJet.h"
#include "xAODTau/DiTauJetContainer.h"
#include "xAODTau/DiTauJetAuxContainer.h"
#include "xAODTau/TauJetContainer.h"

#include "AthLinks/ElementLink.h"

typedef std::vector< ElementLink< xAOD::TrackParticleContainer > >  TrackParticleLinks_t;

namespace DerivationFramework {

  DiTauProngDecorator::DiTauProngDecorator(const std::string& t, const std::string& n, const IInterface* p) : 
    AthAlgTool(t,n,p),
    m_ditauContainerName("DiTauJets")
  {
    declareInterface<DerivationFramework::IAugmentationTool>(this);
    declareProperty("DiTauContainerName", m_ditauContainerName);
  }

  StatusCode DiTauProngDecorator::addBranches() const
  {
    // retrieve container
    const xAOD::DiTauJetContainer* xDTContainer = evtStore()->retrieve< const xAOD::DiTauJetContainer >( m_ditauContainerName );
    if( ! xDTContainer ) {
      ATH_MSG_ERROR ("Couldn't retrieve di-tau container with key: " << m_ditauContainerName );
      return StatusCode::FAILURE;
    }

    for(auto xDT : *xDTContainer){
      std::vector<int> leadsj={0,0};
      std::vector<int> sublsj={0,0};
      if(xDT->nSubjets() > 0){leadsj=getNtracks(xDT,0);}
      if(xDT->nSubjets() >=2){sublsj=getNtracks(xDT,1);}
      SG::AuxElement::Decorator< int > nleadDeco( "ntrk_lead" );
      SG::AuxElement::Decorator< int > nsublDeco( "ntrk_subl" );
      SG::AuxElement::Decorator< int > qleadDeco( "q_lead" );
      SG::AuxElement::Decorator< int > qsublDeco( "q_subl" );      
      nleadDeco(*xDT) = leadsj[0];
      nsublDeco(*xDT) = sublsj[0];
      qleadDeco(*xDT) = leadsj[1];
      qsublDeco(*xDT) = sublsj[1];
    }

    return StatusCode::SUCCESS;
  }

  std::vector<int> DiTauProngDecorator::getNtracks(const xAOD::DiTauJet* xDiTau, int iSubjet) const
  {
    int charge=0;
    std::vector<int> retvals={99,99};
    if (iSubjet < 0 || iSubjet >= xDiTau->nSubjets()) {
      return retvals;
    }
    if (!xDiTau->isAvailable<std::vector<int>>("n_tracks"))
    {
      ATH_MSG_DEBUG("n_tracks decoration not available. Try with track links.");

      if (!xDiTau->isAvailable< TrackParticleLinks_t >("trackLinks") )
      {
        ATH_MSG_WARNING("Track links not available. Return -1.");
        retvals={-1,-1};
        return retvals;
      } 

      TrackParticleLinks_t xTracks = xDiTau->trackLinks();

      TLorentzVector tlvSubjet;
      tlvSubjet.SetPtEtaPhiE( xDiTau->subjetPt(iSubjet),
                              xDiTau->subjetEta(iSubjet),
                              xDiTau->subjetPhi(iSubjet),
                              xDiTau->subjetE(iSubjet) );
        
      TLorentzVector tlvTrack;
      int nTracks = 0;
      for (const auto &xTrack: xTracks) 
      { 
        tlvTrack.SetPtEtaPhiE( (*xTrack)->pt(),
                               (*xTrack)->eta(),
                               (*xTrack)->phi(),
                               (*xTrack)->e() );
        if ( tlvSubjet.DeltaR(tlvTrack) < 0.2 ) {nTracks++;charge += (*xTrack)->charge();}
      }
      retvals={nTracks,charge};
      return retvals;
    }
    retvals={xDiTau->auxdata<std::vector<int>>("n_tracks").at(iSubjet),charge};
    return retvals;

  }
  
}
