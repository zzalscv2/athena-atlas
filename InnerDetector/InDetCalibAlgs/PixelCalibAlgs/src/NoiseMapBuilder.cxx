/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

// PixelCalibAlgs
#include "PixelCalibAlgs/NoiseMapBuilder.h"
#include "PixelCalibAlgs/PixelConvert.h"

// PixelConditions
#include "InDetConditionsSummaryService/IInDetConditionsSvc.h"
#include "PixelConditionsServices/IPixelByteStreamErrorsSvc.h"
#include "PixelConditionsServices/ISpecialPixelMapSvc.h" 
#include "PixelConditionsData/SpecialPixelMap.h"

// Gaudi
#include "GaudiKernel/ITHistSvc.h"

// EDM
#include "InDetRawData/PixelRDO_Container.h"
#include "InDetReadoutGeometry/PixelDetectorManager.h"
#include "InDetReadoutGeometry/SiDetectorElement.h" 
#include "InDetReadoutGeometry/PixelModuleDesign.h" 
#include "InDetReadoutGeometry/SiDetectorElementCollection.h" 
#include "EventInfo/EventInfo.h"
#include "EventInfo/EventID.h"

// geometry
#include "InDetIdentifier/PixelID.h"

// ROOT
#include "TH2.h"
#include "TString.h" 

// standard library
#include <string>
#include <sstream>
#include <algorithm>
#include <map> 
#include <fstream>
#include <cstdlib> 

NoiseMapBuilder::NoiseMapBuilder(const std::string& name, ISvcLocator* pSvcLocator) :
  AthAlgorithm(name, pSvcLocator),
  m_tHistSvc("THistSvc", name),
  m_pixelConditionsSummarySvc("PixelConditionsSummarySvc", name),
  m_BSErrorsSvc("PixelByteStreamErrorsSvc",name),
  m_specialPixelMapSvc("SpecialPixelMapSvc", name), 
  m_pixman(0), 
  m_pixelID(0),
  m_pixelRDOKey("PixelRDOs"),
  m_nEvents(0.),
  m_nEventsHist(nullptr),
  m_nEventsLBHist(nullptr),
  m_disabledModules(nullptr),
  m_overlayedPixelNoiseMap(nullptr),
  m_overlayedIBLDCNoiseMap(nullptr),
  m_overlayedIBLSCNoiseMap(nullptr),
  m_disk1ACut(1.e-3),
  m_disk2ACut(1.e-3),
  m_disk3ACut(1.e-3),
  m_disk1CCut(1.e-3),
  m_disk2CCut(1.e-3),
  m_disk3CCut(1.e-3),
  m_iblCut(1.e-3), 
  m_bLayerCut(1.e-3),
  m_layer1Cut(1.e-3),
  m_layer2Cut(1.e-3),
  m_dbmCut(1.e-3),
  m_hist_lbMax(3001),
  m_longPixelMultiplier(1.5), 
  m_gangedPixelMultiplier(2.), 
  m_occupancyPerBC(true),
  m_nBCReadout(2),
  m_evt_lbMin(0),
  m_evt_lbMax(-1),
  m_calculateNoiseMaps(false)
{
  declareProperty("PixelRDOKey", m_pixelRDOKey, "StoreGate key of pixel RDOs");
  declareProperty("Disk1ACut", m_disk1ACut, "Occupancy cut for Disk1A pixels");
  declareProperty("Disk2ACut", m_disk2ACut, "Occupancy cut for Disk2A pixels");
  declareProperty("Disk3ACut", m_disk3ACut, "Occupancy cut for Disk3A pixels");
  declareProperty("Disk1CCut", m_disk1CCut, "Occupancy cut for Disk1C pixels");
  declareProperty("Disk2CCut", m_disk2CCut, "Occupancy cut for Disk2C pixels");
  declareProperty("Disk3CCut", m_disk3CCut, "Occupancy cut for Disk3C pixels");
  declareProperty("BLayerCut", m_bLayerCut, "Occupancy cut for BLayer pixels");
  declareProperty("Layer1Cut", m_layer1Cut, "Occupancy cut for Layer1 pixels");
  declareProperty("Layer2Cut", m_layer2Cut, "Occupancy cut for Layer2 pixels");
  declareProperty("IBLCut", m_dbmCut, "Occupancy cut for DBM pixels"); 
  declareProperty("NBCReadout", m_nBCReadout, "Number of bunch crossings read out");
  declareProperty("LBMin", m_evt_lbMin, "First lumi block to consider");
  declareProperty("LBMax", m_evt_lbMax, "Last lumi block to consider");
  declareProperty("LongPixelMultiplier", m_longPixelMultiplier, "Multiplier for long pixels");
  declareProperty("GangedPixelMultiplier", m_gangedPixelMultiplier, "Multiplier for ganged pixels");
  declareProperty("OccupancyPerBC", m_occupancyPerBC, "Calculate occupancy per BC or per event");
  declareProperty("CalculateNoiseMaps", m_calculateNoiseMaps, "If false only build hit maps");
  declareProperty("THistSvc", m_tHistSvc, "THistSvc");
  declareProperty("PixelConditionsSummarySvc", m_pixelConditionsSummarySvc, "PixelConditionsSummarySvc");
  declareProperty("PixelByteStreamSummarySvc", m_BSErrorsSvc, "PixelBSErrorsSvc");
  //  declareProperty("isIBL", m_isIBL, "If false IBL not considered"); // kazuki
}


NoiseMapBuilder::~NoiseMapBuilder(){}

std::string NoiseMapBuilder::getDCSIDFromPosition (int barrel_ec, int layer, int modPhi, int module_eta){
  for(unsigned int ii = 0; ii < m_pixelMapping.size(); ii++) {
    if (m_pixelMapping[ii].second.size() != 4) {
      std::cout << "getDCSIDFromPosition: Vector size is not 4!" << std::endl;
      return std::string("Error!");
    }
    if (m_pixelMapping[ii].second[0] != barrel_ec) continue;
    if (m_pixelMapping[ii].second[1] != layer) continue;
    if (m_pixelMapping[ii].second[2] != modPhi) continue;
    if (m_pixelMapping[ii].second[3] != module_eta) continue;
    return m_pixelMapping[ii].first;
  }
  std::cout << "Not found!" << std::endl;
  return std::string("Error!");
}

const std::string NoiseMapBuilder::histoSuffix(const int bec, const int layer){
  std::ostringstream out;
  
  switch(bec) {
  case 0: 
    out << "barrel/";  
    if(layer==0)      { out << "IBL";              }
    else if(layer==1) { out << "B-layer";          }
    else              { out << "Layer" << layer-1; }
    break;
  case +2: out << "endcapA/Disk" << layer+1; break;
  case -2: out << "endcapC/Disk" << layer+1; break;
  case +4: out << "DBMA/Layer"   << layer+1; break;
  case -4: out << "DBMC/Layer"   << layer+1; break;
  default: break;
  }  
  return out.str();
}

std::vector<std::string>& NoiseMapBuilder::splitter(const std::string &str,
						    char delim, 
						    std::vector<std::string> &elems) {
  std::stringstream ss(str);
  std::string item;
  while (std::getline(ss, item, delim)) {
    elems.push_back(item);
  }
  return elems;
}

std::vector<std::string> NoiseMapBuilder::splitter(const std::string &str, 
						   char delim) {
  std::vector<std::string> elems;
  splitter(str, delim, elems);
  return elems;
}

//=========================================================
//
// initialize
//
//=========================================================
StatusCode NoiseMapBuilder::initialize(){
  ATH_MSG_INFO("Initializing NoiseMapBuilder");

  // retrieve THistSvc
  StatusCode sc = m_tHistSvc.retrieve();
  if( !sc.isSuccess() ){
    ATH_MSG_FATAL("Unable to retrieve THistSvc");
    return StatusCode::FAILURE;
  }

  // retrieve PixelConditionsSummarySvc
  sc = m_pixelConditionsSummarySvc.retrieve();
  if(!sc.isSuccess()){
    ATH_MSG_FATAL("Unable to retrieve PixelConditionsSummarySvc");
    return StatusCode::FAILURE;
  }

  // retrieve PixelByteStreamErrorsSvc
  sc = m_BSErrorsSvc.retrieve();
  if(!sc.isSuccess()){
    ATH_MSG_FATAL("Unable to retrieve bytestream errors service");
    return StatusCode::FAILURE;
  }

  // retrieve SpecialPixelMapSvc
  sc = m_specialPixelMapSvc.retrieve();
  if(!sc.isSuccess()){
    ATH_MSG_FATAL("Unable to retrieve SpecialPixelMapSvc");
    return StatusCode::FAILURE;
  }

  // retrieve PixelDetectorManager
  sc = detStore()->retrieve(m_pixman,"Pixel");
  if(!sc.isSuccess()){
    ATH_MSG_FATAL("Unable to retrieve PixelDetectorManager");
    return StatusCode::FAILURE;
  }

  // retrieve PixelID helper
  sc = detStore()->retrieve(m_pixelID, "PixelID");
  if(!sc.isSuccess()){
    ATH_MSG_FATAL("Unable to retrieve PixelID helper");
    return StatusCode::FAILURE;
  }
 
  // resize vectors of histograms
  const Identifier::size_type maxHash = m_pixelID->wafer_hash_max();
  ATH_MSG_DEBUG("PixelID maxHash = " << maxHash);
  m_hitMaps.resize(maxHash);
  m_LBdependence.resize(maxHash);
  m_BCIDdependence.resize(maxHash);
  m_TOTdistributions.resize(maxHash);
  if(m_calculateNoiseMaps)
    m_noiseMaps.resize(maxHash);

  return (registerHistograms()); 
}

