/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// JetConstitFourMomTool.cxx

#include "JetMomentTools/JetConstitFourMomTool.h"

#include "xAODJet/JetConstituentVector.h"
#include "xAODJet/JetTypes.h"

//**********************************************************************

JetConstitFourMomTool::JetConstitFourMomTool(const std::string& myname)
  : asg::AsgTool(myname)
{
#ifndef XAOD_STANDALONE
  declareInterface<IJetModifier>(this);
#endif
}

//**********************************************************************

StatusCode JetConstitFourMomTool::initialize() {

  // Workaround because configuring DataHandleKeyArray directly sneakily removes empty strings
  for(const auto& dhn : m_altColls){
    m_altColls_keys.emplace_back(SG::ReadHandleKey<xAOD::CaloClusterContainer>(dhn));
  }

  for(auto& dh : m_altColls_keys){ATH_CHECK(dh.initialize(!(dh.key().empty())));}

  // Check configuration consistency
  if( m_jetScaleNames.empty() ||
      (m_jetScaleNames.size() != m_altConstitScales.size()) ||
      (m_jetScaleNames.size() != m_altJetScales.size()) ||
      (m_jetScaleNames.size() != m_altColls_keys.size())
      ) {
    ATH_MSG_FATAL("Inconsistency in configuration -- all vector properties must have the same (nonzero) length! Sizes: " 
                  << m_jetScaleNames.size() << " "
                  << m_altConstitScales.size() << " "
                  << m_altJetScales.size() << " "
                  << m_altColls_keys.size());
    return StatusCode::FAILURE;
  }

  m_isDetectorEtaPhi.resize(m_jetScaleNames.size());
  for(size_t iScale=0; iScale<m_jetScaleNames.size(); ++iScale) {
    if(m_jetScaleNames[iScale]=="DetectorEtaPhi") {
      m_isDetectorEtaPhi[iScale] = true;
    }
    if(!m_altJetScales[iScale].empty() && !m_altColls_keys[iScale].key().empty()) {
      ATH_MSG_FATAL("Inconsistency in configuration -- Both AltJetScale and AltConstitColl set for scale " 
		    << m_jetScaleNames[iScale] << "!");
      return StatusCode::FAILURE;      
    }
  }


  return StatusCode::SUCCESS;
}

StatusCode JetConstitFourMomTool::modify(xAOD::JetContainer& jets) const {
  if(jets.empty()) return StatusCode::SUCCESS;

  const size_t nScales=m_jetScaleNames.size();
  // This only really makes sense for clusters now, as signal states don't exist for other types
  std::vector<const xAOD::CaloClusterContainer*> altCollections(nScales,nullptr);
  // Do some setup that doesn't have to be repeated for each jet
  for(size_t iScale=0; iScale<nScales; ++iScale) {
    if(!m_altColls_keys[iScale].key().empty()) { // retrieve alternate constituent collections
      const xAOD::Jet& leadjet = *jets.front();
      if(isValidConstitType(leadjet.getInputType())) {

        auto handle = SG::makeHandle(m_altColls_keys[iScale]);
        if(!handle.isValid()){
          ATH_MSG_WARNING("Failed to retrieve alt cluster collection " 
                          << m_altColls_keys[iScale].key());
          return StatusCode::FAILURE;
        }

        altCollections[iScale] = handle.cptr();
      } else {
        ATH_MSG_WARNING("Alt collection " << m_altColls_keys[iScale].key() << " and jet type " << leadjet.getInputType() << " not supported yet!");
        return StatusCode::FAILURE;
      } // check that jet type/alt collection are implemented
    } // have an alt collection for this scale
  }

  // Limit ourselves to one loop over the constituents
  for(xAOD::Jet *jet : jets) {
    std::vector<xAOD::JetFourMom_t> constitFourVecs(nScales);
    // Get the constituents of the jet
    const xAOD::JetConstituentVector constituents = jet->getConstituents();
    // Gets the constituent iter at the appropriate scale
    xAOD::JetConstitScale constscale = (xAOD::JetConstitScale)(int(m_constitScale));
    for (auto citer = constituents.begin(constscale);
	 citer != constituents.end(constscale); ++citer) {
      for (size_t iScale=0; iScale<nScales; ++iScale) {
	// If altJetScales is set, do nothing in the constituent loop.
	if(m_altJetScales[iScale].empty()) {
	  if(altCollections[iScale]) { // get the index-parallel alternative constituent
	    const xAOD::CaloCluster* cluster = static_cast<const xAOD::CaloCluster*>((*altCollections[iScale])[(*citer)->rawConstituent()->index()]);
	    xAOD::CaloCluster::State currentState= static_cast<xAOD::CaloCluster::State> (m_altConstitScales[iScale]);
	    constitFourVecs[iScale] += xAOD::JetFourMom_t( cluster->pt(currentState), cluster->eta(currentState), 
							   cluster->phi(currentState), cluster->m(currentState) );
	    ATH_MSG_VERBOSE("Add cluster with pt " << cluster->pt() << " to constit fourvec for scale " << m_jetScaleNames[iScale] );
	  } else { // add the constituent 4-mom
	    ATH_MSG_VERBOSE("Add raw constituent with pt " << (*citer)->pt() << " to constit fourvec for scale " << m_jetScaleNames[iScale] );
	    constitFourVecs[iScale] += **citer;
	  }
	}
      } // loop over scales
    } // loop over clusters

    // Now we can set the scales from the various four-vecs we have calculated
    ATH_MSG_VERBOSE("Jet has "
		 << " pt: " << jet->pt()
		 << ", eta: " << jet->eta()
		 << ", phi: " << jet->phi());
    for (size_t iScale=0; iScale<nScales; ++iScale) {
      if(!m_altJetScales[iScale].empty()) {
	// Easy case first -- just copy the momentum state
	constitFourVecs[iScale] = jet->jetP4(m_altJetScales[iScale]);
      }
      ATH_MSG_VERBOSE("Scale " << m_jetScaleNames[iScale] << " has "
		   << " pt: " << constitFourVecs[iScale].Pt()
		   << ", eta: " << constitFourVecs[iScale].Eta()
		   << ", phi: " << constitFourVecs[iScale].Phi());
      if(m_isDetectorEtaPhi[iScale]) {
	const static SG::AuxElement::Accessor<float> acc_modEta("DetectorEta");
	const static SG::AuxElement::Accessor<float> acc_modPhi("DetectorPhi");
	const static SG::AuxElement::Accessor<float> acc_modY("DetectorY");
	acc_modEta(*jet) = constitFourVecs[iScale].Eta();
	acc_modPhi(*jet) = constitFourVecs[iScale].Phi();
	acc_modY(*jet) = constitFourVecs[iScale].Rapidity();
      ATH_MSG_VERBOSE("Detector eta: " << constitFourVecs[iScale].Eta()
		      << ", phi: " << constitFourVecs[iScale].Phi()
		      << ", rapidity: " << constitFourVecs[iScale].Rapidity());
      } else {
	jet->setJetP4(m_jetScaleNames[iScale], constitFourVecs[iScale]);
      }
    } // loop over scales
  } // loop over jets

  return StatusCode::SUCCESS;
}
