/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

//  Table of connection between Slave Board and High-Pt Board.
#ifndef TrigT1TGC_ConnectionSBToHPB_H_
#define TrigT1TGC_ConnectionSBToHPB_H_

#include "TrigT1TGC/TGCBoardConnection.h"
#include "TrigT1TGC/TGCNumbering.h"

namespace LVL1TGCTrigger {

class TGCConnectionSBToHPB : public TGCBoardConnection {
 public:
  int getHPBPortToSB(int type, int index) const;
  void setHPBPortToSB(int type, int index, int port);

  int getHPBIdToSB(int type, int index) const;
  void setHPBIdToSB(int type, int index, int id);

  TGCConnectionSBToHPB();
  virtual ~TGCConnectionSBToHPB();
  TGCConnectionSBToHPB(const TGCConnectionSBToHPB& right);
  TGCConnectionSBToHPB& operator=(const TGCConnectionSBToHPB& right);

 private:
  int* m_HPBPortToSB[NumberOfSlaveBoardType];
  int* m_HPBIdToSB[NumberOfSlaveBoardType];
};

inline
int TGCConnectionSBToHPB::getHPBPortToSB(int type, int index) const {
  return m_HPBPortToSB[type][index];
}

inline
void TGCConnectionSBToHPB::setHPBPortToSB(int type, int index, int port) {
  if(m_HPBPortToSB[type]==0) m_HPBPortToSB[type] = new int [m_id.at(type).size()];
  m_HPBPortToSB[type][index] = port;
}

inline
int TGCConnectionSBToHPB::getHPBIdToSB(int type, int index) const {
  return m_HPBIdToSB[type][index];
}

inline
void TGCConnectionSBToHPB::setHPBIdToSB(int type, int index, int id) {
  if(m_HPBIdToSB[type]==0) m_HPBIdToSB[type] = new int [m_id.at(type).size()];
  m_HPBIdToSB[type][index] = id;
}

}  // end of namespace

#endif  // TrigT1TGC_ConnectionSBToHPB_H_
