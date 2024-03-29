/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#include "cTauRoIThresholdsTool.h"
#include "StoreGate/ReadDecorHandle.h"
#include "StoreGate/exceptions.h"
#include "GaudiKernel/ThreadLocalContext.h"
#include "L1TopoAlgorithms/cTauMultiplicity.h"
#include "L1TopoCoreSim/TopoSteeringStructure.h"


namespace {
  // TODO: avoid hard-coding the WP->int mapping by having cTauMultiplicity::convertIsoToBit return TrigConf::Selection::WP
  bool isocut(TrigConf::Selection::WP WP, const unsigned int bit) {
    // ctauWp will take values 0 (None)/ 1 (Loose)/ 2 (Medium)/ 3 (Tight)
    unsigned int value = 0;
    if ( WP == TrigConf::Selection::WP::NONE ) {value = 0;}
    else if ( WP == TrigConf::Selection::WP::LOOSE ) {value = 1;}
    else if ( WP == TrigConf::Selection::WP::MEDIUM ) {value = 2;}
    else if ( WP == TrigConf::Selection::WP::TIGHT ) {value = 3;}
    // TODO Add a printout in case the WP is not found
    if (bit >= value) {return true;}
    else {return false;}
  };
}

StatusCode cTauRoIThresholdsTool::initialize() {
  ATH_CHECK(RoIThresholdsTool::initialize());
  ATH_CHECK(m_jTauLinkKey.initialize());
  return StatusCode::SUCCESS;
}

uint64_t cTauRoIThresholdsTool::getPattern(const xAOD::eFexTauRoI& eTau,
                                           const RoIThresholdsTool::ThrVec& menuThresholds,
                                           const TrigConf::L1ThrExtraInfoBase& menuExtraInfo) const {
  // Get the jTau matched to the eTau
  using jTauLink_t = ElementLink<xAOD::jFexTauRoIContainer>;
  SG::ReadDecorHandle<xAOD::eFexTauRoIContainer, jTauLink_t> jTauLinkAcc{m_jTauLinkKey, Gaudi::Hive::currentContext()};
  if (not jTauLinkAcc.isPresent()) {
    ATH_MSG_ERROR("Decoration " << m_jTauLinkKey.key() << " is missing, cannot create cTau threshold pattern");
    throw SG::ExcNullReadHandle(m_jTauLinkKey.clid(), m_jTauLinkKey.key(), m_jTauLinkKey.storeHandle().name());
  }
  jTauLink_t jTauLink = jTauLinkAcc(eTau);
  bool matched{jTauLink.isValid()};

  // Variables needed to form a cTau
  // pT in units of 100 MeV
  unsigned int eFexEt{eTau.etTOB()};
  int eFexEta{eTau.iEta()};
  unsigned int isolation_score{0};

  if (matched) {
    const xAOD::jFexTauRoI* jTau = *jTauLink;

    // isolation in units of 200 MeV
    unsigned int jFexIso{jTau->tobIso()};

    std::map<std::string, int> isoFW_CTAU;
    TCS::TopoSteeringStructure::setIsolationFW_CTAU( isoFW_CTAU, menuExtraInfo );

    // The isolation value is multiplied by 2 to normalise to 100 MeV/counts units
    isolation_score = TCS::cTauMultiplicity::convertIsoToBit( isoFW_CTAU, 2*float(jFexIso), float(eFexEt) );

    ATH_MSG_DEBUG("eFex tau eta,phi = " << eTau.iEta() << ", " << eTau.iPhi()
                  << ", jFex tau eta,phi = " << jTau->globalEta() << ", " << jTau->globalPhi()
                  << ", eFex et (100 MeV/counts) = " << eFexEt << ", jFex iso (200 MeV/counts) = " << jFexIso);
  } else {
    ATH_MSG_DEBUG("eFex tau eta,phi = " << eTau.iEta() << ", " << eTau.iPhi()
                  << ", eFex et (100 MeV/counts) = " << eFexEt << ", no matching jTau found");
  }

  uint64_t thresholdMask{0};

  // Iterate through thresholds and see which ones are passed
  for (const std::shared_ptr<TrigConf::L1Threshold>& thrBase : menuThresholds) {
    std::shared_ptr<TrigConf::L1Threshold_cTAU> thr = std::static_pointer_cast<TrigConf::L1Threshold_cTAU>(thrBase);

    // Check isolation threshold - unmatched eTau treated as perfectly isolated, ATR-25927
    bool passIso = matched ? isocut(TrigConf::Selection::WP(thr->isolation()), isolation_score ) : true;

    // Check pt threshold - using iEta coordinate for the eFEX ensures a 0.1 granularity of the eta coordinate,
    // as expected from the menu method thrValue100MeV
    bool passPt = eFexEt > thr->thrValue100MeV(eFexEta);

    if ( passIso && passPt ) {
      thresholdMask |= (1<<thr->mapping());
    }

  } // loop over thr

  return thresholdMask;
}