StatusCode NoiseMapBuilder::registerHistograms(){

  const std::string mapFile = "PixelMapping_Run2.dat";
 
  std::vector<std::string> paths = splitter(std::getenv("DATAPATH"), ':'); 
  bool found(false);  
  for(const auto& x : paths){
    std::ifstream infile( (x+"/"+mapFile).c_str() );
    if( infile.is_open() ){
      ATH_MSG_INFO("Mapping file '" << mapFile << "' found in " << x);

      int tmp_barrel_ec; int tmp_layer; int tmp_modPhi; int tmp_module_eta; std::string tmp_module_name;
      std::vector<int> tmp_position;
      tmp_position.resize(4);
      while(infile >> tmp_barrel_ec >> tmp_layer >> tmp_modPhi >> tmp_module_eta >> tmp_module_name) {
        tmp_position[0] = tmp_barrel_ec;
        tmp_position[1] = tmp_layer;
        tmp_position[2] = tmp_modPhi;
        tmp_position[3] = tmp_module_eta;
        m_pixelMapping.push_back(std::make_pair(tmp_module_name, tmp_position));
      }

      found=true;
      infile.close();
      break;
    }
  }
  
  if( !found ){
    ATH_MSG_FATAL("Mapping file '" << mapFile << "' not found in DATAPATH !!!");
    return StatusCode::FAILURE;
  }

  m_nEventsHist = new TH1D("NEvents", "NEvents", 1, 0, 1);
  m_tHistSvc->regHist("/histfile/NEvents", m_nEventsHist).setChecked();
  
  m_nEventsLBHist = new TH1D("NEventsLB", "NEventsLB", m_hist_lbMax, -0.5, m_hist_lbMax+0.5);
  m_tHistSvc->regHist("/histfile/NEventsLB", m_nEventsLBHist).setChecked();
  
  for(InDetDD::SiDetectorElementCollection::const_iterator iter=m_pixman->getDetectorElementBegin(); 
      iter!=m_pixman->getDetectorElementEnd(); ++iter) {    

    const InDetDD::SiDetectorElement* element = *iter;
    if(!element) continue;
    
    Identifier ident = element->identify();
    if(!m_pixelID->is_pixel(ident)) continue; 
    
    //const InDetDD::PixelModuleDesign* design = dynamic_cast<const InDetDD::PixelModuleDesign*>(&element->design());
    //if(!design) continue;    
    //unsigned int mchips = design->numberOfCircuits();

    int bec        = m_pixelID->barrel_ec(ident);
    int layer      = m_pixelID->layer_disk(ident); 
    int modPhi = m_pixelID->phi_module(ident);
    int module_eta = m_pixelID->eta_module(ident); 
    int modHash = m_pixelID->wafer_hash(ident);

    std::string onlineID = 
      getDCSIDFromPosition(bec,layer,modPhi,module_eta);

    std::ostringstream name;
    
    // hitmap
    if( bec == 0 && layer == 0) // IBL
      m_hitMaps[modHash] = new TH2D(onlineID.c_str(), onlineID.c_str(), 160, 0, 160, 336, 0, 336);
    else if( abs(bec) == 4 ) // DBM
      m_hitMaps[modHash] = new TH2D(onlineID.c_str(), onlineID.c_str(), 80,  0,  80, 336, 0, 336);
    else
      m_hitMaps[modHash] = new TH2D(onlineID.c_str(), onlineID.c_str(), 144, 0, 144, 328, 0, 328);    
    name << "/histfile/hitMaps_" << histoSuffix(bec,layer) << "/" << onlineID;
    m_tHistSvc->regHist(name.str().c_str(), m_hitMaps[modHash]).setChecked();
    name.str(""); name.clear();


    // LB dependence
    m_LBdependence[modHash] = new TH1D(onlineID.c_str(), onlineID.c_str(), m_hist_lbMax, -0.5, m_hist_lbMax + 0.5);
    name  << "/histfile/LBdep_" << histoSuffix(bec,layer) << "/" << onlineID;
    m_tHistSvc->regHist(name.str().c_str(), m_LBdependence[modHash]).setChecked();
    name.str(""); name.clear();
    
    // BCID dependence
    m_BCIDdependence[modHash] = new TH1D(onlineID.c_str(), onlineID.c_str(), 301, -0.5, 300.5);
    name << "/histfile/BCIDdep_" << histoSuffix(bec,layer) << "/" << onlineID;
    m_tHistSvc->regHist(name.str().c_str(), m_BCIDdependence[modHash]).setChecked();
    name.str(""); name.clear();
    
    // TOT
    if( bec == 0 && layer == 0) // IBL
      m_TOTdistributions[modHash] = new TH1D(onlineID.c_str(), onlineID.c_str(), 19, -0.5, 18.5);
    else
      m_TOTdistributions[modHash] = new TH1D(onlineID.c_str(), onlineID.c_str(), 256, -0.5, 255.5);
    name << "/histfile/TOT_" << histoSuffix(bec,layer) << "/" << onlineID;
    m_tHistSvc->regHist(name.str().c_str(), m_TOTdistributions[modHash]).setChecked();
    name.str(""); name.clear();
    
    // noisemap
    if( m_calculateNoiseMaps ){
      if( bec == 0 && layer == 0) // IBL
	m_noiseMaps[modHash] = new TH2C(onlineID.c_str(), onlineID.c_str(), 160, 0, 160, 336, 0, 336);
      else if( abs(bec) == 4 ) // DBM
	m_noiseMaps[modHash] = new TH2C(onlineID.c_str(), onlineID.c_str(), 80, 0, 80, 336, 0, 336);
      else
	m_noiseMaps[modHash] = new TH2C(onlineID.c_str(), onlineID.c_str(), 144, 0, 144, 328, 0, 328);
      name << "/histfile/noiseMaps_" << histoSuffix(bec,layer) << "/" << onlineID;
      m_tHistSvc->regHist(name.str().c_str(), m_noiseMaps[modHash]).setChecked();
      name.str(""); name.clear();
    }
  } // end loop in detector elements 
  
  m_disabledModules = new TH1D("DisabledModules", "Number of events disabled vs. IdentifierHash", 2048, 0, 2048);
  m_tHistSvc->regHist("/histfile/DisabledModules", m_disabledModules).setChecked();
  
  if (m_calculateNoiseMaps) {
    m_overlayedPixelNoiseMap = new TH2D("overlayedPixelNoiseMap", "Noisy pixel map overlayed all Pixel modules", 144, 0, 144, 328, 0, 328);
    m_tHistSvc->regHist("/histfile/overlayedPixelNoiseMap", m_overlayedPixelNoiseMap).setChecked();
    
    m_overlayedIBLDCNoiseMap = new TH2D("overlayedIBLDCNoiseMap", "Noisy pixel map overlayed all IBL Planar modules", 160, 0, 160, 336, 0, 336);
    m_tHistSvc->regHist("/histfile/overlayedIBLDCNoiseMap", m_overlayedIBLDCNoiseMap).setChecked();
    
    m_overlayedIBLSCNoiseMap = new TH2D("overlayedIBLSCNoiseMap", "Noisy pixel map overlayed all IBL 3D modules", 80, 0, 80, 336, 0, 336);
    m_tHistSvc->regHist("/histfile/overlayedIBLSCNoiseMap", m_overlayedIBLSCNoiseMap).setChecked();
  }  
  
  return StatusCode::SUCCESS;
} 



  // kazuki
