/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// SCT_PrepDataToxAOD.cxx
//   Implementation file for class SCT_PrepDataToxAOD
///////////////////////////////////////////////////////////////////

#include "SCT_PrepDataToxAOD.h"

#include "Identifier/Identifier.h"
#include "InDetIdentifier/SCT_ID.h"
#include "InDetRawData/SCT_RDO_Collection.h"
#include "InDetSimEvent/SiHit.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"
#include "xAODTracking/TrackMeasurementValidationAuxContainer.h"

#include "AtlasHepMC/GenParticle.h"

#include "CLHEP/Geometry/Point3D.h"

#include <cmath>

#define AUXDATA(OBJ, TYP, NAME)                                         \
  static const SG::AuxElement::Accessor<TYP> acc_##NAME (#NAME);  acc_##NAME(*(OBJ))

/////////////////////////////////////////////////////////////////////
//
//         Constructor with parameters:
//
/////////////////////////////////////////////////////////////////////
SCT_PrepDataToxAOD::SCT_PrepDataToxAOD(const std::string& name, ISvcLocator* pSvcLocator) :
  AthReentrantAlgorithm(name, pSvcLocator)
{ 
}

/////////////////////////////////////////////////////////////////////
//
//        Initialize method: 
//
/////////////////////////////////////////////////////////////////////
StatusCode SCT_PrepDataToxAOD::initialize()
{
  ATH_CHECK(detStore()->retrieve(m_SCTHelper, "SCT_ID"));

  //make sure we don't write what we don't have
  if (not m_useTruthInfo) {
    m_writeSDOs.set(false);
    m_writeSiHits.set(false);
  }

  ATH_CHECK(m_clustercontainer.initialize());
  ATH_CHECK(m_SDOcontainer.initialize(m_writeSDOs));
  ATH_CHECK(m_sihitContainer.initialize(m_writeSiHits));
  ATH_CHECK(m_multiTruth.initialize(m_useTruthInfo));
  ATH_CHECK(m_rdoContainer.initialize(m_writeRDOinformation));
  ATH_CHECK(m_xAodContainer.initialize());
  ATH_CHECK(m_xAodOffset.initialize());

  ATH_CHECK(m_SCTDetEleCollKey.initialize());

  return StatusCode::SUCCESS;
}

