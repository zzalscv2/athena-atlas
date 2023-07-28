/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "AthenaKernel/getMessageSvc.h"
#include "TrigT1TGC/BIS78TrigOut.h"
#include "TrigT1TGC/TGCArguments.h"

namespace LVL1TGC {

BIS78TrigOut::BIS78TrigOut(const std::vector<int>& BIS78PB,
                           const std::vector<uint8_t>& BIS78eta,
                           const std::vector<uint8_t>& BIS78phi,
                           const std::vector<uint8_t>& BIS78Deta,
                           const std::vector<uint8_t>& BIS78Dphi,
                           const std::vector<uint8_t>& BIS78flag3over3eta,
                           const std::vector<uint8_t>& BIS78flag3over3phi,
                           LVL1TGCTrigger::TGCArguments* tgcargs)
  : m_BIS78PadBoard(BIS78PB), m_BIS78eta_6bit(BIS78eta), m_BIS78phi_6bit(BIS78phi), m_BIS78Deta_3bit(BIS78Deta), m_BIS78Dphi_3bit(BIS78Dphi), m_BIS78flag3over3eta_1bit(BIS78flag3over3eta), m_BIS78flag3over3phi_1bit(BIS78flag3over3phi),
    m_tgcArgs(tgcargs)
{}

BIS78TrigOut::BIS78TrigOut(const std::vector<int>& BIS78PB, LVL1TGCTrigger::TGCArguments* tgcargs)
  : m_BIS78PadBoard(BIS78PB), m_tgcArgs(tgcargs)
{}


BIS78TrigOut& BIS78TrigOut::operator+=(const BIS78TrigOut& right)
{
  if (this != &right) {
    m_BIS78PadBoard.insert(m_BIS78PadBoard.end(), right.m_BIS78PadBoard.begin(), right.m_BIS78PadBoard.end());
    m_BIS78eta_6bit.insert(m_BIS78eta_6bit.end(), right.m_BIS78eta_6bit.begin(), right.m_BIS78eta_6bit.end());
    m_BIS78phi_6bit.insert(m_BIS78phi_6bit.end(), right.m_BIS78phi_6bit.begin(), right.m_BIS78phi_6bit.end());
    m_BIS78Deta_3bit.insert(m_BIS78Deta_3bit.end(), right.m_BIS78Deta_3bit.begin(), right.m_BIS78Deta_3bit.end());
    m_BIS78Dphi_3bit.insert(m_BIS78Dphi_3bit.end(), right.m_BIS78Dphi_3bit.begin(), right.m_BIS78Dphi_3bit.end());
    m_BIS78flag3over3eta_1bit.insert(m_BIS78flag3over3eta_1bit.end(), right.m_BIS78flag3over3eta_1bit.begin(), right.m_BIS78flag3over3eta_1bit.end());
    m_BIS78flag3over3phi_1bit.insert(m_BIS78flag3over3phi_1bit.end(), right.m_BIS78flag3over3phi_1bit.begin(), right.m_BIS78flag3over3phi_1bit.end());
  }
  return *this;
}

void BIS78TrigOut::print() const
{
  MsgStream msg(Athena::getMessageSvc(), "LVL1TGC::BIS78TrigOut");
  if (m_tgcArgs) msg.setLevel(m_tgcArgs->MSGLEVEL());

  for(unsigned int i=0;i!=m_BIS78PadBoard.size();i++){
    msg << MSG::DEBUG
        << "i=" << i<<"\n"
        <<"Size=="<<m_BIS78PadBoard.size()<<"\n"
        << " :: ModuleID=" << static_cast<int>(m_BIS78PadBoard[i])<<"\n"
        << " :: eta_6bit=" << static_cast<int>(m_BIS78eta_6bit[i])<<"\n"
        << " :: phi_6bit=" << static_cast<int>(m_BIS78phi_6bit[i])<<"\n"
        << " :: Deta_3bit=" << static_cast<int>(m_BIS78Deta_3bit[i])<<"\n"
        << " :: Dphi_3bit=" << static_cast<int>(m_BIS78Dphi_3bit[i])<<"\n"
        << " :: flag3over3eta_1bit=" << static_cast<int>(m_BIS78flag3over3eta_1bit[i])<<"\n"
        << " :: flag3over3phi_1bit=" << static_cast<int>(m_BIS78flag3over3phi_1bit[i])<<"\n"
        << endmsg;
  }
}


}  // end of namespace
