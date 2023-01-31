/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigT1TGC/TGCConnectionPPToSB.h"

namespace LVL1TGCTrigger {

TGCConnectionPPToSB::~TGCConnectionPPToSB()
{
  for (int i=0; i < N_PP_PORTS; i++) {
    for (int j=0; j<TGCSector::NumberOfPatchPanelType; j+=1){
      if (m_SBIdToPP[i][j]!=0) delete [] m_SBIdToPP[i][j];
      m_SBIdToPP[i][j]=0;
    }
  }
}

TGCConnectionPPToSB::TGCConnectionPPToSB()
{
  setNumberOfType(TGCSector::NumberOfPatchPanelType);
  for(int i=0; i < N_PP_PORTS; i+=1)
    for(int j=0; j < TGCSector::NumberOfPatchPanelType; j+=1)
      m_SBIdToPP[i][j]=0;
}

TGCConnectionPPToSB::TGCConnectionPPToSB(const TGCConnectionPPToSB& right) : 
  TGCBoardConnection(right)
{
  for (int i=0; i < N_PP_PORTS; i+=1) {
    for (int j=0; j<TGCSector::NumberOfPatchPanelType; j+=1){
      if(m_SBIdToPP[i][j]!=0) delete [] m_SBIdToPP[i][j];
      m_SBIdToPP[i][j] = new int [m_id.at(j).size()];
      for(unsigned int k=0; k < m_id.at(j).size(); k++)
	m_SBIdToPP[i][j][k] = right.m_SBIdToPP[i][j][k];
    }
  }
}

TGCConnectionPPToSB& TGCConnectionPPToSB::operator=(const TGCConnectionPPToSB& right)
{
  if(this!=&right){
    int j;
    for (int i=0; i < N_PP_PORTS; i++) {
      for( j=0; j<TGCSector::NumberOfPatchPanelType; j+=1){
	if(m_SBIdToPP[i][j]!=0) delete [] m_SBIdToPP[i][j];
	m_SBIdToPP[i][j] = new int [m_id.at(j).size()];
	for (unsigned int k=0; k < m_id.at(j).size(); k++)
	  m_SBIdToPP[i][j][k] = right.m_SBIdToPP[i][j][k];
      }
    }
  }
  return *this;
}

}   // end of namespace
