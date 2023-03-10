/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration  
*/
//***************************************************************************  
//		jFEXSmallRJetAlgo - Algorithm for small R jet Algorithm in jFEX
//                              -------------------
//     begin                : 03 11 2020
//     email                : varsiha.sothilingam@cern.ch
//***************************************************************************  
#include <iostream>
#include <vector>
#include "L1CaloFEXSim/jFEXSmallRJetAlgo.h"
#include "L1CaloFEXSim/jTower.h"
#include "L1CaloFEXSim/jTowerContainer.h"
#include "CaloEvent/CaloCellContainer.h"
#include "CaloIdentifier/CaloIdManager.h"
#include "CaloIdentifier/CaloCell_SuperCell_ID.h"
#include "AthenaBaseComps/AthAlgorithm.h"
#include "StoreGate/StoreGateSvc.h"

namespace LVL1{

//Default Constructor
LVL1::jFEXSmallRJetAlgo::jFEXSmallRJetAlgo(const std::string& type, const std::string& name, const IInterface* parent):
   AthAlgTool(type, name, parent)   
  {
  declareInterface<IjFEXSmallRJetAlgo>(this);
  }

/** Destructor */
LVL1::jFEXSmallRJetAlgo::~jFEXSmallRJetAlgo()
{
}
StatusCode LVL1::jFEXSmallRJetAlgo::initialize()
{
   ATH_CHECK(m_jTowerContainerKey.initialize());

   return StatusCode::SUCCESS;

}

//calls container for TT
StatusCode LVL1::jFEXSmallRJetAlgo::safetyTest(){

  SG::ReadHandle<jTowerContainer> jTowerContainer(m_jTowerContainerKey);

  if(! jTowerContainer.isValid()){
    ATH_MSG_FATAL("Could not retrieve  jTowerContainer " << m_jTowerContainerKey.key());

    return StatusCode::FAILURE;
  }
  
  return StatusCode::SUCCESS;
}

void LVL1::jFEXSmallRJetAlgo::setup(int inputTable[7][7], int inputTableDisplaced[7][7]) {
    
    for(int phi=0; phi<7; phi++) {
        for (int eta=0; eta<7; eta++) {
            m_jFEXalgoTowerID[phi][eta] = inputTable[6-phi][eta];
        }
    }

    for(int phi=0; phi<7; phi++) {
        for (int eta=0; eta<7; eta++) {
            m_jFEXalgoTowerID_displaced[phi][eta] = inputTableDisplaced[6-phi][eta];
        }
    }
}


int LVL1::jFEXSmallRJetAlgo::realValue(int ID, int eta){

  return ((int)(ID/pow(10,5)) % 10) % 2==0 ?  eta : -eta ;
  
}


//Gets the ET for the TT. This ET is EM + HAD
unsigned int LVL1::jFEXSmallRJetAlgo::getTTowerET(unsigned int TTID ) {
   if(TTID == 0) {
        return 0;
    } 
    
    if(m_map_Etvalues.find(TTID) != m_map_Etvalues.end()) {
       return m_map_Etvalues[TTID][0];
    }
    
    //we shouldn't arrive here
    return 0;
    
}

//Gets Phi of the TT
unsigned int LVL1::jFEXSmallRJetAlgo::getRealPhi(unsigned int TTID ) {
    if(TTID == 0) {
        return 0;
    }
    SG::ReadHandle<jTowerContainer> jTowerContainer(m_jTowerContainerKey/*,ctx*/);

    unsigned int phi = jTowerContainer->findTower(TTID)->phi();
    return phi;
}
//Gets Eta of the TT
int LVL1::jFEXSmallRJetAlgo::getRealEta(unsigned int TTID ) {
    if(TTID == 0) {
        return 0;
    }
    SG::ReadHandle<jTowerContainer> jTowerContainer(m_jTowerContainerKey/*,ctx*/);
    const LVL1::jTower * tmpTower = jTowerContainer->findTower(TTID);
    return realValue(TTID,tmpTower->eta());
}



//this function calculates seed for a given TT
void LVL1::jFEXSmallRJetAlgo::buildSeeds()
{

    for(int mphi = 1; mphi < 6; mphi++) {
        for(int meta = 1; meta< 6; meta++) {
            
            int seedTotalET = 0;
            int seedTotalET_displaced = 0;
            for(int iphi = -1; iphi < 2; iphi++) {
                for(int ieta = -1; ieta < 2; ieta++) {
                    //for that TT, build the seed
                    //here we sum TT ET to calculate seed
                    seedTotalET += getTTowerET(m_jFEXalgoTowerID[mphi + iphi][meta + ieta]);
                    seedTotalET_displaced += getTTowerET(m_jFEXalgoTowerID_displaced[mphi + iphi][meta + ieta]);
                }
            }
            m_jFEXalgoSearchWindowSeedET[mphi -1][meta -1] = seedTotalET;
            m_jFEXalgoSearchWindowSeedET_displaced[mphi -1][meta -1] = seedTotalET_displaced;
        }
    }
}


bool LVL1::jFEXSmallRJetAlgo::CalculateLM(int mymatrix[5][5]) {

    //here put the 24 conditions to determine if the TT seed is a local maxima.
    int central_seed = mymatrix[2][2];
    for (int iphi = 0; iphi < 5; iphi++) {
        for (int ieta = 0; ieta < 5; ieta++) {
            //avoid comparing central seed to itself
            if ((ieta == 2) && (iphi == 2)) {
                continue;
            }
            //strictly less than central
            if( (iphi > ieta) || (iphi == 0 && ieta == 0) || (iphi == 1 && ieta == 1) ) {
                if(central_seed < mymatrix[iphi][ieta]) {
                    return false;
                }
            }
            //less than or equal to central
            if((iphi < ieta) || (iphi == 3 && ieta == 3) || (iphi == 4 && ieta == 4)) {
                if(central_seed <= mymatrix[iphi][ieta]) {
                    return false;
                }
            }
        }
    }

    return true;
}

//check if central TT is a local maxima
bool LVL1::jFEXSmallRJetAlgo::isSeedLocalMaxima() {
    
    bool isCentralLM   = CalculateLM(m_jFEXalgoSearchWindowSeedET) && (getTTowerET(m_jFEXalgoTowerID[3][3]) >= getTTowerET(m_jFEXalgoTowerID[4][2]));
    bool isDisplacedLM = CalculateLM(m_jFEXalgoSearchWindowSeedET_displaced) && (getTTowerET(m_jFEXalgoTowerID[3][3]) >  getTTowerET(m_jFEXalgoTowerID[2][4]));
    
    if(isCentralLM || isDisplacedLM){
        return true;
    }
    return false;
}


//in this clustering func, the central TT in jet is the parameters
unsigned int LVL1::jFEXSmallRJetAlgo::getSmallClusterET() {

    int SRJetClusterET = 0;
    for(int nphi = -3; nphi< 4; nphi++) {
        for(int neta = -3; neta< 4; neta++) {
            int DeltaRSquared = std::pow(nphi,2)+std::pow(neta,2);
            if(DeltaRSquared < 16) {
                SRJetClusterET += getTTowerET(m_jFEXalgoTowerID[3+nphi][3+neta]);
            }
        }
    }
    return SRJetClusterET;
}


unsigned int LVL1::jFEXSmallRJetAlgo::getSmallETRing(){
  int SmallETRing = getSmallClusterET() - m_jFEXalgoSearchWindowSeedET[3][3];   
  return SmallETRing;
}
         
unsigned int LVL1::jFEXSmallRJetAlgo::getTTIDcentre(){
  return m_jFEXalgoTowerID[3][3];
}


void LVL1::jFEXSmallRJetAlgo::setFPGAEnergy(std::unordered_map<int,std::vector<int> > et_map){
    m_map_Etvalues=et_map;
}

}// end of namespace LVL1

