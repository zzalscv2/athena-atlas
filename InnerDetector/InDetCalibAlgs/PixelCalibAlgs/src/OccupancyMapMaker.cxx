/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// PixelCalibAlgs
#include "PixelCalibAlgs/OccupancyMapMaker.h"
#include "PixelCalibAlgs/PixelConvert.h"

// Gaudi
#include "GaudiKernel/ITHistSvc.h"

// EDM
#include "InDetRawData/PixelRDO_Container.h"

// geometry
#include "InDetIdentifier/PixelID.h"

// Thread safety check
#include "CxxUtils/checker_macros.h"

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

OccupancyMapMaker::OccupancyMapMaker(const std::string& name, ISvcLocator* pSvcLocator) :
  AthAlgorithm(name, pSvcLocator),
  m_tHistSvc("THistSvc", name),
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
  m_calculateNoiseMaps(false),
  m_mapFile("PixelMapping_Run2.dat")
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
  declareProperty("nLBmax", m_hist_lbMax, "Maximum number of LB (for histograms binning)");
  declareProperty("NBCReadout", m_nBCReadout, "Number of bunch crossings read out");
  declareProperty("LBMin", m_evt_lbMin, "First lumi block to consider");
  declareProperty("LBMax", m_evt_lbMax, "Last lumi block to consider");
  declareProperty("LongPixelMultiplier", m_longPixelMultiplier, "Multiplier for long pixels");
  declareProperty("GangedPixelMultiplier", m_gangedPixelMultiplier, "Multiplier for ganged pixels");
  declareProperty("OccupancyPerBC", m_occupancyPerBC, "Calculate occupancy per BC or per event");
  declareProperty("CalculateNoiseMaps", m_calculateNoiseMaps, "If false only build hit maps");
  declareProperty("InputPixelMap", m_mapFile);
  declareProperty("THistSvc", m_tHistSvc, "THistSvc");
}

OccupancyMapMaker::~OccupancyMapMaker(){}

StatusCode OccupancyMapMaker::initialize(){
  ATH_MSG_INFO("Initializing OccupancyMapMaker");

  ATH_CHECK(m_tHistSvc.retrieve());
  ATH_CHECK(m_pixelConditionsTool.retrieve());
  ATH_CHECK(detStore()->retrieve(m_pixelID, "PixelID"));
 
  // resize vectors of histograms
  const Identifier::size_type maxHash = m_pixelID->wafer_hash_max();
  ATH_MSG_INFO("PixelID maxHash = " << maxHash);
  m_hitMaps.resize(maxHash);
  m_LBdependence.resize(maxHash);
  m_BCIDdependence.resize(maxHash);
  m_TOTdistributions.resize(maxHash);
  if(m_calculateNoiseMaps) {
    m_noiseMaps.resize(maxHash);
  }
  return registerHistograms(); 
}

std::string OccupancyMapMaker::getDCSIDFromPosition (int barrel_ec, int layer, int modPhi, int module_eta){
  for(unsigned int ii = 0; ii < m_pixelMapping.size(); ii++) {
    if (m_pixelMapping[ii].second.size() != 4) {
      ATH_MSG_WARNING( "getDCSIDFromPosition: Vector size is not 4!" );
      return std::string("Error!");
    }
    if (m_pixelMapping[ii].second[0] != barrel_ec) continue;
    if (m_pixelMapping[ii].second[1] != layer) continue;
    if (m_pixelMapping[ii].second[2] != modPhi) continue;
    if (m_pixelMapping[ii].second[3] != module_eta) continue;
    return m_pixelMapping[ii].first;
  }
  ATH_MSG_WARNING( "Not found!" );
  return std::string("Error!");
}

