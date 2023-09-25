/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

// INCLUDE HEADER FILES:
#include "egammaOQFlagsBuilder.h"
#include "xAODCaloEvent/CaloCluster.h"
#include "xAODEgamma/Egamma.h"
#include "xAODEgamma/EgammaxAODHelpers.h"
#include <algorithm>
#include <cmath>
#include <vector>

#include "CaloConditions/CaloAffectedRegionInfoVec.h"
#include "CaloIdentifier/CaloCell_ID.h"
#include "CaloIdentifier/LArEM_ID.h"
#include "FourMomUtils/P4Helpers.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "Identifier/HWIdentifier.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/StoreGateSvc.h"

namespace {
bool
findCentralCell(const xAOD::CaloCluster* cluster, Identifier& cellCentrId)
{

  bool thereIsACentrCell = false;

  // LOOP OVER CLUSTER TO FIND THE CENTRAL CELL IN S2
  xAOD::CaloCluster::const_cell_iterator cellIter = cluster->cell_begin();
  xAOD::CaloCluster::const_cell_iterator cellIterEnd = cluster->cell_end();
  float clusEta = cluster->eta();
  float clusPhi = cluster->phi();
  float energymax = -999999.;

  for (; cellIter != cellIterEnd; cellIter++) {
    const CaloCell* cell = (*cellIter);
    if (!cell) {
      continue;
    }
    float eta = cell->eta();
    float phi = cell->phi();
    float energy = cell->energy();
    CaloSampling::CaloSample layer = cell->caloDDE()->getSampling();
    if (fabs(eta - clusEta) < 0.025 &&
        fabs(P4Helpers::deltaPhi(phi, clusPhi)) < 0.025 &&
        (layer == CaloSampling::EMB2 || layer == CaloSampling::EME2) &&
        (energy > energymax)) {
      energymax = energy;
      cellCentrId = cellIter->ID();
      thereIsACentrCell = true;
    }
  }
  return thereIsACentrCell;
}
bool
isCore(const Identifier Id,
       const std::vector<IdentifierHash>& neighbourList,
       const CaloCell_ID* calocellId)
{
  const IdentifierHash hashId = calocellId->calo_cell_hash(Id);
  std::vector<IdentifierHash>::const_iterator it =
    std::find(neighbourList.begin(), neighbourList.end(), hashId);
  return (it != neighbourList.end());
}

std::vector<IdentifierHash>
findNeighbours(const Identifier cellCentrId,
               const LArEM_ID* emHelper,
               const CaloCell_ID* calocellId)
{
  std::vector<IdentifierHash> neighbourList;
  const IdentifierHash hashId = calocellId->calo_cell_hash(cellCentrId);
  emHelper->get_neighbours(hashId, LArNeighbours::all2D, neighbourList);
  return neighbourList;
}

void coreCellHelper(const bool isMissing, const bool isMasked,
                    const bool isSporadicNoise, const bool isAffected,
                    const bool isHighQ, unsigned int& iflag) {
  if (isMissing) {
    iflag |= (0x1 << xAOD::EgammaParameters::MissingFEBCellCore);
  }
  if (isMasked) {
    iflag |= (0x1 << xAOD::EgammaParameters::MaskedCellCore);
  }
  if (isSporadicNoise) {
    iflag |= (0x1 << xAOD::EgammaParameters::SporadicNoiseLowQCore);
  }
  if (isAffected) {
    iflag |= (0x1 << xAOD::EgammaParameters::AffectedCellCore);
  }
  if (isHighQ) {
    iflag |= (0x1 << xAOD::EgammaParameters::HighQCore);
  }
}

void missingHelper(const bool isPresampler, const bool isL1,
                   const bool isStripCoreCell, const bool isL2, const bool isL3,
                   unsigned int& iflag) {
  if (isPresampler) {
    iflag |= (0x1 << xAOD::EgammaParameters::MissingFEBCellEdgePS);
  } else if (isL1) {
    iflag |= (0x1 << xAOD::EgammaParameters::MissingFEBCellEdgeS1);
    if (isStripCoreCell) {
      iflag |= (0x1 << xAOD::EgammaParameters::BadS1Core);
    }
  } else if (isL2) {
    iflag |= (0x1 << xAOD::EgammaParameters::MissingFEBCellEdgeS2);
  } else if (isL3) {
    iflag |= (0x1 << xAOD::EgammaParameters::MissingFEBCellEdgeS3);
  }
}

void maskedHelper(const bool isPresampler, const bool isL1,
                  const bool isStripCoreCell, const bool isL2, const bool isL3,
                  unsigned int& iflag) {
  if (isPresampler) {
    iflag |= (0x1 << xAOD::EgammaParameters::MaskedCellEdgePS);
  } else if (isL1) {
    iflag |= (0x1 << xAOD::EgammaParameters::MaskedCellEdgeS1);
    if (isStripCoreCell) {
      iflag |= (0x1 << xAOD::EgammaParameters::BadS1Core);
    }
  } else if (isL2) {
    iflag |= (0x1 << xAOD::EgammaParameters::MaskedCellEdgeS2);
  } else if (isL3) {
    iflag |= (0x1 << xAOD::EgammaParameters::MaskedCellEdgeS3);
  }
}

void affectedHelper(const bool isPresampler, const bool isL1, const bool isL2,
                    const bool isL3, unsigned int& iflag) {
  if (isPresampler) {
    iflag |= (0x1 << xAOD::EgammaParameters::AffectedCellEdgePS);
  } else if (isL1) {
    iflag |= (0x1 << xAOD::EgammaParameters::AffectedCellEdgeS1);
  } else if (isL2) {
    iflag |= (0x1 << xAOD::EgammaParameters::AffectedCellEdgeS2);
  } else if (isL3) {
    iflag |= (0x1 << xAOD::EgammaParameters::AffectedCellEdgeS3);
  }
}

}  // end anonymous namespace

