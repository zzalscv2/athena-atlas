/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigT1TGC/TGCBoardConnection.h"

namespace LVL1TGCTrigger {

TGCBoardConnection::TGCBoardConnection() {
  m_id.clear();
}

TGCBoardConnection::~TGCBoardConnection() {
  for (unsigned int i=0; i<m_id.size(); i++){
    m_id.at(i).clear();
  }
  m_id.clear();
}

void TGCBoardConnection::setNumberOfType(int ntype) {
  for (unsigned int i=0; i < m_id.size(); i++) {
    m_id.at(i).clear();
  }
  m_id.resize(ntype);
}

void TGCBoardConnection::setNumber(const unsigned int type, int nBoard) {
  if (type < m_id.size()) {
    m_id.at(type).resize(nBoard);
  } else {
    m_id.resize(type+1);
    m_id.at(type).resize(nBoard);
  }
}

void TGCBoardConnection::setId(const unsigned int type, const unsigned int board, int idIn) {
  if (type < m_id.size()) {
    if (board < m_id.at(type).size()) {
      m_id.at(type).at(board) = idIn;
    } else {
      m_id.at(type).resize(board+1);
      m_id.at(type).at(board) = idIn;
    }
  } else {
    m_id.resize(type+1);
    m_id.at(type).resize(board+1);
    m_id.at(type).at(board) = idIn;
  }
}

TGCBoardConnection::TGCBoardConnection(const TGCBoardConnection& right) {
  m_id.resize(right.m_id.size());
  for (unsigned int i=0; i < m_id.size(); i++) {
    m_id.at(i).resize(right.m_id.at(i).size());
    for (unsigned int j=0; j < m_id.at(i).size(); j++) {
      m_id.at(i).at(j) = right.m_id.at(i).at(j);
    }
  }
}

TGCBoardConnection& TGCBoardConnection::operator = (const TGCBoardConnection& right) {
  if (this != &right) {
    m_id.resize(right.m_id.size());
    for (unsigned int i=0; i < m_id.size(); i++) {
      m_id.at(i).resize(right.m_id.at(i).size());
      for (unsigned int j=0; j < m_id.at(i).size(); j++) {
        m_id.at(i).at(j) = right.m_id.at(i).at(j);
      }
    }
  }
  return *this;
}

}   // end of namespace
