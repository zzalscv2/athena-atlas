/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

#include "RecoElectronHistograms.h"
#include "AsgMessaging/Check.h"
#include "GaudiKernel/ITHistSvc.h"

#include "TH2D.h"

using namespace egammaMonitoring;

StatusCode RecoElectronHistograms::initializePlots() {

  ATH_CHECK(ParticleHistograms::initializePlots());

  histoMap2D["eta_nTracks"] =
    new TH2D(Form("%s_%s",m_name.c_str(),"eta_nTracks"),
	     ";#eta;n_{trk}; Events", 60, -4.5, 4.5, 10, 0,10);
  ATH_CHECK(m_rootHistSvc->regHist(m_folder+"eta_nTracks", histoMap2D["eta_nTracks"]));

  return StatusCode::SUCCESS;

} 

void RecoElectronHistograms::fill(const xAOD::Electron& elrec) {

  ParticleHistograms::fill(elrec);
  histoMap2D["eta_nTracks"]->Fill(elrec.eta(),elrec.nTrackParticles());

}
  
