/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

//***************************************************************************
//                           jFexTower2SCellDecorator  -  description
//                              -------------------
//      This reentrant algorithm is meant to decorate the FEX Towers (input data and simulation) with the corresponding matching set of SuperCell from LAr
//      Information about SCellContainer objetcs are in:
//          - https://gitlab.cern.ch/atlas/athena/-/blob/master/Calorimeter/CaloEvent/CaloEvent/CaloCell.h
//      
//     begin                : 01 09 2022
//     email                : sergi.rodriguez@cern.ch
//***************************************************************************/


#include "jFexTower2SCellDecorator.h"
#include "L1CaloFEXSim/jFEXCompression.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <string>
#include <stdio.h> 

namespace LVL1 {

jFexTower2SCellDecorator::jFexTower2SCellDecorator(const std::string& name, ISvcLocator* svc) : AthReentrantAlgorithm(name, svc){}

StatusCode jFexTower2SCellDecorator::initialize() {
    ATH_MSG_INFO( "L1CaloFEXTools/jFexTower2SCellDecorator::initialize()");
    ATH_CHECK( m_SCellKey.initialize() );
    ATH_CHECK( m_jTowersReadKey.initialize() );
    ATH_CHECK( m_SCellEtdecorKey.initialize() );
    ATH_CHECK( m_SCellEtadecorKey.initialize() );
    ATH_CHECK( m_SCellPhidecorKey.initialize() );
    ATH_CHECK( m_towerEtMeVdecorKey.initialize() );
    
    //Reading from CVMFS Trigger Tower and their corresponding SCell ID
    ATH_CHECK(ReadfromFile(m_jFEX2Scellmapping));
    
    return StatusCode::SUCCESS;
}

StatusCode jFexTower2SCellDecorator::execute(const EventContext& ctx) const {
    
    //Reading the Scell container
    SG::ReadHandle<CaloCellContainer> ScellContainer(m_SCellKey, ctx);
    if(!ScellContainer.isValid()) {
        ATH_MSG_FATAL("Could not retrieve collection " << ScellContainer.key() );
        return StatusCode::FAILURE;
    }
    
    //Reading the jTower container
    SG::ReadHandle<xAOD::jFexTowerContainer> jTowerContainer(m_jTowersReadKey, ctx);
    if(!jTowerContainer.isValid()) {
        ATH_MSG_FATAL("Could not retrieve collection " << jTowerContainer.key() );
        return StatusCode::FAILURE;
    }  
     
    if(ScellContainer->empty() || jTowerContainer->empty()){
        ATH_MSG_WARNING("Nothing to decorate here, at least one container is empty. ScellContainer.size="<<ScellContainer->size() << " or jTowerContainer.size=" << jTowerContainer->size() );
        return StatusCode::SUCCESS;
    }
    
    std::unordered_map< uint64_t, const CaloCell*> map_ScellID2ptr;
    
    for(const CaloCell* scell : *ScellContainer){
        const uint64_t ID = scell->ID().get_compact();
        map_ScellID2ptr[ID] = scell;
    }
    
    //Setup Decorator Handlers
    SG::WriteDecorHandle<xAOD::jFexTowerContainer, std::vector<float> > jTowerSCellEt (m_SCellEtdecorKey   , ctx);
    SG::WriteDecorHandle<xAOD::jFexTowerContainer, std::vector<float> > jTowerSCellEta(m_SCellEtadecorKey  , ctx);
    SG::WriteDecorHandle<xAOD::jFexTowerContainer, std::vector<float> > jTowerSCellPhi(m_SCellPhidecorKey  , ctx);
    SG::WriteDecorHandle<xAOD::jFexTowerContainer, int >                jTowerEtMeV   (m_towerEtMeVdecorKey, ctx);
    
    //looping over the jTowers to decorate them!
    for(const xAOD::jFexTower* jTower : *jTowerContainer){
        
        uint32_t jFexID = jTower->jFEXtowerID();
        uint8_t  source = jTower->Calosource();
        uint16_t jFexEt = 0;
        
        if(source == 0 or source == 2 or source == 4){  //for EM layer only.  0 = EMB, 2 = EMEC, 4 = FCAL1
            jFexEt = jTower->jTowerEt_EM();
        }
        else if(source == 1 or source == 3 or source == 5 or source == 6 ){ //for HAD layer only.  1 = TILE, 3 = HEC, 5,6 = FCAL2,3
            jFexEt = jTower->jTowerEt_HAD();
        }
        else{
            ATH_MSG_WARNING("Undefined source element: "<<source);
        }
        
        std::vector<float> scEt;
        std::vector<float> scEta;
        std::vector<float> scPhi;
        
        if(source != 1){ // Source == 1 is belong to Tile Calorimeter, and of course the is not SCell information!
            
            //check that the jFEX Tower ID exists in the map
            if(m_map_TTower2SCells.find(jFexID) == m_map_TTower2SCells.end()) {
                ATH_MSG_ERROR("ID: "<<jFexID<< " not found on map m_map_TTower2SCells");
                return StatusCode::FAILURE;
            }
            
            for (auto const& SCellID : m_map_TTower2SCells.at(jFexID)) {

                //check that the SCell Identifier exists in the map
                if(map_ScellID2ptr.find(SCellID) == map_ScellID2ptr.end()) {
                    ATH_MSG_ERROR("Scell ID: "<<SCellID<< " not found on map map_ScellID2ptr");
                    return StatusCode::FAILURE;
                }

                const CaloCell* myCell = map_ScellID2ptr.at(SCellID);
                scEt.push_back(myCell->et());
                scEta.push_back(myCell->eta());
                scPhi.push_back(myCell->phi());

            }   
        }
        
        // Decorating the tower with the corresponding information
        jTowerSCellEt (*jTower) = scEt;
        jTowerSCellEta(*jTower) = scEta;
        jTowerSCellPhi(*jTower) = scPhi;
        jTowerEtMeV   (*jTower) = static_cast<int>( jFEXCompression::Expand(jFexEt) );
        
    }
    
    // Return gracefully
    return StatusCode::SUCCESS;
}


StatusCode  jFexTower2SCellDecorator::ReadfromFile(const std::string& fileName){
    
    std::string myline;
    
    //openning file with ifstream
    std::ifstream myfile(fileName);
    
    if ( !myfile.is_open() ){
        ATH_MSG_FATAL("Could not open file:" << fileName);
        return StatusCode::FAILURE;
    }
    
    //loading the mapping information into an unordered_map <Fex Tower ID, vector of SCell IDs>
    while ( std::getline (myfile, myline) ) {
        std::vector<uint64_t> SCellvector;
        SCellvector.clear();

        //removing the header of the file (it is just information!)
        if(myline[0] == '#') continue;
        
        //Splitting myline in different substrings
        std::stringstream oneSCellID(myline);
        
        //reading elements
        std::string substr = "";
        int TTID = 0;
        int elem = 0;
        
        while(std::getline(oneSCellID, substr, ' '))
        {
            ++elem;
            if(elem == 1){
               TTID =  std::stoi(substr);
            }
            else{
                //Check if it looks like a SCell Identifier
                if(isBadSCellID(substr)){
                    return StatusCode::FAILURE;
                }
                
                // converts hex number to unsigned long long int
                uint64_t scid_uint64 = std::strtoull(substr.c_str(), nullptr, 0);
                SCellvector.push_back(scid_uint64);                
            }
        }        
        
        m_map_TTower2SCells[TTID] = SCellvector;
        
    }
    myfile.close();

    return StatusCode::SUCCESS;
}

bool jFexTower2SCellDecorator::isBadSCellID(const std::string& ID) const{
    
    // does it start with "0x"?, if so then is a GOOD SCell ID!
    if (ID.find("0x") == std::string::npos) {
        ATH_MSG_ERROR("Invalid SuperCell ID " << ID << ". Expecting hexadecimal number on the mapping file");
        return true;
    }
    return false;
}


}
