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
  return StatusCode::SUCCESS;
} 

StatusCode sTgcRawDataMonAlg::fillHistograms(const EventContext& ctx) const {  
  SG::ReadHandle<Muon::sTgcPrepDataContainer> sTgcContainer(m_sTgcContainerKey, ctx);
  SG::ReadCondHandle<MuonGM::MuonDetectorManager> detectorManagerKey(m_detectorManagerKey, ctx); 
  SG::ReadHandle<xAOD::TrackParticleContainer> meTPContainer(m_meTrkKey, ctx);

  if (!meTPContainer.isValid()) {
    ATH_MSG_FATAL("Could not get track particle container: " << m_meTrkKey.fullKey());
    return StatusCode::FAILURE;
  }
 
  fillsTgcClusterFromTrackHistograms(meTPContainer.cptr());
 
  const int lumiblock = GetEventInfo(ctx) -> lumiBlock();
  
  if (m_dosTgcESD && m_dosTgcOverview) {
    for(const Muon::sTgcPrepDataCollection* coll : *sTgcContainer) {
      for (const Muon::sTgcPrepData* prd : *coll) {
	fillsTgcOccupancyHistograms(prd, detectorManagerKey.cptr());
	fillsTgcLumiblockHistograms(prd, lumiblock);
      }
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
  int sector              = m_idHelperSvc -> sector(id);
  int layer               = getLayer(multiplet, gasGap);
  int stationEtaShiftedModified = (stationEta < 0) ? stationEta - 1 - 3*(sector - 1) : stationEta + 3*(sector - 1);

  if (channelType == sTgcIdHelper::sTgcChannelTypes::Pad) {
    auto padStationEtaMon = Monitored::Scalar<int>("padStationEta_layer_" + std::to_string(layer), stationEtaShiftedModified);
    auto padLumiblockMon  = Monitored::Scalar<int>("padLumiblock_layer_" + std::to_string(layer), lb);
    fill("sTgcLumiblock", padStationEtaMon, padLumiblockMon);
  }

  else if (channelType == sTgcIdHelper::sTgcChannelTypes::Strip) {
    auto stripStationEtaMon = Monitored::Scalar<int>("stripStationEta_layer_" + std::to_string(layer), stationEtaShiftedModified);
    auto stripLumiblockMon  = Monitored::Scalar<int>("stripLumiblock_layer_" + std::to_string(layer), lb);
    fill("sTgcLumiblock", stripStationEtaMon, stripLumiblockMon);
  }

  else if (channelType == sTgcIdHelper::sTgcChannelTypes::Wire) {
    auto wireGroupStationEtaMon = Monitored::Scalar<int>("wireGroupStationEta_layer_" + std::to_string(layer), stationEtaShiftedModified);
    auto wireGroupLumiblockMon  = Monitored::Scalar<int>("wireGroupLumiblock_layer_" + std::to_string(layer), lb);
    fill("sTgcLumiblock", wireGroupStationEtaMon, wireGroupLumiblockMon);
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



