/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
//***************************************************************************
//		jFEXForwardElecAlgo - Algorithm for Forward Electron Algorithm in jFEX
//                              -------------------
//     begin                : 16 11 2021
//     email                : Sergi.Rodriguez@cern.ch
//     email                : ulla.blumenschein@cern.ch
//     email                : sjolin@cern.ch
//***************************************************************************

#include <iostream>
#include <vector>
#include <stdio.h>
#include <math.h>
#include <fstream>
#include "L1CaloFEXSim/jFEXForwardElecAlgo.h"
#include "L1CaloFEXSim/jFEXForwardElecTOB.h"
#include "L1CaloFEXSim/jFEXForwardElecInfo.h"
#include "L1CaloFEXSim/jTower.h"
#include "L1CaloFEXSim/jTowerContainer.h"
#include "CaloEvent/CaloCellContainer.h"
#include "CaloIdentifier/CaloIdManager.h"
#include "CaloIdentifier/CaloCell_SuperCell_ID.h"
#include "AthenaBaseComps/AthAlgorithm.h"
#include "StoreGate/StoreGateSvc.h"
#include "PathResolver/PathResolver.h"

namespace LVL1 {

  //Default Constructor
  LVL1::jFEXForwardElecAlgo::jFEXForwardElecAlgo(const std::string& type, const std::string& name, const IInterface* parent): AthAlgTool(type, name, parent) {
    declareInterface<IjFEXForwardElecAlgo>(this);
  }
  
  /** Destructor */
  LVL1::jFEXForwardElecAlgo::~jFEXForwardElecAlgo() {
  }
  
  StatusCode LVL1::jFEXForwardElecAlgo::initialize() {
    ATH_CHECK(m_jTowerContainerKey.initialize());

    ATH_CHECK(ReadfromFile(PathResolver::find_calib_file(m_SeedRingStr), m_SeedRingMap ));
    ATH_CHECK(ReadfromFile(PathResolver::find_calib_file(m_1stRingStr), m_1stRingMap));
    ATH_CHECK(ReadfromFile(PathResolver::find_calib_file(m_SearchGStr), m_SearchGMap));
    ATH_CHECK(ReadfromFile(PathResolver::find_calib_file(m_SearchGeStr), m_SearchGeMap ));
    ATH_CHECK(ReadfromFile(PathResolver::find_calib_file(m_SearchGTauStr), m_SearchGTauMap));
    ATH_CHECK(ReadfromFile(PathResolver::find_calib_file(m_SearchGeTauStr), m_SearchGeTauMap));

    return StatusCode::SUCCESS;
  }
  
  //calls container for TT
  StatusCode LVL1::jFEXForwardElecAlgo::safetyTest() {
    m_jTowerContainer = SG::ReadHandle<jTowerContainer>(m_jTowerContainerKey);
    if(! m_jTowerContainer.isValid()) {
      ATH_MSG_ERROR("Could not retrieve jTowerContainer " << m_jTowerContainerKey.key());
      return StatusCode::FAILURE;
    }
    return StatusCode::SUCCESS;
  }

  StatusCode LVL1::jFEXForwardElecAlgo::reset() {
    return StatusCode::SUCCESS;
  }
    
  void LVL1::jFEXForwardElecAlgo::setup(int inputTable[FEXAlgoSpaceDefs::jFEX_algoSpace_height][FEXAlgoSpaceDefs::jFEX_wide_algoSpace_width], int jfex, int fpga) {
    std::copy(&inputTable[0][0], &inputTable[0][0] + (FEXAlgoSpaceDefs::jFEX_algoSpace_height*FEXAlgoSpaceDefs::jFEX_wide_algoSpace_width), &m_jFEXalgoTowerID[0][0]);
    m_jfex=jfex;
    m_fpga=fpga;
  }

  //global centre Eta and Phi coord of the TT
  std::array<float,2> LVL1::jFEXForwardElecAlgo::getEtaPhi(uint ttID) {
    if(ttID == 0) {
      return {999,999};
    }
    const LVL1::jTower *tmpTower = m_jTowerContainer->findTower(ttID);
    return {tmpTower->centreEta(),tmpTower->centrePhi()};
  }

