/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigT1TGC/TGCElectronicsSystem.h"
#include "TrigT1TGC/TGCASDOut.h"
#include "TrigT1TGC/TGCDatabaseManager.h"
#include "TrigT1TGC/TGCSector.h"
#include "TrigT1TGC/TGCEvent.h"

#include <iostream>
#include <vector>

namespace LVL1TGCTrigger {

void TGCElectronicsSystem::distributeSignal(LVL1TGCTrigger::TGCEvent* event)
{
  const std::vector<LVL1TGCTrigger::TGCASDOut*>& vecASDOut= event->GetASDOutVector();
  event=0;
  size_t i;
  for( i=0; i< vecASDOut.size(); i++) {
    const LVL1TGCTrigger::TGCASDOut* asd= vecASDOut[i];
    int side = asd->GetTGCReadoutIndex().GetSideNumber();
    int octant = asd->GetTGCReadoutIndex().GetOctantNumber();
    int module = asd->GetTGCReadoutIndex().GetModuleNumber();

    if (getSector(side, octant, module)) {
      if (side < LVL1TGC::kNSide && octant < NumberOfOctant) 
        getSector(side, octant, module)->distributeSignal(asd);
    }
    asd = 0;
  }
  
}

TGCElectronicsSystem::TGCElectronicsSystem()
 : m_DB(0),
   m_tgcArgs(nullptr)
{
  for (int side=0; side < LVL1TGC::kNSide; side++) {
    for (int oct=0; oct < NumberOfOctant; oct++) {
      for (int mod=0; mod < NumberOfModule; mod++) {
        m_sector[side][oct][mod]=0;
      } // loop module
    } // loop octant
  } //loop side

}

TGCElectronicsSystem::TGCElectronicsSystem(TGCArguments* tgcargs,
					   TGCDatabaseManager* database)
: m_DB(database),
  m_tgcArgs(tgcargs)
{
  // TMDB
  m_tmdb.reset(new LVL1TGC::TGCTMDB());

  // NSW
  if(tgcargs->USE_NSW()){
    m_nsw.reset(new LVL1TGC::TGCNSW());
  }

  // RPC BIS78
  if(tgcargs->USE_BIS78()){
    m_bis78.reset(new LVL1TGC::TGCBIS78());
  }

  int SectorId;
  LVL1TGCTrigger::TGCRegionType RegionType;
  LVL1TGCTrigger::TGCForwardBackwardType forwardBackward;
  for (int side=0; side < LVL1TGC::kNSide; side++) {
    for(int oct=0; oct < NumberOfOctant; oct++){
      for(int mod=0; mod < NumberOfModule; mod++){
        SectorId   = getSectorId(side,oct,mod);
        RegionType = getRegionType(mod);
        forwardBackward = getForwardBackward(side,oct,mod);
        m_sector[side][oct][mod] = new TGCSector(tgcArgs(),SectorId, RegionType, 
						 forwardBackward, 
						 m_DB,
						 m_tmdb,
						 m_nsw,
						 m_bis78);
      } // loop module
    } // loop octant
  } //loop side
}

LVL1TGCTrigger::TGCRegionType TGCElectronicsSystem::getRegionType(int mod) const
{
  // Module:                
  //   <----phi-----      
  //  7   6  4   3  1   0   Endcap                    
  //    8      5      2     Forward
  //
  //  Inner
  //   <----phi-----      
  //      11   10   9   Endcap                   
  //  14     13   12    Forward

  if(mod<9)
    if(mod%3==2) return LVL1TGCTrigger::Forward;
    else return LVL1TGCTrigger::Endcap;
  else if(mod<12)
    return LVL1TGCTrigger::Endcap;
  else
    return LVL1TGCTrigger::Forward;
}

TGCForwardBackwardType TGCElectronicsSystem::getForwardBackward(int side, int oct, int mod) const
{
  bool isEndcap = false;
  bool isInner = false;
  int sec=-1;
  if(mod<9){
    if(mod%3==2)
      sec = mod/3+oct*3;
    if(mod%3==0||mod%3==1){
      sec = mod-mod/3+oct*6;
      isEndcap = true;
    }
  } else if(mod<12){
    isEndcap = true;
    isInner = true;
    sec = mod%3+oct*3;
  } else {
    isInner = true;
    sec = mod%3+oct*3;
  }
  
  TGCForwardBackwardType forwardBackward = ForwardSector;

  // for new cabling service 
  // A-side : side == 0
  // C-side : side == 1 
  bool isAside = (side ==0);
  
  // backward/Forward definition is same 
  // compared with TGCIdBase::isBackward() in TGCcablingInterface
  // because strip layer is swapped in default
  if ( isEndcap ){
    if (!isInner){
      if(isAside) {
	if (sec%2==0) forwardBackward = ForwardSector;
	else          forwardBackward = BackwardSector;
      } else {
	if (sec%2==1) forwardBackward = ForwardSector;
	else          forwardBackward = BackwardSector;
      }
    } else {
      // EI
      // Special case of EI11
      if (sec == 15) {
	if(isAside ) forwardBackward = ForwardSector;
	else          forwardBackward = BackwardSector;
      } else if (sec == 16) {
	if(isAside ) forwardBackward = BackwardSector;
	else          forwardBackward = ForwardSector;
      } else {
	if (isAside) {  
	  if (sec%3==2) forwardBackward = BackwardSector;
	  else          forwardBackward = ForwardSector;
	} else {
	  if (sec%3!=2) forwardBackward = BackwardSector;
	  else          forwardBackward = ForwardSector;
	}
      }
    }
  } else {
    if (isAside) forwardBackward = BackwardSector; // all Backward
    else         forwardBackward = ForwardSector;  // all Forward
  }
  return forwardBackward;
}

int TGCElectronicsSystem::getSectorId(int side, int oct, int mod) const
{
  return NumberOfModule*NumberOfOctant*side+NumberOfModule*oct+mod;
}

TGCSector* TGCElectronicsSystem::getSector(LVL1TGCTrigger::TGCReadoutIndex index) const
{ 
  if (index.GetSideNumber()<0) return 0;
  return m_sector[index.GetSideNumber()][index.GetOctantNumber()][index.GetModuleNumber()];
}

TGCElectronicsSystem::~TGCElectronicsSystem()
{
  for (int i=0; i < LVL1TGC::kNSide; i++) {
    for (int j=0; j < NumberOfOctant; j++) {
      for (int k=0; k < NumberOfModule; k++) {
        if(m_sector[i][j][k]!=0) delete m_sector[i][j][k];
        m_sector[i][j][k]=0;
      }  // loop module
    }  // loop octant
  }  // loop side
}

  
} //end of namespace bracket
