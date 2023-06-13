/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include <typeinfo>

#include "LArRawEvent/LArRawSC.h"


LArRawSC::operator std::string() const{
 std::string digitString = typeid( *this ).name();
 return digitString ;
}

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
