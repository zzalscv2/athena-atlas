/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

#include "RecoElectronHistograms.h"
#include "AsgMessaging/Check.h"
#include "GaudiKernel/ITHistSvc.h"

#include "TH1D.h"

using namespace egammaMonitoring;

StatusCode RecoElectronHistograms::initializePlots() {

  histoMap["nTracks"] = new TH1D(Form("%s_%s",m_name.c_str(),"nTracks"), ";n_{trk}; Events", 10, 0,10);
  ATH_CHECK(m_rootHistSvc->regHist(m_folder+"nTracks", histoMap["nTracks"]));
  
  ATH_CHECK(ParticleHistograms::initializePlots());

  return StatusCode::SUCCESS;

} 

void RecoElectronHistograms::fill(const xAOD::Electron& elrec) {

  ParticleHistograms::fill(elrec);
  histoMap["nTracks"]->Fill(elrec.nTrackParticles());

}
  