/////////////////////////////////////////////////////////////////////
//
//        Execute method: 
//
/////////////////////////////////////////////////////////////////////
StatusCode SCT_PrepDataToxAOD::execute(const EventContext& ctx) const
{
  // the cluster ambiguity map
  std::map<Identifier, const SCT_RDORawData*> idToRAWDataMap;
  if (m_writeRDOinformation) {
    SG::ReadHandle<SCT_RDO_Container> rdoContainer{m_rdoContainer, ctx};
    if (rdoContainer.isValid()) {
      // get all the RIO_Collections in the container
      for (const auto collection: *rdoContainer) {
        //get all the RDOs in the collection
        for (const auto rdo : *collection) {
          if (rdo==nullptr) {
            ATH_MSG_WARNING("Null SCT RDO. Skipping it");
            continue;
          }
          Identifier rdoId{rdo->identify()};
          idToRAWDataMap.insert(std::pair<Identifier, const SCT_RDORawData*>{rdoId, rdo});
        } // collection
      } // Have container;
    } else if (m_firstEventWarnings) {
      ATH_MSG_WARNING("Failed to retrieve SCT RDO container");
    }
  }
  ATH_MSG_DEBUG("Size of RDO map is " << idToRAWDataMap.size());

  const PRD_MultiTruthCollection* prdmtColl{nullptr};
  if (m_useTruthInfo) {
    SG::ReadHandle<PRD_MultiTruthCollection> prdmtCollHandle{m_multiTruth, ctx};
    if (prdmtCollHandle.isValid()) {
      prdmtColl = &*prdmtCollHandle;
    }
  }

  const InDetSimDataCollection* sdoCollection{nullptr};
  if (m_writeSDOs) {
    SG::ReadHandle<InDetSimDataCollection> sdoCollectionHandle{m_SDOcontainer, ctx};
    if (sdoCollectionHandle.isValid()) {
      sdoCollection = &*sdoCollectionHandle;
    }
  }

  std::vector<std::vector<const SiHit*>> siHits(m_SCTHelper->wafer_hash_max());
  if (m_writeSiHits) {
    SG::ReadHandle<SiHitCollection> sihitCollection{m_sihitContainer, ctx};
    if (sihitCollection.isValid()) {
      for (const SiHit& siHit: *sihitCollection) {
        // Check if it is an SCT hit
        if (not siHit.isSCT()) continue;

        Identifier wafer_id{m_SCTHelper->wafer_id(siHit.getBarrelEndcap(),
                                                  siHit.getLayerDisk(),
                                                  siHit.getPhiModule(),
                                                  siHit.getEtaModule(),
                                                  siHit.getSide())};
        IdentifierHash wafer_hash{m_SCTHelper->wafer_hash(wafer_id)};
        siHits[wafer_hash].push_back(&siHit);
      }
    }
  }
  
  // Mandatory. This is needed and required if this algorithm is scheduled.
  SG::ReadHandle<InDet::SCT_ClusterContainer> sctClusterContainer{m_clustercontainer, ctx};
  if (not sctClusterContainer.isValid()) {
    ATH_MSG_FATAL("Cannot retrieve SCT PrepDataContainer " << m_clustercontainer.key());
    return StatusCode::FAILURE;
  }

  // Create the xAOD container and its auxiliary store:
  SG::WriteHandle<xAOD::TrackMeasurementValidationContainer> xaod{m_xAodContainer, ctx};
  ATH_CHECK(xaod.record(std::make_unique<xAOD::TrackMeasurementValidationContainer>(),
                        std::make_unique<xAOD::TrackMeasurementValidationAuxContainer>()));
  
  SG::WriteHandle<std::vector<unsigned int>> offsets{m_xAodOffset, ctx};
  ATH_CHECK(offsets.record(std::make_unique<std::vector<unsigned int>>(m_SCTHelper->wafer_hash_max(), 0)));

  // Loop over the container
  unsigned int counter{0};
  for (const auto clusterCollection: *sctClusterContainer) {
    //Fill Offset container
    (*offsets)[clusterCollection->identifyHash()] = counter;
    
    // skip empty collections
    if (clusterCollection->empty()) continue;

    xaod->resize(counter + clusterCollection->size());
    // loop over collection and convert to xAOD
    for (const InDet::SCT_Cluster* prd: *clusterCollection) {
      Identifier clusterId{prd->identify()};
      if (not clusterId.is_valid()) {
        ATH_MSG_WARNING("SCT cluster identifier is not valid!");
      }

      // create and add xAOD object
      xAOD::TrackMeasurementValidation* xprd{new xAOD::TrackMeasurementValidation()};
      xaod->at(counter) = xprd;
      ++counter;

      //Set Identifier
      xprd->setIdentifier(clusterId.get_compact());

      //Set Global Position
      Amg::Vector3D gpos{prd->globalPosition()};
      xprd->setGlobalPosition(gpos.x(), gpos.y(), gpos.z());

      //Set Local Position
      const Amg::Vector2D& locpos{prd->localPosition()};
      float locY{0.};
      float locX{static_cast<float>(locpos.x())};
      if ((not std::isinf(locpos.y()) or std::isnan(locpos.y()))) {
        if (locpos.y()>=1e-07) locY = locpos.y();
      } else { 
        locY = -9999.;
      }

      // Set local error matrix
      xprd->setLocalPosition(locX, locY);

      const Amg::MatrixX& localCov{prd->localCovariance()};
      if (localCov.size() == 1) {
        xprd->setLocalPositionError(localCov(0, 0), 0., 0.);
      } else if (localCov.size() == 4) {
        xprd->setLocalPositionError(localCov(0, 0), localCov(1, 1), localCov(0, 1));
      } else {
        xprd->setLocalPositionError(0., 0., 0.);
      }
         
      // Set vector of hit identifiers
      std::vector<uint64_t> rdoIdentifierList;
      rdoIdentifierList.reserve(prd->rdoList().size());
      for (const auto& hitIdentifier: prd->rdoList()) {
        rdoIdentifierList.push_back(hitIdentifier.get_compact());
      }
      xprd->setRdoIdentifierList(rdoIdentifierList);

      //Add SCT specific information
      const InDet::SiWidth cw{prd->width()};
      AUXDATA(xprd, int, SiWidth) = static_cast<int>(cw.colRow()[0]);
      AUXDATA(xprd, int, hitsInThirdTimeBin) = static_cast<int>(prd->hitsInThirdTimeBin());

      AUXDATA(xprd, int, bec)        = m_SCTHelper->barrel_ec(clusterId);
      AUXDATA(xprd, int, layer)      = m_SCTHelper->layer_disk(clusterId);
      AUXDATA(xprd, int, phi_module) = m_SCTHelper->phi_module(clusterId);
      AUXDATA(xprd, int, eta_module) = m_SCTHelper->eta_module(clusterId);
      AUXDATA(xprd, int, side)       = m_SCTHelper->side(clusterId);
   
      // Add the Detector element ID  --  not sure if needed as we have the informations above
      const InDetDD::SiDetectorElement* de{prd->detectorElement()};
      uint64_t detElementId{0};
      if (de) {
        Identifier detId{de->identify()};
        if (detId.is_valid()) {
          detElementId = detId.get_compact();
        }
      }
      AUXDATA(xprd, uint64_t, detectorElementID) = detElementId;
   
      //Add details about the individual hits 
      if (m_writeRDOinformation) {
        addRDOInformation(xprd, prd, idToRAWDataMap);
      }
      
      // Use the MultiTruth Collection to get a list of all true particle contributing to the cluster
      if (m_useTruthInfo) {
        if (prdmtColl) {
          std::vector<int> barcodes;
          auto range{prdmtColl->equal_range(clusterId)};
          for (auto& i{range.first}; i!=range.second; ++i) {
            barcodes.push_back(i->second.barcode());
          }
          AUXDATA(xprd, std::vector<int>, truth_barcode) = barcodes;
        }
      }

      // Use the SDO Collection to get a list of all true particle contributing to the cluster per readout element
      //  Also get the energy deposited by each true particle per readout element   
      if (m_writeSDOs) {
        if (sdoCollection) {
          addSDOInformation(xprd, prd, sdoCollection);
        }
      }
    
      // Now Get the most detailed truth from the SiHits
      // Note that this could get really slow if there are a lot of hits and clusters
      if (m_writeSiHits) {
        addSiHitInformation(xprd, prd, &siHits[prd->detectorElement()->identifyHash()]);
      }
    }
  }
  ATH_MSG_DEBUG(" recorded SCT_PrepData objects: size " << xaod->size());

  m_firstEventWarnings = false; //disable one-time warnings

  return StatusCode::SUCCESS;
}

