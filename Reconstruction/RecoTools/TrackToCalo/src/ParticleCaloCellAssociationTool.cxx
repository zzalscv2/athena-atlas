/*
   Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#include "ParticleCaloCellAssociationTool.h"
// forward declares
#include <cmath>
#include <memory>

#include "CaloEvent/CaloCellContainer.h"
#include "CaloUtils/CaloCellList.h"
#include "ParticleCaloExtension/ParticleCellAssociationCollection.h"
#include "RecoToolInterfaces/IParticleCaloExtensionTool.h"
#include "TrackToCalo/CaloCellHelpers.h"
#include "TrkCaloExtension/CaloExtension.h"
#include "TrkCaloExtension/CaloExtensionCollection.h"
#include "TrkCaloExtension/CaloExtensionHelpers.h"
#include "xAODTracking/TrackingPrimitives.h"

namespace Rec {

ParticleCaloCellAssociationTool::ParticleCaloCellAssociationTool(
  const std::string& t,
  const std::string& n,
  const IInterface* p)
  : AthAlgTool(t, n, p)
  , m_defaultSelector(0.4)
{
  declareInterface<IParticleCaloCellAssociationTool>(this);
}

ParticleCaloCellAssociationTool::~ParticleCaloCellAssociationTool() = default;

StatusCode
ParticleCaloCellAssociationTool::initialize()
{
  /* Retrieve track extrapolator from ToolService */
  ATH_CHECK(m_caloExtensionTool.retrieve());

  m_defaultSelector.setConeSize(m_coneSize);

  if (!m_cellContainerName.key().empty()) {
    ATH_CHECK(m_cellContainerName.initialize());
  }
  
  ATH_CHECK(m_caloMgrKey.initialize());
  return StatusCode::SUCCESS;
}

StatusCode
ParticleCaloCellAssociationTool::finalize()
{
  return StatusCode::SUCCESS;
}

std::unique_ptr<ParticleCellAssociation>
ParticleCaloCellAssociationTool::particleCellAssociation(
  const xAOD::IParticle& particle,
  float dr,
  const CaloCellContainer* container,
  const CaloExtensionCollection* extensionCache) const
{
  ATH_MSG_DEBUG(" particleCellAssociation: ptr " << &particle << " dr " << dr);
  // get the extrapolation into the calo
  std::unique_ptr<const Trk::CaloExtension> caloExtensionUPtr;
  const Trk::CaloExtension* caloExtension = nullptr;
  if (extensionCache)
    caloExtension =
      m_caloExtensionTool->caloExtension(particle, *extensionCache);
  else {
    caloExtensionUPtr = m_caloExtensionTool->caloExtension(
      Gaudi::Hive::currentContext(), particle);
    caloExtension = caloExtensionUPtr.get();
  }
  if (!caloExtension) {
    ATH_MSG_DEBUG("Failed to get calo extension");
    return nullptr;
  }
  if (caloExtension->caloLayerIntersections().empty()) {
    ATH_MSG_DEBUG(
      "Received a caloExtension object without track extrapolation");
    return nullptr;
  }
  // retrieve the cell container if not provided, return false it retrieval
  // failed
  if (!container) {
    if (m_cellContainerName.key().empty()) {
      ATH_MSG_DEBUG("Failed to get calo cell container");
      return nullptr;
    }
    SG::ReadHandle<CaloCellContainer> cccHdl(m_cellContainerName);
    container = &(*cccHdl);
  }
  std::vector<const CaloCell*> cells;
  // update cone size in case it is smaller than the default
  if (dr < m_coneSize) {
    dr = m_coneSize;
  }
  associateCells(*container, *caloExtension, dr, cells);

  // get cell intersections
  ParticleCellAssociation::CellIntersections cellIntersections;
  getCellIntersections(*caloExtension, cells, cellIntersections);
  if (!caloExtensionUPtr)
    // Have to manually copy the calo extension object. Clearly the class
    // wants to be shared through a shared_ptr but this clearly is not an
    // option
    caloExtensionUPtr = std::make_unique<Trk::CaloExtension>(
      caloExtension->caloEntryLayerIntersection()
        ? caloExtension->caloEntryLayerIntersection()->clone()
        : nullptr,
      caloExtension->muonEntryLayerIntersection()
        ? caloExtension->muonEntryLayerIntersection()->clone()
        : nullptr,
      std::vector<Trk::CurvilinearParameters>(
        caloExtension->caloLayerIntersections()));
  return std::make_unique<ParticleCellAssociation>(caloExtensionUPtr.release(),
                                                   std::move(cells),
                                                   dr,
                                                   std::move(cellIntersections),
                                                   container);
}

