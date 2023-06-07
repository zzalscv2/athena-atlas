/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////
// Package : sTgcRawDataMonAlg
// Author: Sebastian Fuenzalida Garrido
// Local supervisor: Edson Carquin Lopez
// Technical supervisor: Gerardo Vasquez
//
// DESCRIPTION:
// Subject: sTgc --> sTgc raw data monitoring
///////////////////////////////////////////////////////////////////////////////////////////////////////

#include "StgcRawDataMonitoring/StgcRawDataMonAlg.h"

/////////////////////////////////////////////////////////////////////////////
// *********************************************************************
// Public Methods
// ********************************************************************* 
/////////////////////////////////////////////////////////////////////////////
 
// Pad Trigger Branch -> Testing 

sTgcRawDataMonAlg::sTgcRawDataMonAlg(const std::string& name, ISvcLocator* pSvcLocator) : AthMonitorAlgorithm(name, pSvcLocator) {
  //Declare the property 
}

StatusCode sTgcRawDataMonAlg::initialize() {   
  ATH_CHECK(AthMonitorAlgorithm::initialize());
  ATH_CHECK(m_idHelperSvc.retrieve());
  ATH_CHECK(m_sTgcContainerKey.initialize());
  ATH_CHECK(m_detectorManagerKey.initialize());
  ATH_CHECK(m_meTrkKey.initialize());
  ATH_CHECK(m_residualPullCalculator.retrieve());
  ATH_CHECK(m_muonKey.initialize());
  ATH_CHECK(m_rdoKey.initialize(SG::AllowEmpty)); 
  
  return StatusCode::SUCCESS;
} 

StatusCode sTgcRawDataMonAlg::fillHistograms(const EventContext& ctx) const {  
  const int lumiblock = GetEventInfo(ctx) -> lumiBlock();

  SG::ReadHandle<Muon::sTgcPrepDataContainer> sTgcContainer(m_sTgcContainerKey, ctx);
  SG::ReadCondHandle<MuonGM::MuonDetectorManager> detectorManagerKey(m_detectorManagerKey, ctx); 
  SG::ReadHandle<xAOD::TrackParticleContainer> meTPContainer(m_meTrkKey, ctx);
  SG::ReadHandle<xAOD::MuonContainer> muonContainer(m_muonKey, ctx);
  
  if (!meTPContainer.isValid()) {
    ATH_MSG_FATAL("Could not get track particle container: " << m_meTrkKey.fullKey());
    return StatusCode::FAILURE;
  }
 
  if(!m_rdoKey.key().empty()){
    SG::ReadHandle<Muon::NSW_PadTriggerDataContainer> NSWpadTriggerContainer(m_rdoKey, ctx);
    if (!NSWpadTriggerContainer.isValid()) {
      ATH_MSG_FATAL("Could not get pad trigger data container: " << m_rdoKey.fullKey());
      return StatusCode::FAILURE;
    }
    fillsTgcPadTriggerDataHistograms(NSWpadTriggerContainer.cptr(), lumiblock);
  } 

  if (!muonContainer.isValid()) {
    ATH_MSG_FATAL("Could not get muon container: " << m_muonKey.fullKey());
    return StatusCode::FAILURE;
  }

  fillsTgcClusterFromTrackHistograms(meTPContainer.cptr());
  fillsTgcEfficiencyHistograms(muonContainer.cptr(), detectorManagerKey.cptr());

  for(const Muon::sTgcPrepDataCollection* coll : *sTgcContainer) {
    for (const Muon::sTgcPrepData* prd : *coll) {
      fillsTgcOccupancyHistograms(prd, detectorManagerKey.cptr());
      fillsTgcLumiblockHistograms(prd, lumiblock);
    }
  }
  
  return StatusCode::SUCCESS;
}