const std::string OccupancyMapMaker::histoSuffix(const int bec, const int layer){
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

std::vector<std::string>& OccupancyMapMaker::splitter(const std::string &str,
						    char delim, 
						    std::vector<std::string> &elems) {
  std::stringstream ss(str);
  std::string item;
  while (std::getline(ss, item, delim)) {
    elems.push_back(item);
  }
  return elems;
}

std::vector<std::string> OccupancyMapMaker::splitter(const std::string &str, 
						   char delim) {
  std::vector<std::string> elems;
  splitter(str, delim, elems);
  return elems;
}

StatusCode OccupancyMapMaker::registerHistograms(){

  char* getenvPath = std::getenv("DATAPATH");
  const unsigned int maxPathStringLength{3000};
  if((not getenvPath) or (strlen(getenvPath) > maxPathStringLength) ){
      ATH_MSG_FATAL( "Unable to retrieve environmental DATAPATH" );
      return StatusCode::FAILURE;
  }
  std::stringstream tmpSstr{};
  tmpSstr<<getenvPath;
  std::string tmppath(tmpSstr.str());
 
  std::vector<std::string> paths = splitter(tmppath, ':'); 
  bool found(false);  
  for(const auto& x : paths){
    std::ifstream infile( (x+"/"+m_mapFile).c_str() );
    if( infile.is_open() ){
      ATH_MSG_INFO("Mapping file '" << m_mapFile << "' found in " << x);

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
    ATH_MSG_FATAL("Mapping file '" << m_mapFile << "' not found in DATAPATH !!!");
    return StatusCode::FAILURE;
  }

  m_nEventsHist = new TH1D("NEvents", "NEvents", 1, 0, 1);
  m_tHistSvc->regHist("/histfile/NEvents", m_nEventsHist).ignore();
  
  m_nEventsLBHist = new TH1D("NEventsLB", "NEventsLB", m_hist_lbMax, -0.5, m_hist_lbMax+0.5);
  m_tHistSvc->regHist("/histfile/NEventsLB", m_nEventsLBHist).ignore();

  for (PixelID::const_id_iterator wafer_it=m_pixelID->wafer_begin(); wafer_it!=m_pixelID->wafer_end(); ++wafer_it) {
    Identifier ident = *wafer_it;
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
    m_tHistSvc->regHist(name.str().c_str(), m_hitMaps[modHash]).ignore();
    name.str(""); name.clear();


    // LB dependence
    m_LBdependence[modHash] = new TH1D(onlineID.c_str(), onlineID.c_str(), m_hist_lbMax, -0.5, m_hist_lbMax + 0.5);
    name  << "/histfile/LBdep_" << histoSuffix(bec,layer) << "/" << onlineID;
    m_tHistSvc->regHist(name.str().c_str(), m_LBdependence[modHash]).ignore();
    name.str(""); name.clear();
    
    // BCID dependence
    m_BCIDdependence[modHash] = new TH1D(onlineID.c_str(), onlineID.c_str(), 301, -0.5, 300.5);
    name << "/histfile/BCIDdep_" << histoSuffix(bec,layer) << "/" << onlineID;
    m_tHistSvc->regHist(name.str().c_str(), m_BCIDdependence[modHash]).ignore();
    name.str(""); name.clear();
    
    // TOT
    if( bec == 0 && layer == 0) // IBL
      m_TOTdistributions[modHash] = new TH1D(onlineID.c_str(), onlineID.c_str(), 19, -0.5, 18.5);
    else
      m_TOTdistributions[modHash] = new TH1D(onlineID.c_str(), onlineID.c_str(), 256, -0.5, 255.5);
    name << "/histfile/TOT_" << histoSuffix(bec,layer) << "/" << onlineID;
    m_tHistSvc->regHist(name.str().c_str(), m_TOTdistributions[modHash]).ignore();
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
      m_tHistSvc->regHist(name.str().c_str(), m_noiseMaps[modHash]).ignore();
      name.str(""); name.clear();
    }
  } // end loop in detector elements 
  
  m_disabledModules = new TH1D("DisabledModules", "Number of events disabled vs. IdentifierHash", 2048, 0, 2048);
  m_tHistSvc->regHist("/histfile/DisabledModules", m_disabledModules).ignore();
  
  if (m_calculateNoiseMaps) {
    m_overlayedPixelNoiseMap = new TH2D("overlayedPixelNoiseMap", "Noisy pixel map overlayed all Pixel modules", 144, 0, 144, 328, 0, 328);
    m_tHistSvc->regHist("/histfile/overlayedPixelNoiseMap", m_overlayedPixelNoiseMap).ignore();
    
    m_overlayedIBLDCNoiseMap = new TH2D("overlayedIBLDCNoiseMap", "Noisy pixel map overlayed all IBL Planar modules", 160, 0, 160, 336, 0, 336);
    m_tHistSvc->regHist("/histfile/overlayedIBLDCNoiseMap", m_overlayedIBLDCNoiseMap).ignore();
    
    m_overlayedIBLSCNoiseMap = new TH2D("overlayedIBLSCNoiseMap", "Noisy pixel map overlayed all IBL 3D modules", 80, 0, 80, 336, 0, 336);
    m_tHistSvc->regHist("/histfile/overlayedIBLSCNoiseMap", m_overlayedIBLSCNoiseMap).ignore();
  }  
  
  return StatusCode::SUCCESS;
} 

//=========================================================
//
// execute
//
//=========================================================
StatusCode OccupancyMapMaker::execute(){
  ATH_MSG_DEBUG( "Executing OccupancyMapMaker" );

  const EventContext& ctx = Gaudi::Hive::currentContext();

  // check LB is in allowed range
  unsigned int LB =  ctx.eventID().lumi_block();
  if( (LB < m_evt_lbMin) || (m_evt_lbMax >= m_evt_lbMin && LB > m_evt_lbMax) ){
    ATH_MSG_VERBOSE("Event in lumiblock " << LB <<
		    " not in selected range [" << m_evt_lbMin << "," << m_evt_lbMax << "] => skipped");    
    return StatusCode::SUCCESS;
  }

  // retrieve PixelRDO container
  const PixelRDO_Container* pixelRDOs = nullptr;
  StatusCode sc = evtStore()->retrieve(pixelRDOs, m_pixelRDOKey);
  if( !sc.isSuccess() ){
    ATH_MSG_FATAL( "Unable to retrieve pixel RDO container at " << m_pixelRDOKey );
    return StatusCode::FAILURE;
  } ATH_MSG_DEBUG( "Pixel RDO container retrieved" );

  // loop in RDO container
  for(PixelRDO_Container::const_iterator coll=pixelRDOs->begin(); 
      coll!=pixelRDOs->end(); ++coll){

    const InDetRawDataCollection<PixelRDORawData>* PixelRDOCollection(*coll);
    if(PixelRDOCollection != 0){
      Identifier moduleID = PixelRDOCollection->identify();
      IdentifierHash modHash = m_pixelID->wafer_hash(moduleID);
      ATH_MSG_VERBOSE("moduleID, modHash = " << moduleID << " , " << modHash);
      
      // exclude module if reported as not good by PixelConditionsSummaryTool
      if( !(m_pixelConditionsTool->isGood(modHash, ctx)) ) {
	ATH_MSG_VERBOSE("Module excluded as reported not good by PixelConditionsSummaryTool");
        continue;
      }

      // exclude module if containg FE synch errors
      if (m_pixelConditionsTool->hasBSError(modHash, ctx)) {
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

        m_hitMaps[modHash]->Fill(pixel_eta, pixel_phi);
        m_LBdependence[modHash]->Fill(LB);
        m_BCIDdependence[modHash]->Fill(BCID);
        m_TOTdistributions[modHash]->Fill(TOT);
      }
    }
  }


  // [sgs] why is this done in every event ???
  for(unsigned int moduleHash = 0; moduleHash < m_pixelID->wafer_hash_max(); moduleHash++) {
    if( !m_pixelConditionsTool->isActive( moduleHash, ctx ) ){
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
StatusCode OccupancyMapMaker::finalize() {
  ATH_MSG_INFO("Finalizing OccupancyMapMaker");

  if(m_occupancyPerBC)
    m_nEvents *= m_nBCReadout;
 
  const int minLogOccupancy = 8;
  const double minOccupancy = pow(10.,-minLogOccupancy);
  
  TH1D* globalOccupancy= new TH1D("occupancy", "Pixel occupancy", minLogOccupancy*10, -minLogOccupancy, 0.);
  m_tHistSvc->regHist("/histfile/occupancy",globalOccupancy).ignore();

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
    m_tHistSvc->regHist(("/histfile/occupancy"+comp).c_str(), h_occupancy[comp]).ignore();
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
    m_tHistSvc->regHist(("/histfile/occupancy"+type).c_str(), h_occupancy[type]).ignore();
  }
  vtype.clear();

  //------------------------
  // number of hits 
  //------------------------  

  // IBL
  TH2F* nhitsPlotBI=new TH2F("nhitsPlotBI", "Number of hits BI;module_eta;module_phi", 20, -10, 10, 14, -0.5, 13.5); 
  m_tHistSvc->regHist("/histfile/nhitsPlotBI",nhitsPlotBI).ignore();

  // B-layer
  TH2F* nhitsPlotB0=new TH2F("nhitsPlotB0", "Number of hits B0;module_eta;module_phi", 13, -6.5, 6.5, 22, -0.5, 21.5);
  m_tHistSvc->regHist("/histfile/nhitsPlotB0",nhitsPlotB0).ignore();

  // barrel layer 1
  TH2F* nhitsPlotB1=new TH2F("nhitsPlotB1", "Number of hits B1;module_eta;module_phi", 13, -6.5, 6.5, 38, -0.5, 37.5);
  m_tHistSvc->regHist("/histfile/nhitsPlotB1",nhitsPlotB1).ignore();

  // barrel layer 2
  TH2F* nhitsPlotB2=new TH2F("nhitsPlotB2", "Number of hits B2;module_eta;module_phi", 13,- 6.5, 6.5, 52, -0.5, 51.5);
  m_tHistSvc->regHist("/histfile/nhitsPlotB2",nhitsPlotB2).ignore();

  // endcap
  TH2F* nhitsPlotEC=new TH2F("nhitsPlotEC", "Number of hits Endcap;Disk;module_phi", 7, -3.5, 3.5, 48, -0.5, 47.5);
  m_tHistSvc->regHist("/histfile/nhitsPlotEC",nhitsPlotEC).ignore();
  
  // DBM
  TH2F* nhitsPlotDBM=new TH2F("nhitsPlotDBM", "Number of hits DBM;Layer;module_phi",7,-3.5,3.5,4,-0.5,3.5);
  m_tHistSvc->regHist("/histfile/nhitsPlotDBM",nhitsPlotDBM).ignore();

  //------------------------
  // hits w/o noise
  //------------------------  

  // IBL
  TH2F* nhitsNoNoisePlotBI=new TH2F("nhitsNoNoisePlotBI","Number of hits without Noise BI;module_eta;module_phi", 20, -10, 10, 14, -0.5, 13.5); 
  m_tHistSvc->regHist("/histfile/nhitsNoNoisePlotBI",nhitsNoNoisePlotBI).ignore();

  // B-layer
  TH2F* nhitsNoNoisePlotB0=new TH2F("nhitsNoNoisePlotB0","Number of hits without Noise B0;module_eta;module_phi", 13, -6.5, 6.5, 22, -0.5, 21.5);
  m_tHistSvc->regHist("/histfile/nhitsNoNoisePlotB0",nhitsNoNoisePlotB0).ignore();

  // barrel layer 1
  TH2F* nhitsNoNoisePlotB1=new TH2F("nhitsNoNoisePlotB1","Number of hits without Noise B1;module_eta;module_phi", 13, -6.5, 6.5, 38, -0.5, 37.5);
  m_tHistSvc->regHist("/histfile/nhitsNoNoisePlotB1",nhitsNoNoisePlotB1).ignore();

  // barrel layer 2
  TH2F* nhitsNoNoisePlotB2=new TH2F("nhitsNoNoisePlotB2","Number of hits without Noise B2;module_eta;module_phi", 13, -6.5, 6.5, 52, -0.5, 51.5);
  m_tHistSvc->regHist("/histfile/nhitsNoNoisePlotB2",nhitsNoNoisePlotB2).ignore();

  //------------------------
  // disabled pixels
  //------------------------  

  // IBL
  TH2F* disablePlotBI=new TH2F("disablePlotBI", "Disabled pixels BI;module_eta;module_phi", 20, -10, 10, 14, -0.5, 13.5); 
  m_tHistSvc->regHist("/histfile/disablePlotBI",disablePlotBI).ignore();

  // B-layer
  TH2F* disablePlotB0=new TH2F("disablePlotB0", "Disabled pixels B0;module_eta;module_phi", 13,- 6.5, 6.5, 22, -0.5, 21.5);
  m_tHistSvc->regHist("/histfile/disablePlotB0",disablePlotB0).ignore();

  // barrel layer 1
  TH2F* disablePlotB1=new TH2F("disablePlotB1", "Disabled pixels B1;module_eta;module_phi", 13, -6.5, 6.5, 38, -0.5, 37.5);
  m_tHistSvc->regHist("/histfile/disablePlotB1",disablePlotB1).ignore();

  // barrel layer 2
  TH2F* disablePlotB2=new TH2F("disablePlotB2", "Disabled pixels B2;module_eta;module_phi", 13, -6.5, 6.5, 52, -0.5, 51.5);
  m_tHistSvc->regHist("/histfile/disablePlotB2",disablePlotB2).ignore();

  // endcap
  TH2F* disablePlotEC=new TH2F("disablePlotEC", "Disabled pixels Endcap;Disk;module_phi", 7, -3.5, 3.5, 48, -0.5, 47.5);
  m_tHistSvc->regHist("/histfile/disablePlotEC",disablePlotEC).ignore();

  // DBM
  TH2F* disablePlotDBM=new TH2F("disablePlotDBM", "Disabled pixels DBM;Layer;module_phi", 7, -3.5, 3.5, 4, -0.5, 3.5);
  m_tHistSvc->regHist("/histfile/disablePlotDBM",disablePlotDBM).ignore();

  TH1D* maskedPlot= new TH1D("maskedPlot","Disabled pixel per module",50,0.5,50.5);
  m_tHistSvc->regHist("/histfile/maskedPlot",maskedPlot).ignore();

  int totalDisabledPixels=0;
  int totalDisabledModules=0;
  int modulesWithHits=0;
  int modulesWithDisabledPixels=0;

  const EventContext& ctx{Gaudi::Hive::currentContext()};
  for (PixelID::const_id_iterator wafer_it=m_pixelID->wafer_begin(); wafer_it!=m_pixelID->wafer_end(); ++wafer_it) {
    Identifier ident = *wafer_it;
    if(!m_pixelID->is_pixel(ident)) continue;  

    int bec     = m_pixelID->barrel_ec (ident);
    int layer   = m_pixelID->layer_disk(ident);
    int modPhi  = m_pixelID->phi_module(ident);
    int modEta  = m_pixelID->eta_module(ident); 
    int modHash = m_pixelID->wafer_hash(ident);
    int phi_max = m_pixelID->phi_index_max(ident);
    int eta_max = m_pixelID->eta_index_max(ident);

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
    else if(bec == 0) { // Barrel
      if(layer == 0)        { cut=m_iblCut;    nhitsNoNoisePlot=nhitsNoNoisePlotBI; comp="IBL";     }
        else if(layer == 1) { cut=m_bLayerCut; nhitsNoNoisePlot=nhitsNoNoisePlotB0; comp="B-layer"; }
        else if(layer == 2) { cut=m_layer1Cut; nhitsNoNoisePlot=nhitsNoNoisePlotB1; comp="Layer1";  }
        else if(layer == 3) { cut=m_layer2Cut; nhitsNoNoisePlot=nhitsNoNoisePlotB2; comp="Layer2";  }
    } 

    if (!m_pixelConditionsTool->hasBSError(modHash, ctx) && m_hitMaps[modHash]->GetEntries()==0) {
      if(bec == 0) {
	if(layer == 0)      { disablePlotBI->Fill(modEta, modPhi, -1); }
	else if(layer == 1) { disablePlotB0->Fill(modEta, modPhi, -1); }
	else if(layer == 2) { disablePlotB1->Fill(modEta, modPhi, -1); }
	else if(layer == 3) { disablePlotB2->Fill(modEta, modPhi, -1); }
      }
      else if(bec ==  2) { disablePlotEC->Fill(layer+1,     modPhi, -1); }
      else if(bec == -2) { disablePlotEC->Fill(-(layer+1),  modPhi, -1); }
      else if(bec ==  4) { disablePlotDBM->Fill(layer+1,    modPhi, -1); }
      else if(bec == -4) { disablePlotDBM->Fill(-(layer+1), modPhi, -1); }

      totalDisabledModules++;
      continue;
    }
    else if( m_hitMaps[modHash]->GetEntries() != 0 ) {
      if(bec == 0) {
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
    bool isIBL3D = ( bec==0 && layer==0 && (modEta <= -7 || modEta >= 6) ) ? true : false;

    for(int pixel_eta=0; pixel_eta<=eta_max; pixel_eta++){
      for(int pixel_phi=0; pixel_phi<=phi_max; pixel_phi++){

        // kazuki added from here
        int pixel_eta_on_chip = (bec==0 && layer==0) ? pixel_eta % 80 : pixel_eta % 18; // column
        int pixel_phi_on_chip = (pixel_phi <= 163) ? pixel_phi : 327 - pixel_phi; // eta
        if (bec == 0 && layer == 0) pixel_phi_on_chip = pixel_phi;
        int pixelType = 0;

        if (bec == 0 && layer == 0) { // ----- IBL ----- //
          if( !isIBL3D && (pixel_eta_on_chip == 0 || pixel_eta_on_chip == 80 - 1) ){
            pixelType = 1; // long
          }
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
	  double occupancy = 0;
	  if( m_nEvents != 0 ) 
	    occupancy = static_cast<double>(m_hitMaps[modHash]->GetBinContent(pixel_eta+1, pixel_phi+1)) /
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
            if(bec == 0) nhitsNoNoisePlot->Fill(modEta, modPhi, m_hitMaps[modHash]->GetBinContent(pixel_eta+1, pixel_phi+1));
          }
        } // end if ( type != "Invalid" )
      } // end for loop on pixel_phi
    } // end for loop on pixel_eta

    if ( thisModuleCut > 0 ) {
      totalDisabledPixels+=thisModuleCut;
      maskedPlot->Fill( static_cast<double>(thisModuleCut) );
      modulesWithDisabledPixels++;
   
      if(bec == 0) {
	if(layer == 0)      { disablePlotBI->Fill(modEta, modPhi, thisModuleCut); }
	else if(layer == 1) { disablePlotB0->Fill(modEta, modPhi, thisModuleCut); }
	else if(layer == 2) { disablePlotB1->Fill(modEta, modPhi, thisModuleCut); }
	else if(layer == 3) { disablePlotB2->Fill(modEta, modPhi, thisModuleCut); }
      }
      else if(bec ==  2) { disablePlotEC->Fill(layer+1,    modPhi, thisModuleCut); }
      else if(bec == -2) { disablePlotEC->Fill(-(layer+1), modPhi, thisModuleCut); }
    }
  } // end loop in detector elements
  
  ATH_MSG_INFO("Modules disabled = " << totalDisabledModules);
  ATH_MSG_INFO("Modules with hits = " << modulesWithHits);
  ATH_MSG_INFO("Modules with disabled pixels = " << modulesWithDisabledPixels);
  ATH_MSG_INFO("Total disabled pixels = " << totalDisabledPixels);

  return StatusCode::SUCCESS;
  
} // end finalize

