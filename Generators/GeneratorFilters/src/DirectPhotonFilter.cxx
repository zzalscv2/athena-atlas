/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "GeneratorFilters/DirectPhotonFilter.h"
#include <limits>
#include <algorithm>
#include "TruthUtils/HepMCHelpers.h"

DirectPhotonFilter::DirectPhotonFilter(const std::string& name, ISvcLocator* pSvcLocator)
  : GenFilter(name, pSvcLocator)
{
  declareProperty("NPhotons", m_NPhotons = 1);
  declareProperty("OrderPhotons",m_OrderPhotons = true);
  declareProperty("Ptmin",m_Ptmin = std::vector<double>(m_NPhotons, 10000.));
  declareProperty("Ptmax",m_Ptmax = std::vector<double>(m_NPhotons, std::numeric_limits<double>::max()));
  declareProperty("Etacut", m_EtaRange = 2.50);
  declareProperty("AllowSUSYDecay",m_AllowSUSYDecay = false);

}

StatusCode DirectPhotonFilter::filterInitialize() {

  ATH_MSG_INFO("Initialising DirectPhoton filter with OrderPhotons="<<m_OrderPhotons);

  if (m_Ptmin.size()>m_NPhotons || m_Ptmax.size()>m_NPhotons) {
    ATH_MSG_ERROR("Too many Ptmin/max for given NPhotons");
    return StatusCode::FAILURE;
  }

  if (!m_OrderPhotons && (m_Ptmin.size()>1 || m_Ptmax.size()>1)) {
    ATH_MSG_ERROR("Too many Ptmin/max requirements for OrderPhotons=false case.");
    return StatusCode::FAILURE;
  }

  // allow specifying only one pTmin/max to be applied to all (further) photons
  // for backward compatibility
  if (m_Ptmin.size()<m_NPhotons) {
    size_t origsize = m_Ptmin.size();
    double lastPt = m_Ptmin.back();
    m_Ptmin.resize(m_NPhotons);
    for (size_t i=origsize; i<m_NPhotons; ++i) m_Ptmin[i]=lastPt;
  }
  if (m_Ptmax.size()<m_NPhotons) {
    size_t origsize = m_Ptmax.size();
    double lastPt = m_Ptmax.back();
    m_Ptmax.resize(m_NPhotons);
    for (size_t i=origsize; i<m_NPhotons; ++i) m_Ptmax[i]=lastPt;
  }
  return StatusCode::SUCCESS;
}

bool DirectPhotonFilterCmpByPt(const HepMC::ConstGenParticlePtr& p1, const HepMC::ConstGenParticlePtr& p2) {
  return (p1->momentum().perp()>p2->momentum().perp());
}

StatusCode DirectPhotonFilter::filterEvent() {
  std::vector<HepMC::ConstGenParticlePtr> promptPhotonsInEta;

  int phot = 0;
  for(const HepMC::GenEvent* genEvt : *events_const()) {
    ATH_MSG_DEBUG("----->>> Process : " << HepMC::signal_process_id(genEvt));
    // Find all prompt photons with within given eta range
    for (const auto& pitr: *genEvt) {
      if (pitr->pdg_id() == 22 &&
          MC::isStable(pitr) &&
          std::abs(pitr->momentum().pseudoRapidity()) <= m_EtaRange) {
        
        // iterate over parent particles to exclude photons from hadron decays
        auto prodVtx = pitr->production_vertex();
        bool fromHadron(false);
#ifdef HEPMC3
        for (const auto& parent:  prodVtx->particles_in()) {
#else
        for (auto parent_it = prodVtx->particles_begin(HepMC::parents); parent_it != prodVtx->particles_end(HepMC::parents); ++parent_it) {
          auto parent=*parent_it;
#endif
          int pdgindex =  std::abs(parent->pdg_id()); 
          ATH_MSG_DEBUG("Looping on Production (parents) vertex : " << parent->pdg_id() << parent);
          if (pdgindex > 100) {
            fromHadron = true;
            if (m_AllowSUSYDecay && ( (pdgindex > 1000000 && pdgindex < 1000040) || (pdgindex > 2000000 && pdgindex < 2000016) ) ) fromHadron = false;
          }
        }
        phot++;
        if (!fromHadron) promptPhotonsInEta.push_back(pitr);
        else ATH_MSG_DEBUG("non-prompt photon ignored");
      }
    }
  }

  ATH_MSG_DEBUG("number of photons" << phot);

  if (promptPhotonsInEta.size()<m_NPhotons) {
    setFilterPassed(false);
  }
  else {
    for (const auto& photon: promptPhotonsInEta) {

      ATH_MSG_DEBUG("Found prompt photon with pt="<<photon->momentum().perp());
    }
    if (m_OrderPhotons) { // apply cuts to leading/subleading/... photon as specified
      std::sort(promptPhotonsInEta.begin(), promptPhotonsInEta.end(), DirectPhotonFilterCmpByPt);

      bool pass = true;
      for (size_t i = 0; i < m_NPhotons; ++i) {
        double pt = promptPhotonsInEta[i]->momentum().perp();
        if (pt < m_Ptmin[i] || pt > m_Ptmax[i]) {

          ATH_MSG_DEBUG("  rejected pt="<<pt);
          pass = false;
        }
      }

      if (pass) {
         ATH_MSG_DEBUG("Passed!");
         }
      setFilterPassed(pass);
    }
    else { // just require NPhotons to pass m_Ptmin/max[0]
      size_t NPhotons=0;
      for (size_t i = 0; i < promptPhotonsInEta.size(); ++i) {
        double pt = promptPhotonsInEta[i]->momentum().perp();
        if (pt > m_Ptmin[0] && pt < m_Ptmax[0]) ++NPhotons;
      }

      if (NPhotons>=m_NPhotons) ATH_MSG_DEBUG("Passed!");
      setFilterPassed(NPhotons>=m_NPhotons);
    }
  }
  return StatusCode::SUCCESS;
}