void sTgcRawDataMonAlg::fillsTgcOccupancyHistograms(const Muon::sTgcPrepData* sTgcObject, const MuonGM::MuonDetectorManager* muonDetectorManagerObject) const {
  Identifier id    = sTgcObject -> identify();

  if(!id.is_valid()) {
    ATH_MSG_DEBUG("Invalid identifier found in Muon::sTgcPrepData");
    return;
  }

  std::string stationName = m_idHelperSvc -> stgcIdHelper().stationNameString(m_idHelperSvc -> stgcIdHelper().stationName(id));
  int stationEta          = m_idHelperSvc -> stgcIdHelper().stationEta(id);
  int stationPhi          = m_idHelperSvc -> stgcIdHelper().stationPhi(id);
  int multiplet           = m_idHelperSvc -> stgcIdHelper().multilayer(id);
  int gasGap              = m_idHelperSvc -> stgcIdHelper().gasGap(id);    
  int channelType         = m_idHelperSvc -> stgcIdHelper().channelType(id);
  int sector              = m_idHelperSvc -> sector(id);
  int sectorsTotal        = getSectors(id);
  int layer               = getLayer(multiplet, gasGap);
  int stationEtaShifted   = (stationEta   < 0) ? stationEta   - 1: stationEta;
  int sectorsTotalShifted = (sectorsTotal < 0) ? sectorsTotal - 1: sectorsTotal; 

  if (channelType == sTgcIdHelper::sTgcChannelTypes::Pad) {
    int padNumber = m_idHelperSvc -> stgcIdHelper().channel(id);       
    Identifier idPadQ1 = m_idHelperSvc -> stgcIdHelper().channelID(stationName, 1, stationPhi, multiplet, gasGap, channelType, 1);    
    Identifier idPadQ2 = m_idHelperSvc -> stgcIdHelper().channelID(stationName, 2, stationPhi, multiplet, gasGap, channelType, 1);
    const MuonGM::sTgcReadoutElement* sTgcReadoutObjectPadQ1 = muonDetectorManagerObject -> getsTgcReadoutElement(idPadQ1);
    const MuonGM::sTgcReadoutElement* sTgcReadoutObjectPadQ2 = muonDetectorManagerObject -> getsTgcReadoutElement(idPadQ2);
    int maxPadNumberQ1 = sTgcReadoutObjectPadQ1 -> maxPadNumber(idPadQ1);
    int maxPadNumberQ2 = sTgcReadoutObjectPadQ2 -> maxPadNumber(idPadQ2);    
            
    if (std::abs(stationEta) == 1) {
      auto sectorMon    = Monitored::Scalar<int>("sector_layer_" + std::to_string(layer), sectorsTotalShifted);
      auto padNumberMon = Monitored::Scalar<int>("padNumber_layer_" + std::to_string(layer), padNumber);
      fill("sTgcOccupancy", sectorMon, padNumberMon);
    }
  
    else if (std::abs(stationEta) == 2) {
      auto sectorMon    = Monitored::Scalar<int>("sector_layer_" + std::to_string(layer), sectorsTotalShifted);
      auto padNumberMon = Monitored::Scalar<int>("padNumber_layer_" + std::to_string(layer), padNumber + maxPadNumberQ1);
      fill("sTgcOccupancy", sectorMon, padNumberMon);
    }
   
    else {
      auto sectorMon    = Monitored::Scalar<int>("sector_layer_" + std::to_string(layer), sectorsTotalShifted);
      auto padNumberMon = Monitored::Scalar<int>("padNumber_layer_" + std::to_string(layer), padNumber + maxPadNumberQ1 + maxPadNumberQ2);
      fill("sTgcOccupancy", sectorMon, padNumberMon);
    }
    
    auto sectorSidedMon       = Monitored::Scalar<int>("sector_layer_" + std::to_string(layer), sector);
    auto stationEtaSidedMon   = Monitored::Scalar<int>("stationEta_layer_" + std::to_string(layer), stationEtaShifted);
    fill("sTgcQuadOccupancy", sectorSidedMon, stationEtaSidedMon);
  }
  
  else if (channelType == sTgcIdHelper::sTgcChannelTypes::Strip) {
    int stripNumber      = m_idHelperSvc -> stgcIdHelper().channel(id);
    Identifier idStripQ1 = m_idHelperSvc -> stgcIdHelper().channelID(stationName, 1, stationPhi, multiplet, gasGap, channelType, 1);    
    Identifier idStripQ2 = m_idHelperSvc -> stgcIdHelper().channelID(stationName, 2, stationPhi, multiplet, gasGap, channelType, 1);
    const MuonGM::sTgcReadoutElement* sTgcReadoutObjectStripQ1 = muonDetectorManagerObject -> getsTgcReadoutElement(idStripQ1);
    const MuonGM::sTgcReadoutElement* sTgcReadoutObjectStripQ2 = muonDetectorManagerObject -> getsTgcReadoutElement(idStripQ2);    
    int maxStripNumberQ1 = sTgcReadoutObjectStripQ1 -> numberOfStrips(idStripQ1);
    int maxStripNumberQ2 = sTgcReadoutObjectStripQ2 -> numberOfStrips(idStripQ2);
            
    if (std::abs(stationEta) == 1) {
      auto sectorMon      = Monitored::Scalar<int>("sector_layer_" + std::to_string(layer), sectorsTotalShifted);
      auto stripNumberMon = Monitored::Scalar<int>("stripNumber_layer_" + std::to_string(layer), stripNumber);
      fill("sTgcOccupancy", sectorMon, stripNumberMon);
    }
    
    else if (std::abs(stationEta) == 2) {
      auto sectorMon      = Monitored::Scalar<int>("sector_layer_" + std::to_string(layer), sectorsTotalShifted);
      auto stripNumberMon = Monitored::Scalar<int>("stripNumber_layer_" + std::to_string(layer), stripNumber + maxStripNumberQ1 + 1);
      fill("sTgcOccupancy", sectorMon, stripNumberMon);
    }
    
    else {
      auto sectorMon      = Monitored::Scalar<int>("sector_layer_" + std::to_string(layer), sectorsTotalShifted);
      auto stripNumberMon = Monitored::Scalar<int>("stripNumber_layer_" + std::to_string(layer), stripNumber + maxStripNumberQ1 + maxStripNumberQ2 + 1);
      fill("sTgcOccupancy", sectorMon, stripNumberMon);
    }
    
    auto sectorSidedMon         = Monitored::Scalar<int>("sector_layer_" + std::to_string(layer), sector);
    auto stationEtaSidedMon     = Monitored::Scalar<int>("stationEta_layer_" + std::to_string(layer), stationEtaShifted); 
    fill("sTgcQuadOccupancy", sectorSidedMon, stationEtaSidedMon);
  }
  
  else if (channelType == sTgcIdHelper::sTgcChannelTypes::Wire) {
    int wireGroupNumber      = m_idHelperSvc -> stgcIdHelper().channel(id);
    Identifier idWireGroupQ3 = m_idHelperSvc -> stgcIdHelper().channelID("STL", 3, stationPhi, 1, 3, channelType, 1);
    const MuonGM::sTgcReadoutElement* sTgcReadoutObjectWireGroupQ3 = muonDetectorManagerObject -> getsTgcReadoutElement(idWireGroupQ3);
    int maxWireGroupNumberQ3 = sTgcReadoutObjectWireGroupQ3 -> numberOfStrips(idWireGroupQ3);
    
    auto stationEtaMon      = Monitored::Scalar<int>("stationEta_layer_" + std::to_string(layer), stationEtaShifted);
    auto wireGroupNumberMon = Monitored::Scalar<int>("wireGroupNumber_layer_" + std::to_string(layer), wireGroupNumber + (sector - 1)*maxWireGroupNumberQ3); 
    fill("sTgcOccupancy", stationEtaMon, wireGroupNumberMon);

    auto sectorSidedMon             = Monitored::Scalar<int>("sector_layer_" + std::to_string(layer), sector);
    auto stationEtaSidedMon         = Monitored::Scalar<int>("stationEta_layer_" + std::to_string(layer), stationEtaShifted); 
    fill("sTgcQuadOccupancy", sectorSidedMon, stationEtaSidedMon);
  }
}

