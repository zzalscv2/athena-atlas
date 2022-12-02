/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
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
  ATH_CHECK(m_segmentManagerKey.initialize());
  
  return StatusCode::SUCCESS;
} 

StatusCode sTgcRawDataMonAlg::fillHistograms(const EventContext& ctx) const {  
  SG::ReadHandle<Muon::sTgcPrepDataContainer> sTgcContainer(m_sTgcContainerKey, ctx);
  SG::ReadCondHandle<MuonGM::MuonDetectorManager> detectorManagerKey(m_detectorManagerKey, ctx);
  SG::ReadHandle<Trk::SegmentCollection> segmentContainer(m_segmentManagerKey, ctx);
  
  if(!segmentContainer.isValid()) {
    ATH_MSG_ERROR("Could not get segmentContainer");      
    return StatusCode::FAILURE;
  }
  
  const int lumiblock = GetEventInfo(ctx) -> lumiBlock();
  
  if (m_dosTgcESD && m_dosTgcOverview) {
    for(const Muon::sTgcPrepDataCollection* coll : *sTgcContainer) {
      for (const Muon::sTgcPrepData* prd : *coll) {
	fillsTgcOverviewHistograms(prd, *coll);
	fillsTgcOccupancyHistograms(prd, detectorManagerKey.cptr());
	fillsTgcLumiblockHistograms(prd, lumiblock);
	fillsTgcTimingHistograms(prd);
      }
    }
  }
  
  fillsTgcClusterFromSegmentsHistograms(segmentContainer.cptr());
  
  return StatusCode::SUCCESS;
}