  std::array<uint,2> LVL1::jFEXForwardElecAlgo::getEtEmHad(uint ttID) {
    if(ttID == 0) {
      return {0,0};
    }
    uint TT_EtEM = 0;
    if(m_map_Etvalues_EM.find(ttID) != m_map_Etvalues_EM.end()) {
      TT_EtEM = m_map_Etvalues_EM[ttID][0];
    }
    uint TT_EtHad = 0;
    if(m_map_Etvalues_HAD.find(ttID) != m_map_Etvalues_HAD.end()) {
      TT_EtHad = m_map_Etvalues_HAD[ttID][0];
    }

    return {TT_EtEM, TT_EtHad};
  }

  void LVL1::jFEXForwardElecAlgo::setFPGAEnergy(
    std::unordered_map<int,std::vector<int> > etmapEM,
    std::unordered_map<int,std::vector<int> > etmapHAD) {
    m_map_Etvalues_EM=etmapEM; 
    m_map_Etvalues_HAD=etmapHAD;
  }
  
  std::unordered_map<uint, LVL1::jFEXForwardElecInfo> LVL1::jFEXForwardElecAlgo::eleClusterList(void) {
    std::unordered_map<uint, LVL1::jFEXForwardElecInfo> clusterList;
    std::vector<int> lower_centre_neta;
    std::vector<int> upper_centre_neta;
    m_lowerEM_eta = 0;
    m_upperEM_eta = 0;
   
    //check if we are in module 0 or 5 and assign corrrect eta FEXAlgoSpace parameters
    if(m_jfex == 0) {
      //Module 0 
      lower_centre_neta.assign({FEXAlgoSpaceDefs::jFEX_algoSpace_C_EMB_start_eta, FEXAlgoSpaceDefs::jFEX_algoSpace_C_EMIE_start_eta, FEXAlgoSpaceDefs::jFEX_algoSpace_C_FCAL_start_eta});
      upper_centre_neta.assign({FEXAlgoSpaceDefs::jFEX_algoSpace_C_EMB_end_eta, FEXAlgoSpaceDefs::jFEX_algoSpace_C_EMIE_end_eta,FEXAlgoSpaceDefs::jFEX_algoSpace_C_FCAL_end_eta });
      m_lowerEM_eta = FEXAlgoSpaceDefs::jFEX_algoSpace_C_lowerEM_eta;
      m_upperEM_eta = FEXAlgoSpaceDefs::jFEX_algoSpace_C_upperEM_eta;
    }
    else {
      //Module 5
      lower_centre_neta.assign({FEXAlgoSpaceDefs::jFEX_algoSpace_A_EMB_eta, FEXAlgoSpaceDefs::jFEX_algoSpace_A_EMIE_eta, FEXAlgoSpaceDefs::jFEX_algoSpace_A_FCAL_start_eta});
      upper_centre_neta.assign({FEXAlgoSpaceDefs::jFEX_algoSpace_A_EMIE_eta, FEXAlgoSpaceDefs::jFEX_algoSpace_A_FCAL_start_eta, FEXAlgoSpaceDefs::jFEX_algoSpace_A_FCAL_end_eta});
      m_lowerEM_eta = FEXAlgoSpaceDefs::jFEX_algoSpace_A_lowerEM_eta;
      m_upperEM_eta = FEXAlgoSpaceDefs::jFEX_algoSpace_A_upperEM_eta;
    }

    //define phi FEXAlgoSpace parameters
    std::vector<int> lower_centre_nphi{FEXAlgoSpaceDefs::jFEX_algoSpace_EMB_start_phi, FEXAlgoSpaceDefs::jFEX_algoSpace_EMIE_start_phi,  FEXAlgoSpaceDefs::jFEX_algoSpace_FCAL_start_phi};
    std::vector<int> upper_centre_nphi{FEXAlgoSpaceDefs::jFEX_algoSpace_EMB_end_phi, FEXAlgoSpaceDefs::jFEX_algoSpace_EMIE_end_phi,  FEXAlgoSpaceDefs::jFEX_algoSpace_FCAL_end_phi};

    //loop over different EM/FCAL1 eta phi core fpga regions. These are potential seed  towers for electron clusters 
    for(uint i = 0; i<3; i++) {
      for(int nphi = lower_centre_nphi[i]; nphi < upper_centre_nphi[i]; nphi++) {
	      for(int neta = lower_centre_neta[i]; neta < upper_centre_neta[i]; neta++) {
          
          // ignore  seeds for |eta| < 2.3 or from the first FCAL eta bin                
          if (m_jfex == 0 && neta >= FEXAlgoSpaceDefs::jFEX_algoSpace_C_FwdEl_start) continue;
          if (m_jfex == 5 && neta <= FEXAlgoSpaceDefs::jFEX_algoSpace_A_FwdEl_start) continue;
          if (m_jfex == 0 && neta == FEXAlgoSpaceDefs::jFEX_algoSpace_C_FCAL1_1st)   continue;
          if (m_jfex == 5 && neta == FEXAlgoSpaceDefs::jFEX_algoSpace_A_FCAL1_1st)   continue;
	         
          // define ttID (only FCAL1 in the third region) which will be the key for class in map, ignore tower ID = 0
          uint ttID = m_jFEXalgoTowerID[nphi][neta];
          if(ttID == 0) continue;
          
          jFEXForwardElecInfo elCluster;
          elCluster.setup(m_jfex, ttID, neta, nphi);
          const auto [centreTT_eta, centreTT_phi] = getEtaPhi(ttID);
          const auto [centreTT_EtEM, centreTT_EtHad] = getEtEmHad(ttID);
          elCluster.setCoreTTfPhi(centreTT_phi);
          elCluster.setCoreTTfEta(centreTT_eta);
          elCluster.setCoreTTEtEM(centreTT_EtEM);                  
          elCluster.setNextTTEtEM(0);
          elCluster.setNextTTID(0);
          elCluster.setTTEtEMiso(0);

          // cluster with jet radius for |eta|>=2.5 else tau radius
          auto& gMap=(fabs(centreTT_eta)>=2.5 ? m_SearchGMap : m_SearchGTauMap);
          auto& geMap=(fabs(centreTT_eta)>=2.5 ? m_SearchGeMap : m_SearchGeTauMap);
          
          // check if seed has strictly more energy than its neighbours
          {
            auto it_seed_map = gMap.find(ttID);
            if(it_seed_map == gMap.end()) {
                ATH_MSG_ERROR("Could not find TT" << ttID << " in the seach (>) local maxima for jets file.");
                continue;
            }
            bool breakout=false;
            for (const auto& gtt : it_seed_map->second ){
              auto [tmp_EtEM,tmp_EtHad] = getEtEmHad(gtt);
              if(tmp_EtEM>=centreTT_EtEM) {
                breakout=true;
                break;
              }
              // also test if cluster highest energy nearest neighbor
              if (tmp_EtEM>elCluster.getNextTTEtEM()) {
                elCluster.setNextTTEtEM(tmp_EtEM);
                elCluster.setNextTTID(gtt);
              }
            }
            if (breakout) continue;
          }

          // check if seed has equal or more energy than its neighbours
          {
            auto it_seed_map = geMap.find(ttID);
            if(it_seed_map == geMap.end()) {
                ATH_MSG_ERROR("Could not find TT" << ttID << " in the seach (>) local maxima for jets file.");
                continue;
            }
            bool breakout=false;
            for (const auto& gtt : it_seed_map->second ){
              auto [tmp_EtEM,tmp_EtHad] = getEtEmHad(gtt);
              if( tmp_EtEM>=centreTT_EtEM) {
                breakout=true;
                break;
              }
              // also test if cluster nearest neighbor
              if (tmp_EtEM>elCluster.getNextTTEtEM()){
                elCluster.setNextTTEtEM(tmp_EtEM);
                elCluster.setNextTTID(gtt);
              }
            }
            if (breakout) continue;
          }

          // sum up EM isolation using the seed ring (<0.2) and 1st ring (<0.4) and remove cluster
          {
            int sumEt = 0;
            {
              auto it_seed_map = m_SeedRingMap.find(ttID);
              if(it_seed_map == m_SeedRingMap.end()) {
                  ATH_MSG_ERROR("Could not find TT" << ttID << " in Jet seed file.");
                  continue;
              }
              for(const auto& gtt : it_seed_map->second){
                  auto [tmp_EtEM,tmp_EtHad] = getEtEmHad(gtt);
                  sumEt += tmp_EtEM;  
              }
            }
            auto it_seed_map = m_1stRingMap.find(ttID);
            if(it_seed_map == m_1stRingMap.end()) {
                ATH_MSG_ERROR("Could not find TT" << ttID << " in Jet seed file.");
                continue;
            }
            for(const auto& gtt : it_seed_map->second){
                auto [tmp_EtEM,tmp_EtHad] = getEtEmHad(gtt);
                sumEt += tmp_EtEM;  
            }
            elCluster.setTTEtEMiso(sumEt-centreTT_EtEM-elCluster.getNextTTEtEM());
          }

          // Calculate Ethad below |eta|<3.2, higher eta Ethad computed at the end of algo
          if(fabs(centreTT_eta) < 3.2){
            elCluster.setTTEtHad1(centreTT_EtHad);
          }else{
            elCluster.setTTEtHad1(0);
          }

          // save this cluster in the list
          clusterList[ttID] = elCluster;
        }//eta
      }//phi
    }// 3 regions
    return clusterList;
  } 