void sTgcRawDataMonAlg::fillsTgcLumiblockHistograms(const Muon::sTgcPrepData* sTgcObject, int lb) const {
  Identifier id = sTgcObject -> identify();

  if(!id.is_valid()) {
    ATH_MSG_DEBUG("Invalid identifier found in Muon::sTgcPrepData");
    return;
  }
  
  std::string stationName = m_idHelperSvc -> stgcIdHelper().stationNameString(m_idHelperSvc -> stgcIdHelper().stationName(id));
  int stationEta          = m_idHelperSvc -> stgcIdHelper().stationEta(id);
  int multiplet           = m_idHelperSvc -> stgcIdHelper().multilayer(id);
  int gasGap              = m_idHelperSvc -> stgcIdHelper().gasGap(id);    
  int channelType         = m_idHelperSvc -> stgcIdHelper().channelType(id);
  int layer               = getLayer(multiplet, gasGap);
  int sectorsTotal        = getSectors(id);
  int sectorsTotalShifted = (sectorsTotal < 0) ? sectorsTotal - 1: sectorsTotal;

  if (channelType == sTgcIdHelper::sTgcChannelTypes::Pad) {
    auto padStationEtaMon = Monitored::Scalar<int>("padStationEta_quad_" + std::to_string(std::abs(stationEta)) + "_layer_" + std::to_string(layer), sectorsTotalShifted);
    auto padLumiblockMon  = Monitored::Scalar<int>("padLumiblock_quad_" + std::to_string(std::abs(stationEta)) + "_layer_" + std::to_string(layer), lb);
    fill("sTgcLumiblockPad_quad_" + std::to_string(std::abs(stationEta)), padStationEtaMon, padLumiblockMon);
  }

  else if (channelType == sTgcIdHelper::sTgcChannelTypes::Strip) {
    auto stripStationEtaMon = Monitored::Scalar<int>("stripStationEta_quad_" + std::to_string(std::abs(stationEta)) + "_layer_" + std::to_string(layer), sectorsTotalShifted);
    auto stripLumiblockMon  = Monitored::Scalar<int>("stripLumiblock_quad_" + std::to_string(std::abs(stationEta)) + "_layer_" + std::to_string(layer), lb);
    fill("sTgcLumiblockStrip_quad_" + std::to_string(std::abs(stationEta)), stripStationEtaMon, stripLumiblockMon);
  }

  else if (channelType == sTgcIdHelper::sTgcChannelTypes::Wire) {
    auto wireStationEtaMon = Monitored::Scalar<int>("wireStationEta_quad_" + std::to_string(std::abs(stationEta)) + "_layer_" + std::to_string(layer), sectorsTotalShifted);
    auto wireLumiblockMon  = Monitored::Scalar<int>("wireLumiblock_quad_" + std::to_string(std::abs(stationEta)) + "_layer_" + std::to_string(layer), lb);
    fill("sTgcLumiblockWire_quad_" + std::to_string(std::abs(stationEta)), wireStationEtaMon, wireLumiblockMon);
  }
}

