/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigT1TGC/TGCConnectionSBToHPB.h"

namespace LVL1TGCTrigger {

TGCConnectionSBToHPB::~TGCConnectionSBToHPB()
{
  int j;
  for( j=0; j<NumberOfSlaveBoardType; j+=1){
    if(m_HPBPortToSB[j]!=0) delete [] m_HPBPortToSB[j];
    m_HPBPortToSB[j]=0;

    if(m_HPBIdToSB[j]!=0) delete [] m_HPBIdToSB[j];
    m_HPBIdToSB[j]=0;
  }
}

TGCConnectionSBToHPB::TGCConnectionSBToHPB()
{
  // NO HPB for Inner
  setNumberOfType(4);
  
  int j;
  for( j=0; j<NumberOfSlaveBoardType; j+=1){
    m_HPBPortToSB[j]=0;
    m_HPBIdToSB[j]=0;
  }
}

TGCConnectionSBToHPB::TGCConnectionSBToHPB(const TGCConnectionSBToHPB& right) :
  TGCBoardConnection(right)
{
  int j;
  for( j=0; j<NumberOfSlaveBoardType; j+=1){
    if(m_HPBPortToSB[j]!=0) delete [] m_HPBPortToSB[j];
    m_HPBPortToSB[j] = new int [m_id.at(j).size()];
    if(m_HPBIdToSB[j]!=0) delete [] m_HPBIdToSB[j];
    m_HPBIdToSB[j] = new int [m_id.at(j).size()];
    for(unsigned int k=0; k < m_id.at(j).size(); k++) {
      m_HPBPortToSB[j][k] = right.m_HPBPortToSB[j][k];
      m_HPBIdToSB[j][k] = right.m_HPBIdToSB[j][k];
    }
  }
}

TGCConnectionSBToHPB& TGCConnectionSBToHPB::operator=(const TGCConnectionSBToHPB& right)
{
  if(this!=&right){
    TGCBoardConnection::operator=(right); // call base class assignment operator
    int j;
    for( j=0; j<NumberOfSlaveBoardType; j+=1){
      if(m_HPBPortToSB[j]!=0) delete [] m_HPBPortToSB[j];
      m_HPBPortToSB[j] = new int [m_id.at(j).size()];
      if(m_HPBIdToSB[j]!=0) delete [] m_HPBIdToSB[j];
      m_HPBIdToSB[j] = new int [m_id.at(j).size()];
      for (unsigned int k=0; k<m_id.at(j).size(); k++) {
	m_HPBPortToSB[j][k] = right.m_HPBPortToSB[j][k];
	m_HPBIdToSB[j][k] = right.m_HPBIdToSB[j][k];
      }
    }
  }
  return *this;
}

} //end of namespace bracket
