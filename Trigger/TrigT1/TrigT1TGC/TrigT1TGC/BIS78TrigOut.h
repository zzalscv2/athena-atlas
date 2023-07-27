/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef BIS78_TRIGOUT_H
#define BIS78_TRIGOUT_H

#include <vector>

namespace LVL1TGCTrigger {
class TGCArguments;
}

namespace LVL1TGC {

class BIS78TrigOut {
 protected:
  std::vector<int>            m_BIS78PadBoard;   // 0 ~ 15 ??????????
  std::vector<uint8_t>            m_BIS78eta_6bit;     //
  std::vector<uint8_t>            m_BIS78phi_6bit;      //
  std::vector<uint8_t>            m_BIS78Deta_3bit;    //
  std::vector<uint8_t>            m_BIS78Dphi_3bit;    //
  std::vector<uint8_t>            m_BIS78flag3over3eta_1bit;
  std::vector<uint8_t>            m_BIS78flag3over3phi_1bit;

 public:
  BIS78TrigOut() = default;
  BIS78TrigOut(const std::vector<int>& BIS78Trigger,
               const std::vector<uint8_t>& BIS78eta,
               const std::vector<uint8_t>& BIS78phi,
               const std::vector<uint8_t>& BIS78Deta,
               const std::vector<uint8_t>& BIS78Dphi,
               const std::vector<uint8_t>& BIS78flag3over3over3Eta,
               const std::vector<uint8_t>& BIS78flag3over3Phi,
               LVL1TGCTrigger::TGCArguments* tgcargs=nullptr);
  BIS78TrigOut(const std::vector<int>& BIS78Trigger, LVL1TGCTrigger::TGCArguments* tgcargs=nullptr);
  
  BIS78TrigOut& operator+=(const BIS78TrigOut& right);
  bool operator==(const BIS78TrigOut& right) const
  {
    return (this==&right);
  }
 
  bool operator!=(const BIS78TrigOut& right) const
  {
    return (this!=&right);
  }


  // set functons
  void setBIS78PadBoard(int BIS78PB){ m_BIS78PadBoard.push_back(BIS78PB); }
  void setEta(uint8_t BIS78eta){ m_BIS78eta_6bit.push_back(BIS78eta); }
  void setPhi(uint8_t BIS78phi){ m_BIS78phi_6bit.push_back(BIS78phi); }
  void setDeta(uint8_t BIS78Deta){ m_BIS78Deta_3bit.push_back(BIS78Deta); }
  void setDphi(uint8_t BIS78Dphi){ m_BIS78Dphi_3bit.push_back(BIS78Dphi); }
  void setflag3over3eta(uint8_t BIS78flag3over3eta){ m_BIS78flag3over3eta_1bit.push_back(BIS78flag3over3eta); }
  void setflag3over3phi(uint8_t BIS78flag3over3phi){ m_BIS78flag3over3phi_1bit.push_back(BIS78flag3over3phi); }
  void clear() { m_BIS78PadBoard.clear(); m_BIS78eta_6bit.clear();  m_BIS78phi_6bit.clear(); m_BIS78Deta_3bit.clear(); m_BIS78Dphi_3bit.clear(); m_BIS78flag3over3eta_1bit.clear(); m_BIS78flag3over3phi_1bit.clear(); }

  // get functions
  const std::vector<int>& getBIS78PadBoard() const {return m_BIS78PadBoard; }
  const std::vector<uint8_t>& getBIS78eta() const {return m_BIS78eta_6bit; }
  const std::vector<uint8_t>& getBIS78phi() const {return m_BIS78phi_6bit; }
  const std::vector<uint8_t>& getBIS78Deta() const {return m_BIS78Deta_3bit; }
  const std::vector<uint8_t>& getBIS78Dphi() const {return m_BIS78Dphi_3bit; }
  const std::vector<uint8_t>& getBIS78flag3over3eta() const {return m_BIS78flag3over3eta_1bit; }
  const std::vector<uint8_t>& getBIS78flag3over3phi() const {return m_BIS78flag3over3phi_1bit; }

  // print methods 
  void print() const;

 private:
  LVL1TGCTrigger::TGCArguments* m_tgcArgs{nullptr};
};

} //end of namespace bracket

#endif
