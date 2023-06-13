/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "LArRawEvent/LArDigit.h"
#include <cstdio>
#include <typeinfo>

LArDigit::operator std::string() const{
  std::string digitString = typeid( *this ).name();
 return digitString ;
}

// set method
void LArDigit::setSamples(const std::vector<short>& samples)
{
  m_samples.clear();
  m_samples = samples;
}
