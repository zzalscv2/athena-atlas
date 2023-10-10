/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "egammaSuperClusterBuilder.h"

#include "CaloDetDescr/CaloDetDescrManager.h"
#include "CaloUtils/CaloClusterStoreHelper.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"
#include "xAODCaloEvent/CaloCluster.h"
#include "xAODCaloEvent/CaloClusterAuxContainer.h"
#include "xAODEgamma/Egamma.h"
#include "xAODEgamma/EgammaEnums.h"
#include "xAODEgamma/EgammaxAODHelpers.h"

#include <cmath>
#include <memory>

egammaSuperClusterBuilder::egammaSuperClusterBuilder(const std::string& name,
                                                     ISvcLocator* pSvcLocator)
  : egammaSuperClusterBuilderBase(name, pSvcLocator)
  , m_egTypeForCalibration(xAOD::EgammaParameters::electron)
{
}

StatusCode
egammaSuperClusterBuilder::initialize()
{
  ATH_MSG_DEBUG(" Initializing egammaSuperClusterBuilder");

  // the data handle keys
  if (m_calibrationType == "electron") {
    m_egTypeForCalibration = xAOD::EgammaParameters::electron;
  } else if (m_calibrationType == "photon") {
    m_egTypeForCalibration = xAOD::EgammaParameters::unconvertedPhoton;
  } else {
    ATH_MSG_ERROR("Unsupported calibration for " << m_calibrationType);
    return StatusCode::FAILURE;
  }

  return egammaSuperClusterBuilderBase::initialize();
}


xAOD::EgammaParameters::EgammaType
egammaSuperClusterBuilder::getEgammaRecType([[maybe_unused]]const egammaRec *egRec) const {
  return m_egTypeForCalibration;
}

// assume egammaRecs != 0, since the ReadHadler is valid
// assume seed egammaRec has a valid cluster, since it has been already used
std::vector<std::size_t>
egammaSuperClusterBuilder::searchForSecondaryClusters(
  std::size_t seedIndex,
  const EgammaRecContainer* egammaRecs,
  std::vector<bool>& isUsed) const
{

  std::vector<std::size_t> secondaryIndices;

  const auto* const seedEgammaRec = (*egammaRecs)[seedIndex];
  const xAOD::CaloCluster* const seedCaloClus = seedEgammaRec->caloCluster();

  // for stats
  int nWindowClusters = 0;
  // Now loop over the potential secondary clusters
  for (std::size_t i = 0; i < egammaRecs->size(); ++i) {
    // if already used continue
    if (isUsed[i]) {
      continue;
    }
    const auto* const secEgammaRec = (*egammaRecs)[i];
    const xAOD::CaloCluster* const secClus = secEgammaRec->caloCluster();
    if (!secClus) {
      ATH_MSG_WARNING(
        "The potentially secondary egammaRec does not have a cluster");
      continue;
    }
    bool addCluster = false;
    if (matchesInWindow(seedCaloClus, secClus)) {
      ATH_MSG_DEBUG("Cluster with Et: " << secClus->et()
                                        << " matched in window");
      ++nWindowClusters;
      addCluster = true;
    }
    // Add it to the list of secondary clusters if it matches.
    if (addCluster) {
      secondaryIndices.push_back(i);
      isUsed[i] = true;
    }
  }
  ATH_MSG_DEBUG("Found: " << secondaryIndices.size() << " secondaries");
  ATH_MSG_DEBUG("window clusters: " << nWindowClusters);
  return secondaryIndices;
}