/*  InDetDD::SiDetectorElementCollection::const_iterator iter, itermin, itermax;
  itermin = m_pixman->getDetectorElementBegin();
  itermax = m_pixman->getDetectorElementEnd();
  if(m_pixelID->wafer_hash_max() > 1744) m_isIBL = true; // #modules only Pixel is 1744
  //std::cout << "DEBUG: wafer_hash_max = " << m_pixelID->wafer_hash_max() << std::endl;
  std::map<int, std::string> barrel_ec2string;
  barrel_ec2string[0] = "barrel";
  barrel_ec2string[2] = "endcapA";
  barrel_ec2string[-2] = "endcapC";
  barrel_ec2string[4] = "DBMA";
  barrel_ec2string[-4] = "DBMC";
  std::map<int, std::string> layer2flavour;
  if (m_isIBL) {
    layer2flavour[0] = "IBL";
    layer2flavour[1] = "B-layer";
    layer2flavour[2] = "Layer1";
    layer2flavour[3] = "Layer2";
  } else {
    layer2flavour[0] = "B-layer";
    layer2flavour[1] = "Layer1";
    layer2flavour[2] = "Layer2";
  }
  // initialize histograms
  for( iter = itermin; iter != itermax; ++iter) {
    const InDetDD::SiDetectorElement* element = *iter;
    if(element == 0) continue;
    Identifier ident = element->identify();
    if(!m_pixelID->is_pixel(ident)) continue;  // OK this Element is included
    const InDetDD::PixelModuleDesign* design = dynamic_cast<const InDetDD::PixelModuleDesign*>(&element->design());
    if(!design) continue;
    //unsigned int mchips = design->numberOfCircuits();
    int barrel     = m_pixelID->barrel_ec (ident);
    int layer      = m_pixelID->layer_disk(ident); // kazuki
    int module_phi = m_pixelID->phi_module(ident);
    int module_eta = m_pixelID->eta_module(ident); // kazuki
    int moduleHash = m_pixelID->wafer_hash(ident);

    // debug
    // std::cout << moduleHash << " " << NoiseMapBuilder::getDCSIDFromPosition(barrel,layer,module_phi,module_eta) << std::endl;

    std::ostringstream names;
    TString tmp_onlineID;
    std::string onlineID;
    if (m_isIBL) { // --- IBL --- //
      if ( barrel == 0 ) { // barrel
        if (layer == 0) { // IBL
          m_moduleHashList.push_back(moduleHash);

          //tmp_onlineID = Form("LI_S%02d_",module_phi + 1) + eta2moduleID_IBL[module_eta];
          //onlineID = std::string(tmp_onlineID);
          onlineID = NoiseMapBuilder::getDCSIDFromPosition(barrel,layer,module_phi,module_eta);
          // hit map
          m_hitMaps[moduleHash] = new TH2D(onlineID.c_str(), onlineID.c_str(), 160, -0., 160., 336, 0., 336.);
          names << "/histfile/hitMaps_barrel/IBL/" << onlineID;
          m_tHistSvc->regHist(names.str().c_str(), m_hitMaps[moduleHash]).setChecked();
          names.str(""); names.clear();
          // LB dependence
          m_LBdependence[moduleHash] = new TH1D(onlineID.c_str(), onlineID.c_str(), nLBmax, -0.5, nLBmax + 0.5);
          names << "/histfile/LBdep_barrel/IBL/" << onlineID;
          m_tHistSvc->regHist(names.str().c_str(), m_LBdependence[moduleHash]).setChecked();
          names.str(""); names.clear();
          // BCID dependence
          m_BCIDdependence[moduleHash] = new TH1D(onlineID.c_str(), onlineID.c_str(), 301, -0.5, 300.5);
          names << "/histfile/BCIDdep_barrel/IBL/" << onlineID;
          m_tHistSvc->regHist(names.str().c_str(), m_BCIDdependence[moduleHash]).setChecked();
          names.str(""); names.clear();
          // TOT
          m_TOTdistributions[moduleHash] = new TH1D(onlineID.c_str(), onlineID.c_str(), 19, -0.5, 18.5);
          names << "/histfile/TOT_barrel/IBL/" << onlineID;
          m_tHistSvc->regHist(names.str().c_str(), m_TOTdistributions[moduleHash]).setChecked();
          names.str(""); names.clear();
          // noise map
          if( m_calculateNoiseMaps ){
            m_noiseMaps[moduleHash] = new TH2C(onlineID.c_str(), onlineID.c_str(), 160, -0., 160., 336, 0., 336.);
            names << "/histfile/noiseMaps_barrel/IBL/" << onlineID;
            m_tHistSvc->regHist(names.str().c_str(), m_noiseMaps[moduleHash]).setChecked();
            names.str(""); names.clear();
          }
        } else if (layer == 1) { // BLayer
          m_moduleHashList.push_back(moduleHash);

          //tmp_onlineID = Form("L%d_",layer - 1) + phi2moduleID_BLayer[module_phi] + eta2moduleID_PixelBarrel[module_eta];
          //onlineID = std::string(tmp_onlineID);
          onlineID = NoiseMapBuilder::getDCSIDFromPosition(barrel,layer,module_phi,module_eta);
          // hit map
          m_hitMaps[moduleHash] = new TH2D(onlineID.c_str(), onlineID.c_str(), 144, -0., 144., 328, 0., 328.);
          names << "/histfile/hitMaps_barrel/B-layer/" << onlineID;
          m_tHistSvc->regHist(names.str().c_str(), m_hitMaps[moduleHash]).setChecked();
          names.str(""); names.clear();
          // LB dependence
          m_LBdependence[moduleHash] = new TH1D(onlineID.c_str(), onlineID.c_str(), nLBmax, -0.5, nLBmax + 0.5);
          names << "/histfile/LBdep_barrel/B-layer/" << onlineID;
          m_tHistSvc->regHist(names.str().c_str(), m_LBdependence[moduleHash]).setChecked();
          names.str(""); names.clear();
          // BCID dependence
          m_BCIDdependence[moduleHash] = new TH1D(onlineID.c_str(), onlineID.c_str(), 301, -0.5, 300.5);
          names << "/histfile/BCIDdep_barrel/B-layer/" << onlineID;
          m_tHistSvc->regHist(names.str().c_str(), m_BCIDdependence[moduleHash]).setChecked();
          names.str(""); names.clear();
          // TOT
          m_TOTdistributions[moduleHash] = new TH1D(onlineID.c_str(), onlineID.c_str(), 256, -0.5, 255.5);
          names << "/histfile/TOT_barrel/B-layer/" << onlineID;
          m_tHistSvc->regHist(names.str().c_str(), m_TOTdistributions[moduleHash]).setChecked();
          names.str(""); names.clear();
          // noise map
          if( m_calculateNoiseMaps ){
            m_noiseMaps[moduleHash] = new TH2C(onlineID.c_str(), onlineID.c_str(), 144, -0., 144., 328, 0., 328.);
            names << "/histfile/noiseMaps_barrel/B-layer/" << onlineID;
            m_tHistSvc->regHist(names.str().c_str(), m_noiseMaps[moduleHash]).setChecked();
            names.str(""); names.clear();
          }
        } else { // Layer-1,2
          m_moduleHashList.push_back(moduleHash);

          //if(layer == 2) tmp_onlineID = Form("L%d_",layer - 1) + phi2moduleID_Layer1[module_phi] + eta2moduleID_PixelBarrel[module_eta];
          //else if(layer == 3) tmp_onlineID = Form("L%d_",layer - 1) + phi2moduleID_Layer2[module_phi] + eta2moduleID_PixelBarrel[module_eta];

          //onlineID = std::string(tmp_onlineID);
          onlineID = NoiseMapBuilder::getDCSIDFromPosition(barrel,layer,module_phi,module_eta);
          // hit map
          m_hitMaps[moduleHash] = new TH2D(onlineID.c_str(), onlineID.c_str(), 144, -0., 144., 328, 0., 328.);
          names << "/histfile/hitMaps_barrel/Layer" << layer - 1 << "/" << onlineID;
          m_tHistSvc->regHist(names.str().c_str(), m_hitMaps[moduleHash]).setChecked();
          names.str(""); names.clear();
          // LB dependence
          m_LBdependence[moduleHash] = new TH1D(onlineID.c_str(), onlineID.c_str(), nLBmax, -0.5, nLBmax + 0.5);
          names << "/histfile/LBdep_barrel/Layer" << layer - 1 << "/" << onlineID;
          m_tHistSvc->regHist(names.str().c_str(), m_LBdependence[moduleHash]).setChecked();
          names.str(""); names.clear();
          // BCID dependence
          m_BCIDdependence[moduleHash] = new TH1D(onlineID.c_str(), onlineID.c_str(), 301, -0.5, 300.5);
          names << "/histfile/BCIDdep_barrel/Layer" << layer - 1 << "/" <<onlineID;
          m_tHistSvc->regHist(names.str().c_str(), m_BCIDdependence[moduleHash]).setChecked();
          names.str(""); names.clear();
          // TOT
          m_TOTdistributions[moduleHash] = new TH1D(onlineID.c_str(), onlineID.c_str(), 256, -0.5, 255.5);
          names << "/histfile/TOT_barrel/Layer" << layer - 1 << "/" << onlineID;
          m_tHistSvc->regHist(names.str().c_str(), m_TOTdistributions[moduleHash]).setChecked();
          names.str(""); names.clear();
          // noise map
          if( m_calculateNoiseMaps ){
            m_noiseMaps[moduleHash] = new TH2C(onlineID.c_str(), onlineID.c_str(), 144, -0., 144., 328, 0., 328.);
            names << "/histfile/noiseMaps_barrel/Layer" << layer - 1 << "/" << onlineID;
            m_tHistSvc->regHist(names.str().c_str(), m_noiseMaps[moduleHash]).setChecked();
            names.str(""); names.clear();
          }
        }
      } else if ( barrel == 2 ) { // ECA
        m_moduleHashList.push_back(moduleHash);

        //tmp_onlineID = Form("D%dA_",layer + 1) + phi2moduleID_ECA[module_phi];
        //onlineID = std::string(tmp_onlineID);
        onlineID = NoiseMapBuilder::getDCSIDFromPosition(barrel,layer,module_phi,module_eta);
        // hitmap
        m_hitMaps[moduleHash] = new TH2D(onlineID.c_str(), onlineID.c_str(), 144, -0., 144., 328, 0., 328.);
        names << "/histfile/hitMaps_endcapA/Disk" << (layer + 1) << "/" << onlineID;
        m_tHistSvc->regHist(names.str().c_str(), m_hitMaps[moduleHash]).setChecked();
        names.str(""); names.clear();
        // Lumi Block dependence
        m_LBdependence[moduleHash] = new TH1D(onlineID.c_str(), onlineID.c_str(), nLBmax, -0.5, nLBmax + 0.5);
        names << "/histfile/LBdep_endcapA/Disk" << (layer + 1) << "/" << onlineID;
        m_tHistSvc->regHist(names.str().c_str(), m_LBdependence[moduleHash]).setChecked();
        names.str(""); names.clear();
        // BCID dependence
        m_BCIDdependence[moduleHash] = new TH1D(onlineID.c_str(), onlineID.c_str(), 301, -0.5, 300.5);
        names << "/histfile/BCIDdep_endcapA/Layer" << (layer + 1) << "/" <<onlineID;
        m_tHistSvc->regHist(names.str().c_str(), m_BCIDdependence[moduleHash]).setChecked();
        names.str(""); names.clear();
        // TOT
        m_TOTdistributions[moduleHash] = new TH1D(onlineID.c_str(), onlineID.c_str(), 256, -0.5, 255.5);
        names << "/histfile/TOT_endcapA/Layer" << (layer + 1) << "/" << onlineID;
        m_tHistSvc->regHist(names.str().c_str(), m_TOTdistributions[moduleHash]).setChecked();
        names.str(""); names.clear();
        // noise map
        if( m_calculateNoiseMaps ){
          m_noiseMaps[moduleHash] = new TH2C(onlineID.c_str(), onlineID.c_str(), 144, -0., 144., 328, 0., 328.);
          names << "/histfile/noiseMaps_endcapA/Disk" << (layer + 1) << "/" << onlineID;
          m_tHistSvc->regHist(names.str().c_str(), m_noiseMaps[moduleHash]).setChecked();
          names.str(""); names.clear();
        }
      } else if ( barrel == -2) { // ECC
        m_moduleHashList.push_back(moduleHash);

        //tmp_onlineID = Form("D%dC_",layer + 1) + phi2moduleID_ECC[module_phi];
        //onlineID = std::string(tmp_onlineID);
        onlineID = NoiseMapBuilder::getDCSIDFromPosition(barrel,layer,module_phi,module_eta);
        std::ostringstream names;
        // hitmap
        m_hitMaps[moduleHash] = new TH2D(onlineID.c_str(), onlineID.c_str(), 144, -0., 144., 328, 0., 328.);
        names << "/histfile/hitMaps_endcapC/Disk" << (layer + 1) << "/" << onlineID;
        m_tHistSvc->regHist(names.str().c_str(), m_hitMaps[moduleHash]).setChecked();
        names.str(""); names.clear();
        // LB dependence
        m_LBdependence[moduleHash] = new TH1D(onlineID.c_str(), onlineID.c_str(), nLBmax, -0.5, nLBmax + 0.5);
        names << "/histfile/LBdep_endcapC/Disk" << (layer + 1) << "/" << onlineID;
        m_tHistSvc->regHist(names.str().c_str(), m_LBdependence[moduleHash]).setChecked();
        names.str(""); names.clear();
        // BCID dependence
        m_BCIDdependence[moduleHash] = new TH1D(onlineID.c_str(), onlineID.c_str(), 301, -0.5, 300.5);
        names << "/histfile/BCIDdep_endcapC/Layer" << (layer + 1) << "/" <<onlineID;
        m_tHistSvc->regHist(names.str().c_str(), m_BCIDdependence[moduleHash]).setChecked();
        names.str(""); names.clear();
        // TOT
        m_TOTdistributions[moduleHash] = new TH1D(onlineID.c_str(), onlineID.c_str(), 256, -0.5, 255.5);
        names << "/histfile/TOT_endcapC/Layer" << (layer + 1) << "/" << onlineID;
        m_tHistSvc->regHist(names.str().c_str(), m_TOTdistributions[moduleHash]).setChecked();
        names.str(""); names.clear();
        // noise map
        if( m_calculateNoiseMaps ){
          m_noiseMaps[moduleHash] = new TH2C(onlineID.c_str(), onlineID.c_str(), 144, -0., 144., 328, 0., 328.);
          names << "/histfile/noiseMaps_endcapC/Disk" << (layer + 1) << "/" << onlineID;
          m_tHistSvc->regHist(names.str().c_str(), m_noiseMaps[moduleHash]).setChecked();
          names.str(""); names.clear();
        }
      } else if ( barrel == 4 ) { // DBM A
        m_moduleHashList.push_back(moduleHash);
        //tmp_onlineID = Form("LI_S15_A_" + phi2moduleID_DBM[module_phi] + "_A%d", phi2moduleNum_DBM[module_phi] + layer);
        //onlineID = std::string(tmp_onlineID);
        onlineID = NoiseMapBuilder::getDCSIDFromPosition(barrel,layer,module_phi,module_eta);
        std::ostringstream names;
        // hitmap
        m_hitMaps[moduleHash] = new TH2D(onlineID.c_str(), onlineID.c_str(), 80, -0., 80., 336, 0., 336.);
        names << "/histfile/hitMaps_DBMA/Layer" << (layer + 1) << "/" << onlineID;
        m_tHistSvc->regHist(names.str().c_str(), m_hitMaps[moduleHash]).setChecked();
        names.str(""); names.clear();
        // LB dependence
        m_LBdependence[moduleHash] = new TH1D(onlineID.c_str(), onlineID.c_str(), nLBmax, -0.5, nLBmax + 0.5);
        names << "/histfile/LBdep_DBMA/Layer" << (layer + 1) << "/" << onlineID;
        m_tHistSvc->regHist(names.str().c_str(), m_LBdependence[moduleHash]).setChecked();
        names.str(""); names.clear();
        // BCID dependence
        m_BCIDdependence[moduleHash] = new TH1D(onlineID.c_str(), onlineID.c_str(), 301, -0.5, 300.5);
        names << "/histfile/BCIDdep_DBMA/Layer" << (layer + 1) << "/" <<onlineID;
        m_tHistSvc->regHist(names.str().c_str(), m_BCIDdependence[moduleHash]).setChecked();
        names.str(""); names.clear();
        // TOT
        m_TOTdistributions[moduleHash] = new TH1D(onlineID.c_str(), onlineID.c_str(), 256, -0.5, 255.5);
        names << "/histfile/TOT_DBMA/Layer" << (layer + 1) << "/" << onlineID;
        m_tHistSvc->regHist(names.str().c_str(), m_TOTdistributions[moduleHash]).setChecked();
        names.str(""); names.clear();
        // noise map
        if( m_calculateNoiseMaps ){
          m_noiseMaps[moduleHash] = new TH2C(onlineID.c_str(), onlineID.c_str(), 80, -0., 80., 336, 0., 336.);
          names << "/histfile/noiseMaps_DBMA/Layer" << (layer + 1) << "/" << onlineID;
          m_tHistSvc->regHist(names.str().c_str(), m_noiseMaps[moduleHash]).setChecked();
          names.str(""); names.clear();
        }
      } else if ( barrel == -4) { // DBM C
        m_moduleHashList.push_back(moduleHash);
        //tmp_onlineID = Form("LI_S15_C_" + phi2moduleID_DBM[module_phi] + "_C%d", phi2moduleNum_DBM[module_phi] + layer);
        //onlineID = std::string(tmp_onlineID);
        onlineID = NoiseMapBuilder::getDCSIDFromPosition(barrel,layer,module_phi,module_eta);
        std::ostringstream names;
        // hitmap
        m_hitMaps[moduleHash] = new TH2D(onlineID.c_str(), onlineID.c_str(), 80, -0., 80., 336, 0., 336.);
        names << "/histfile/hitMaps_DBMC/Layer" << (layer + 1) << "/" << onlineID;
        m_tHistSvc->regHist(names.str().c_str(), m_hitMaps[moduleHash]).setChecked();
        names.str(""); names.clear();
        // LB dependence
	m_LBdependence[moduleHash] = new TH1D(onlineID.c_str(), onlineID.c_str(), nLBmax, -0.5, nLBmax + 0.5);
        names << "/histfile/LBdep_DBMC/Layer" << (layer + 1) << "/" << onlineID;
        m_tHistSvc->regHist(names.str().c_str(), m_LBdependence[moduleHash]).setChecked();
        names.str(""); names.clear();
        // BCID dependence
        m_BCIDdependence[moduleHash] = new TH1D(onlineID.c_str(), onlineID.c_str(), 301, -0.5, 300.5);
        names << "/histfile/BCIDdep_DBMC/Layer" << (layer + 1) << "/" <<onlineID;
        m_tHistSvc->regHist(names.str().c_str(), m_BCIDdependence[moduleHash]).setChecked();
        names.str(""); names.clear();
        // TOT
        m_TOTdistributions[moduleHash] = new TH1D(onlineID.c_str(), onlineID.c_str(), 256, -0.5, 255.5);
        names << "/histfile/TOT_DBMC/Layer" << (layer + 1) << "/" << onlineID;
        m_tHistSvc->regHist(names.str().c_str(), m_TOTdistributions[moduleHash]).setChecked();
        names.str(""); names.clear();
        // noise map
        if( m_calculateNoiseMaps ){
          m_noiseMaps[moduleHash] = new TH2C(onlineID.c_str(), onlineID.c_str(), 80, -0., 80., 336, 0., 336.);
          names << "/histfile/noiseMaps_DBMC/Layer" << (layer + 1) << "/" << onlineID;
          m_tHistSvc->regHist(names.str().c_str(), m_noiseMaps[moduleHash]).setChecked();
          names.str(""); names.clear();
        }
      }
    } // end if m_isIBL
*/

    //for (const auto& moduleHashInList : m_moduleHashList) {
    //  std::cout << "[DEBUG] moduleHash " << moduleHashInList << std::endl;
    //}
    //  // endcap A
    //  for(int layer = 0; layer < 3; layer++){
    //    for(int module_phi = 0; module_phi < 48; module_phi++){
    //
    //      Identifier moduleID = m_pixelID->wafer_id(2, layer, module_phi, 0);
    //      IdentifierHash moduleHash = m_pixelID->wafer_hash(moduleID);
    //      std::cout << "DEBUG: " << "moduleHash ECA = " << moduleHash << std::endl; // kazuki
    //      m_moduleHashList.push_back(int(moduleHash)); // kazuki
    //
    //      std::string onlineID;
    //      TString tmp_onlineID;
    //      //
    //      if (m_isIBL) { // ----- IBL ----- //
    //        tmp_onlineID = Form("D%dA_",layer + 1) + phi2moduleID_ECA[module_phi];
    //        onlineID = std::string(tmp_onlineID); // TString 2 std::string
    //      } else { // default
    //        unsigned int hashID = ( ((m_pixelID->barrel_ec(moduleID) + 2) / 2) << 25 ) +
    //          ( m_pixelID->layer_disk(moduleID) << 23) +
    //          ( m_pixelID->phi_module(moduleID) << 17) +
    //          ( (m_pixelID->eta_module(moduleID) + 6) << 13);
    //
    //        //std::string onlineID = PixelConvert::DCSID(PixelConvert::OnlineID(hashID));
    //        onlineID = PixelConvert::DCSID(PixelConvert::OnlineID(hashID));
    //      }
    //
    //      std::ostringstream names;
    //
    //      m_hitMaps[moduleHash] = new TH2D(onlineID.c_str(), onlineID.c_str(), 144, -0., 144., 328, 0., 328.);
    //
    //      names << "/histfile/hitMaps_endcapA/Disk" << (layer + 1) << "/" << onlineID;
    //      m_tHistSvc->regHist(names.str().c_str(), m_hitMaps[moduleHash]).setChecked();
    //      names.str(""); names.clear();
    //
    //      m_LBdependence[moduleHash] = new TH1D(onlineID.c_str(), onlineID.c_str(), 2001, -0.5, 2000.5);
    //
    //      names << "/histfile/LBdep_endcapA/Disk" << (layer + 1) << "/" << onlineID;
    //      m_tHistSvc->regHist(names.str().c_str(), m_LBdependence[moduleHash]).setChecked();
    //      names.str(""); names.clear();
    //
    //      if( m_calculateNoiseMaps ){
    //        m_noiseMaps[moduleHash] = new TH2C(onlineID.c_str(), onlineID.c_str(), 144, -0., 144., 328, 0., 328.);
    //
    //        names << "/histfile/noiseMaps_endcapA/Disk" << (layer + 1) << "/" << onlineID;
    //        m_tHistSvc->regHist(names.str().c_str(), m_noiseMaps[moduleHash]).setChecked();
    //        names.str(""); names.clear();
    //      }
    //    }
    //  }

    // endcap C

    /*
       for(int layer = 0; layer < 3; layer++){
       for(int module_phi = 0; module_phi < 48; module_phi++){

       Identifier moduleID = m_pixelID->wafer_id(-2, layer, module_phi, 0);
       IdentifierHash moduleHash = m_pixelID->wafer_hash(moduleID);
       std::cout << "DEBUG: " << "moduleHash ECC = " << moduleHash << std::endl; // kazuki
       m_moduleHashList.push_back(int(moduleHash));

       std::string onlineID;
       TString tmp_onlineID;
//
if (m_isIBL) { // ----- IBL ----- //
tmp_onlineID = Form("D%dC_",layer + 1) + phi2moduleID_ECC[module_phi];
onlineID = std::string(tmp_onlineID); // TString 2 std::string
} else { // default
unsigned int hashID = ( ((m_pixelID->barrel_ec(moduleID) + 2) / 2) << 25 ) +
( m_pixelID->layer_disk(moduleID) << 23) +
( m_pixelID->phi_module(moduleID) << 17) +
( (m_pixelID->eta_module(moduleID) + 6) << 13);

    //std::string onlineID = PixelConvert::DCSID(PixelConvert::OnlineID(hashID));
    onlineID = PixelConvert::DCSID(PixelConvert::OnlineID(hashID));
    }

    std::ostringstream names;

    m_hitMaps[moduleHash] = new TH2D(onlineID.c_str(), onlineID.c_str(), 144, -0., 144., 328, 0., 328.);

    names << "/histfile/hitMaps_endcapC/Disk" << (layer + 1) << "/" << onlineID;
    m_tHistSvc->regHist(names.str().c_str(), m_hitMaps[moduleHash]).setChecked();
    names.str(""); names.clear();

    m_LBdependence[moduleHash] = new TH1D(onlineID.c_str(), onlineID.c_str(), 2001, -0.5, 2000.5);

    names << "/histfile/LBdep_endcapC/Disk" << (layer + 1) << "/" << onlineID;
    m_tHistSvc->regHist(names.str().c_str(), m_LBdependence[moduleHash]).setChecked();
    names.str(""); names.clear();

    if( m_calculateNoiseMaps ){
    m_noiseMaps[moduleHash] = new TH2C(onlineID.c_str(), onlineID.c_str(), 144, -0., 144., 328, 0., 328.);

    names << "/histfile/noiseMaps_endcapC/Disk" << (layer + 1) << "/" << onlineID;
    m_tHistSvc->regHist(names.str().c_str(), m_noiseMaps[moduleHash]).setChecked();
    names.str(""); names.clear();
    }
    }
    }
    */


