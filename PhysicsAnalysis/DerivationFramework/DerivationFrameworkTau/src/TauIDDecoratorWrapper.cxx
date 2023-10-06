/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "DerivationFrameworkTau/TauIDDecoratorWrapper.h"
#include "StoreGate/ReadHandle.h"
#include "xAODCore/ShallowCopy.h"

namespace DerivationFramework {

  TauIDDecoratorWrapper::TauIDDecoratorWrapper(const std::string& t, const std::string& n, const IInterface* p) : 
    AthAlgTool(t,n,p)
  {
    declareInterface<DerivationFramework::IAugmentationTool>(this);
  }

  StatusCode TauIDDecoratorWrapper::initialize()
  {
    // initialize tauRecTools tools
    ATH_CHECK( m_tauIDTools.retrieve() );

    // parse the properties of TauWPDecorator tools
    for (const auto& tool : m_tauIDTools) {
      if (tool->type() != "TauWPDecorator") continue;

      // check whether we must compute eVeto WPs, as this requires the recalculation of a variable
      BooleanProperty useAbsEta("UseAbsEta", false);
      ATH_CHECK( tool->getProperty(&useAbsEta) ); 
      if (useAbsEta.value()) {
	m_doEvetoWP = true;
      }

      // retrieve the names of ID scores and WPs
      StringProperty scoreName("ScoreName", "");
      ATH_CHECK( tool->getProperty(&scoreName) );
      // the original RNNEleScore should not be overriden
      if (scoreName.value() != "RNNEleScore") {
	m_scores.push_back(scoreName.value());
      }

      StringProperty newScoreName("NewScoreName", "");
      ATH_CHECK( tool->getProperty(&newScoreName) );
      m_scores.push_back(newScoreName.value());
      
      StringArrayProperty decorWPNames("DecorWPNames", {});
      ATH_CHECK( tool->getProperty(&decorWPNames) );
      for (const auto& WP : decorWPNames.value()) m_WPs.push_back(WP);
    }
    
    // initialize read handle key
    ATH_CHECK( m_tauContainerKey.initialize() );

    return StatusCode::SUCCESS;
  }

  StatusCode TauIDDecoratorWrapper::finalize()
  {
    return StatusCode::SUCCESS;
  }

  StatusCode TauIDDecoratorWrapper::addBranches() const
  {
    // retrieve tau container
    SG::ReadHandle<xAOD::TauJetContainer> tauJetsReadHandle(m_tauContainerKey);
    if (!tauJetsReadHandle.isValid()) {
      ATH_MSG_ERROR ("Could not retrieve TauJetContainer with key " << tauJetsReadHandle.key());
      return StatusCode::FAILURE;
    }
    const xAOD::TauJetContainer* tauContainer = tauJetsReadHandle.cptr();

    // create shallow copy
    auto shallowCopy = xAOD::shallowCopyContainer (*tauContainer);
    
    static const SG::AuxElement::Accessor<float> acc_absEtaLead("ABS_ETA_LEAD_TRACK");

    for (auto tau : *shallowCopy.first) {

      // ABS_ETA_LEAD_TRACK is removed from the AOD content and must be redecorated when computing eVeto WPs
      // note: this redecoration is not robust against charged track thinning, but charged tracks should never be thinned      
      if (m_doEvetoWP) {
	float absEtaLead = -1111.;
	if(tau->nTracks() > 0) {
	  const xAOD::TrackParticle* track = tau->track(0)->track();
	  absEtaLead = std::abs( track->eta() );
	}
	acc_absEtaLead(*tau) = absEtaLead;
      }

      // pass the shallow copy to the tools
      for (const auto& tool : m_tauIDTools) {
	ATH_CHECK( tool->execute(*tau) );
      }

      // copy over the relevant decorations (scores and working points)
      const xAOD::TauJet* xTau = tauContainer->at(tau->index());
      for (const std::string& score : m_scores) {
	xTau->auxdecor<float>(score) = tau->auxdataConst<float>(score);
      }
      for (const std::string& WP : m_WPs) {
	xTau->auxdecor<char>(WP) = tau->auxdataConst<char>(WP);
      }
    }

    delete shallowCopy.first;
    delete shallowCopy.second;

    return StatusCode::SUCCESS;
  }
}