void SCT_PrepDataToxAOD::addSDOInformation(xAOD::TrackMeasurementValidation* xprd,
                                           const InDet::SCT_Cluster* prd,
                                           const InDetSimDataCollection* sdoCollection) const
{
  std::vector<int> sdo_word;
  std::vector<std::vector<int>> sdo_depositsBarcode;
  std::vector<std::vector<float>> sdo_depositsEnergy;
  // find hit
  for (const auto& hitIdentifier: prd->rdoList()) {
    auto pos{sdoCollection->find(hitIdentifier)};
    if (pos == sdoCollection->end()) continue;
    sdo_word.push_back(pos->second.word());
    std::vector<int> sdoDepBC(pos->second.getdeposits().size(), -1);
    std::vector<float> sdoDepEnergy(pos->second.getdeposits().size());
    unsigned int nDepos{0};
    for (auto& deposit: pos->second.getdeposits()) {
        if (deposit.first) sdoDepBC[nDepos] = deposit.first.barcode();
        ATH_MSG_DEBUG(" SDO Energy Deposit " << deposit.second);
        sdoDepEnergy[nDepos] = deposit.second;
        nDepos++;
    }
    sdo_depositsBarcode.push_back(sdoDepBC);
    sdo_depositsEnergy.push_back(sdoDepEnergy);
  }
  AUXDATA(xprd, std::vector<int>, sdo_words) = sdo_word;
  AUXDATA(xprd, std::vector<std::vector<int>>, sdo_depositsBarcode) = sdo_depositsBarcode;
  AUXDATA(xprd, std::vector<std::vector<float>>, sdo_depositsEnergy) = sdo_depositsEnergy;
}