void sTgcRawDataMonAlg::fillsTgcOverviewHistograms(const Muon::sTgcPrepData* sTgcObject, const Muon::MuonPrepDataCollection<Muon::sTgcPrepData> &prd) const {   
  auto chargeMon = Monitored::Collection("charge", prd, [] (const Muon::sTgcPrepData* aux) 
					 {
					   return aux -> charge();
					 });
  
  auto numberOfStripsPerClusterMon = Monitored::Collection("numberOfStripsPerCluster", prd, [] (const Muon::sTgcPrepData* aux) 
							   {
							     const std::vector<Identifier> &stripIds = aux -> rdoList(); 
							     return stripIds.size();
							   });
  
  auto timeMon = Monitored::Collection("time", prd, [] (const Muon::sTgcPrepData* aux) 
				       {
					 return aux -> time();
				       });

  fill("sTgcOverview", chargeMon, numberOfStripsPerClusterMon, timeMon);
  
  std::vector<short int> stripTimesVec           = sTgcObject -> stripTimes();
  std::vector<int> stripChargesVec               = sTgcObject -> stripCharges();
  std::vector<short unsigned int> stripNumberVec = sTgcObject -> stripNumbers();

  auto stripTimesMon   = Monitored::Collection("stripTimes", stripTimesVec);
  auto stripChargesMon = Monitored::Collection("stripCharges", stripChargesVec);
  auto stripNumbersMon = Monitored::Collection("stripNumbers", stripNumberVec);

  fill("sTgcOverview", stripTimesMon, stripChargesMon, stripNumbersMon);

  auto xMon = Monitored::Collection("x", prd, [] (const Muon::sTgcPrepData* aux) 
				    {
				      Amg::Vector3D pos = aux -> globalPosition(); 
				      return pos.x();
				    });
  
  auto yMon = Monitored::Collection("y", prd, [] (const Muon::sTgcPrepData* aux) 
				    {
				      Amg::Vector3D pos = aux -> globalPosition(); 
				      return pos.y();
				    });
  
  auto zMon = Monitored::Collection("z", prd, [] (const Muon::sTgcPrepData* aux) 
				    {
				      Amg::Vector3D pos = aux -> globalPosition(); 
				      return pos.z();
				    });
  
  auto rMon = Monitored::Collection("r", prd, [] (const Muon::sTgcPrepData* aux) 
				    {
				      Amg::Vector3D pos = aux -> globalPosition(); 
				      return std::hypot(pos.x(), pos.y());
				    });

  fill("sTgcOverview", xMon, yMon, zMon, rMon);
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
    auto padHit        = 1;
            
    if (std::abs(stationEta) == 1) {
      auto sectorMon    = Monitored::Scalar<int>("sector_layer_" + std::to_string(layer), sectorsTotalShifted);
      auto padNumberMon = Monitored::Scalar<int>("padNumber_layer_" + std::to_string(layer), padNumber);
      auto padHitMon    = Monitored::Scalar<int>("padHit_layer_" + std::to_string(layer), (int) padHit);
      fill("sTgcOccupancy", sectorMon, padNumberMon, padHitMon);
    }
  
    else if (std::abs(stationEta) == 2) {
      auto sectorMon    = Monitored::Scalar<int>("sector_layer_" + std::to_string(layer), sectorsTotalShifted);
      auto padNumberMon = Monitored::Scalar<int>("padNumber_layer_" + std::to_string(layer), padNumber + maxPadNumberQ1);
      auto padHitMon    = Monitored::Scalar<int>("padHit_layer_" + std::to_string(layer), (int) padHit);
      fill("sTgcOccupancy", sectorMon, padNumberMon, padHitMon);
    }
   
    else {
      auto sectorMon    = Monitored::Scalar<int>("sector_layer_" + std::to_string(layer), sectorsTotalShifted);
      auto padNumberMon = Monitored::Scalar<int>("padNumber_layer_" + std::to_string(layer), padNumber + maxPadNumberQ1 + maxPadNumberQ2);
      auto padHitMon    = Monitored::Scalar<int>("padHit_layer_" + std::to_string(layer), (int) padHit);
      fill("sTgcOccupancy", sectorMon, padNumberMon, padHitMon);
    }
    
    auto sectorSidedMon       = Monitored::Scalar<int>("sector_layer_" + std::to_string(layer), sector);
    auto stationEtaSidedMon   = Monitored::Scalar<int>("stationEta_layer_" + std::to_string(layer), stationEtaShifted);
    auto padHitLayersSidedMon = Monitored::Scalar<int>("padHitLayers_layer_" + std::to_string(layer), (int) padHit);
    fill("sTgcLayers", sectorSidedMon, stationEtaSidedMon, padHitLayersSidedMon);
  }
  
  else if (channelType == sTgcIdHelper::sTgcChannelTypes::Strip) {
    int stripNumber      = m_idHelperSvc -> stgcIdHelper().channel(id);
    Identifier idStripQ1 = m_idHelperSvc -> stgcIdHelper().channelID(stationName, 1, stationPhi, multiplet, gasGap, channelType, 1);    
    Identifier idStripQ2 = m_idHelperSvc -> stgcIdHelper().channelID(stationName, 2, stationPhi, multiplet, gasGap, channelType, 1);
    const MuonGM::sTgcReadoutElement* sTgcReadoutObjectStripQ1 = muonDetectorManagerObject -> getsTgcReadoutElement(idStripQ1);
    const MuonGM::sTgcReadoutElement* sTgcReadoutObjectStripQ2 = muonDetectorManagerObject -> getsTgcReadoutElement(idStripQ2);    
    int maxStripNumberQ1 = sTgcReadoutObjectStripQ1 -> numberOfStrips(idStripQ1);
    int maxStripNumberQ2 = sTgcReadoutObjectStripQ2 -> numberOfStrips(idStripQ2);
    auto stripHit        = 1;
            
    if (std::abs(stationEta) == 1) {
      auto sectorMon      = Monitored::Scalar<int>("sector_layer_" + std::to_string(layer), sectorsTotalShifted);
      auto stripNumberMon = Monitored::Scalar<int>("stripNumber_layer_" + std::to_string(layer), stripNumber);
      auto stripHitMon    = Monitored::Scalar<int>("stripHit_layer_" + std::to_string(layer), (int) stripHit);
      fill("sTgcOccupancy", sectorMon, stripNumberMon, stripHitMon);
    }
    
    else if (std::abs(stationEta) == 2) {
      auto sectorMon      = Monitored::Scalar<int>("sector_layer_" + std::to_string(layer), sectorsTotalShifted);
      auto stripNumberMon = Monitored::Scalar<int>("stripNumber_layer_" + std::to_string(layer), stripNumber + maxStripNumberQ1 + 1);
      auto stripHitMon    = Monitored::Scalar<int>("stripHit_layer_" + std::to_string(layer), (int) stripHit);
      fill("sTgcOccupancy", sectorMon, stripNumberMon, stripHitMon);
    }
    
    else {
      auto sectorMon      = Monitored::Scalar<int>("sector_layer_" + std::to_string(layer), sectorsTotalShifted);
      auto stripNumberMon = Monitored::Scalar<int>("stripNumber_layer_" + std::to_string(layer), stripNumber + maxStripNumberQ1 + maxStripNumberQ2 + 1);
      auto stripHitMon    = Monitored::Scalar<int>("stripHit_layer_" + std::to_string(layer), (int) stripHit);
      fill("sTgcOccupancy", sectorMon, stripNumberMon, stripHitMon);
    }
    
    auto sectorSidedMon         = Monitored::Scalar<int>("sector_layer_" + std::to_string(layer), sector);
    auto stationEtaSidedMon     = Monitored::Scalar<int>("stationEta_layer_" + std::to_string(layer), stationEtaShifted); 
    auto stripHitLayersSidedMon = Monitored::Scalar<int>("stripHitLayers_layer_" + std::to_string(layer), (int) stripHit);
    fill("sTgcLayers", sectorSidedMon, stationEtaSidedMon, stripHitLayersSidedMon);
  }
  
  else if (channelType == sTgcIdHelper::sTgcChannelTypes::Wire) {
    int wireGroupNumber      = m_idHelperSvc -> stgcIdHelper().channel(id);
    Identifier idWireGroupQ3 = m_idHelperSvc -> stgcIdHelper().channelID("STL", 3, stationPhi, 1, 3, channelType, 1);
    const MuonGM::sTgcReadoutElement* sTgcReadoutObjectWireGroupQ3 = muonDetectorManagerObject -> getsTgcReadoutElement(idWireGroupQ3);
    int maxWireGroupNumberQ3 = sTgcReadoutObjectWireGroupQ3 -> numberOfStrips(idWireGroupQ3);
    auto wireGroupHit        = 1;
    
    auto stationEtaMon      = Monitored::Scalar<int>("stationEta_layer_" + std::to_string(layer), stationEtaShifted);
    auto wireGroupNumberMon = Monitored::Scalar<int>("wireGroupNumber_layer_" + std::to_string(layer), wireGroupNumber + (sector - 1)*maxWireGroupNumberQ3); 
    auto wireGroupHitMon    = Monitored::Scalar<int>("wireGroupHit_layer_" + std::to_string(layer), (int) wireGroupHit);
    fill("sTgcOccupancy", stationEtaMon, wireGroupNumberMon, wireGroupHitMon);

    auto sectorSidedMon             = Monitored::Scalar<int>("sector_layer_" + std::to_string(layer), sector);
    auto stationEtaSidedMon         = Monitored::Scalar<int>("stationEta_layer_" + std::to_string(layer), stationEtaShifted); 
    auto wireGroupHitLayersSidedMon = Monitored::Scalar<int>("wireGroupHitLayers_layer_" + std::to_string(layer), (int) wireGroupHit);
    fill("sTgcLayers", sectorSidedMon, stationEtaSidedMon, wireGroupHitLayersSidedMon);
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
    auto padHit = 1;
    auto padStationEtaMon = Monitored::Scalar<int>("padStationEta_layer_" + std::to_string(layer), stationEtaShiftedModified);
    auto padLumiblockMon  = Monitored::Scalar<int>("padLumiblock_layer_" + std::to_string(layer), lb);
    auto padHitMon        = Monitored::Scalar<int>("padHit_layer_" + std::to_string(layer), padHit);
    fill("sTgcLumiblock", padStationEtaMon, padLumiblockMon, padHitMon);
  }

  else if (channelType == sTgcIdHelper::sTgcChannelTypes::Strip) {
    auto stripHit = 1;
    auto stripStationEtaMon = Monitored::Scalar<int>("stripStationEta_layer_" + std::to_string(layer), stationEtaShiftedModified);
    auto stripLumiblockMon  = Monitored::Scalar<int>("stripLumiblock_layer_" + std::to_string(layer), lb);
    auto stripHitMon        = Monitored::Scalar<int>("stripHit_layer_" + std::to_string(layer), stripHit);
    fill("sTgcLumiblock", stripStationEtaMon, stripLumiblockMon, stripHitMon);
  }

  else if (channelType == sTgcIdHelper::sTgcChannelTypes::Wire) {
    auto wireGroupHit = 1;
    auto wireGroupStationEtaMon = Monitored::Scalar<int>("wireGroupStationEta_layer_" + std::to_string(layer), stationEtaShiftedModified);
    auto wireGroupLumiblockMon  = Monitored::Scalar<int>("wireGroupLumiblock_layer_" + std::to_string(layer), lb);
    auto wireGroupHitMon        = Monitored::Scalar<int>("wireGroupHit_layer_" + std::to_string(layer), wireGroupHit);
    fill("sTgcLumiblock", wireGroupStationEtaMon, wireGroupLumiblockMon, wireGroupHitMon);
  }
}