void sTgcRawDataMonAlg::fillsTgcClusterFromTrackHistograms(const xAOD::TrackParticleContainer*  trkPartCont) const {
  for (const xAOD::TrackParticle* meTP : *trkPartCont) {
    const Trk::Track* meTrack = meTP -> track();
    if(!meTrack) continue;
 
    for (const Trk::TrackStateOnSurface* trk_state : *meTrack -> trackStateOnSurfaces()) {
      const Trk::MeasurementBase* it = trk_state -> measurementOnTrack();
      if(!it) continue;
      
      const Trk::RIO_OnTrack* rot = dynamic_cast<const Trk::RIO_OnTrack*>(it);
      if(!rot) continue;
      
      Identifier rot_id = rot -> identify();

      if(!rot_id.is_valid()) {
	ATH_MSG_DEBUG("Invalid identifier found in Trk::RIO_OnTrack");
	continue;
      }

      if(!m_idHelperSvc -> issTgc(rot_id)) continue;
      
      const Muon::sTgcClusterOnTrack* cluster = dynamic_cast<const Muon::sTgcClusterOnTrack*>(rot);
      if(!cluster) continue;
      
      const Muon::sTgcPrepData* prd = cluster -> prepRawData();
      if (!prd) continue;

      int channelType = m_idHelperSvc  -> stgcIdHelper().channelType(rot_id);
      int stEta       =  m_idHelperSvc -> stgcIdHelper().stationEta(rot_id);
      int multi       = m_idHelperSvc  -> stgcIdHelper().multilayer(rot_id);
      int gap         = m_idHelperSvc  -> stgcIdHelper().gasGap(rot_id);
      int sector      = m_idHelperSvc -> sector(rot_id);
      int sectorsTotal        = getSectors(rot_id);
      int sectorsTotalShifted = (sectorsTotal < 0) ? sectorsTotal - 1: sectorsTotal;
      int layer               = getLayer(multi, gap);
      int iside               = (stEta > 0) ? 1 : 0;
      std::string side        = GeometricSectors::sTgcSide[iside];

      if (channelType == sTgcIdHelper::sTgcChannelTypes::Pad) {
	float padCharge     = prd -> charge();
	auto padChargeMon = Monitored::Scalar<float>("padTrackCharge_" + side + "_quad_" + std::to_string(std::abs(stEta)) + "_sector_" + std::to_string(sector)  + "_layer_" + std::to_string(layer), padCharge);
	fill("padCharge_" + side + std::to_string(sector) + "_quad_" + std::to_string(std::abs(stEta)), padChargeMon);

	short int padTiming = prd -> time();
	auto padSectorSidedMon = Monitored::Scalar<int>("padTrackSectorSided_layer_" + std::to_string(layer), sectorsTotalShifted);
	auto padTimingMon      = Monitored::Scalar<float>("padTrackTiming_layer_" + std::to_string(layer), padTiming);
	fill("sTgcTiming", padSectorSidedMon, padTimingMon);
        
	auto padSectorSidedExpertMon = Monitored::Scalar<int>("padTrackSectorSided_quad_" + std::to_string(std::abs(stEta)) + "_layer_" + std::to_string(layer), sectorsTotalShifted);
	auto padTimingExpertMon      = Monitored::Scalar<float>("padTrackTiming_quad_" + std::to_string(std::abs(stEta)) + "_layer_" + std::to_string(layer), padTiming);
	fill("padTiming_quad_" + std::to_string(std::abs(stEta)), padSectorSidedExpertMon, padTimingExpertMon);
      }
      
      else if (channelType == sTgcIdHelper::sTgcChannelTypes::Strip) {
	const std::vector<Identifier>& stripIds = prd->rdoList();
	unsigned int csize = stripIds.size();
	
	std::vector<short int> stripTimesVec = prd -> stripTimes();
	std::vector<int> stripChargesVec = prd -> stripCharges();

	float stripClusterTimes = 0;
	float stripClusterCharges = 0;
	
	for (unsigned int sIdx = 0; sIdx < csize; ++sIdx) {
	  stripClusterTimes += stripTimesVec.at(sIdx);
	  stripClusterCharges += stripChargesVec.at(sIdx);
	}
	
	stripClusterTimes /= stripTimesVec.size();
	
	auto stripClusterChargesPerSideQuadMon = Monitored::Scalar<float>("stripTrackCharge_" + side  + "_quad_" + std::to_string(std::abs(stEta)) + "_sector_" + std::to_string(sector)  +  "_layer_" + std::to_string(layer), stripClusterCharges);
	fill("stripCharge_" + side + std::to_string(sector) + "_quad_" + std::to_string(std::abs(stEta)), stripClusterChargesPerSideQuadMon);
	
	auto stripClusterSectorSidedMon = Monitored::Scalar<int>("stripTrackSectorSided_layer_" + std::to_string(layer), sectorsTotalShifted);
	auto stripClusterTimesMon       = Monitored::Scalar<float>("stripTrackTiming_layer_" + std::to_string(layer), stripClusterTimes);
	auto stripClusterSizeMon        = Monitored::Scalar<unsigned int>("stripTrackClusterSize_layer_" + std::to_string(layer), csize);
	fill("sTgcTiming", stripClusterSectorSidedMon, stripClusterTimesMon);
	fill("sTgcOverview", stripClusterSectorSidedMon, stripClusterTimesMon, stripClusterSizeMon);
      
	auto stripSectorSidedExpertMon = Monitored::Scalar<int>("stripTrackSectorSided_quad_" + std::to_string(std::abs(stEta)) + "_layer_" + std::to_string(layer), sectorsTotalShifted);
	auto stripTimingExpertMon      = Monitored::Scalar<float>("stripTrackTiming_quad_" + std::to_string(std::abs(stEta)) + "_layer_" + std::to_string(layer), stripClusterTimes);
	fill("stripTiming_quad_" + std::to_string(std::abs(stEta)), stripSectorSidedExpertMon, stripTimingExpertMon);


	std::unique_ptr<const Trk::ResidualPull> resPull(m_residualPullCalculator -> residualPull(trk_state -> measurementOnTrack(), trk_state -> trackParameters(), Trk::ResidualPull::ResidualType::Biased));	

	if (resPull) {
	  float residual = resPull -> residual()[Trk::locX];
	  auto residualMon  = Monitored::Scalar<float>("residual_" + side + "_quad_" + std::to_string(std::abs(stEta)) + "_sector_" + std::to_string(sector) + "_layer_" + std::to_string(layer), residual);
	  fill("sTgcResiduals_" + side + std::to_string(sector) + "_quad_" + std::to_string(std::abs(stEta)), residualMon);	    
	}
      }

      else if (channelType == sTgcIdHelper::sTgcChannelTypes::Wire) {
	float wireGroupCharge     = prd -> charge();
	auto wireGroupChargeMon = Monitored::Scalar<float>("wireGroupTrackCharge_" + side + "_quad_" + std::to_string(std::abs(stEta)) + "_sector_" + std::to_string(sector)  + "_layer_" + std::to_string(layer), wireGroupCharge);
	fill("wireGroupCharge_" + side + std::to_string(sector) + "_quad_" + std::to_string(std::abs(stEta)), wireGroupChargeMon);
	
	short int wireGroupTiming = prd -> time();
	auto wireGroupSectorSidedMon = Monitored::Scalar<int>("wireGroupTrackSectorSided_layer_" + std::to_string(layer), sectorsTotalShifted);
	auto wireGroupTimingMon      = Monitored::Scalar<float>("wireGroupTrackTiming_layer_" + std::to_string(layer), wireGroupTiming);
	fill("sTgcTiming", wireGroupSectorSidedMon, wireGroupTimingMon);
      
	auto wireSectorSidedExpertMon = Monitored::Scalar<int>("wireTrackSectorSided_quad_" + std::to_string(std::abs(stEta)) + "_layer_" + std::to_string(layer), sectorsTotalShifted);
	auto wireTimingExpertMon      = Monitored::Scalar<float>("wireTrackTiming_quad_" + std::to_string(std::abs(stEta)) + "_layer_" + std::to_string(layer), wireGroupTiming);
	fill("wireTiming_quad_" + std::to_string(std::abs(stEta)), wireSectorSidedExpertMon, wireTimingExpertMon);
      }
    }
  }
}

