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
  
  return StatusCode::SUCCESS;
} 

StatusCode sTgcRawDataMonAlg::fillHistograms(const EventContext& ctx) const {  
  SG::ReadHandle<Muon::sTgcPrepDataContainer> sTgcContainer(m_sTgcContainerKey, ctx);
  SG::ReadCondHandle<MuonGM::MuonDetectorManager> detectorManagerKey(m_detectorManagerKey, ctx);
  
  if (m_dosTgcESD && m_dosTgcOverview) {
    for(const Muon::sTgcPrepDataCollection* coll : *sTgcContainer) {
      for (const Muon::sTgcPrepData* prd : *coll) {
	fillsTgcOverviewHistograms(prd, *coll);
	fillsTgcSummaryHistograms(prd, detectorManagerKey.cptr());
      }
    }
  }
  
  return StatusCode::SUCCESS;
}

void sTgcRawDataMonAlg::fillsTgcOverviewHistograms(const Muon::sTgcPrepData *sTgcObject, const Muon::MuonPrepDataCollection<Muon::sTgcPrepData> &prd) const {   
  auto chargeMon = Monitored::Collection("charge", prd, [] (const Muon::sTgcPrepData *aux) 
					 {
					   return aux -> charge();
					 });
  
  auto numberOfStripsPerClusterMon = Monitored::Collection("numberOfStripsPerCluster", prd, [] (const Muon::sTgcPrepData *aux) 
							   {
							     const std::vector<Identifier> &stripIds = aux -> rdoList(); 
							     return stripIds.size();
							   });
  
  auto timeMon = Monitored::Collection("time", prd, [] (const Muon::sTgcPrepData *aux) 
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

  auto xMon = Monitored::Collection("x", prd, [] (const Muon::sTgcPrepData *aux) 
				    {
				      Amg::Vector3D pos = aux -> globalPosition(); 
				      return pos.x();
				    });
  
  auto yMon = Monitored::Collection("y", prd, [] (const Muon::sTgcPrepData *aux) 
				    {
				      Amg::Vector3D pos = aux -> globalPosition(); 
				      return pos.y();
				    });
  
  auto zMon = Monitored::Collection("z", prd, [] (const Muon::sTgcPrepData *aux) 
				    {
				      Amg::Vector3D pos = aux -> globalPosition(); 
				      return pos.z();
				    });
  
  auto rMon = Monitored::Collection("r", prd, [] (const Muon::sTgcPrepData *aux) 
				    {
				      Amg::Vector3D pos = aux -> globalPosition(); 
				      return std::hypot(pos.x(), pos.y());
				    });

  fill("sTgcOverview", xMon, yMon, zMon, rMon);
}

void sTgcRawDataMonAlg::fillsTgcSummaryHistograms(const Muon::sTgcPrepData *sTgcObject, const MuonGM::MuonDetectorManager *muonDetectorManagerObject) const {
  Identifier id    = sTgcObject -> identify();

  if(!id.is_valid()) {
    ATH_MSG_DEBUG("Invalid identifier found in Muon::sTgcPrepData");
    return;
  }

  std::string stationName = m_idHelperSvc -> stgcIdHelper().stationNameString(m_idHelperSvc -> stgcIdHelper().stationName(id));
  int stationEta          = m_idHelperSvc -> stgcIdHelper().stationEta(id);
  int stationPhi          = m_idHelperSvc -> stgcIdHelper().stationPhi(id);
  int iside               = (stationEta > 0) ? 1 : 0;
  std::string side        = GeometricSectors::sTgc_Side[iside];
  int multiplet           = m_idHelperSvc -> stgcIdHelper().multilayer(id);
  int gasGap              = m_idHelperSvc -> stgcIdHelper().gasGap(id);    
  int channelType         = m_idHelperSvc -> stgcIdHelper().channelType(id);
  int sector              = m_idHelperSvc -> sector(id);
  int sectorsTotal        = getSectors(id);
  int stationEtaShifted   = (stationEta   < 0) ? stationEta   - 1: stationEta;
  int sectorsTotalShifted = (sectorsTotal < 0) ? sectorsTotal - 1: sectorsTotal; 

  if (channelType == sTgcIdHelper::sTgcChannelTypes::Pad) {
    int padNumber = m_idHelperSvc -> stgcIdHelper().channel(id);       
    Identifier idPadQ1 = m_idHelperSvc -> stgcIdHelper().channelID(stationName, 1, stationPhi, multiplet, gasGap, channelType, 1);    
    Identifier idPadQ2 = m_idHelperSvc -> stgcIdHelper().channelID(stationName, 2, stationPhi, multiplet, gasGap, channelType, 1);
    const MuonGM::sTgcReadoutElement *sTgcReadoutObjectPadQ1 = muonDetectorManagerObject -> getsTgcReadoutElement(idPadQ1);
    const MuonGM::sTgcReadoutElement *sTgcReadoutObjectPadQ2 = muonDetectorManagerObject -> getsTgcReadoutElement(idPadQ2);
    int maxPadNumberQ1 = sTgcReadoutObjectPadQ1 -> maxPadNumber(idPadQ1);
    int maxPadNumberQ2 = sTgcReadoutObjectPadQ2 -> maxPadNumber(idPadQ2);    
    auto padHit        = 1;
            
    if (std::abs(stationEta) == 1) {
      auto sectorMon    = Monitored::Scalar<int>("sector_multiplet_" + std::to_string(multiplet) + "_gasgap_" + std::to_string(gasGap), sectorsTotalShifted);
      auto padNumberMon = Monitored::Scalar<int>("padNumber_multiplet_" + std::to_string(multiplet) + "_gasgap_" + std::to_string(gasGap), padNumber);
      auto padHitMon    = Monitored::Scalar<int>("padHit_multiplet_" + std::to_string(multiplet) + "_gasgap_" + std::to_string(gasGap), (int) padHit);
      fill("sTgcOccupancy", sectorMon, padNumberMon, padHitMon);
    }
  
    else if (std::abs(stationEta) == 2) {
      auto sectorMon    = Monitored::Scalar<int>("sector_multiplet_" + std::to_string(multiplet) + "_gasgap_" + std::to_string(gasGap), sectorsTotalShifted);
      auto padNumberMon = Monitored::Scalar<int>("padNumber_multiplet_" + std::to_string(multiplet) + "_gasgap_" + std::to_string(gasGap), padNumber + maxPadNumberQ1);
      auto padHitMon    = Monitored::Scalar<int>("padHit_multiplet_" + std::to_string(multiplet) + "_gasgap_" + std::to_string(gasGap), (int) padHit);
      fill("sTgcOccupancy", sectorMon, padNumberMon, padHitMon);
    }
   
    else {
      auto sectorMon    = Monitored::Scalar<int>("sector_multiplet_" + std::to_string(multiplet) + "_gasgap_" + std::to_string(gasGap), sectorsTotalShifted);
      auto padNumberMon = Monitored::Scalar<int>("padNumber_multiplet_" + std::to_string(multiplet) + "_gasgap_" + std::to_string(gasGap), padNumber + maxPadNumberQ1 + maxPadNumberQ2);
      auto padHitMon    = Monitored::Scalar<int>("padHit_multiplet_" + std::to_string(multiplet) + "_gasgap_" + std::to_string(gasGap), (int) padHit);
      fill("sTgcOccupancy", sectorMon, padNumberMon, padHitMon);
    }
    
    auto sectorSidedMon       = Monitored::Scalar<int>("sector_multiplet_"  + std::to_string(multiplet) + "_gasgap_" + std::to_string(gasGap), sector);
    auto stationEtaSidedMon   = Monitored::Scalar<int>("stationEta_multiplet_" + std::to_string(multiplet) + "_gasgap_" + std::to_string(gasGap), stationEtaShifted);
    auto padHitLayersSidedMon = Monitored::Scalar<int>("padHitLayers_multiplet_"  + std::to_string(multiplet) + "_gasgap_" + std::to_string(gasGap), (int) padHit);
    fill("sTgcLayers", sectorSidedMon, stationEtaSidedMon, padHitLayersSidedMon);
  }
  
  else if (channelType == sTgcIdHelper::sTgcChannelTypes::Strip) {
    int stripNumber      = m_idHelperSvc -> stgcIdHelper().channel(id);
    Identifier idStripQ1 = m_idHelperSvc -> stgcIdHelper().channelID(stationName, 1, stationPhi, multiplet, gasGap, channelType, 1);    
    Identifier idStripQ2 = m_idHelperSvc -> stgcIdHelper().channelID(stationName, 2, stationPhi, multiplet, gasGap, channelType, 1);
    const MuonGM::sTgcReadoutElement *sTgcReadoutObjectStripQ1 = muonDetectorManagerObject -> getsTgcReadoutElement(idStripQ1);
    const MuonGM::sTgcReadoutElement *sTgcReadoutObjectStripQ2 = muonDetectorManagerObject -> getsTgcReadoutElement(idStripQ2);    
    int maxStripNumberQ1 = sTgcReadoutObjectStripQ1 -> numberOfStrips(idStripQ1);
    int maxStripNumberQ2 = sTgcReadoutObjectStripQ2 -> numberOfStrips(idStripQ2);
    auto stripHit        = 1;
            
    if (std::abs(stationEta) == 1) {
      auto sectorMon      = Monitored::Scalar<int>("sector_multiplet_" + std::to_string(multiplet) + "_gasgap_" + std::to_string(gasGap), sectorsTotalShifted);
      auto stripNumberMon = Monitored::Scalar<int>("stripNumber_multiplet_" + std::to_string(multiplet) + "_gasgap_" + std::to_string(gasGap), stripNumber);
      auto stripHitMon    = Monitored::Scalar<int>("stripHit_multiplet_" + std::to_string(multiplet) + "_gasgap_" + std::to_string(gasGap), (int) stripHit);
      fill("sTgcOccupancy", sectorMon, stripNumberMon, stripHitMon);
    }
    
    else if (std::abs(stationEta) == 2) {
      auto sectorMon      = Monitored::Scalar<int>("sector_multiplet_" + std::to_string(multiplet) + "_gasgap_" + std::to_string(gasGap), sectorsTotalShifted);
      auto stripNumberMon = Monitored::Scalar<int>("stripNumber_multiplet_" + std::to_string(multiplet) + "_gasgap_" + std::to_string(gasGap), stripNumber + maxStripNumberQ1 + 1);
      auto stripHitMon    = Monitored::Scalar<int>("stripHit_multiplet_" + std::to_string(multiplet) + "_gasgap_" + std::to_string(gasGap), (int) stripHit);
      fill("sTgcOccupancy", sectorMon, stripNumberMon, stripHitMon);
    }
    
    else {
      auto sectorMon      = Monitored::Scalar<int>("sector_multiplet_" + std::to_string(multiplet) + "_gasgap_" + std::to_string(gasGap), sectorsTotalShifted);
      auto stripNumberMon = Monitored::Scalar<int>("stripNumber_multiplet_" + std::to_string(multiplet) + "_gasgap_" + std::to_string(gasGap), stripNumber + maxStripNumberQ1 + maxStripNumberQ2 + 1);
      auto stripHitMon    = Monitored::Scalar<int>("stripHit_multiplet_" + std::to_string(multiplet) + "_gasgap_" + std::to_string(gasGap), (int) stripHit);
      fill("sTgcOccupancy", sectorMon, stripNumberMon, stripHitMon);
    }
    
    auto sectorSidedMon         = Monitored::Scalar<int>("sector_multiplet_" + std::to_string(multiplet) + "_gasgap_" + std::to_string(gasGap), sector);
    auto stationEtaSidedMon     = Monitored::Scalar<int>("stationEta_multiplet_" + std::to_string(multiplet) + "_gasgap_" + std::to_string(gasGap), stationEtaShifted); 
    auto stripHitLayersSidedMon = Monitored::Scalar<int>("stripHitLayers_multiplet_" + std::to_string(multiplet) + "_gasgap_" + std::to_string(gasGap), (int) stripHit);
    fill("sTgcLayers", sectorSidedMon, stationEtaSidedMon, stripHitLayersSidedMon);
  }
  
  else if (channelType == sTgcIdHelper::sTgcChannelTypes::Wire) {
    int wireGroupNumber      = m_idHelperSvc -> stgcIdHelper().channel(id);
    Identifier idWireGroupQ3 = m_idHelperSvc -> stgcIdHelper().channelID("STL", 3, stationPhi, 1, 3, channelType, 1);
    const MuonGM::sTgcReadoutElement *sTgcReadoutObjectWireGroupQ3 = muonDetectorManagerObject -> getsTgcReadoutElement(idWireGroupQ3);
    int maxWireGroupNumberQ3 = sTgcReadoutObjectWireGroupQ3 -> numberOfStrips(idWireGroupQ3);
    auto wireGroupHit        = 1;
    
    auto stationEtaMon      = Monitored::Scalar<int>("stationEta_multiplet_" + std::to_string(multiplet) + "_gasgap_" + std::to_string(gasGap), stationEtaShifted);
    auto wireGroupNumberMon = Monitored::Scalar<int>("wireGroupNumber_multiplet_" + std::to_string(multiplet) + "_gasgap_" + std::to_string(gasGap), wireGroupNumber + (sector - 1)*maxWireGroupNumberQ3); 
    auto wireGroupHitMon    = Monitored::Scalar<int>("wireGroupHit_multiplet_" + std::to_string(multiplet) + "_gasgap_" + std::to_string(gasGap), (int) wireGroupHit);
    fill("sTgcLayers", stationEtaMon, wireGroupNumberMon, wireGroupHitMon);

    auto sectorSidedMon             = Monitored::Scalar<int>("sector_multiplet_" + std::to_string(multiplet) + "_gasgap_" + std::to_string(gasGap), sector);
    auto stationEtaSidedMon         = Monitored::Scalar<int>("stationEta_multiplet_" + std::to_string(multiplet) + "_gasgap_" + std::to_string(gasGap), stationEtaShifted); 
    auto wireGroupHitLayersSidedMon = Monitored::Scalar<int>("wireGroupHitLayers_multiplet_" + std::to_string(multiplet) + "_gasgap_" + std::to_string(gasGap), (int) wireGroupHit);
    fill("sTgcLayers", sectorSidedMon, stationEtaSidedMon, wireGroupHitLayersSidedMon);
  }
}