void SCT_PrepDataToxAOD::addSiHitInformation(xAOD::TrackMeasurementValidation* xprd, 
                                             const InDet::SCT_Cluster* prd,
                                             const std::vector<const SiHit*>* siHits) const
{
  std::vector<SiHit> matchingHits;
  findAllHitsCompatibleWithCluster(prd, siHits, matchingHits);

  long unsigned int numHits{matchingHits.size()};

  std::vector<float> sihit_energyDeposit(numHits, 0.);
  std::vector<float> sihit_meanTime(numHits, 0.);
  std::vector<int> sihit_barcode(numHits, 0);
  
  std::vector<float> sihit_startPosX(numHits, 0.);
  std::vector<float> sihit_startPosY(numHits, 0.);
  std::vector<float> sihit_startPosZ(numHits, 0.);

  std::vector<float> sihit_endPosX(numHits, 0);
  std::vector<float> sihit_endPosY(numHits, 0);
  std::vector<float> sihit_endPosZ(numHits, 0);

  int hitNumber{0};
  const InDetDD::SiDetectorElement* de{prd->detectorElement()};
  if (de) {
    for (const SiHit& sihit : matchingHits) {
      sihit_energyDeposit[hitNumber] = sihit.energyLoss();
      sihit_meanTime[hitNumber] = sihit.meanTime();
      sihit_barcode[hitNumber] = sihit.particleLink().barcode();
    
      // Convert Simulation frame into reco frame
      const HepGeom::Point3D<double>& startPos{sihit.localStartPosition()};

      Amg::Vector2D pos{de->hitLocalToLocal(startPos.z(), startPos.y())};
      sihit_startPosX[hitNumber] = pos[0];
      sihit_startPosY[hitNumber] = pos[1];
      sihit_startPosZ[hitNumber] = startPos.x();

      const HepGeom::Point3D<double>& endPos{sihit.localEndPosition()};
      pos= de->hitLocalToLocal(endPos.z(), endPos.y());
      sihit_endPosX[hitNumber] = pos[0];
      sihit_endPosY[hitNumber] = pos[1];
      sihit_endPosZ[hitNumber] = endPos.x();
      ++hitNumber;
    }
  }

  AUXDATA(xprd, std::vector<float>, sihit_energyDeposit) = sihit_energyDeposit;
  AUXDATA(xprd, std::vector<float>, sihit_meanTime) = sihit_meanTime;
  AUXDATA(xprd, std::vector<int>, sihit_barcode) = sihit_barcode;
  
  AUXDATA(xprd, std::vector<float>, sihit_startPosX) = sihit_startPosX;
  AUXDATA(xprd, std::vector<float>, sihit_startPosY) = sihit_startPosY;
  AUXDATA(xprd, std::vector<float>, sihit_startPosZ) = sihit_startPosZ;

  AUXDATA(xprd, std::vector<float>, sihit_endPosX) = sihit_endPosX;
  AUXDATA(xprd, std::vector<float>, sihit_endPosY) = sihit_endPosY;
  AUXDATA(xprd, std::vector<float>, sihit_endPosZ) = sihit_endPosZ;
}