void sTgcRawDataMonAlg::fillsTgcPadTriggerDataHistograms(const Muon::NSW_PadTriggerDataContainer* NSWpadTriggerObject, int lb) const {
  for (const Muon::NSW_PadTriggerData* rdo : *NSWpadTriggerObject ) {
    for (size_t it = 0; it < rdo -> getNumberOfTriggers(); ++it) {
      std::vector<unsigned int> phiIds  = rdo -> getTriggerPhiIds();
      std::vector<unsigned int> bandIds = rdo -> getTriggerBandIds();
      std::vector<unsigned int> relBCID = rdo -> getTriggerRelBcids();
      std::vector<unsigned int> hitpfeb = rdo -> getHitPfebs();

      bool sideA = rdo -> sideA();
      bool largeSector = rdo -> largeSector();
      
      int iside = (sideA) ? 1 : 0;
      int isize = (largeSector) ? 1 : 0; 
      
      std::string side = GeometricSectors::sTgcSide[iside];
      std::string size = GeometricSectors::sTgcSize[isize];

      unsigned int sourceId = rdo -> getSourceid();
      int sectorNumber = sourceidToSector(sourceId, sideA);
      unsigned int numberOfTriggers = rdo -> getNumberOfTriggers();

      auto phiIdsSidedSizedMon  = Monitored::Collection("phiIds_"  + side + "_" + size, phiIds);
      auto bandIdsSidedSizedMon = Monitored::Collection("bandIds_" + side + "_" + size, bandIds);
      fill("padTriggerShifter", phiIdsSidedSizedMon, bandIdsSidedSizedMon);

      auto relBCIDMon = Monitored::Collection("relBCID", relBCID);
      auto hitpfebMon = Monitored::Collection("hitpfeb", hitpfeb);
      auto lbMon = Monitored::Scalar<int>("lb", lb);
      auto sectorMon = Monitored::Scalar<int>("sector", sectorNumber);
      auto numberOfTriggersMon = Monitored::Scalar<int>("numberOfTriggers", numberOfTriggers);
      fill("padTriggerShifter", relBCIDMon, lbMon, sectorMon, numberOfTriggersMon, hitpfebMon);

      auto lbPerSectorMon = Monitored::Scalar<int>("lb_" + side + "_sector_" + std::to_string(std::abs(sectorNumber)), lb);
      auto bandIDperSectorMon = Monitored::Collection("bandIds_" + side + "_sector_" + std::to_string(std::abs(sectorNumber)), bandIds);
      auto numberOfTriggersPerSectorMon = Monitored::Scalar<int>("numberOfTriggers_" + side + "_sector_" + std::to_string(std::abs(sectorNumber)), numberOfTriggers);
      auto phiIdsSidedSizedPerSectorMon  = Monitored::Collection("phiIds_"  + side + "_sector_" + std::to_string(std::abs(sectorNumber)), phiIds);
      auto bandIdsSidedSizedPerSectorMon = Monitored::Collection("bandIds_" + side + "_sector_" + std::to_string(std::abs(sectorNumber)), bandIds);
      fill("padTriggerExpert", lbPerSectorMon, bandIDperSectorMon, numberOfTriggersPerSectorMon, phiIdsSidedSizedPerSectorMon, bandIdsSidedSizedPerSectorMon);
    }
  }
}