void sTgcRawDataMonAlg::fillsTgcTimingHistograms(const Muon::sTgcPrepData* sTgcObject) const {
  Identifier id = sTgcObject -> identify();

  if(!id.is_valid()) {
    ATH_MSG_DEBUG("Invalid identifier found in Muon::sTgcPrepData");
    return;
  }

  int channelType          = m_idHelperSvc -> stgcIdHelper().channelType(id);
  int multiplet            = m_idHelperSvc -> stgcIdHelper().multilayer(id);
  int gasGap               = m_idHelperSvc -> stgcIdHelper().gasGap(id);
  int sectorsTotal         = getSectors(id);
  int sectorsTotalShifted  = (sectorsTotal < 0) ? sectorsTotal - 1: sectorsTotal;
  int layer                = getLayer(multiplet, gasGap);
  short int time           = sTgcObject -> time();
  
  if (channelType == sTgcIdHelper::sTgcChannelTypes::Pad) {
    auto padHit = 1;
    auto padSectorSidedMon = Monitored::Scalar<int>("padSectorSided_layer_" + std::to_string(layer), sectorsTotalShifted);
    auto padTimingMon      = Monitored::Scalar<int>("padTiming_layer_" + std::to_string(layer), time);
    auto padHitMon         = Monitored::Scalar<int>("padHit_layer_" + std::to_string(layer), padHit);
    fill("sTgcTiming", padSectorSidedMon, padTimingMon, padHitMon);
  }
  
  else if (channelType == sTgcIdHelper::sTgcChannelTypes::Wire) {
    auto wireGroupHit = 1;
    auto wireGroupSectorSidedMon = Monitored::Scalar<int>("wireGroupSectorSided_layer_" + std::to_string(layer), sectorsTotalShifted);
    auto wireGroupTimingMon      = Monitored::Scalar<int>("wireGroupTiming_layer_" + std::to_string(layer), time);
    auto wireGroupHitMon         = Monitored::Scalar<int>("wireGroupHit_layer_" + std::to_string(layer), wireGroupHit);
    fill("sTgcTiming", wireGroupSectorSidedMon, wireGroupTimingMon, wireGroupHitMon);
  }  
}