  std::unordered_map<uint, jFEXForwardElecInfo> LVL1::jFEXForwardElecAlgo::calculateEDM() {
    // setting the lower/upper eta range for the FCAL 2 and 3 since they are not added in the seed information yet 
    int lowerFCAL_eta = FEXAlgoSpaceDefs::jFEX_algoSpace_C_lowerFCAL_eta;
    int upperFCAL_eta = FEXAlgoSpaceDefs::jFEX_algoSpace_C_upperFCAL_eta;
    int lowerFCAL2_eta = FEXAlgoSpaceDefs::jFEX_algoSpace_C_lowerFCAL2_eta;
    int upperFCAL2_eta = FEXAlgoSpaceDefs::jFEX_algoSpace_C_upperFCAL_eta;

    if(m_jfex == 5) {
      //Module 5                                                                                                                                   
      lowerFCAL_eta = FEXAlgoSpaceDefs::jFEX_algoSpace_A_lowerFCAL_eta;
      upperFCAL_eta = FEXAlgoSpaceDefs::jFEX_algoSpace_A_upperFCAL_eta;
      lowerFCAL2_eta = FEXAlgoSpaceDefs::jFEX_algoSpace_A_lowerFCAL_eta;
      upperFCAL2_eta = FEXAlgoSpaceDefs::jFEX_algoSpace_A_upperFCAL2_eta;
    }

    // Retrieve FCAl1 clusters          
    std::unordered_map<uint, jFEXForwardElecInfo> clusterList = eleClusterList();
    for(auto& [ttID,elCluster] : clusterList) {
      float centreTT_phi = elCluster.getCoreTTfPhi();
      float centreTT_eta = elCluster.getCoreTTfEta();
      if(fabs(centreTT_eta)<3.2) continue;// only FCAL clusters

      // Adding the closest FCAL 2 and 3 TT to the hadronic energy  
      float deltaRminl2 = 999, deltaRminl2b = 999, deltaRminl2c = 999, deltaRminl3 = 999;
      uint TTmin2 = 0, TTmin2b = 0, TTmin2c = 0, TTmin3 = 0;
      for(int nphi = 0; nphi < 8; nphi++) {
    	  for(int neta = lowerFCAL_eta; neta < upperFCAL_eta; neta++) {
	        int auxTTID = m_jFEXalgoTowerID[nphi][neta];
	        auto [ TT_eta,TT_phi ] = getEtaPhi(auxTTID);
	        // correct for transition over 2PI
	        if(m_fpga==0 || m_fpga==3) {
	          if(m_fpga==0) {
	            if(TT_phi>M_PI){
		            TT_phi = TT_phi-m_2PI;
	            }
	          }
	          else {
	            if(TT_phi<M_PI){
		            TT_phi = TT_phi+m_2PI;
	            }
	          }
	        }

          // Search for hadronic SC closest in DR
          int DeltaR = std::round( (std::pow((centreTT_eta - TT_eta),2) + std::pow((centreTT_phi - TT_phi),2)) * 1e5   );
          if ( DeltaR < m_Edge_dR4 ) {
            if (neta> lowerFCAL2_eta-1 && neta < upperFCAL2_eta){
              // EtHad1, FCAL second layer 
              if( DeltaR < deltaRminl2){
                deltaRminl2c  = deltaRminl2b;
                deltaRminl2b  = deltaRminl2;
                deltaRminl2   = DeltaR;
                TTmin2c = TTmin2b;
                TTmin2b = TTmin2;
                TTmin2 = auxTTID;
      	      }
              else if ( DeltaR < deltaRminl2b){
                deltaRminl2c  = deltaRminl2b;
		            deltaRminl2b  = DeltaR;
                TTmin2c = TTmin2b;
                TTmin2b = auxTTID;
	            }
              else if ( DeltaR < deltaRminl2c){
                deltaRminl2c  = DeltaR;
		            TTmin2c = auxTTID; 
	            }
	          }
            else{
	            // EtHad2, FCAL 3rd layer
	            if( DeltaR < deltaRminl3){
                deltaRminl3   = DeltaR;
                TTmin3 = auxTTID;
      	      }
	          }
	        }// search cone
	      }
      }//search end

      //EHad1
      auto [TT_EtEM2, TT_EtHad2] = getEtEmHad(TTmin2);
      elCluster.setTTEtHad1(uint(TT_EtHad2));
      // special treatment for ieta = 22, 2nd cell in FCAL1, eta ~3.2
      if(elCluster.getCoreTTiEta() == FEXAlgoSpaceDefs::jFEX_algoSpace_FCAL1_2nd){ 
  	    auto [TT_EtEM2b, TT_EtHad2b] = getEtEmHad(TTmin2b);
	      elCluster.addTTEtHad1(uint(TT_EtHad2b));
	      if((centreTT_phi> 0.9 && centreTT_phi<1.1) || (centreTT_phi> 4.1 && centreTT_phi<4.3)){
	        auto [TT_EtEM2c, TT_EtHad2c] = getEtEmHad(TTmin2c);
	        elCluster.addTTEtHad1(uint(TT_EtHad2c));
	      } 
      }//special cases

      //EtHad2
      auto [TT_EtEM3, TT_EtHad3] = getEtEmHad(TTmin3);
      elCluster.setTTEtHad2(TT_EtHad3);
    }// loop ver local maxima

    return clusterList;
  }
      
