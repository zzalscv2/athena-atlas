/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration 
*/


#include "TrigT1TGC/TGCNSW.h"
#include "TrigT1TGC/NSWTrigOut.h"

namespace LVL1TGC {

TGCNSW::TGCNSW()
: AthMessaging("LVL1TGC::TGCNSW") {
  for (int side=0; side < 2; side++) {
    for (int TP=0; TP < NumberOfNSWTriggerProcesser; TP++) {
      std::vector<int> vecTP;
      vecTP.push_back(TP);
      m_buffer[side][TP].reset(new NSWTrigOut(side, vecTP)); 
    }
  }
}

StatusCode TGCNSW::retrieve(SG::ReadHandleKey<Muon::NSW_TrigRawDataContainer> key) {
    ATH_MSG_DEBUG("retrieve");
    this->eraseOutput();

    //The following part will be available when NSW Trigger Output is available.

    SG::ReadHandle<Muon::NSW_TrigRawDataContainer> readNSW_TrigRawDataContainer(key);
    if(!readNSW_TrigRawDataContainer.isValid()){
      ATH_MSG_ERROR("Cannot retrieve NSW TrigRawData Container.");
      return StatusCode::FAILURE;
    }
    const Muon::NSW_TrigRawDataContainer* nsw_TrigRawDataContainer = readNSW_TrigRawDataContainer.cptr();
    for(const Muon::NSW_TrigRawData* nsw_sector : *nsw_TrigRawDataContainer){
      for(const Muon::NSW_TrigRawDataSegment* nsw_trk : *nsw_sector){
        int nsw_sideId = (nsw_sector->sectorSide()=='A')?0:1;
        this->setOutput(nsw_sideId,        // side
                        nsw_sector->sectorId(), // Sector number in NSW
                        nsw_trk->rIndex(),      // R-index
                        nsw_trk->phiIndex(),    // Phi-index
                        nsw_trk->deltaTheta(), // Delta theta index
                        nsw_trk->lowRes(),
                        nsw_trk->phiRes(),
                        nsw_trk->monitor());   // monitoring flag
      }
    }

    return StatusCode::SUCCESS;
}


std::shared_ptr<const NSWTrigOut> 
TGCNSW::getOutput(LVL1TGCTrigger::TGCRegionType region ,int side,int TGC_TriggerSector) const
{ 
  auto indexIsOk = [this](int idx)->bool{
    if (not ((idx >=0 ) and (idx < NumberOfNSWTriggerProcesser))){
      ATH_MSG_ERROR("index out of range in TGCNSW::getOutput");
      return false;
    }
    return true;
  };
    
  std::shared_ptr<NSWTrigOut> trigNSW_output;
  trigNSW_output.reset(new NSWTrigOut());
  trigNSW_output->clear();
  if ( (side<0)||(side>1) ) return 0;
  if(region==LVL1TGCTrigger::ENDCAP){
    if ( TGC_TriggerSector<0 || TGC_TriggerSector>47 ) return 0;

    int temp=int((TGC_TriggerSector-2)/6);
    int NSW_TriggerSector=temp*2+int((TGC_TriggerSector-1-6*temp)/5);
    if(TGC_TriggerSector==0 || TGC_TriggerSector==1) {
      NSW_TriggerSector=15;
    }

    //
    // SL input 0-1
    if (not indexIsOk(NSW_TriggerSector)) return nullptr;
    *trigNSW_output+=*m_buffer[side][NSW_TriggerSector];
    // SL input 2-3
    if ( NSW_TriggerSector == 15 ) {
      *trigNSW_output+=*m_buffer[side][0];
    } else {
      if (not indexIsOk(NSW_TriggerSector + 1)) return nullptr;
      *trigNSW_output+=*m_buffer[side][NSW_TriggerSector+1];    
    }
    
    if( !(TGC_TriggerSector%6==4 || TGC_TriggerSector%6==5) ){
      return trigNSW_output;
    }
    
    ////Add one more Large Sector Info
    if ( NSW_TriggerSector==14 ) {
      ////for TGC sector 46-47
      *trigNSW_output+=*m_buffer[side][0];    
    } else {
      ////and for 4-5, 10-11, 16-17, 22-23, 28-29, 34-35, 40-41
      if (not indexIsOk(NSW_TriggerSector + 2)) return nullptr;
      *trigNSW_output+=*m_buffer[side][NSW_TriggerSector+2];
    }
    
    return trigNSW_output;
  }

  if(region==LVL1TGCTrigger::FORWARD){
    if ( TGC_TriggerSector<0 || TGC_TriggerSector>23 ) return 0;

    ////NSW Large Secor Info
    int temp=int((TGC_TriggerSector-4)/6);
    int NSW_TriggerSector=temp*4+int((TGC_TriggerSector-6*temp-4)/2)+2;
    if(TGC_TriggerSector==0 || TGC_TriggerSector==1)
      NSW_TriggerSector=15;
    else if(TGC_TriggerSector==2 || TGC_TriggerSector==3)
      NSW_TriggerSector=0;

    //// SL input 0-1
    if (not indexIsOk(NSW_TriggerSector)) return nullptr;
    *trigNSW_output+=*m_buffer[side][NSW_TriggerSector];
    //// SL input 2-3, 4-5
    if(TGC_TriggerSector==0 || TGC_TriggerSector==1){
      *trigNSW_output+=*m_buffer[side][0];
      *trigNSW_output+=*m_buffer[side][1];
    }else if(TGC_TriggerSector==22 || TGC_TriggerSector==23){
      if (not indexIsOk(NSW_TriggerSector + 1)) return nullptr;
      *trigNSW_output+=*m_buffer[side][NSW_TriggerSector+1];
      *trigNSW_output+=*m_buffer[side][0];
    }else{
      if (not indexIsOk(NSW_TriggerSector + 2)) return nullptr;
      *trigNSW_output+=*m_buffer[side][NSW_TriggerSector+1];
      *trigNSW_output+=*m_buffer[side][NSW_TriggerSector+2];
    }    

    return trigNSW_output;
  } 


  return 0;  
}


void TGCNSW::setOutput(int side, int NSWTriggerProcesser, uint8_t NSWeta_8bit, uint8_t NSWphi_6bit, uint8_t NSWDtheta_5bit, bool lowRes, bool phiRes, bool NSWmonitor)
{
  if ( (side<0)||(side>1) ) return;  //side 0::Aside 1::Cside
  if ( (NSWTriggerProcesser<0) || (NSWTriggerProcesser>=NumberOfNSWTriggerProcesser) ) return;
  m_buffer[side][NSWTriggerProcesser]->setSide(side);
  m_buffer[side][NSWTriggerProcesser]->setNSWTriggerProcessor(NSWTriggerProcesser);
  m_buffer[side][NSWTriggerProcesser]->setEta(NSWeta_8bit);
  m_buffer[side][NSWTriggerProcesser]->setPhi(NSWphi_6bit);
  m_buffer[side][NSWTriggerProcesser]->setDtheta(NSWDtheta_5bit);
  m_buffer[side][NSWTriggerProcesser]->setLowRes(lowRes);
  m_buffer[side][NSWTriggerProcesser]->setPhiRes(phiRes);
  m_buffer[side][NSWTriggerProcesser]->setMonitor(NSWmonitor);
}
 
void TGCNSW::eraseOutput()
{
  for(int idx1=0; idx1<2; idx1++){
    for (int idx2=0; idx2<NumberOfNSWTriggerProcesser; idx2++){
      m_buffer[idx1][idx2]->clear(); 
    }
  }
}

void TGCNSW::print() const
{
  for(int idx1=0; idx1<2; idx1++){
    for (int idx2=0; idx2<NumberOfNSWTriggerProcesser; idx2++){
      m_buffer[idx1][idx2]->print(); 
    }
  }
}
  

}   // end of namespace