// barrel


// DBM
/*
   std::map<int, int, TString> layereta2moduleID_DBM;
   layereta2moduleID_DBM[0][2] = TString("LI_S15_A_12_M1_A1_2");
   layereta2moduleID_DBM[1][2] = TString("LI_S15_A_12_M1_A2_2");
   layereta2moduleID_DBM[2][2] = TString("LI_S15_A_12_M1_A3_2");
   layereta2moduleID_DBM[0][3] = TString("LI_S15_A_34_M2_A4_2");
   layereta2moduleID_DBM[1][3] = TString("LI_S15_A_34_M2_A5_2");
   layereta2moduleID_DBM[2][3] = TString("LI_S15_A_34_M2_A6_2");
   layereta2moduleID_DBM[0][0] = TString("LI_S15_A_12_M3_A1_1");
   layereta2moduleID_DBM[1][0] = TString("LI_S15_A_12_M3_A2_1");
   layereta2moduleID_DBM[2][0] = TString("LI_S15_A_12_M3_A3_1");
   layereta2moduleID_DBM[0][1] = TString("LI_S15_A_34_M4_A4_1");
   layereta2moduleID_DBM[1][1] = TString("LI_S15_A_34_M4_A5_1");
   layereta2moduleID_DBM[2][1] = TString("LI_S15_A_34_M4_A6_1");
   */
