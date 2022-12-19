/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TrigT1TGC_BoardConnection_H_
#define TrigT1TGC_BoardConnection_H_

#include <vector>
#include <iostream>

namespace LVL1TGCTrigger {

class TGCBoardConnection {
 public:
  TGCBoardConnection();
  virtual ~TGCBoardConnection();

  TGCBoardConnection(const TGCBoardConnection& right);
  TGCBoardConnection& operator = (const TGCBoardConnection& right);

  inline int getNumberOfType() const { return m_id.size(); }
  int getNumber(const unsigned int type) const;
  int getId(const unsigned int type, const unsigned int board) const;

  void setNumberOfType(int ntype);
  void setNumber(const unsigned int type, int nBoard);
  void setId(const unsigned int type, const unsigned int board, int idIn);

 protected:
  std::vector<std::vector<int>> m_id;   // [type][board]
};

inline int TGCBoardConnection::getNumber(const unsigned int type) const {
  if (m_id.size() <= type) {
    std::cerr << "TGCBoardConnection::getNumber : No defined type provided" << std::endl;
    return -1;
  }
  return m_id.at(type).size();
}

inline int TGCBoardConnection::getId(const unsigned int type, const unsigned int board) const {
  if (m_id.size() <= type ||
      m_id.at(type).size() <= board) {
    std::cerr << "TGCBoardConnection::getId : Undefined board is provided" << std::endl;
    return -1;
  }
  return m_id.at(type).at(board);
}


} //end of namespace bracket

#endif  // TrigT1TGC_BoardConnection_H_
