/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// ref. SOS051V07,S0S052V06 
#include "TrigT1TGC/TGCWireTripletSB.h"
#include "TrigT1TGC/TGCPatchPanel.h"
#include "TrigT1TGC/TGCPatchPanelOut.h"
#include <iostream>
#include <cstdlib>

namespace LVL1TGCTrigger {

TGCWireTripletSB::TGCWireTripletSB()
 : TGCSlaveBoard()
{}

void TGCWireTripletSB::createSlaveBoardOut()
{
  if(m_coincidenceOut!=0){
    if ( m_slaveBoardOut != 0 ) delete m_slaveBoardOut;
    m_slaveBoardOut = new  TGCSlaveBoardOut(this, m_bid);
    m_slaveBoardOut->clear();
    m_slaveBoardOut->setNumberOfData(s_NumberOfWireTripletSBData);

    // fill SlaveBoardOut.
    // select largest R hit in each sections.
    int lengthOfSection = (m_coincidenceOut->getLength() - 2*s_NChAdjInWTSB)/s_NumberOfWireTripletSBData;
    int i,j;
    for( i=0; i<s_NumberOfWireTripletSBData; i+=1){ // i=0:a 1:b 2:c
      m_slaveBoardOut->setPos(i,-1);
      m_slaveBoardOut->setPos(i,0);
      m_slaveBoardOut->setHit(i,false);
      for( j=0; j<lengthOfSection; j+=1) {
        if(m_coincidenceOut->getChannel(s_NChAdjInWTSB + j + i*lengthOfSection)){
          m_slaveBoardOut->setPos(i,j);
          m_slaveBoardOut->setHit(i,true);
          break;
        }
      }
      if(m_slaveBoardOut->getHit(i)){
        m_slaveBoardOut->setbPos(i, m_slaveBoardOut->getPos(i));
      }
    }
  }
}

void TGCWireTripletSB::doCoincidence()
{
  const TGCHitPattern* pattern[2];
  // pattern[0] has hit pattern of layer0[36-71].
  // pattern[1] has hit pattern of layer1[0-35] and layer2[36-71] .
  pattern[0] = m_patchPanelOut->getHitPattern(0);
  pattern[1] = m_patchPanelOut->getHitPattern(1);
  
  if(pattern[1] != 0) {  // necessary for 2/3 coincidence. 
    const int unitLength = pattern[1]->getLength() / 2;
    const int totalLength = 3 * unitLength;

    m_lengthOfCoincidenceOut = s_LengthOfWTSBCoincidenceOut;
    if (m_coincidenceOut != 0) delete m_coincidenceOut;
    m_coincidenceOut = new TGCHitPattern(m_lengthOfCoincidenceOut);

    // rearrange bit pattern for coincidence.
    std::vector<bool> b(totalLength);
    for (int i=0; i < unitLength; i++) {
      if (pattern[0] != 0) {
        b[3*i] = pattern[0]->getChannel(i+unitLength); //layer0(smallest in eta)
      } else {
        b[3*i] = 0;
      }
      if (pattern[1] != 0) {
        b[3*i+1] = pattern[1]->getChannel(i); // layer1 
        b[3*i+2] = pattern[1]->getChannel(i+unitLength); //layer2
      } else {
        b[3*i+1] = 0;
        b[3*i+2] = 0;
      }
    }

    // perform 2/3 coincidence
    std::vector<bool> output(totalLength);
    for (int i=0; i < totalLength-8; i++) {
      output[i] = (b[i+3] & b[i+5] & !b[i+1] & !b[i+7]) |
                  (b[i+3] & b[i+4] & !b[i+2]) |
                  (b[i+4] & b[i+5] & !b[i+6]) |
                  (b[i+6] & b[i+2] &  b[i+4] & !b[i+3] & !b[i+5] & !b[i+1] & !b[i+7]);
    }

    int base = 0;
    for (int i=0; i<m_lengthOfCoincidenceOut; i++) {
      m_coincidenceOut->setChannel(i, output[i+base]);
    }
  }
}

TGCWireTripletSB& TGCWireTripletSB::operator=(const TGCWireTripletSB& right)
{
  if ( this != &right ) {
    m_id = right.m_id;
    m_bid = right.m_bid;
    m_idHighPtBoard = right.m_idHighPtBoard;
    m_type = right.m_type;
    m_lengthOfCoincidenceOut = right.m_lengthOfCoincidenceOut;
    *m_slaveBoardOut = *right.m_slaveBoardOut;
    *m_patchPanel = *right.m_patchPanel;
    *m_patchPanelOut = *right.m_patchPanelOut;

    if ( m_lengthOfCoincidenceOut > 0) { 
      if ( m_coincidenceOut ) delete  m_coincidenceOut; 
      m_coincidenceOut = new TGCHitPattern (m_lengthOfCoincidenceOut);
    }
  }
  return *this;
}



} //end of namespace bracket