//for(int layer = 0; layer < 3; layer++)
//      for(int layer = 0; layer < nlayer; layer++) // kazuki
//      {
//
//        /*
//           int module_phi_max = 0;
//           if(layer == 0) module_phi_max = 22;
//           if(layer == 1) module_phi_max = 38;
//           if(layer == 2) module_phi_max = 52;
//           */
//
//        int module_eta_min(0);
//        int module_eta_max(0);
//        if(m_isIBL && layer == 0) { // IBL
//          module_eta_min = -10;
//          module_eta_max = 9;
//        } else { // Pixel Barrel
//          module_eta_min = -6;
//          module_eta_max = 6;
//        }
//
//        // kazuki
//        /*
//           std::cout << "DEBUG: " << "\n"
//           << "m_pixelID->phi_module_max(0) = " << m_pixelID->phi_module_max(m_pixelID->wafer_id(-2, 0, 0, 0)) << "\n"
//           << "m_pixelID->eta_module_max(0) = " << m_pixelID->eta_module_max(m_pixelID->wafer_id(-2, 0, 0, 0)) << "\n"
//           << "m_pixelID->eta_module_min(0) = " << m_pixelID->eta_module_min(m_pixelID->wafer_id(-2, 0, 0, 0)) << "\n"
//           << "m_pixelID->phi_index_max(0) = " << m_pixelID->phi_index_max(m_pixelID->wafer_id(-2, 0, 0, 0)) << "\n"
//           << "m_pixelID->eta_index_max(0) = " << m_pixelID->eta_index_max(m_pixelID->wafer_id(-2, 0, 0, 0)) << "\n"
//           << "m_pixelID->phi_module_max(144) = " << m_pixelID->phi_module_max(m_pixelID->wafer_id(0, 0, 0, 0)) << "\n"
//           << "m_pixelID->eta_module_max(144) = " << m_pixelID->eta_module_max(m_pixelID->wafer_id(0, 0, 0, 0)) << "\n"
//           << "m_pixelID->eta_module_min(144) = " << m_pixelID->eta_module_min(m_pixelID->wafer_id(0, 0, 0, 0)) << "\n"
//           << "m_pixelID->phi_index_max(144) = " << m_pixelID->phi_index_max(m_pixelID->wafer_id(0, 0, 0, 0)) << "\n"
//           << "m_pixelID->eta_index_max(144) = " << m_pixelID->eta_index_max(m_pixelID->wafer_id(0, 0, 0, 0)) << "\n"
//           << "m_pixelID->phi_module_max(0, 1, 0, 0) = " << m_pixelID->phi_module_max(m_pixelID->wafer_id(0, 1, 0, 0)) << "\n"
//           << "m_pixelID->eta_module_max(0, 1, 0, 0) = " << m_pixelID->eta_module_max(m_pixelID->wafer_id(0, 1, 0, 0)) << "\n"
//           << "m_pixelID->eta_module_min(0, 1, 0, 0) = " << m_pixelID->eta_module_min(m_pixelID->wafer_id(0, 1, 0, 0)) << "\n"
//           << "m_pixelID->phi_index_max(0, 1, 0, 0) = " << m_pixelID->phi_index_max(m_pixelID->wafer_id(0, 1, 0, 0)) << "\n"
//           << "m_pixelID->eta_index_max(0, 1, 0, 0) = " << m_pixelID->eta_index_max(m_pixelID->wafer_id(0, 1, 0, 0)) << "\n"
//           << "m_pixelID->phi_module_max(0, 2, 0, 0) = " << m_pixelID->phi_module_max(m_pixelID->wafer_id(0, 2, 0, 0)) << "\n"
//           << "m_pixelID->eta_module_max(0, 2, 0, 0) = " << m_pixelID->eta_module_max(m_pixelID->wafer_id(0, 2, 0, 0)) << "\n"
//           << "m_pixelID->eta_module_min(0, 2, 0, 0) = " << m_pixelID->eta_module_min(m_pixelID->wafer_id(0, 2, 0, 0)) << "\n"
//           << "m_pixelID->phi_index_max(0, 2, 0, 0) = " << m_pixelID->phi_index_max(m_pixelID->wafer_id(0, 2, 0, 0)) << "\n"
//           << "m_pixelID->eta_index_max(0, 2, 0, 0) = " << m_pixelID->eta_index_max(m_pixelID->wafer_id(0, 2, 0, 0)) << "\n"
//           << "m_pixelID->phi_module_max(0, 3, 0, 0) = " << m_pixelID->phi_module_max(m_pixelID->wafer_id(0, 3, 0, 0)) << "\n"
//           << "m_pixelID->eta_module_max(0, 3, 0, 0) = " << m_pixelID->eta_module_max(m_pixelID->wafer_id(0, 3, 0, 0)) << "\n"
//           << "m_pixelID->eta_module_min(0, 3, 0, 0) = " << m_pixelID->eta_module_min(m_pixelID->wafer_id(0, 3, 0, 0)) << "\n"
//           << "m_pixelID->phi_index_max(0, 3, 0, 0) = " << m_pixelID->phi_index_max(m_pixelID->wafer_id(0, 3, 0, 0)) << "\n"
//           << "m_pixelID->eta_index_max(0, 3, 0, 0) = " << m_pixelID->eta_index_max(m_pixelID->wafer_id(0, 3, 0, 0)) << "\n"
//           << "m_pixelID->phi_module_max(1740) = " << m_pixelID->phi_module_max(m_pixelID->wafer_id(2, 0, 0, 0)) << "\n"
//           << "m_pixelID->eta_module_max(1740) = " << m_pixelID->eta_module_max(m_pixelID->wafer_id(2, 0, 0, 0)) << "\n"
//           << "m_pixelID->eta_module_min(1740) = " << m_pixelID->eta_module_min(m_pixelID->wafer_id(2, 0, 0, 0)) << "\n"
//           << "m_pixelID->phi_index_max(1740) = " << m_pixelID->phi_index_max(m_pixelID->wafer_id(2, 0, 0, 0)) << "\n"
//           << "m_pixelID->eta_index_max(1740) = " << m_pixelID->eta_index_max(m_pixelID->wafer_id(2, 0, 0, 0))
//           << std::endl;
//           std::cout << "DEBUG: " << "\n"
//           << "m_pixelID->wafer_hash_max(): " << m_pixelID->wafer_hash_max() << "\n"
//           << "m_pixelID->pixel_hash_max(): " << m_pixelID->pixel_hash_max()
//           << std::endl;
//           */
//        //for(int module_phi = 0; module_phi < module_phi_max; module_phi++)
//        for(int module_phi = 0; module_phi < module_phi_max[layer]; module_phi++) // kazuki
//        {
//          //for(int module_eta = -6; module_eta < 7; module_eta++)
//          for(int module_eta = module_eta_min; module_eta <= module_eta_max; module_eta++)
//          {
//
//            Identifier moduleID = m_pixelID->wafer_id(0, layer, module_phi, module_eta);
//            IdentifierHash moduleHash = m_pixelID->wafer_hash(moduleID);
//            std::cout << "DEBUG: " << "moduleHash Barrel = " << moduleHash << std::endl; // kazuki
//            m_moduleHashList.push_back(int(moduleHash));
//
//            std::string onlineID;
//            TString tmp_onlineID;
//            //
//            if (m_isIBL) { // ----- IBL ----- //
//              if (layer == 0) {
//                tmp_onlineID = Form("LI_S%02d_",module_phi + 1) + eta2moduleID_IBL[module_eta];
//              } else if (layer == 1) {
//                tmp_onlineID = Form("L%d_",layer - 1) + phi2moduleID_BLayer[module_phi] + eta2moduleID_PixelBarrel[module_eta];
//              } else {
//                tmp_onlineID = Form("L%d_",layer - 1) + phi2moduleID_Layer12[module_phi] + eta2moduleID_PixelBarrel[module_eta];
//              }
//              onlineID = std::string(tmp_onlineID); // TString 2 std::string
//            } else { // default
//              unsigned int hashID = ( ((m_pixelID->barrel_ec(moduleID) + 2) / 2) << 25 ) +
//                ( m_pixelID->layer_disk(moduleID) << 23) +
//                ( m_pixelID->phi_module(moduleID) << 17) +
//                ( (m_pixelID->eta_module(moduleID) + 6) << 13);
//
//              onlineID = PixelConvert::DCSID(PixelConvert::OnlineID(hashID));
//            }
//
//            std::ostringstream names;
//
//            if(m_isIBL && layer == 0) { // ----- IBL ----- //
//              m_hitMaps[moduleHash] = new TH2D(onlineID.c_str(), onlineID.c_str(), 160, -0., 160., 336, 0., 336.);
//            } else {
//              m_hitMaps[moduleHash] = new TH2D(onlineID.c_str(), onlineID.c_str(), 144, -0., 144., 328, 0., 328.);
//            }
//
//            if(m_isIBL) { // ----- IBL ----- //
//              if(layer == 0) names << "/histfile/hitMaps_barrel/IBL/" << onlineID;
//              else if(layer == 1) names << "/histfile/hitMaps_barrel/B-Layer/" << onlineID;
//              else names << "/histfile/hitMaps_barrel/Layer" << layer - 1 << "/" << onlineID;
//            } else { // only Pixel
//              if(layer == 0) names << "/histfile/hitMaps_barrel/B-layer/" << onlineID;
//              else names << "/histfile/hitMaps_barrel/Layer" << layer << "/" << onlineID;
//            }
//            m_tHistSvc->regHist(names.str().c_str(), m_hitMaps[moduleHash]).setChecked();
//            names.str(""); names.clear();
//
//            m_LBdependence[moduleHash] = new TH1D(onlineID.c_str(), onlineID.c_str(), 2001, -0.5, 2000.5);
//
//            if(m_isIBL) { // ----- IBL ----- //
//              if(layer == 0) names << "/histfile/LBdep_barrel/IBL/" << onlineID;
//              else if(layer == 1) names << "/histfile/LBdep_barrel/B-Layer/" << onlineID;
//              else names << "/histfile/LBdep_barrel/Layer" << layer - 1 << "/" << onlineID;
//            } else { // only Pixel
//              if(layer == 0) names << "/histfile/LBdep_barrel/B-layer/" << onlineID;
//              else names << "/histfile/LBdep_barrel/Layer" << layer << "/" << onlineID;
//            }
//            m_tHistSvc->regHist(names.str().c_str(), m_LBdependence[moduleHash]).setChecked();
//            names.str(""); names.clear();
//
//            if( m_calculateNoiseMaps ){
//              if(m_isIBL && layer == 0) { // ----- IBL ----- //
//                m_noiseMaps[moduleHash] = new TH2C(onlineID.c_str(), onlineID.c_str(), 244, -0., 244., 377, 0., 377.);
//              } else {
//                m_noiseMaps[moduleHash] = new TH2C(onlineID.c_str(), onlineID.c_str(), 144, -0., 144., 328, 0., 328.);
//              }
//
//              if(m_isIBL) {
//                if(layer == 0) names << "/histfile/noiseMaps_barrel/IBL/" << onlineID;
//                else if(layer == 1) names << "/histfile/noiseMaps_barrel/B-layer/" << onlineID;
//                else names << "/histfile/noiseMaps_barrel/Layer" << layer - 1 << "/" << onlineID;
//              } else { // only Pixel
//                if(layer == 0) names << "/histfile/noiseMaps_barrel/B-layer/" << onlineID;
//                else names << "/histfile/noiseMaps_barrel/Layer" << layer << "/" << onlineID;
//              }
//              m_tHistSvc->regHist(names.str().c_str(), m_noiseMaps[moduleHash]).setChecked();
//              names.str(""); names.clear();
//            }
//          }
//        }
//      }