  StatusCode LVL1::jFEXForwardElecAlgo::ReadfromFile(const std::string & fileName, std::unordered_map<unsigned int, std::vector<unsigned int> >& fillingMap){
    std::string myline;
    //opening file with ifstream
    std::ifstream myfile(fileName);
    if ( !myfile.is_open() ){
        ATH_MSG_ERROR("Could not open file:" << fileName);
        return StatusCode::FAILURE;
    }
    
    //loading the mapping information
    while ( std::getline (myfile, myline) ) {
        //removing the header of the file (it is just information!)
        if(myline[0] == '#') continue;
        
        //Splitting myline in different substrings
        std::stringstream oneLine(myline);
        
        //reading elements
        std::vector<unsigned int> elements;
        std::string element;
        while(std::getline(oneLine, element, ' '))
        {
            elements.push_back(std::stoi(element));
        }
        
        // We should have at least two elements! Central TT and (at least) itself
        if(elements.size() < 1){
            ATH_MSG_ERROR("Unexpected number of elemennts (<1 expected) in file: "<< fileName);
            return StatusCode::FAILURE;
        }
        
        //Central TiggerTower
        unsigned int TTID = elements.at(0);
        
        // rest of TTs that need to be checked
        elements.erase(elements.begin());
        fillingMap[TTID] = elements;        
    }
    myfile.close();

    return StatusCode::SUCCESS;
  }   
}// end of namespace LVL1
