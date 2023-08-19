/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include <iostream>
#include <iomanip>
#include "TrigT1TGC/TGCReadoutIndex.h"

namespace LVL1TGCTrigger {

//////////////////////////////////
TGCReadoutIndex::TGCReadoutIndex()
 : m_zDirection(kZ_FORWARD), m_octantNumber(0),
   m_moduleNumber(0), m_rNumber(0), m_layerNumber(0)
{
}

///////////////////////////////////////////////////////////
TGCReadoutIndex::TGCReadoutIndex(TGCZDirection iz, int ioct, 
				 int imd, int ir, int ilyr)
 : m_zDirection(iz), m_octantNumber(ioct),
   m_moduleNumber(imd), m_rNumber(ir), m_layerNumber(ilyr)
{
}

///////////////////////////////////////////////////////////

void TGCReadoutIndex::Print() const
{
  static const char gZDirName[kTotalNumTGCZDirection] = {'F', 'B'};
  std::cout << "  " << gZDirName[m_zDirection] << "-" 
            << std::setw(1) << m_octantNumber << "-"
            << std::setw(2) << m_moduleNumber << "-" 
            << m_rNumber << "-"
            << m_layerNumber;
}


} //end of namespace bracket