//m_disabledModules = new TH1D("DisabledModules", "Number of events disabled vs. IdentifierHash", 1744, 0, 1744);
     /*  }
  m_disabledModules = new TH1D("DisabledModules", "Number of events disabled vs. IdentifierHash", 2048, 0, 2048);
  m_tHistSvc->regHist("/histfile/DisabledModules", m_disabledModules).setChecked();

  if (m_calculateNoiseMaps) {
    m_overlayedPixelNoiseMap = new TH2D("overlayedPixelNoiseMap", "Noisy pixel map overlayed all Pixel modules", 144, -0., 144., 328, 0., 328.);
    m_tHistSvc->regHist("/histfile/overlayedPixelNoiseMap", m_overlayedPixelNoiseMap).setChecked();

    m_overlayedIBLDCNoiseMap = new TH2D("overlayedIBLDCNoiseMap", "Noisy pixel map overlayed all IBL Planar modules", 160, -0., 160., 336, 0., 336.);
    m_overlayedIBLSCNoiseMap = new TH2D("overlayedIBLSCNoiseMap", "Noisy pixel map overlayed all IBL 3D modules", 80, -0., 80., 336, 0., 336.);
    if (m_isIBL) {
      m_tHistSvc->regHist("/histfile/overlayedIBLDCNoiseMap", m_overlayedIBLDCNoiseMap).setChecked();
      m_tHistSvc->regHist("/histfile/overlayedIBLSCNoiseMap", m_overlayedIBLSCNoiseMap).setChecked();
    }
  }

  return StatusCode::SUCCESS;
}
     */


//=========================================================
//
// execute
//
//=========================================================
StatusCode NoiseMapBuilder::execute(){
  ATH_MSG_DEBUG( "Executing NoiseMapBuilder" );

  // retrieve EventInfo
  const EventInfo* eventInfo;
  StatusCode sc = sgSvc()->retrieve(eventInfo);
  if( !sc.isSuccess() ){
    ATH_MSG_FATAL( "Unable to retrieve event info" );
    return StatusCode::FAILURE;
  } ATH_MSG_DEBUG( "Event info retrieved" );

  // check LB is in allowed range
  int LB =  static_cast<int>(eventInfo->event_ID()->lumi_block());
  if( (LB < m_evt_lbMin) || ( m_evt_lbMax >= m_evt_lbMin && LB > m_evt_lbMax) ){
    ATH_MSG_VERBOSE( "Event in lumiblock " << eventInfo->event_ID()->lumi_block() <<
		     " not in selected range [" << m_evt_lbMin << "," << m_evt_lbMax << "] => skipped");    
    return StatusCode::SUCCESS;
  }

  // retrieve PixelRDO container
  const DataHandle< PixelRDO_Container > pixelRDOs;
  sc = sgSvc()->retrieve(pixelRDOs, m_pixelRDOKey);
  if( !sc.isSuccess() ){
    ATH_MSG_FATAL( "Unable to retrieve pixel RDO container at " << m_pixelRDOKey );
    return StatusCode::FAILURE;
  } ATH_MSG_DEBUG( "Pixel RDO container retrieved" );

  // loop in RDO container
  for(PixelRDO_Container::const_iterator coll=pixelRDOs->begin(); 
      coll!=pixelRDOs->end(); coll++){

    const InDetRawDataCollection<PixelRDORawData>* PixelRDOCollection(*coll);
    if(PixelRDOCollection != 0){
      Identifier moduleID = PixelRDOCollection->identify();
      IdentifierHash modHash = m_pixelID->wafer_hash(moduleID);
      ATH_MSG_VERBOSE("moduleID, modHash = " << moduleID << " , " << modHash);
      
      // exclude module if reported as not good by PixelConditionsSummarySvc
      if( !(m_pixelConditionsSummarySvc->isGood(modHash)) ) {
	ATH_MSG_VERBOSE("Module excluded as reported not good by PixelConditionsSummarySvc");
        continue;
      }

      // exclude module if containg FE synch errors
      int errors = m_BSErrorsSvc->getModuleErrors(modHash);
      if ( ( errors & 0x0001C000 ) ) {
	ATH_MSG_VERBOSE("Module excluded as containing FE synch errors");
	continue;
      }
      
      for(DataVector<PixelRDORawData>::const_iterator rdo = PixelRDOCollection->begin();
          rdo!=PixelRDOCollection->end(); ++rdo){
        Identifier rdoID = (*rdo)->identify();
        unsigned int pixel_eta = m_pixelID->eta_index(rdoID);
        unsigned int pixel_phi = m_pixelID->phi_index(rdoID);

        int TOT = (*rdo)->getToT(); // it returns a 8 bits "word"
        int BCID = (*rdo)->getBCID();
        //int LVL1ID = (*rdo)->getLVL1ID();
        //int LVL1A = (*rdo)->getLVL1A();

        //if( std::find(m_moduleHashList.begin(), m_moduleHashList.end(), moduleHash ) == m_moduleHashList.end() ) continue;
        //if ( std::binary_search(m_moduleHashList.begin(), m_moduleHashList.end(), moduleHash) == false ) continue;
        m_hitMaps[modHash]->Fill(pixel_eta, pixel_phi);
        m_LBdependence[modHash]->Fill(LB);
        m_BCIDdependence[modHash]->Fill(BCID);
        m_TOTdistributions[modHash]->Fill(TOT);
      }
    }
  }


  // [sgs] why is this done in every event ???
  for(unsigned int moduleHash = 0; moduleHash < m_pixelID->wafer_hash_max(); moduleHash++) {
    if( !m_pixelConditionsSummarySvc->isActive( moduleHash ) ){
      m_disabledModules->Fill( moduleHash );
    }
  }
  
  m_nEvents++;
  m_nEventsHist->Fill(0.5);
  m_nEventsLBHist->Fill(LB);

  return StatusCode::SUCCESS;
}