void sTgcRawDataMonAlg::fillsTgcClusterFromSegmentsHistograms(const Trk::SegmentCollection* segmentObject) const {
  for (Trk::SegmentCollection::const_iterator s = segmentObject -> begin(); s != segmentObject -> end(); ++s) {
    const Muon::MuonSegment* muonSegmentObject = dynamic_cast<const Muon::MuonSegment*>(*s);
    
    if (muonSegmentObject == nullptr) {
      ATH_MSG_DEBUG("No pointer to segment!");
      break;
    }

    for (unsigned int irot = 0; irot < muonSegmentObject -> numberOfContainedROTs(); ++irot) {
      const Trk::RIO_OnTrack* rioOnTrackObject = muonSegmentObject -> rioOnTrack(irot);
      if (!rioOnTrackObject) continue;
      Identifier rioOnTrackID = rioOnTrackObject -> identify();
      const Muon::sTgcClusterOnTrack* sTgcClusterOnTrackObject = dynamic_cast<const Muon::sTgcClusterOnTrack*>(rioOnTrackObject);
      if (!sTgcClusterOnTrackObject) continue;
      const Muon::sTgcPrepData* prd = sTgcClusterOnTrackObject -> prepRawData();
      const std::vector<Identifier>& stripIds = prd->rdoList();
      unsigned int csize = stripIds.size();
       
      int channelType         = m_idHelperSvc -> stgcIdHelper().channelType(rioOnTrackID);
      int stationEta          = m_idHelperSvc -> stgcIdHelper().stationEta(rioOnTrackID);
      int multiplet           = m_idHelperSvc -> stgcIdHelper().multilayer(rioOnTrackID);
      int gasGap              = m_idHelperSvc -> stgcIdHelper().gasGap(rioOnTrackID);
      int iside               = (stationEta > 0) ? 1 : 0;
      std::string side        = GeometricSectors::sTgcSide[iside];
      int sectorsTotal        = getSectors(rioOnTrackID);
      int layer               = getLayer(multiplet, gasGap);
      int sectorsTotalShifted = (sectorsTotal < 0) ? sectorsTotal - 1: sectorsTotal;

      
      if (channelType == sTgcIdHelper::sTgcChannelTypes::Strip) {
	std::vector<short int> stripTimesVec = prd -> stripTimes();
	float stripClusterTimes = 0;
	
	for (unsigned int sIdx = 0; sIdx < csize; ++sIdx) {
	  stripClusterTimes += stripTimesVec.at(sIdx);
	}
	
	stripClusterTimes /= stripTimesVec.size();
	
	auto stripClusterHit = 1;
	auto stripClusterSectorSidedMon = Monitored::Scalar<int>("stripClusterSectorSided_layer_" + std::to_string(layer), sectorsTotalShifted);
	auto stripClusterTimesMon       = Monitored::Scalar<float>("stripClusterTiming_layer_" + std::to_string(layer), stripClusterTimes);
	auto stripClusterSizeMon        = Monitored::Scalar<unsigned int>("stripClusterSize_layer_" + std::to_string(layer), csize);
	auto stripClusterHitMon         = Monitored::Scalar<int>("stripClusterHit_layer_" + std::to_string(layer), stripClusterHit);
	fill("sTgcClusterFromSegment", stripClusterSectorSidedMon, stripClusterTimesMon, stripClusterSizeMon, stripClusterHitMon);
      }
    }
  }
}

