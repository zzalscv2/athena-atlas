/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/*
Authors: Teng Jian Khoo <teng.jian.khoo@cern.ch>
         Russell Smith  <rsmith@cern.ch>

These selections are available as a courtesy for users to easily validate they are correctly using the METMaker code.
These are the cuts used in the METRebuilder, so you can easily compare and make sure you are doing the same thing with the METMaker
These selections are NOT optimized, especially the muon term, and as such should be optimized within physics groups
SW note no changes required for MT mode
*/
#include "METUtilities/CutsMETMaker.h"

//namespace CutsMETMaker{

StatusCode CutsMETMaker::accept(const xAOD::Muon* mu)
{

  if(mu->pt()<2.5e3 || mu->pt()/cosh(mu->eta())<4e3) return StatusCode::FAILURE;
  if(mu->muonType()==xAOD::Muon::MuonStandAlone) {
    // only take forward SA -- need a max eta cut?
    if(fabs(mu->eta())<2.5) return StatusCode::FAILURE;
    uint8_t nPrecision=0;
    if(!mu->primaryTrackParticleLink().isValid()) return StatusCode::FAILURE;
    mu->primaryTrackParticle()->summaryValue(nPrecision,xAOD::numberOfPrecisionLayers);
    if(nPrecision<3) return StatusCode::FAILURE;
  } // selection for StandAlone muons
  else if(mu->muonType()==xAOD::Muon::Combined || mu->muonType()==xAOD::Muon::SegmentTagged) {
    if(fabs(mu->eta())>2.5) return StatusCode::FAILURE;

    // could add some error checking to make sure we successfully read the details
    uint8_t nPixHits(0), nSctHits(0);
    if(!mu->primaryTrackParticleLink().isValid()) return StatusCode::FAILURE;
    mu->primaryTrackParticle()->summaryValue(nPixHits,xAOD::numberOfPixelHits);
    mu->primaryTrackParticle()->summaryValue(nSctHits,xAOD::numberOfSCTHits);

    if(nPixHits<3) return StatusCode::FAILURE;
    if(nPixHits+nSctHits<5) return StatusCode::FAILURE;
  } // selection for SegmentTagged and Combined muons
  else {return StatusCode::FAILURE;} // don't CutMETMaker::accept forward muons or calo tagged

  return StatusCode::SUCCESS;
}

StatusCode CutsMETMaker::accept(const xAOD::Electron* el)
{

  //ATH_MSG_VERBOSE("Test electron quality."
  // << " pT = " << el->pt()
  // << " eta = " << el->eta()
  // << " phi = " << el->phi());

  bool testPID = 0;
  el->passSelection(testPID,"Medium");
  //ATH_MSG_VERBOSE("Electron PID \"Medium\" tests " << (testPID ? " GOOD" : "BAD") );
  if( !testPID ) return StatusCode::FAILURE;

  //ATH_MSG_VERBOSE("Electron author = " << el->author() << " test " << (el->author()&17));
  if( !(el->author()&17) ) return StatusCode::FAILURE;

  if( el->pt()<10e3 ) return StatusCode::FAILURE;
  if( fabs(el->eta())>2.47 ) return StatusCode::FAILURE;

  // if( m_el_rejectCrack ) {
  //   if( fabs(el->eta())>1.37 &&
  // 	  fabs(el->eta())<1.52 ) return StatusCode::FAILURE;
  // }

  //ATH_MSG_VERBOSE("CutMETMaker::Accepted this electron");

  return StatusCode::SUCCESS;
}

StatusCode CutsMETMaker::accept(const xAOD::Photon* ph)
{

  //ATH_MSG_VERBOSE("Test photon quality."
  // << " pT = " << ph->pt()
  // << " eta = " << ph->eta()
  // << " phi = " << ph->phi());

  bool testPID = 0;
  ph->passSelection(testPID,"Tight");
  //ATH_MSG_VERBOSE("Photon PID \"Tight\" tests " << (testPID ? " GOOD" : "BAD") );
  if( !testPID ) return StatusCode::FAILURE;

  //ATH_MSG_VERBOSE("Photon author = " << ph->author() << " test " << (ph->author()&20));
  if( !(ph->author()&20) ) return StatusCode::FAILURE;

  if( ph->pt()<10e3 ) return StatusCode::FAILURE;
  if( fabs(ph->eta())>2.47 ) return StatusCode::FAILURE;

  //ATH_MSG_VERBOSE("CutMETMaker::Accepted this photon");

  return StatusCode::SUCCESS;
}

StatusCode CutsMETMaker::accept(const xAOD::TauJet* tau)
{
  //ATH_MSG_VERBOSE("Testing tau with pt " << tau->pt() << ", eta " << tau->eta());
  //ATH_MSG_VERBOSE("Tau ID discriminants:"
  // << " jet " << tau->discriminant(xAOD::TauJetParameters::BDTJetScore)
  // << " el " << tau->discriminant(xAOD::TauJetParameters::BDTEleScore)
  // << " mu " << tau->flag(xAOD::TauJetParameters::MuonFlag));

  if(tau->pt()<20e3 || fabs(tau->eta())>2.5) return StatusCode::FAILURE;
  // need to accommodate more than one of these?
  if(!tau->isTau( xAOD::TauJetParameters::IsTauFlag(xAOD::TauJetParameters::JetRNNSigMedium) )) return StatusCode::FAILURE;
  if(tau->isTau( xAOD::TauJetParameters::IsTauFlag(xAOD::TauJetParameters::EleRNNMedium) )) return StatusCode::FAILURE;
  if(tau->isTau( xAOD::TauJetParameters::IsTauFlag(xAOD::TauJetParameters::MuonVeto) )) return StatusCode::FAILURE;

  return StatusCode::SUCCESS;
}
//}