ParticleCellAssociation*
ParticleCaloCellAssociationTool::particleCellAssociation(
  const xAOD::IParticle& particle,
  float dr,
  IParticleCaloCellAssociationTool::Cache& cache,
  const CaloCellContainer* container,
  const CaloExtensionCollection* extensionCache) const
{
  /*if not there , default ctor for unique_ptr (nullptr)*/
  std::unique_ptr<ParticleCellAssociation>& association =
    cache[particle.index()];
  if (association == nullptr) {
    association =
      particleCellAssociation(particle, dr, container, extensionCache);
  }
  return association.get();
}

void
ParticleCaloCellAssociationTool::getCellIntersections(
  const Trk::CaloExtension& extension,
  const std::vector<const CaloCell*>& cells,
  ParticleCellAssociation::CellIntersections& cellIntersections) const
{
  // use 3D pathlength in cells

  bool use3D = true;

  cellIntersections.reserve(extension.caloLayerIntersections().size() * 1.3);

  CaloExtensionHelpers::EntryExitLayerMap entryExitLayerMap;
  CaloExtensionHelpers::entryExitLayerMap(extension, entryExitLayerMap);
  ATH_MSG_DEBUG("EntryExitLayerMap " << entryExitLayerMap.size());

  CaloExtensionHelpers::ScalarLayerMap eLossLayerMap, pathLenLayerMap;
  CaloExtensionHelpers::eLossLayerMap(extension, eLossLayerMap);
  CaloExtensionHelpers::pathLenLayerMap(extension, pathLenLayerMap);

  ATH_MSG_DEBUG("Getting cells intersections using cells " << cells.size());
  for (const auto* cell : cells) {
    // get sampling and look up entry/exit points
    CaloSampling::CaloSample sample = cell->caloDDE()->getSampling();

    auto pos = entryExitLayerMap.find(sample);
    if (pos == entryExitLayerMap.end())
      continue;
    //
    // pos2 and weight2 are introduced because the PreSamplerB has sometimes a
    // very small size
    //         PresamplerB and EMB1 are merged
    //
    auto pos2 = pos;
    if (sample == CaloSampling::PreSamplerB) {
      pos2 = entryExitLayerMap.find(CaloSampling::EMB1);
      if (pos2 == entryExitLayerMap.end()) {
        pos2 = pos;
      }
    }
    //// calculate 3D path length
    double path = 0.;

    double drFix = cell->caloDDE()->dr();
    double dzFix = cell->caloDDE()->dz();
    //      double dphi = cell->caloDDE()->dphi();

    int isample = cell->caloDDE()->getSampling();
    bool barrel = false;
    if (cell->caloDDE()->getSubCalo() == CaloCell_ID::TILE)
      barrel = true;
    if (sample == CaloSampling::PreSamplerB || sample == CaloSampling::EMB1 ||
        sample == CaloSampling::EMB2 || sample == CaloSampling::EMB3)
      barrel = true;

    double drTG = fabs((pos->second.first - pos2->second.second).perp());
    double dzTG = fabs((pos->second.first - pos2->second.second).z());

    if (barrel)
      ATH_MSG_VERBOSE(" barrel cell sampling "
                      << cell->caloDDE()->getSampling() << " dr "
                      << cell->caloDDE()->dr() << " drTG " << drTG);
    if (!barrel)
      ATH_MSG_VERBOSE(" endcap cell sampling "
                      << cell->caloDDE()->getSampling() << " dz "
                      << cell->caloDDE()->dz() << " dzTG " << dzTG);

    if (drFix == 0.) {
      // recalculate the r values from the other cells
      // BUG/FEATURE: extract dr from cell container for sampling 4 5 6 7 needed
      // EME BUG/FEATURE: extract dr from cell container for sampling 8 9 10 11
      // needed HEC
      if (cell->caloDDE()->deta() > 0) {
        double theta = atan2(cell->caloDDE()->r(), cell->z());
        double dtheta =
          2 * cell->caloDDE()->deta() * sin(theta / 2.) * cos(theta / 2);
        if (theta + dtheta < M_PI) {
          double dr =
            fabs(cell->z() * tan(theta + dtheta) - cell->z() * tan(theta));
          drFix = fabs(dr);
          double detaCheck =
            -log(tan((theta + dtheta) / 2.)) + log(tan((theta) / 2.));
          ATH_MSG_VERBOSE(" FIX cell sampling "
                          << cell->caloDDE()->getSampling() << " deta "
                          << cell->caloDDE()->deta() << " detaCheck "
                          << detaCheck << " drFix " << drFix);
        } else {
          ATH_MSG_WARNING(" FIXR cell sampling failed: theta "
                          << theta << " dtheta " << dtheta << " sum/pi "
                          << (theta + dtheta) * M_1_PI << " deta "
                          << cell->caloDDE()->deta());
        }
        //          ATH_MSG_VERBOSE(" FIX cell sampling deta " << deta << "
        //          dtheta " << dtheta  << "  scale " <<  scale << " theta " <<
        //          theta );
      } else {
        double drMin = 100000.;
        int dscut = 1;
        if (!barrel)
          dscut = 0;
        const CaloCell* cellFound = nullptr;
        for (const auto* celln : cells) {
          if (cell == celln)
            continue;
          if (cell->caloDDE()->getSubCalo() == celln->caloDDE()->getSubCalo()) {
            int dsample = isample - celln->caloDDE()->getSampling();
            if (abs(dsample) == dscut) {
              double drNew = fabs(cell->caloDDE()->r() - celln->caloDDE()->r());
              if (drNew < 1)
                continue;
              if (drNew < drMin) {
                drMin = drNew;
                cellFound = celln;
              }
            }
          }
        }
        drFix = drMin;
        ATH_MSG_VERBOSE(" Problem cell sampling "
                        << cell->caloDDE()->getSampling() << " x "
                        << cell->caloDDE()->x() << " y " << cell->caloDDE()->y()
                        << " z " << cell->caloDDE()->z() << " dr "
                        << cell->caloDDE()->dr() << " drFix " << drFix
                        << " drTG " << drTG);
        if (cellFound)
          ATH_MSG_VERBOSE(" cellFound sampling "
                          << cellFound->caloDDE()->getSampling() << " x "
                          << cellFound->caloDDE()->x() << " y "
                          << cellFound->caloDDE()->y() << " z "
                          << cellFound->caloDDE()->z() << " dr "
                          << cellFound->caloDDE()->dr() << " dscut " << dscut
                          << " drFix " << drFix);
      }
    }

    if (dzFix == 0.) {
      // recalculate z values from the other cells
      // BUG/FEATURE: extract dz from cell container for sampling 0 1 2 3 needed
      // EMB
      if (cell->caloDDE()->deta() > 0) {
        double theta = atan2(cell->caloDDE()->r(), cell->z());
        double dtheta =
          2 * cell->caloDDE()->deta() * sin(theta / 2.) * cos(theta / 2);
        if (theta + dtheta < M_PI) {
          double dz = fabs(cell->caloDDE()->r() / tan(theta + dtheta) -
                           cell->caloDDE()->r() / tan(theta));
          dzFix = dz;
        } else {
          ATH_MSG_WARNING(" FIXZ cell sampling failed: theta "
                          << theta << " dtheta " << dtheta << " sum/pi "
                          << (theta + dtheta) * M_1_PI << " deta "
                          << cell->caloDDE()->deta());
        }
        double detaCheck =
          -log(tan((theta + dtheta) / 2.)) + log(tan((theta) / 2.));
        ATH_MSG_VERBOSE(" Fix cell sampling "
                        << cell->caloDDE()->getSampling() << " deta "
                        << cell->caloDDE()->deta() << " detaCheck  "
                        << detaCheck << " dtheta " << dtheta << " dzFix "
                        << dzFix);
      } else {
        double dzMin = 100000.;
        int dscut = 1;
        if (barrel)
          dscut = 0;
        const CaloCell* cellFound = nullptr;
        for (const auto* celln : cells) {
          if (cell == celln)
            continue;
          if (cell->caloDDE()->getSubCalo() == celln->caloDDE()->getSubCalo()) {
            int isample2 = celln->caloDDE()->getSampling();
            if (abs(isample - isample2) == dscut) {
              double dzNew = fabs(cell->caloDDE()->z() - celln->caloDDE()->z());
              if (dzNew < 1)
                continue;
              if (dzNew < dzMin) {
                dzMin = dzNew;
                cellFound = celln;
              }
            }
          }
        }
        dzFix = dzMin;
        ATH_MSG_VERBOSE(" Problem cell sampling "
                        << cell->caloDDE()->getSampling() << " x "
                        << cell->caloDDE()->x() << " y " << cell->caloDDE()->y()
                        << " z " << cell->caloDDE()->z() << " dz "
                        << cell->caloDDE()->dz() << " dzFix " << dzFix
                        << " dzTG " << dzTG);
        if (cellFound)
          ATH_MSG_VERBOSE(" cellFound sampling "
                          << cellFound->caloDDE()->getSampling() << " x "
                          << cellFound->caloDDE()->x() << " y "
                          << cellFound->caloDDE()->y() << " z "
                          << cellFound->caloDDE()->z() << " dz "
                          << cellFound->caloDDE()->dz() << " dscut " << dscut
                          << " dzFix " << dzFix);
      }
    }
    //
    // always use fixed values that correspond to the Calorimeter Tracking
    // Geometry these are different from the CaloCell values
    //

    if (cell->energy() > 50.)
      ATH_MSG_DEBUG(" cell sampling and size "
                    << cell->caloDDE()->getSampling() << " cell energy "
                    << cell->energy() << " dzFix " << dzFix << " dzTG " << dzTG
                    << " drFix " << drFix << " drTG " << drTG << " barrel "
                    << barrel);

    if (!barrel)
      dzFix = dzTG;
    if (barrel)
      drFix = drTG;

    if (use3D) {
      // m_pathLenUtil.pathInsideCell( *cell, entryExitLayerMap);
      double pathInMM = PathLengthUtils::get3DPathLength(
        *cell, pos->second.first, pos2->second.second, drFix, dzFix);
      double totpath = (pos->second.first - pos2->second.second).mag();
      path = totpath != 0 ? pathInMM / totpath : 0.;
      if (path > 0 || cell->energy() > 50.) {
        ATH_MSG_DEBUG(" cell sampling and size "
                      << cell->caloDDE()->getSampling() << " cell energy "
                      << cell->energy() << " drFix " << drFix << " dzFix "
                      << dzFix << " path " << path << " length TG " << totpath);
        ATH_MSG_DEBUG(" cell dr " << cell->caloDDE()->dr() << " cell dz "
                                  << cell->caloDDE()->dz() << " deta "
                                  << cell->caloDDE()->deta());
      }
    }

    //// calculate 2D path length (method2)
    double path2 = 0.;

    if (!use3D)
      path2 = pathInsideCell(*cell, pos->second.first, pos2->second.second);

    if (path2 <= 0. && path <= 0.)
      continue;

    // auto entrancePair = entryExitLayerMap.find(entranceID);
    auto eLossPair = eLossLayerMap.find(sample);
    double eLoss = 0.;
    //
    // Just store total expected eloss
    //
    if (eLossPair != eLossLayerMap.end()) {
      eLoss = eLossPair->second;
      if (sample == CaloSampling::PreSamplerB) {
        auto eLossPair2 = eLossLayerMap.find(CaloSampling::EMB1);
        if (eLossPair2 != eLossLayerMap.end()) {
          eLoss = 0.5 * (eLossPair->second) + 0.5 * (eLossPair2->second);
        }
      } else if (sample == CaloSampling::EMB1) {
        auto eLossPair2 = eLossLayerMap.find(CaloSampling::PreSamplerB);
        if (eLossPair2 != eLossLayerMap.end()) {
          eLoss = 0.5 * (eLossPair->second) + 0.5 * (eLossPair2->second);
        }
      }
    } // IF

    ATH_MSG_DEBUG(" PATH3D = "
                  << path << " PATH2D = " << path2 << " eLoss " << eLoss
                  << " cell energy " << (cell)->energy() << " radius "
                  << cell->caloDDE()->r() << " phi " << cell->caloDDE()->phi()
                  << " dr " << cell->caloDDE()->dr() << " dphi "
                  << cell->caloDDE()->dphi() << " x " << cell->caloDDE()->x()
                  << " y " << cell->caloDDE()->y() << " z "
                  << cell->caloDDE()->z() << " dx " << cell->caloDDE()->dx()
                  << " dy " << cell->caloDDE()->dy() << " dz "
                  << cell->caloDDE()->dz() << " volume "
                  << cell->caloDDE()->volume());

    cellIntersections.emplace_back(
      cell, new ParticleCellIntersection(*cell, eLoss, use3D ? path : path2));
  }
  ATH_MSG_DEBUG(" added cell intersections  " << cellIntersections.size());
}

void
ParticleCaloCellAssociationTool::associateCells(
  const CaloCellContainer& container,
  const Trk::CaloExtension& caloExtension,
  float dr,
  std::vector<const CaloCell*>& cells) const
{
  const Trk::TrackParameters* pars = caloExtension.caloEntryLayerIntersection();
  if (!pars) {
    ATH_MSG_DEBUG("associateCells() - NO TrackParameters found in "
                  "caloExtension.caloEntryLayerIntersection()");
    return;
  }

  double eta = pars->position().eta();
  double phi = pars->position().phi();

  // Use Calorimeter list for CPU reasons
  SG::ReadCondHandle<CaloDetDescrManager> caloMgrHandle{m_caloMgrKey};
  const CaloDetDescrManager* caloMgr=*caloMgrHandle;
  CaloCellList myList(caloMgr,&container);
  myList.select(eta, phi, dr);
  cells.reserve(myList.ncells());
  cells.insert(cells.end(), myList.begin(), myList.end());
  ATH_MSG_DEBUG("associated cells " << cells.size() << " using cone " << dr);
}

} // namespace Rec
