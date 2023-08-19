/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigT1TGC/TGCDatabaseManager.h"
#include "TrigT1TGC/TGCArguments.h"
#include "TrigT1TGC/TGCConnectionPPToSL.h"
#include "TrigT1TGC/BigWheelCoincidenceLUT.h"
#include "TrigT1TGC/TGCEIFICoincidenceMap.h"
#include "TrigT1TGC/TGCTileMuCoincidenceLUT.h"
#include "TrigT1TGC/TGCNSWCoincidenceMap.h"
#include "TrigT1TGC/TGCBIS78CoincidenceMap.h"
#include "TrigT1TGC/TGCGoodMF.h"
#include "TrigT1TGC/TGCConnectionASDToPP.h"
#include "TrigT1TGC/TGCConnectionInPP.h"
#include "TrigT1TGC/TGCPatchPanel.h"

#include "AthenaBaseComps/AthMsgStreamMacros.h"

namespace LVL1TGCTrigger {

TGCConnectionInPP* TGCDatabaseManager::getConnectionInPP(TGCPatchPanel* patchPanel) const 
{
  if(!patchPanel) return 0;

  PatchPanelIDs patchPanelIDs(5, -1);
  patchPanelIDs.at(0) = (patchPanel->getRegion() == TGCRegionType::FORWARD ? 0 : 1);
  patchPanelIDs.at(1) = patchPanel->getType();
  patchPanelIDs.at(2) = patchPanel->getId();
  patchPanelIDs.at(3) = (patchPanel->getAdjacentPP(0) ? patchPanel->getAdjacentPP(0)->getId() : -1); 
  patchPanelIDs.at(4) = (patchPanel->getAdjacentPP(1) ? patchPanel->getAdjacentPP(1)->getId() : -1);
  
  std::map<PatchPanelIDs, std::pair<const TGCConnectionInPP, PatchPanelPointers> >::const_iterator cit 
    = m_patchPanelToConnectionInPP.find(patchPanelIDs);
  // If this PatchPanel is already registered, create a clone connectionInPP
  if(cit!=m_patchPanelToConnectionInPP.end()) { 
    TGCConnectionInPP* newConnectionInPP = new TGCConnectionInPP(((*cit).second).first);
    PatchPanelPointers oldPatchPanelPointers = ((*cit).second).second; 
    // Need to replace PatchPanel pointers of newConnectionInPP by ones of patchPanel
    bool isReplaced = newConnectionInPP->replacePatchPanelPointers(patchPanel, oldPatchPanelPointers);
    if(!isReplaced) {
      delete newConnectionInPP;
      newConnectionInPP = 0;
    }
    return newConnectionInPP;
  }
  
  return 0;
}

void TGCDatabaseManager::addConnectionInPP(const TGCPatchPanel* patchPanel, 
					   const TGCConnectionInPP* connectionInPP) 
{
  if(!patchPanel || !connectionInPP) return; 

  PatchPanelIDs patchPanelIDs(5, -1);
  patchPanelIDs.at(0) = (patchPanel->getRegion() == TGCRegionType::FORWARD ? 0 : 1);
  patchPanelIDs.at(1) = patchPanel->getType();
  patchPanelIDs.at(2) = patchPanel->getId();
  patchPanelIDs.at(3) = (patchPanel->getAdjacentPP(0) ? patchPanel->getAdjacentPP(0)->getId() : -1); 
  patchPanelIDs.at(4) = (patchPanel->getAdjacentPP(1) ? patchPanel->getAdjacentPP(1)->getId() : -1);

  PatchPanelPointers patchPanelPointers;
  patchPanelPointers.push_back(patchPanel);
  patchPanelPointers.push_back(patchPanel->getAdjacentPP(0));
  patchPanelPointers.push_back(patchPanel->getAdjacentPP(1));

  TGCConnectionInPP newConnectionInPP(*connectionInPP);
  std::pair<const TGCConnectionInPP, PatchPanelPointers> cInPP_PPPs(newConnectionInPP, patchPanelPointers); 

  m_patchPanelToConnectionInPP.insert(std::pair<PatchPanelIDs, std::pair<const TGCConnectionInPP, PatchPanelPointers> >
				      (patchPanelIDs, cInPP_PPPs));
}

TGCDatabaseManager::TGCDatabaseManager()
 : AthMessaging("LVL1TGC::TGCDatabaseManager"),
   m_tgcArgs(nullptr)
{
  for(int j=0; j<NumberOfRegionType; j+=1){
    for(int i=0; i<TGCSector::NumberOfPatchPanelType; i+=1){
       for(int k=0; k<TotalNumForwardBackwardType; k+=1){
         m_ASDToPP[j][i][k] = 0;
       }
    }
  }
  for(int i=0; i<NumberOfRegionType; i+=1){
    m_PPToSL[i] = 0;
  }
  for (int side=0; side < LVL1TGC::kNSide; side++) {
    m_mapEIFI[side] = 0;
  }
}

TGCDatabaseManager::TGCDatabaseManager(TGCArguments* tgcargs,
				       const SG::ReadCondHandleKey<TGCTriggerData>& readCondKey,
				       const SG::ReadCondHandleKey<TGCTriggerLUTs>& readLUTsCondKey)
 : AthMessaging("LVL1TGC::TGCDatabaseManager"),
   m_tgcArgs(tgcargs)
{
  setLevel(tgcArgs()->MSGLEVEL());

  bool status = true;

  ATH_MSG_DEBUG("Read database for connection from ASD to PP.");

  for(int i=0; i<TGCSector::NumberOfPatchPanelType; i+=1){
    for(int k=0; k<TotalNumForwardBackwardType; k+=1){
      m_ASDToPP[0][i][k] = new TGCConnectionASDToPP;
      m_ASDToPP[1][i][k] = new TGCConnectionASDToPP;
      status = status
             && m_ASDToPP[0][i][k]->readData(TGCRegionType::FORWARD, i, (TGCForwardBackwardType)k)
             && m_ASDToPP[1][i][k]->readData(TGCRegionType::ENDCAP, i, (TGCForwardBackwardType)k);
    }
  }
  for(int i=0; i<NumberOfRegionType; i+=1){
    m_PPToSL[i] = new TGCConnectionPPToSL;
  }
  status = status && m_PPToSL[0]->readData(TGCRegionType::FORWARD)
                  && m_PPToSL[1]->readData(TGCRegionType::ENDCAP);

  // Temporary solution for Run 3 simulation (to be migrated to CONDDB
  tgcArgs()->set_USE_CONDDB(false);

  // CW for SL (ONLY available for Run-3 development phase)
  std::string ver_BW   = "v05";
  std::string ver_EIFI = "v07";
  std::string ver_TILE = "v01";
  std::string ver_NSW  = "v01";
  std::string ver_BIS78  = "v01"; // OK?
  std::string ver_HotRoI = "v1";

  // EIFI Coincidence Map
  ATH_MSG_DEBUG("start to create EIFI coincidence map.");
  for (int side=0; side < LVL1TGC::kNSide; side++) {
    m_mapEIFI[side] = new LVL1TGC::TGCEIFICoincidenceMap(tgcArgs(), readCondKey, ver_EIFI, side);
  }

  // Big Wheel Coincidence LUT
  m_bigWheelLUT.reset(new LVL1TGC::BigWheelCoincidenceLUT(tgcArgs(), readLUTsCondKey, ver_BW));

  // Tile-Muon LUT
  m_tileMuLUT.reset(new LVL1TGC::TGCTileMuCoincidenceLUT(tgcArgs(), readCondKey, ver_TILE));

  // NSW coincidence Map
  if(tgcArgs()->USE_NSW()){
    for (int side=0; side < LVL1TGC::kNSide; side++) {
      for (int oct=0; oct<NumberOfOctant; oct++) {
        for(int mod=0; mod<NumberOfModuleInBW; mod++){
          // NSW Coincidence Map
          m_mapNSW[side][oct][mod].reset(new TGCNSWCoincidenceMap(tgcArgs(),ver_NSW,side,oct,mod));
        }
      }
    }
  }
    
  // BIS78 coincidence Map
  if(tgcArgs()->USE_BIS78()){
    m_mapBIS78.reset(new LVL1TGC::TGCBIS78CoincidenceMap(tgcArgs(),ver_BIS78));
  }

  //Hot RoI LUT
  m_mapGoodMF.reset(new LVL1TGC::TGCGoodMF(tgcArgs(),ver_HotRoI));
}

void TGCDatabaseManager::deleteConnectionPPToSL()
{
  for(int i=0; i<NumberOfRegionType; i+=1){
    delete m_PPToSL[i];
    m_PPToSL[i]=0;
  }
}

TGCDatabaseManager::~TGCDatabaseManager()
{
  int i,j,k;
  for( j=0; j<NumberOfRegionType; j+=1){
    for( i=0; i<TGCSector::NumberOfPatchPanelType; i+=1){
      for( k=0; k<TotalNumForwardBackwardType; k+=1){
	if(m_ASDToPP[j][i][k]!=0) delete m_ASDToPP[j][i][k];
	m_ASDToPP[j][i][k]=0;
      }
    }
    if(m_PPToSL[j]!=0) delete m_PPToSL[j];
    m_PPToSL[j]=0;
  }

  for (int side=0; side < LVL1TGC::kNSide; side++) {
    delete m_mapEIFI[side];
  }
}

TGCDatabaseManager::TGCDatabaseManager(const TGCDatabaseManager& right)
 : AthMessaging("LVL1TGC::TGCDatabaseManager")
{
  for(int j=0; j<NumberOfRegionType; j+=1){
    for(int i=0; i<TGCSector::NumberOfPatchPanelType; i+=1){
      for(int  k=0; k<TotalNumForwardBackwardType; k+=1){
	m_ASDToPP[j][i][k] = 0;
      }
    }
  }
  for( int i=0; i<NumberOfRegionType; i+=1) m_PPToSL[i] = 0;
  for (int side=0; side < LVL1TGC::kNSide; side++) {
    m_mapEIFI[side] = 0;
  }

  *this = right;
}

TGCDatabaseManager&
TGCDatabaseManager::operator=(const TGCDatabaseManager& right)
{
  if(this!=&right){
    m_tgcArgs = right.m_tgcArgs;
    for( int j=0; j<NumberOfRegionType; j+=1){
      for( int i=0; i<TGCSector::NumberOfPatchPanelType; i+=1){
	for( int k=0; k<TotalNumForwardBackwardType; k+=1){
	  if(m_ASDToPP[j][i][k]!=0) delete m_ASDToPP[j][i][k];
	  m_ASDToPP[j][i][k] = new TGCConnectionASDToPP;
	  *m_ASDToPP[j][i][k] = *right.m_ASDToPP[j][i][k];
	}
      }
    }
    for(int i=0; i<NumberOfRegionType; i+=1){
      if(m_PPToSL[i]!=0) delete m_PPToSL[i];
      m_PPToSL[i] = new TGCConnectionPPToSL( *right.m_PPToSL[i] );
    }
    
    for (int side=0; side<LVL1TGC::kNSide; side++) {
      if (m_mapEIFI[side]!=0) delete m_mapEIFI[side];
      m_mapEIFI[side] = new LVL1TGC::TGCEIFICoincidenceMap(*(right.m_mapEIFI[side]));
    }

    m_patchPanelToConnectionInPP = right.m_patchPanelToConnectionInPP;
  }
  return *this;
}

std::string TGCDatabaseManager::getFilename(int type)
{
  switch (type) {
   case 0: //ASD2PP
    return "MuonTGC_Cabling_ASD2PP.db";
   case 1: //PP
    return "MuonTGC_Cabling_PP.db";
   case 2: //PP2SL
    return "MuonTGC_Cabling_PP2SL.db";
  }
  return "";
}

const std::vector<std::string> TGCDatabaseManager::splitCW(const std::string& input, char delimiter)
{
  std::istringstream stream(input);

  std::string field;
  std::vector<std::string> result;
  while (std::getline(stream, field, delimiter)) {
    result.push_back(field);
  }
  return result;
}

} //end of namespace bracket