void sTgcRawDataMonAlg::fillsTgcEfficiencyHistograms(const xAOD::MuonContainer*  muonContainer, const MuonGM::MuonDetectorManager* muonDetectorManagerObject) const {
  sTGCeff effPlots[2][3][16][2]; 
  unsigned int stPhi = 0;
  std::string largerSector = "", smallSector = "";
  std::string stName = "";
  
  for (const xAOD::Muon* mu : *muonContainer) {
    if(mu -> pt() < m_cutPt) continue; 
    int author = mu -> author();
    if(!(author == xAOD::Muon_v1::MuonType::Combined || author == xAOD::Muon_v1::MuonType::MuonStandAlone)) continue; 

    const xAOD::TrackParticle* meTP = mu -> trackParticle(xAOD::Muon::TrackParticleType::ExtrapolatedMuonSpectrometerTrackParticle);
    if(meTP == nullptr) continue;

    const Trk::Track* meTrack = meTP -> track();
    if(!meTrack) continue;

    for(const Trk::TrackStateOnSurface* trkState : *meTrack->trackStateOnSurfaces()) {
      if (!trkState->type(Trk::TrackStateOnSurface::Measurement)) continue;

      Identifier surfaceId = (trkState) -> surface().associatedDetectorElementIdentifier();
      if(!m_idHelperSvc -> issTgc(surfaceId)) continue;

      const Trk::MeasurementBase* meas = trkState->measurementOnTrack();
      if(!meas) continue;
      
      const Trk::RIO_OnTrack* rot = dynamic_cast<const Trk::RIO_OnTrack*>(meas);
      if(!rot) continue;

      Identifier rot_id = rot -> identify();
      if(!m_idHelperSvc -> issTgc(rot_id)) continue;
      
      stName = m_idHelperSvc -> stgcIdHelper().stationNameString(m_idHelperSvc -> stgcIdHelper().stationName(rot_id));
      stPhi           = m_idHelperSvc -> stgcIdHelper().stationPhi(rot_id);
      int stEta       = m_idHelperSvc -> stgcIdHelper().stationEta(rot_id);
      int multi       = m_idHelperSvc -> stgcIdHelper().multilayer(rot_id);
      int gap         = m_idHelperSvc -> stgcIdHelper().gasGap(rot_id);
      int sector      = m_idHelperSvc -> sector(rot_id);
      int channelType = m_idHelperSvc  -> stgcIdHelper().channelType(rot_id);
      int iside       = (stEta > 0) ? 1 : 0;

      if (m_idHelperSvc -> isSmallChamber(rot_id)) {
	smallSector = stName;
      }
      
      else {
	largerSector = stName;
      }
      
      const Amg::Vector3D& positionsMultiplet = (trkState) -> trackParameters() -> position();      
      float xPosStripInMultiplet = positionsMultiplet.x();   
      float yPosStripInMultiplet = positionsMultiplet.y();
      float zPosStripInMultiplet = positionsMultiplet.z();
      float rPosStripInMultiplet = std::hypot(xPosStripInMultiplet, yPosStripInMultiplet);   
      
      if( ! (std::find(effPlots[iside][std::abs(stEta) - 1][sector - 1][multi - 1].layerMultiplet.begin(), effPlots[iside][std::abs(stEta) - 1][sector - 1][multi - 1].layerMultiplet.end(), gap) != effPlots[iside][std::abs(stEta) - 1][sector - 1][multi - 1].layerMultiplet.end()) ) {
	if (channelType == sTgcIdHelper::sTgcChannelTypes::Strip) {
	  effPlots[iside][std::abs(stEta) - 1][sector - 1][multi - 1].layerMultiplet.push_back(gap);
	  effPlots[iside][std::abs(stEta) - 1][sector - 1][multi - 1].xPosMultiplet.push_back(xPosStripInMultiplet);
	  effPlots[iside][std::abs(stEta) - 1][sector - 1][multi - 1].yPosMultiplet.push_back(yPosStripInMultiplet);
	  effPlots[iside][std::abs(stEta) - 1][sector - 1][multi - 1].zPosMultiplet.push_back(zPosStripInMultiplet);
	  effPlots[iside][std::abs(stEta) - 1][sector - 1][multi - 1].rPosMultiplet.push_back(rPosStripInMultiplet);
	}
      }
    }
  }
  
  for (unsigned int isideIndex = 0; isideIndex <= 1; ++isideIndex) {
    for (unsigned int stEtaIndex = 1; stEtaIndex <= 3; ++stEtaIndex) {
      for (unsigned int sectorIndex = 1; sectorIndex <= 16; ++sectorIndex) {
	for (unsigned int multiIndex = 1; multiIndex <= 2; ++multiIndex) { 
	  if (effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].layerMultiplet.size() == 4) {	
	    for (unsigned int gapIndex = 1; gapIndex <= effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].layerMultiplet.size(); ++gapIndex) {
	      float xPos = effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].xPosMultiplet.at(gapIndex - 1);
	      float yPos = effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].yPosMultiplet.at(gapIndex - 1);
	      float rPos = effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].rPosMultiplet.at(gapIndex - 1);
	      
	      std::string side = GeometricSectors::sTgcSide[isideIndex];
	      int quad = (side == "A") ? stEtaIndex : -stEtaIndex;
	      
	      bool isValid = false;
	      const Identifier IDeffLayerStrip = m_idHelperSvc -> stgcIdHelper().channelID((stName == "STS") ? smallSector : largerSector, quad, stPhi, multiIndex, gapIndex, sTgcIdHelper::sTgcChannelTypes::Strip, 1, isValid);
	     
	      if (!isValid) { 
		ATH_MSG_DEBUG("Identifier of eff layer isn't valid"); 
		continue;
	      }
	      
	      int gasGapEff = m_idHelperSvc -> stgcIdHelper().gasGap(IDeffLayerStrip);
	      int layerEff  = getLayer(multiIndex, gasGapEff);
	      
	      auto effQuestion = true;
	      auto effQuestionMon = Monitored::Scalar<bool>("hitLayer", effQuestion);
	      
	      auto rPosStripmon = Monitored::Scalar<float>("rPosStrip_" + side + "_sector_" + std::to_string(sectorIndex)  + "_layer_" + std::to_string(layerEff), rPos);
	      fill("rPosStrip_" + side + std::to_string(sectorIndex), rPosStripmon, effQuestionMon);
	      
	      auto xPosStripmon = Monitored::Scalar<float>("xPosStrip_" + side + "_layer_" + std::to_string(layerEff), xPos); 
	      auto yPosStripmon = Monitored::Scalar<float>("yPosStrip_" + side + "_layer_" + std::to_string(layerEff), yPos);
	      fill("efficiencyOverview", xPosStripmon, yPosStripmon, effQuestionMon);
	      fill("sTgcOverview", xPosStripmon, yPosStripmon, effQuestionMon);
	    }
	  } // close 4 out 4 if case
	  
	  else if (effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].layerMultiplet.size() == 3) {
	    std::vector<int> refLayers(4, 0);
	    
	    refLayers.at(effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].layerMultiplet.at(0) - 1) = effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].layerMultiplet.at(0);
	    refLayers.at(effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].layerMultiplet.at(1) - 1) = effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].layerMultiplet.at(1);
	    refLayers.at(effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].layerMultiplet.at(2) - 1) = effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].layerMultiplet.at(2);
	    
	    int probeL1 = -999;
	    
	    for (long unsigned int layerIt = 0; layerIt < refLayers.size(); ++layerIt) {
	      if (refLayers.at(layerIt) == 0) {probeL1 = layerIt + 1;}
	    }
	    
	    std::string side = GeometricSectors::sTgcSide[isideIndex];
	    int quad = (side == "A") ? stEtaIndex : -stEtaIndex;

	    bool isValid = false;
	    const Identifier idProbeL1 = m_idHelperSvc -> stgcIdHelper().channelID((stName == "STS") ? smallSector : largerSector, quad, stPhi, multiIndex, probeL1, sTgcIdHelper::sTgcChannelTypes::Strip, 1, isValid);
	    
	    if (!isValid) { 
	      ATH_MSG_DEBUG("Identifier of probe L1 is invalid"); 
	      continue;
	    }

	    Amg::Vector3D posProbeL1{Amg::Vector3D::Zero()};
	    (muonDetectorManagerObject -> getsTgcReadoutElement(idProbeL1)) -> stripGlobalPosition(idProbeL1, posProbeL1); 
	    float posZprobeL1 = posProbeL1.z();
	    
	    float xSlope = (effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].xPosMultiplet.at(0) - effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].xPosMultiplet.at(1))/(effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].zPosMultiplet.at(0) - effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].zPosMultiplet.at(1));
	    float ySlope = (effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].yPosMultiplet.at(0) - effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].yPosMultiplet.at(1))/(effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].zPosMultiplet.at(0) - effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].zPosMultiplet.at(1));
	    float rSlope = (effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].rPosMultiplet.at(0) - effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].rPosMultiplet.at(1))/(effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].zPosMultiplet.at(0) - effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].zPosMultiplet.at(1));
	    
	    float xPosProbeLayer1 = effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].xPosMultiplet.at(0) + xSlope*(posZprobeL1 - effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].zPosMultiplet.at(0));
	    float yPosProbeLayer1 = effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].yPosMultiplet.at(0) + ySlope*(posZprobeL1 - effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].zPosMultiplet.at(0));
	    float rPosProbeLayer1 = effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].rPosMultiplet.at(0) + rSlope*(posZprobeL1 - effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].zPosMultiplet.at(0));
	    
	    int layerProbeL1 = getLayer(multiIndex, probeL1);
	    
	    auto effQuestion = false;
	    auto effQuestionMon = Monitored::Scalar<bool>("hitLayer", effQuestion);
	    
	    auto rPosStripProbeL1mon = Monitored::Scalar<float>("rPosStrip_" + side + "_sector_" + std::to_string(sectorIndex)  + "_layer_" + std::to_string(layerProbeL1), rPosProbeLayer1);
	    fill("rPosStrip_" + side + std::to_string(sectorIndex), rPosStripProbeL1mon, effQuestionMon);
	      
	    auto xPosStripProbeL1mon = Monitored::Scalar<float>("xPosStrip_" + side + "_layer_" + std::to_string(layerProbeL1), xPosProbeLayer1); 
	    auto yPosStripProbeL1mon = Monitored::Scalar<float>("yPosStrip_" + side + "_layer_" + std::to_string(layerProbeL1), yPosProbeLayer1);
	    fill("efficiencyOverview", xPosStripProbeL1mon, yPosStripProbeL1mon, effQuestionMon);
	    fill("sTgcOverview", xPosStripProbeL1mon, yPosStripProbeL1mon, effQuestionMon);
	    
	  } // close 3 out 4 if case
	  
	  else if (effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].layerMultiplet.size() == 2) {
	    std::vector<int> refLayers(4, 0);
	    
	    refLayers.at(effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].layerMultiplet.at(0) - 1) = effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].layerMultiplet.at(0);
	    refLayers.at(effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].layerMultiplet.at(1) - 1) = effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].layerMultiplet.at(1);
	      
	    std::vector<int> probeLayers;
	    
	    for (long unsigned int layerIt = 0; layerIt < refLayers.size(); ++layerIt) {
	      if (refLayers.at(layerIt) == 0) {probeLayers.push_back(layerIt + 1);}
	    }
	    
	    int probeL1 = probeLayers.at(0);
	    int probeL2 = probeLayers.at(1);
	    
	    std::string side = GeometricSectors::sTgcSide[isideIndex];
	    int quad = (side == "A") ? stEtaIndex : -stEtaIndex;
	    
	    bool isValid = false;
	    
	    const Identifier idProbeL1 = m_idHelperSvc -> stgcIdHelper().channelID((stName == "STS") ? smallSector : largerSector, quad, stPhi, multiIndex, probeL1, sTgcIdHelper::sTgcChannelTypes::Strip, 1, isValid);
	    
	    if (!isValid) {
	      ATH_MSG_DEBUG("Identifier of probe L1 is invalid"); 
	      continue;
	    }
	      
	    const Identifier idProbeL2 = m_idHelperSvc -> stgcIdHelper().channelID((stName == "STS") ? smallSector : largerSector, quad, stPhi, multiIndex, probeL2, sTgcIdHelper::sTgcChannelTypes::Strip, 1, isValid);
	    
	    if (!isValid) { 
	      ATH_MSG_DEBUG("Identifier of probe L2 is invalid"); 
	      continue;
	    }
	    
	    Amg::Vector3D posProbeL1{Amg::Vector3D::Zero()};
	    (muonDetectorManagerObject -> getsTgcReadoutElement(idProbeL1)) -> stripGlobalPosition(idProbeL1, posProbeL1); 
	    float posZprobeL1 = posProbeL1.z();
	    
	    Amg::Vector3D posProbeL2{Amg::Vector3D::Zero()};
	    (muonDetectorManagerObject -> getsTgcReadoutElement(idProbeL2)) -> stripGlobalPosition(idProbeL2, posProbeL2); 
	    float posZprobeL2 = posProbeL2.z();
	    
	    float xSlope = (effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].xPosMultiplet.at(0) - effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].xPosMultiplet.at(1))/(effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].zPosMultiplet.at(0) - effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].zPosMultiplet.at(1));
	    float ySlope = (effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].yPosMultiplet.at(0) - effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].yPosMultiplet.at(1))/(effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].zPosMultiplet.at(0) - effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].zPosMultiplet.at(1));
	    float rSlope = (effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].rPosMultiplet.at(0) - effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].rPosMultiplet.at(1))/(effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].zPosMultiplet.at(0) - effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].zPosMultiplet.at(1));
	    
	    float xPosProbeLayer1 = effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].xPosMultiplet.at(0) + xSlope*(posZprobeL1 - effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].zPosMultiplet.at(0));
	    float yPosProbeLayer1 = effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].yPosMultiplet.at(0) + ySlope*(posZprobeL1 - effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].zPosMultiplet.at(0));
	    float rPosProbeLayer1 = effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].rPosMultiplet.at(0) + rSlope*(posZprobeL1 - effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].zPosMultiplet.at(0));
	    
	    float xPosProbeLayer2 = effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].xPosMultiplet.at(0) + xSlope*(posZprobeL2 - effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].zPosMultiplet.at(0));
	    float yPosProbeLayer2 = effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].yPosMultiplet.at(0) + ySlope*(posZprobeL2 - effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].zPosMultiplet.at(0));
	    float rPosProbeLayer2 = effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].rPosMultiplet.at(0) + rSlope*(posZprobeL2 - effPlots[isideIndex][stEtaIndex - 1][sectorIndex - 1][multiIndex - 1].zPosMultiplet.at(0));
	    
	    int layerProbeL1   = getLayer(multiIndex, probeL1);    
	    int layerProbeL2   = getLayer(multiIndex, probeL2);
	    
	    auto effQuestion = false;
	    auto effQuestionMon = Monitored::Scalar<bool>("hitLayer", effQuestion);
	    
	    auto rPosStripProbeL1mon = Monitored::Scalar<float>("rPosStrip_" +  side  + "_sector_" + std::to_string(sectorIndex)  + "_layer_" + std::to_string(layerProbeL1), rPosProbeLayer1);
	    fill("rPosStrip_" + side + std::to_string(sectorIndex), rPosStripProbeL1mon, effQuestionMon);
	    
	    auto xPosStripProbeL1mon = Monitored::Scalar<float>("xPosStrip_" + side + "_layer_" + std::to_string(layerProbeL1), xPosProbeLayer1); 
	    auto yPosStripProbeL1mon = Monitored::Scalar<float>("yPosStrip_" + side + "_layer_" + std::to_string(layerProbeL1), yPosProbeLayer1);
	    fill("efficiencyOverview", xPosStripProbeL1mon, yPosStripProbeL1mon, effQuestionMon);
	    fill("sTgcOverview", xPosStripProbeL1mon, yPosStripProbeL1mon, effQuestionMon);
	    
	    auto rPosStripProbeL2mon = Monitored::Scalar<float>("rPosStrip_" +  side +  "_sector_" + std::to_string(sectorIndex)  + "_layer_" + std::to_string(layerProbeL2), rPosProbeLayer2);
	    fill("rPosStrip_" + side + std::to_string(sectorIndex), rPosStripProbeL2mon, effQuestionMon);
	    
	    auto xPosStripProbeL2mon = Monitored::Scalar<float>("xPosStrip_" + side + "_layer_" + std::to_string(layerProbeL2), xPosProbeLayer2); 
	    auto yPosStripProbeL2mon = Monitored::Scalar<float>("yPosStrip_" + side + "_layer_" + std::to_string(layerProbeL2), yPosProbeLayer2);
	    fill("efficiencyOverview", xPosStripProbeL2mon, yPosStripProbeL2mon, effQuestionMon);
	    fill("sTgcOverview", xPosStripProbeL2mon, yPosStripProbeL2mon, effQuestionMon); 
	  } // close 2 out 4 if case 
	} // multiIndex loop end
      } // sectorIndex loop end
    } // stEtaIndex loop end
  } // isideIndex loop end
}


