/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "LArRawEvent/LArRawSC.h"

// set method
void LArRawSC::setEnergies(const std::vector<int>& energies)
{
  m_energies = energies;
}

void LArRawSC::setBCIds(const std::vector<unsigned short>& bcids)
{
  m_BCId = bcids;
}

void LArRawSC::setTauEnergies( const std::vector < int >& tauEnergies)
{
  m_tauEnergies=tauEnergies;
}

void LArRawSC::setPassTauSelection( const std::vector < bool >& pass)
{
  m_passTauSelection=pass;
}
