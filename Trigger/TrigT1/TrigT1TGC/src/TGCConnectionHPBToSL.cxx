/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigT1TGC/TGCConnectionHPBToSL.h"

namespace LVL1TGCTrigger {

// default constructor
TGCConnectionHPBToSL::TGCConnectionHPBToSL()
{
  setNumberOfType(TGCSector::NumberOfHighPtBoardType);
  for(int j=0; j<TGCSector::NumberOfHighPtBoardType; j++) m_SLPortToHPB[j]=0;
}

// Copy constructor
TGCConnectionHPBToSL::TGCConnectionHPBToSL(const TGCConnectionHPBToSL& right)
 : TGCBoardConnection(right)
{
  for(int j=0; j<TGCSector::NumberOfHighPtBoardType; j++) {
    if(m_SLPortToHPB[j]!=0) delete [] m_SLPortToHPB[j];
    m_SLPortToHPB[j] = new int [m_id.at(j).size()];

    for (unsigned int k=0; k < m_id.at(j).size(); k++) m_SLPortToHPB[j][k] = right.m_SLPortToHPB[j][k];
  }
}


TGCConnectionHPBToSL::~TGCConnectionHPBToSL()
{
  for(int j=0; j<TGCSector::NumberOfHighPtBoardType; j++) {
    if(m_SLPortToHPB[j]!=0) delete [] m_SLPortToHPB[j];
    m_SLPortToHPB[j]=0;
  }
}

TGCConnectionHPBToSL& TGCConnectionHPBToSL::operator=(const TGCConnectionHPBToSL& right)
{
  if(this!=&right){
    TGCBoardConnection::operator=(right); // call base class assignment operator
    for(int j=0; j<TGCSector::NumberOfHighPtBoardType; j+=1){
      if(m_SLPortToHPB[j]!=0) delete [] m_SLPortToHPB[j];
      m_SLPortToHPB[j] = new int [m_id.at(j).size()]; 
      for (unsigned int k=0; k < m_id.at(j).size(); k++)
	m_SLPortToHPB[j][k] = right.m_SLPortToHPB[j][k];
    }
  }
  return *this;
}

} //end of namespace bracket