void SCT_PrepDataToxAOD::findAllHitsCompatibleWithCluster(const InDet::SCT_Cluster* prd, 
                                                          const std::vector<const SiHit*>* siHits,
                                                          std::vector<SiHit>& matchingHits) const
{
  ATH_MSG_VERBOSE("Got " << siHits->size() << " SiHits to look through");

  // Check if we have detector element  --  needed to find the local position of the SiHits
  const InDetDD::SiDetectorElement* de{prd->detectorElement()};
  if (de==nullptr) return;

  std::vector<const SiHit*> multiMatchingHits;

  for (const SiHit* siHit: *siHits) {
    // Now we have all hits in the module that match lets check to see if they match the cluster
    // Must be within +/- 1 hits of any hit in the cluster to be included
    
    HepGeom::Point3D<double> averagePosition{siHit->localStartPosition() + siHit->localEndPosition()};
    averagePosition *= 0.5;
    Amg::Vector2D pos{de->hitLocalToLocal(averagePosition.z(), averagePosition.y())};
    InDetDD::SiCellId diode{de->cellIdOfPosition(pos)};
 
    for (const auto& hitIdentifier: prd->rdoList()) {
      ATH_MSG_DEBUG("Truth Strip " <<  diode.phiIndex() << " Cluster Strip " << m_SCTHelper->strip(hitIdentifier));

      if (std::abs(static_cast<int>(diode.phiIndex()) - m_SCTHelper->strip(hitIdentifier))<=1) {
        multiMatchingHits.push_back(siHit);
        break;
      }
    }
  }

  matchingHits.reserve(multiMatchingHits.size());
  //Now we will now make 1 SiHit for each true particle if the SiHits "touch" other 
  std::vector<const SiHit*>::iterator siHitIter{multiMatchingHits.begin()};
  std::vector<const SiHit*>::iterator siHitIter2{multiMatchingHits.begin()};
  ATH_MSG_DEBUG("Found " << multiMatchingHits.size() << " SiHit ");
  for (; siHitIter != multiMatchingHits.end(); ++siHitIter) {
    const SiHit* lowestXPos{*siHitIter};
    const SiHit* highestXPos{*siHitIter};

    // We will merge these hits
    std::vector<const SiHit*> ajoiningHits;
    ajoiningHits.push_back(*siHitIter);
  
    siHitIter2 = siHitIter+1;
    auto bc = (*siHitIter)->particleLink().barcode();
    while (siHitIter2 != multiMatchingHits.end()) {
      // Need to come from the same truth particle
      if ( bc != (*siHitIter2)->particleLink().barcode()) {
        ++siHitIter2;
        continue;
      }

      constexpr double maxDiff = 0.00005;
      // Check to see if the SiHits are compatible with each other.
      if (std::abs((highestXPos->localEndPosition().x()-(*siHitIter2)->localStartPosition().x()))<maxDiff and
          std::abs((highestXPos->localEndPosition().y()-(*siHitIter2)->localStartPosition().y()))<maxDiff and
          std::abs((highestXPos->localEndPosition().z()-(*siHitIter2)->localStartPosition().z()))<maxDiff) {
        highestXPos = *siHitIter2;
        ajoiningHits.push_back(*siHitIter2);
        // Dont use hit  more than once
        siHitIter2 = multiMatchingHits.erase(siHitIter2);
      } else if (std::abs((lowestXPos->localStartPosition().x()-(*siHitIter2)->localEndPosition().x()))<maxDiff and
                 std::abs((lowestXPos->localStartPosition().y()-(*siHitIter2)->localEndPosition().y()))<maxDiff and
                 std::abs((lowestXPos->localStartPosition().z()-(*siHitIter2)->localEndPosition().z()))<maxDiff) {
        lowestXPos = *siHitIter2;
        ajoiningHits.push_back(*siHitIter2);
        // Dont use hit  more than once
        siHitIter2 = multiMatchingHits.erase(siHitIter2);
      } else {
        ++siHitIter2;
      }
    }
    
    if (ajoiningHits.size()==0) {
      ATH_MSG_ERROR("This should really never happen");
      continue;
    } else if (ajoiningHits.size()==1) {
      // Copy Si Hit ready to return
      matchingHits.emplace_back(*ajoiningHits[0]);
      continue;
    } else {
      //  Build new SiHit and merge information together.
      ATH_MSG_DEBUG("Merging " << ajoiningHits.size() << " SiHits together.");
      float energyDep{0.};
      float time{0.};
      for (auto& siHit: ajoiningHits) {
        energyDep += siHit->energyLoss();
        time += siHit->meanTime();
      }
      time /= static_cast<float>(ajoiningHits.size());
      matchingHits.emplace_back(lowestXPos->localStartPosition(), 
            highestXPos->localEndPosition(),
            energyDep,
            time,
            (*siHitIter)->particleLink().barcode(),
            1, // 0 for pixel 1 for SCT
            (*siHitIter)->getBarrelEndcap(),
            (*siHitIter)->getLayerDisk(),
            (*siHitIter)->getEtaModule(),
            (*siHitIter)->getPhiModule(),
            (*siHitIter)->getSide());
    }
  }
}

void SCT_PrepDataToxAOD::addRDOInformation(xAOD::TrackMeasurementValidation* xprd, 
                                           const InDet::SCT_Cluster* prd,
                                           const std::map<Identifier, const SCT_RDORawData*>& idToRAWDataMap) const {
  std::vector<int> strip(prd->rdoList().size(), -1);
  std::vector<int> timebin(prd->rdoList().size(), -1);
  std::vector<int> groupsize(prd->rdoList().size(), -1);

  unsigned int nRDOs{0};
  for (const auto& hitIdentifier: prd->rdoList()) {
    auto result{idToRAWDataMap.find(hitIdentifier)};
    if (result != idToRAWDataMap.end()) {
      const SCT_RDORawData* sctRdo{result->second};
      strip[nRDOs] = m_SCTHelper->strip(sctRdo->identify());
      const SCT3_RawData* rdo3{dynamic_cast<const SCT3_RawData*>(sctRdo)};
      if (rdo3) {
        timebin[nRDOs] = rdo3->getTimeBin();
        groupsize[nRDOs] = rdo3->getGroupSize();
      }
    }
    nRDOs++;
  }
  
  AUXDATA(xprd, std::vector<int>, rdo_strip) = strip;
  AUXDATA(xprd, std::vector<int>, rdo_timebin) = timebin;
  AUXDATA(xprd, std::vector<int>, rdo_groupsize) = groupsize;
}

/////////////////////////////////////////////////////////////////////
//
//        Finalize method: 
//
/////////////////////////////////////////////////////////////////////
StatusCode SCT_PrepDataToxAOD::finalize()
{
  return StatusCode::SUCCESS;
}