egammaOQFlagsBuilder::egammaOQFlagsBuilder(const std::string& type,
                                           const std::string& name,
                                           const IInterface* parent)
  : AthAlgTool(type, name, parent)
  , m_emHelper(nullptr)
  , m_calocellId(nullptr)
{
  declareInterface<IegammaOQFlagsBuilder>(this);
}

egammaOQFlagsBuilder::~egammaOQFlagsBuilder() = default;

StatusCode
egammaOQFlagsBuilder::initialize()
{
  ATH_CHECK(m_cellsKey.initialize());
  ATH_CHECK(m_bcContKey.initialize());
  ATH_CHECK(m_affKey.initialize());

  // Get CaloAffectedTool
  ATH_CHECK(m_affectedTool.retrieve());
  ATH_CHECK(detStore()->retrieve(m_calocellId, "CaloCell_ID"));
  ATH_CHECK(detStore()->retrieve(m_emHelper));

  return StatusCode::SUCCESS;
}

StatusCode
egammaOQFlagsBuilder::finalize()
{
  return StatusCode::SUCCESS;
}

StatusCode
egammaOQFlagsBuilder::execute(const EventContext& ctx,
                              xAOD::Egamma& eg) const
{
  // Protection against bad pointers
  const xAOD::CaloCluster* cluster = eg.caloCluster();
  if (!cluster) {
    return StatusCode::SUCCESS;
  }
  if (cluster->size() == 0) {
    return StatusCode::SUCCESS;
  }
  //
  const float clusterEta = cluster->eta();
  //
  // In case we have the sizes set during the cluster construction.
  int etaSize = cluster->getClusterEtaSize();
  int phiSize = cluster->getClusterPhiSize();
  // If no proper size could be found automatically, deduce by hand
  // for the known std cases
  if (etaSize == 0 && phiSize == 0) {
    bool isBarrel = xAOD::EgammaHelpers::isBarrel(cluster);
    if (xAOD::EgammaHelpers::isElectron(&eg)) {
      etaSize = (isBarrel ? 3 : 5);
      phiSize = (isBarrel ? 7 : 5);
    } else if (xAOD::EgammaHelpers::isConvertedPhoton(&eg)) {
      etaSize = (isBarrel ? 3 : 5);
      phiSize = (isBarrel ? 7 : 5);
    } else { // unconverted photons
      etaSize = (isBarrel ? 3 : 5);
      phiSize = (isBarrel ? 7 : 5);
    }
  }

  unsigned int iflag = eg.OQ();

  // Set timing bit
  const double absEnergyGeV = fabs(cluster->e() * (1. / Gaudi::Units::GeV));
  if (absEnergyGeV != 0 &&
      fabs(cluster->time()) > m_TCut + m_TCutVsE / absEnergyGeV) {
    iflag |= (0x1 << xAOD::EgammaParameters::OutTime);
  }

  // Declare totE and badE for LarQ cleaning
  double totE = 0;
  double badE = 0;
  double energyCellMax = 0;
  // Find the central cell in the middle layer
  Identifier cellCentrId;
  bool foundCentralCell = findCentralCell(cluster, cellCentrId);
  if (foundCentralCell) {
    // Find the list of neighbours cells, to define the 3x3 cluster core
    std::vector<IdentifierHash> neighbourList =
      findNeighbours(cellCentrId, m_emHelper, m_calocellId);
    // Get Bad-channel info for this event
    SG::ReadCondHandle<LArBadChannelCont> larBadChanHdl{ m_bcContKey, ctx };
    const LArBadChannelCont* larBadChanCont = *larBadChanHdl;

    // Loop over all the Lar cluster cells
    xAOD::CaloCluster::const_cell_iterator cellIter = cluster->cell_begin();
    xAOD::CaloCluster::const_cell_iterator cellIterEnd = cluster->cell_end();
    for (; cellIter != cellIterEnd; cellIter++) {
      const CaloCell* cell = (*cellIter);
      if (!cell) {
        continue;
      }
      // Check we are not tile
      if (cell->caloDDE()->is_tile()) {
        continue;
      }
      // Find cell parameters and properties
      const float eta = cell->eta();
      // float phi = cell->phi(); // no longer used
      const float qual = cell->quality();
      const bool isHighQ = qual >= 4000;
      const CaloSampling::CaloSample layer = cell->caloDDE()->getSampling();

      const bool isMissing = ((cell->provenance() & 0x0A00) == 0x0A00);
      const bool isMasked = ((cell->provenance() & 0x0A00) == 0x0800);
      const bool isPresampler = (layer == CaloSampling::PreSamplerB ||
                                 layer == CaloSampling::PreSamplerE);
      const bool isL1 =
        (layer == CaloSampling::EMB1 || layer == CaloSampling::EME1);
      const bool isL2 =
        (layer == CaloSampling::EMB2 || layer == CaloSampling::EME2);
      const bool isL3 =
        (layer == CaloSampling::EMB3 || layer == CaloSampling::EME3);

      // Calculate badE et totE
      if ((cell->provenance() & 0x2000) && !(cell->provenance() & 0x0800)) {
        totE += cell->e();
        if (cell->e() > energyCellMax) {
          energyCellMax = cell->e();
        }
        if (qual > m_QCellCut) {
          badE += cell->e();
        }
      }
      const bool isACoreCell = isCore(cell->ID(), neighbourList, m_calocellId);

      bool isStripCoreCell = false;
      if ((layer == CaloSampling::EMB1 || layer == CaloSampling::EME1) &&
          fabs(eta - clusterEta) < 0.025 / 2.) {
        isStripCoreCell = true;
      }

      // Set HEC bit
      if (layer >= CaloSampling::HEC0 && layer <= CaloSampling::HEC3 &&
          qual > m_QCellHECCut) {
        iflag |= (0x1 << xAOD::EgammaParameters::HECHighQ);
      }

      // Set LAr bits
      const LArBadChannel bc = larBadChanCont->offlineStatus(cell->ID());
      const bool isAffected =
        (bc.deadCalib() || bc.lowNoiseHG() || bc.lowNoiseMG() ||
         bc.lowNoiseLG() || bc.distorted() || bc.unstable() ||
         bc.unstableNoiseHG() || bc.unstableNoiseMG() || bc.unstableNoiseLG() ||
         bc.peculiarCalibrationLine() || bc.almostDead() || bc.shortProblem());

      const bool isSporadicNoise =
          (bc.sporadicBurstNoise() && qual < m_QCellSporCut);

      if (isACoreCell) {
        coreCellHelper(isMissing, isMasked, isSporadicNoise, isAffected,
                       isHighQ, iflag);
      }  // end if isACoreCell
      else {
        if (isMissing) {
          missingHelper(isPresampler, isL1, isStripCoreCell, isL2, isL3, iflag);
        }  // isMissing
        if (isMasked) {
          maskedHelper(isPresampler, isL1, isStripCoreCell, isL2, isL3, iflag);
        }  // isMasked
        if (isSporadicNoise) {
          iflag |= (0x1 << xAOD::EgammaParameters::SporadicNoiseLowQEdge);
        }
        if (isAffected) {
          affectedHelper(isPresampler, isL1, isL2, isL3, iflag);
        }  // is affected
        if (isHighQ) {
          iflag |= (0x1 << xAOD::EgammaParameters::HighQEdge);
        }
      }
    }  // end loop over LAr cells

    // Set LArQCleaning bit
    double egammaLArQCleaning = 0;
    if (totE != 0) {
      egammaLArQCleaning = badE / totE;
    }
    if (egammaLArQCleaning > m_LArQCut) {
      iflag |= (0x1 << xAOD::EgammaParameters::LArQCleaning);
    }
    // Set HighRcell bit//
    double ratioCell = 0;
    if (totE != 0) {
      ratioCell = energyCellMax / totE;
    }
    if (ratioCell > m_RcellCut) {
      iflag |= (0x1 << xAOD::EgammaParameters::HighRcell);
    }
  } // close if found central cell

  // Check the HV components
  float deta = 0;
  float dphi = 0;

  // Get affected info for this event
  SG::ReadCondHandle<CaloAffectedRegionInfoVec> affHdl{ m_affKey, ctx };
  const CaloAffectedRegionInfoVec* affCont = *affHdl;
  if (!affCont) {
    ATH_MSG_WARNING("Do not have affected regions info, is this expected ?");
  }

  //--------------> PRE SAMPLER
  deta = 0.5 * 0.025 * etaSize;
  dphi = 0.5 * 0.025 * phiSize;

  bool isNonNominalHVPS = (m_affectedTool->isAffected(cluster,
                                                      affCont,
                                                      deta,
                                                      dphi,
                                                      CaloSampling::PreSamplerE,
                                                      CaloSampling::PreSamplerE,
                                                      1) ||
                           m_affectedTool->isAffected(cluster,
                                                      affCont,
                                                      deta,
                                                      dphi,
                                                      CaloSampling::PreSamplerB,
                                                      CaloSampling::PreSamplerB,
                                                      1));
  if (isNonNominalHVPS) {
    iflag |= (0x1 << xAOD::EgammaParameters::NonNominalHVPS);
  }

  bool isDeadHVPS = (m_affectedTool->isAffected(cluster,
                                                affCont,
                                                deta,
                                                dphi,
                                                CaloSampling::PreSamplerE,
                                                CaloSampling::PreSamplerE,
                                                2) ||
                     m_affectedTool->isAffected(cluster,
                                                affCont,
                                                deta,
                                                dphi,
                                                CaloSampling::PreSamplerB,
                                                CaloSampling::PreSamplerB,
                                                2));
  if (isDeadHVPS) {
    iflag |= (0x1 << xAOD::EgammaParameters::DeadHVPS);
  }

  //---------------> SAMPLING 2 : CLUSTER CORE
  deta = 0.5 * 0.025 * 3.;
  dphi = 0.5 * 0.025 * 3.;
  const bool isDeadHVS2Core = (m_affectedTool->isAffected(cluster,
                                                          affCont,
                                                          deta,
                                                          dphi,
                                                          CaloSampling::EMB2,
                                                          CaloSampling::EMB2,
                                                          2) ||
                               m_affectedTool->isAffected(cluster,
                                                          affCont,
                                                          deta,
                                                          dphi,
                                                          CaloSampling::EME2,
                                                          CaloSampling::EME2,
                                                          2));

  if (isDeadHVS2Core) {
    iflag |= (0x1 << xAOD::EgammaParameters::DeadHVS1S2S3Core);
  }
  //----------------> SAMPLINGS 1,2,3 : CLUSTER EDGE
  deta = 0.5 * 0.025 * etaSize;
  dphi = 0.5 * 0.025 * phiSize;

  const bool isNonNominalHVS1S2S3 =
    (m_affectedTool->isAffected(cluster,
                                affCont,
                                deta,
                                dphi,
                                CaloSampling::EMB1,
                                CaloSampling::EMB1,
                                1) ||
     m_affectedTool->isAffected(cluster,
                                affCont,
                                deta,
                                dphi,
                                CaloSampling::EMB2,
                                CaloSampling::EMB2,
                                1) ||
     m_affectedTool->isAffected(cluster,
                                affCont,
                                deta,
                                dphi,
                                CaloSampling::EMB3,
                                CaloSampling::EMB3,
                                1) ||
     m_affectedTool->isAffected(cluster,
                                affCont,
                                deta,
                                dphi,
                                CaloSampling::EME1,
                                CaloSampling::EME1,
                                1) ||
     m_affectedTool->isAffected(cluster,
                                affCont,
                                deta,
                                dphi,
                                CaloSampling::EME2,
                                CaloSampling::EME2,
                                1) ||
     m_affectedTool->isAffected(cluster,
                                affCont,
                                deta,
                                dphi,
                                CaloSampling::EME3,
                                CaloSampling::EME3,
                                1));
  if (isNonNominalHVS1S2S3) {
    iflag |= (0x1 << xAOD::EgammaParameters::NonNominalHVS1S2S3);
  }

  const bool isDeadHVS1S2S3Edge =
    (m_affectedTool->isAffected(cluster,
                                affCont,
                                deta,
                                dphi,
                                CaloSampling::EMB1,
                                CaloSampling::EMB1,
                                2) ||
     m_affectedTool->isAffected(cluster,
                                affCont,
                                deta,
                                dphi,
                                CaloSampling::EMB2,
                                CaloSampling::EMB2,
                                2) ||
     m_affectedTool->isAffected(cluster,
                                affCont,
                                deta,
                                dphi,
                                CaloSampling::EMB3,
                                CaloSampling::EMB3,
                                2) ||
     m_affectedTool->isAffected(cluster,
                                affCont,
                                deta,
                                dphi,
                                CaloSampling::EME1,
                                CaloSampling::EME1,
                                2) ||
     m_affectedTool->isAffected(cluster,
                                affCont,
                                deta,
                                dphi,
                                CaloSampling::EME2,
                                CaloSampling::EME2,
                                2) ||
     m_affectedTool->isAffected(cluster,
                                affCont,
                                deta,
                                dphi,
                                CaloSampling::EME3,
                                CaloSampling::EME3,
                                2));

  if (isDeadHVS1S2S3Edge) {
    iflag |= (0x1 << xAOD::EgammaParameters::DeadHVS1S2S3Edge);
  }

  eg.setOQ(iflag);
  ATH_MSG_DEBUG("Executing egammaOQFlagsBuilder::execute");
  return StatusCode::SUCCESS;
}

