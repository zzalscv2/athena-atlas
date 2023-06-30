/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "LArRawEvent/LArDigit.h"

// set method
void LArDigit::setSamples(const std::vector<short>& samples)
{
  m_samples.clear();
  m_samples = samples;
}