//=========================================================
//
// finalize
//
//=========================================================
StatusCode NoiseMapBuilder::finalize() {
  ATH_MSG_INFO("Finalizing NoiseMapBuilder");

  if(m_occupancyPerBC)
    m_nEvents *= m_nBCReadout;
 
  const int minLogOccupancy = 8;
  const double minOccupancy = pow(10.,-minLogOccupancy);
  
  TH1D* globalOccupancy= new TH1D("occupancy", "Pixel occupancy", minLogOccupancy*10, -minLogOccupancy, 0.);
  m_tHistSvc->regHist("/histfile/occupancy",globalOccupancy).setChecked();

  std::map<std::string, TH1D*> h_occupancy;

  // occupancy histograms for different components of the Pixel detector
  std::vector<std::string> vcomponent;
  vcomponent.push_back("Disk1A");
  vcomponent.push_back("Disk2A");
  vcomponent.push_back("Disk3A");
  vcomponent.push_back("Disk1C");
  vcomponent.push_back("Disk2C");
  vcomponent.push_back("Disk3C");
  vcomponent.push_back("IBL"); 
  vcomponent.push_back("B-layer");
  vcomponent.push_back("Layer1");
  vcomponent.push_back("Layer2");
  vcomponent.push_back("DBMA");
  vcomponent.push_back("DBMC");

  for(std::vector<std::string>::const_iterator cit=vcomponent.begin(); cit!=vcomponent.end(); ++cit) {
    const std::string comp = (*cit);
    h_occupancy[comp] = new TH1D( ("occupancy"+comp).c_str(), ("Pixel occupancy "+comp).c_str(),
				  minLogOccupancy*10, -minLogOccupancy, 0);    
    m_tHistSvc->regHist(("/histfile/occupancy"+comp).c_str(), h_occupancy[comp]).setChecked();
  }
  vcomponent.clear();
  
  // occupancy histograms for different pixel types
  std::vector<std::string> vtype;
  vtype.push_back("Normal");
  vtype.push_back("Ganged");
  vtype.push_back("InterGanged");
  vtype.push_back("Long");
  vtype.push_back("Long-Ganged");
  vtype.push_back("Long-InterGanged");
 for(std::vector<std::string>::const_iterator cit=vtype.begin(); cit!=vtype.end(); ++cit){
    const std::string type = (*cit);
    h_occupancy[type] = 
      new TH1D( ("occupancy"+type).c_str(), ("Pixel occupancy "+type).c_str(), 
		minLogOccupancy*10, -minLogOccupancy, 0);
    m_tHistSvc->regHist(("/histfile/occupancy"+type).c_str(), h_occupancy[type]).setChecked();
  }
  vtype.clear();

  //------------------------
  // number of hits 
  //------------------------  

  // IBL
  TH2F* nhitsPlotBI=new TH2F("nhitsPlotBI", "Number of hits BI;module_eta;module_phi", 20, -10, 10, 14, -0.5, 13.5); 
  m_tHistSvc->regHist("/histfile/nhitsPlotBI",nhitsPlotBI).setChecked();

  // B-layer
  TH2F* nhitsPlotB0=new TH2F("nhitsPlotB0", "Number of hits B0;module_eta;module_phi", 13, -6.5, 6.5, 22, -0.5, 21.5);
  m_tHistSvc->regHist("/histfile/nhitsPlotB0",nhitsPlotB0).setChecked();

  // barrel layer 1
  TH2F* nhitsPlotB1=new TH2F("nhitsPlotB1", "Number of hits B1;module_eta;module_phi", 13, -6.5, 6.5, 38, -0.5, 37.5);
  m_tHistSvc->regHist("/histfile/nhitsPlotB1",nhitsPlotB1).setChecked();

  // barrel layer 2
  TH2F* nhitsPlotB2=new TH2F("nhitsPlotB2", "Number of hits B2;module_eta;module_phi", 13,- 6.5, 6.5, 52, -0.5, 51.5);
  m_tHistSvc->regHist("/histfile/nhitsPlotB2",nhitsPlotB2).setChecked();

  // endcap
  TH2F* nhitsPlotEC=new TH2F("nhitsPlotEC", "Number of hits Endcap;Disk;module_phi", 7, -3.5, 3.5, 48, -0.5, 47.5);
  m_tHistSvc->regHist("/histfile/nhitsPlotEC",nhitsPlotEC).setChecked();
  
  // DBM
  TH2F* nhitsPlotDBM=new TH2F("nhitsPlotDBM", "Number of hits DBM;Layer;module_phi",7,-3.5,3.5,4,-0.5,3.5);
  m_tHistSvc->regHist("/histfile/nhitsPlotDBM",nhitsPlotDBM).setChecked();

  //------------------------
  // hits w/o noise
  //------------------------  

  // IBL
  TH2F* nhitsNoNoisePlotBI=new TH2F("nhitsNoNoisePlotBI","Number of hits without Noise BI;module_eta;module_phi", 20, -10, 10, 14, -0.5, 13.5); 
  m_tHistSvc->regHist("/histfile/nhitsNoNoisePlotBI",nhitsNoNoisePlotBI).setChecked();

  // B-layer
  TH2F* nhitsNoNoisePlotB0=new TH2F("nhitsNoNoisePlotB0","Number of hits without Noise B0;module_eta;module_phi", 13, -6.5, 6.5, 22, -0.5, 21.5);
  m_tHistSvc->regHist("/histfile/nhitsNoNoisePlotB0",nhitsNoNoisePlotB0).setChecked();

  // barrel layer 1
  TH2F* nhitsNoNoisePlotB1=new TH2F("nhitsNoNoisePlotB1","Number of hits without Noise B1;module_eta;module_phi", 13, -6.5, 6.5, 38, -0.5, 37.5);
  m_tHistSvc->regHist("/histfile/nhitsNoNoisePlotB1",nhitsNoNoisePlotB1).setChecked();

  // barrel layer 2
  TH2F* nhitsNoNoisePlotB2=new TH2F("nhitsNoNoisePlotB2","Number of hits without Noise B2;module_eta;module_phi", 13, -6.5, 6.5, 52, -0.5, 51.5);
  m_tHistSvc->regHist("/histfile/nhitsNoNoisePlotB2",nhitsNoNoisePlotB2).setChecked();

  //------------------------
  // disabled pixels
  //------------------------  

  // IBL
  TH2F* disablePlotBI=new TH2F("disablePlotBI", "Disabled pixels BI;module_eta;module_phi", 20, -10, 10, 14, -0.5, 13.5); 
  m_tHistSvc->regHist("/histfile/disablePlotBI",disablePlotBI).setChecked();

  // B-layer
  TH2F* disablePlotB0=new TH2F("disablePlotB0", "Disabled pixels B0;module_eta;module_phi", 13,- 6.5, 6.5, 22, -0.5, 21.5);
  m_tHistSvc->regHist("/histfile/disablePlotB0",disablePlotB0).setChecked();

  // barrel layer 1
  TH2F* disablePlotB1=new TH2F("disablePlotB1", "Disabled pixels B1;module_eta;module_phi", 13, -6.5, 6.5, 38, -0.5, 37.5);
  m_tHistSvc->regHist("/histfile/disablePlotB1",disablePlotB1).setChecked();

  // barrel layer 2
  TH2F* disablePlotB2=new TH2F("disablePlotB2", "Disabled pixels B2;module_eta;module_phi", 13, -6.5, 6.5, 52, -0.5, 51.5);
  m_tHistSvc->regHist("/histfile/disablePlotB2",disablePlotB2).setChecked();

  // endcap
  TH2F* disablePlotEC=new TH2F("disablePlotEC", "Disabled pixels Endcap;Disk;module_phi", 7, -3.5, 3.5, 48, -0.5, 47.5);
  m_tHistSvc->regHist("/histfile/disablePlotEC",disablePlotEC).setChecked();

  // DBM
  TH2F* disablePlotDBM=new TH2F("disablePlotDBM", "Disabled pixels DBM;Layer;module_phi", 7, -3.5, 3.5, 4, -0.5, 3.5);
  m_tHistSvc->regHist("/histfile/disablePlotDBM",disablePlotDBM).setChecked();

  TH1D* maskedPlot= new TH1D("maskedPlot","Disabled pixel per module",50,0.5,50.5);
  m_tHistSvc->regHist("/histfile/maskedPlot",maskedPlot).setChecked();

  int totalDisabledPixels=0;
  int totalDisabledModules=0;
  int modulesWithHits=0;
  int modulesWithDisabledPixels=0;

 
  //  std::sort(m_moduleHashList.begin(), m_moduleHashList.end());
  //  if(m_pixelID->wafer_hash_max() > 1744) m_isIBL = true; // #modules only Pixel is 1744
  //for(unsigned int moduleHash = 0; moduleHash < m_pixelID->wafer_hash_max(); moduleHash++)
  //for(std::vector<int>::iterator it = m_moduleHashList.begin(); it != m_moduleHashList.end(); ++it)

  //
  // loop in detector elements
  //
  for(InDetDD::SiDetectorElementCollection::const_iterator iter=m_pixman->getDetectorElementBegin(); 
      iter!=m_pixman->getDetectorElementEnd(); ++iter) {    

    const InDetDD::SiDetectorElement* element = *iter;
    if(element == 0) continue;

    Identifier ident = element->identify();
    if(!m_pixelID->is_pixel(ident)) continue;  

    int bec     = m_pixelID->barrel_ec (ident);
    int layer   = m_pixelID->layer_disk(ident);
    int modPhi  = m_pixelID->phi_module(ident);
    int modEta  = m_pixelID->eta_module(ident); 
    int modHash = m_pixelID->wafer_hash(ident);
    int phi_max = m_pixelID->phi_index_max(ident);
    int eta_max = m_pixelID->eta_index_max(ident);

    /*
    Identifier moduleID = m_pixelID->wafer_id(IdentifierHash(moduleHash));
    int barrel     = m_pixelID->barrel_ec(moduleID); // -2=ECC, 0=Barrel, 2=ECA
    int layer      = m_pixelID->layer_disk(moduleID);
    int module_phi = m_pixelID->phi_module(moduleID);
    int module_eta = m_pixelID->eta_module(moduleID);
    int phi_max    = m_pixelID->phi_index_max(moduleID);
    int eta_max    = m_pixelID->eta_index_max(moduleID);
    */

    //const InDetDD::PixelModuleDesign* design = dynamic_cast<const InDetDD::PixelModuleDesign*>(&element->design());
    //    if(!design) continue;
    //unsigned int mchips = design->numberOfCircuits();

   
    TH2F* nhitsNoNoisePlot=0; 
    std::string comp;
    double cut = 0.;

    if(bec != 0) { // Disk or DBM
      if(bec == 2) { 
        if(layer == 0)      {cut=m_disk1ACut; comp="Disk1A"; }
        else if(layer == 1) {cut=m_disk2ACut; comp="Disk2A"; }
        else if(layer == 2) {cut=m_disk3ACut; comp="Disk3A"; }
      }
      else if(bec == -2) { 
        if(layer == 0)      { cut=m_disk1CCut; comp="Disk1C"; }
        else if(layer == 1) { cut=m_disk2CCut; comp="Disk2C"; }
        else if(layer == 2) { cut=m_disk3CCut; comp="Disk3C"; }
      }
      else if(bec ==  4) { cut=m_dbmCut; comp="DBMA"; }
      else if(bec == -4) { cut=m_dbmCut; comp="DBMC"; }
    } 
    else if( bec == 0 ) { // Barrel
      if(layer == 0)        { cut=m_iblCut;    nhitsNoNoisePlot=nhitsNoNoisePlotBI; comp="IBL";     }
        else if(layer == 1) { cut=m_bLayerCut; nhitsNoNoisePlot=nhitsNoNoisePlotB0; comp="B-layer"; }
        else if(layer == 2) { cut=m_layer1Cut; nhitsNoNoisePlot=nhitsNoNoisePlotB1; comp="Layer1";  }
        else if(layer == 3) { cut=m_layer2Cut; nhitsNoNoisePlot=nhitsNoNoisePlotB2; comp="Layer2";  }
      } 

    if( m_BSErrorsSvc->getReadEvents(modHash)==0 && m_hitMaps[modHash]->GetEntries()==0 ) {

      /*      if (!m_isIBL) {
        unsigned int hashID = ( ((m_pixelID->barrel_ec(ident) + 2) / 2) << 25 ) +
          ( m_pixelID->layer_disk(ident) << 23) +
          ( m_pixelID->phi_module(ident) << 17) +
          ( (m_pixelID->eta_module(ident) + 6) << 13);
        ATH_MSG_INFO( "Disabled module "
            << PixelConvert::OnlineID(hashID) << "\t"
            << PixelConvert::DCSID(PixelConvert::OnlineID(hashID)) );
      }
      */

      if (bec== 0) {
	if(layer == 0)      { disablePlotBI->Fill(modEta,modPhi,-1); }
	else if(layer == 1) { disablePlotB0->Fill(modEta,modPhi,-1); }
	else if(layer == 2) { disablePlotB1->Fill(modEta,modPhi,-1); }
	else if(layer == 3) { disablePlotB2->Fill(modEta,modPhi,-1); }
      }
      else if(bec ==  2) { disablePlotEC->Fill(layer+1,     modPhi, -1); }
      else if(bec == -2) { disablePlotEC->Fill(-(layer+1),  modPhi, -1); }
      else if(bec ==  4) { disablePlotDBM->Fill(layer+1,    modPhi, -1); }
      else if(bec == -4) { disablePlotDBM->Fill(-(layer+1), modPhi, -1); }

      totalDisabledModules++;
      continue;
    }
    else if( m_hitMaps[modHash]->GetEntries() != 0 ) {
      if (bec== 0) {
	if(layer == 0)      { nhitsPlotBI->Fill(modEta, modPhi, m_hitMaps[modHash]->GetEntries()); }
	else if(layer == 1) { nhitsPlotB0->Fill(modEta, modPhi, m_hitMaps[modHash]->GetEntries()); }
	else if(layer == 2) { nhitsPlotB1->Fill(modEta, modPhi, m_hitMaps[modHash]->GetEntries()); }
	else if(layer == 3) { nhitsPlotB2->Fill(modEta, modPhi, m_hitMaps[modHash]->GetEntries()); }
      }
      else if(bec ==  2) { nhitsPlotEC->Fill(layer+1,     modPhi, m_hitMaps[modHash]->GetEntries()); }
      else if(bec == -2) { nhitsPlotEC->Fill(-(layer+1),  modPhi, m_hitMaps[modHash]->GetEntries()); }
      else if(bec ==  4) { nhitsPlotDBM->Fill(layer+1,    modPhi, m_hitMaps[modHash]->GetEntries()); }
      else if(bec == -4) { nhitsPlotDBM->Fill(-(layer+1), modPhi, m_hitMaps[modHash]->GetEntries()); }

      modulesWithHits++;
    }

    int thisModuleCut = 0;
    bool isIBL3D = ( bec == 0 && layer == 0 && (modEta <= -7 || modEta >= 6) ) ? true : false;

    for(int pixel_eta = 0; pixel_eta <= eta_max; pixel_eta++){
      for(int pixel_phi = 0; pixel_phi <= phi_max; pixel_phi++){

        // kazuki added from here
        int pixel_eta_on_chip = (bec == 0 && layer == 0) ? pixel_eta % 80 : pixel_eta % 18; // column
        int pixel_phi_on_chip = (pixel_phi <= 163) ? pixel_phi : 327 - pixel_phi; // eta
        if (bec == 0 && layer == 0) pixel_phi_on_chip = pixel_phi;
        int pixelType = 0;

        if (bec == 0 && layer == 0) { // ----- IBL ----- //
          if( !isIBL3D && (pixel_eta_on_chip == 0 || pixel_eta_on_chip == 80 - 1) ){
            pixelType = 1; // long
          }
          //else if(pixel_eta_on_chip > 0 && pixel_eta_on_chip < 80 - 1) // pixel size = 50x250 um2
          else { // pixel size = 50x250 um2
            pixelType = 0; // normal
          }
        } else { // Pixel
          if(pixel_eta_on_chip > 0 && pixel_eta_on_chip < 18 - 1){ // pixel size = 50x400 um2
            pixelType = 0; // normal
            for(int kk = 0; kk < 3; kk++){
              // row 154,156,158                       = inter-ganged
              // row 153,155,157,159,  160,161,162,163 = ganged
              if(pixel_phi_on_chip == (153 + 2 * kk + 1)){
                pixelType = 5; // inter-ganged (dealt as normal)
                break;
              }
              if(pixel_phi_on_chip == (153 + 2 * kk) || pixel_phi_on_chip >= 159){
                pixelType = 2; // ganged
                break;
              }
            }
          }
          else if(pixel_eta_on_chip == 0 || pixel_eta_on_chip == 18 - 1){
            pixelType =  1; //long
            for(int kk = 0; kk < 3; kk++){
              if(pixel_phi_on_chip == (153 + 2 * kk + 1)){
                pixelType = 6; // long inter-ganged (dealt as long)
                break;
              }
              if(pixel_phi_on_chip == (153 + 2 * kk) || pixel_phi_on_chip >= 159){
                pixelType = 3; // long ganged
                break;
              }
            }
          }
          else
            pixelType =  8; //invalid pixel_phi/pixel_eta pair
        }
        // to here

        std::string type;
         // kazuki commented out
        //int pixel_eta_on_chip = (m_isIBL && bec == 0 && layer == 0) ? pixel_eta % 80 : pixel_eta % 18; // column
        //int pixel_phi_on_chip = (pixel_phi <= 163) ? pixel_phi : 327 - pixel_phi; // eta
        //if (m_isIBL && bec == 0 && layer == 0) pixel_phi_on_chip = pixel_phi;
         //  unsigned int pixelType = ModuleSpecialPixelMap::
          // pixelType( pixel_eta_on_chip, pixel_phi_on_chip, mchips );

        switch(pixelType) {
          case 0:
            type = "Normal";
            break;
          case 1:
            type = "Long";
            break;
          case 2:
            type = "Ganged";
            break;
          case 3:
            type = "Long-Ganged";
            break;
          case 5:
            type = "Long-InterGanged";
            break;
          case 6:
            type = "InterGanged";
            break;
          case 8:
          default:
            type = "Invalid";
            break;
        }

        double thiscut = cut;
        if( type == "Ganged" )                thiscut *= m_gangedPixelMultiplier;
        else if( type == "Long" )             thiscut *= m_longPixelMultiplier;
        else if( type == "Long-InterGanged" ) thiscut *= m_longPixelMultiplier;
        else if( type == "Long-Ganged" )      thiscut *= m_longPixelMultiplier * m_gangedPixelMultiplier;

        if( type != "Invalid" ){
          double occupancy = static_cast<double>(m_hitMaps[modHash]->GetBinContent(pixel_eta+1, pixel_phi+1)) /
            static_cast<double>(m_nEvents);
	  
          if( occupancy < minOccupancy ) occupancy = minOccupancy;
          globalOccupancy->Fill(log10(occupancy));
          h_occupancy[comp]->Fill(log10(occupancy));
          h_occupancy[type]->Fill(log10(occupancy));

          if( occupancy > thiscut ) {
            thisModuleCut++;

            if( m_calculateNoiseMaps ){
              m_noiseMaps[modHash]->Fill(pixel_eta, pixel_phi);
              if (comp == "IBL") {
                if(modEta >= -6 && modEta <= 5) m_overlayedIBLDCNoiseMap->Fill(pixel_eta, pixel_phi); // Planar
                if(modEta <= -7 || modEta >= 6) m_overlayedIBLSCNoiseMap->Fill(pixel_eta, pixel_phi); // 3D
              }
              else m_overlayedPixelNoiseMap->Fill(pixel_eta, pixel_phi);
            }
          } else {
            if ( bec == 0 ) nhitsNoNoisePlot->Fill(modEta,modPhi, m_hitMaps[modHash]->GetBinContent(pixel_eta+1, pixel_phi+1));
          }
        } // end if ( type != "Invalid" )
      } // end for loop on pixel_phi
    } // end for loop on pixel_eta

    if ( thisModuleCut > 0 ) {
      totalDisabledPixels+=thisModuleCut;
      maskedPlot->Fill( static_cast<double>(thisModuleCut) );
      modulesWithDisabledPixels++;
   
      if(bec== 0){
	if(layer == 0)      { disablePlotBI->Fill(modEta, modPhi, thisModuleCut); }
	else if(layer == 1) { disablePlotB0->Fill(modEta, modPhi, thisModuleCut); }
	else if(layer == 2) { disablePlotB1->Fill(modEta, modPhi, thisModuleCut); }
	else if(layer == 3) { disablePlotB2->Fill(modEta, modPhi, thisModuleCut); }
      }
      else if(bec== 2) { disablePlotEC->Fill(layer+1,    modPhi, thisModuleCut); }
      else if(bec==-2) { disablePlotEC->Fill(-(layer+1), modPhi, thisModuleCut); }
    }
  } // end for loop on modHash
  
  ATH_MSG_INFO("Modules disabled = " << totalDisabledModules);
  ATH_MSG_INFO("Modules with hits = " << modulesWithHits);
  ATH_MSG_INFO("Modules with disabled pixels = " << modulesWithDisabledPixels);
  ATH_MSG_INFO("Total disabled pixels = " << totalDisabledPixels);

  return StatusCode::SUCCESS;
  
} // end finalize

