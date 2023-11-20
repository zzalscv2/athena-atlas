/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "LArRawConditions/LArPedestalMC.h" 

#include <iostream> 
using namespace std ;

LArPedestalMC::LArPedestalMC() {}

LArPedestalMC::~LArPedestalMC() {}

/* Fill transient object in ATHENA *****************************************
 */
void LArPedestalMC::set(const std::vector<float>& vPedestal,
                        const std::vector<float>& vPedestalRMS )
{
  m_vPedestal    = vPedestal;
  m_vPedestalRMS = vPedestalRMS;

}


/* retrieve Pedestal ******************************************************
 */
float LArPedestalMC::pedestal(const HWIdentifier& /*CellID*/, int /*gain*/) const 
{ 
  if (!m_vPedestal.empty()) return m_vPedestal[0];
  else  return LArElecCalib::ERRORCODE;
}

float LArPedestalMC::pedestalRMS(const HWIdentifier& /*CellID*/, int /*gain*/) const 
{ 

  if (!m_vPedestalRMS.empty()) return m_vPedestalRMS[0];
  else  return LArElecCalib::ERRORCODE;

}
