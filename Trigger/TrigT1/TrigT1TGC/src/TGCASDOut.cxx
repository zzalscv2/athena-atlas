/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include <iostream>
#include <iomanip>

#include "GaudiKernel/SystemOfUnits.h"
#include "TrigT1TGC/TGCASDOut.h"

namespace LVL1TGCTrigger {

TGCASDOut::TGCASDOut(TGCReadoutIndex tgcrindex, 
		     TGCSignalType sigtype, int id, double tof)
  : m_tgcReadoutIndex(tgcrindex),
    m_signalType(sigtype), m_hitID(id), m_hitToF(tof)
{
}


void TGCASDOut::SetParams(TGCSignalType signal_type, int id, double tof)
{
  m_signalType= signal_type;
  m_hitID= id;
  m_hitToF= tof;
}


void TGCASDOut::Print() const
{
  const char* const strsig[3]= { "N/A", "Wire", "Strip" };

  m_tgcReadoutIndex.Print(); 
  std::cout << "::" << std::setw(9) << strsig[m_signalType] 
            << ":: ID=" << std::setw(3) << m_hitID
            << ", tof=" << std::setw(5) << std::setprecision(1) << m_hitToF/Gaudi::Units::ns << "ns"
            << std::setprecision(6) << std::endl;
}


} //end of namespace bracket
