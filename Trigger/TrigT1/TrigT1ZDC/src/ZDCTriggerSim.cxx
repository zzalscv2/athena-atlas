/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "ZDCTriggerSim.h"
#include <iostream>
#include <stdexcept>

//dump stream data
void ZDCTriggerSimBase::dump(std::ostream& strm) const
{
  for (auto entry : m_stack) {
    strm << entry->getType() << ": ";
    entry->dump(strm);
    strm << std::endl;
  }
}

//Obtain 3 bit output from 4*2 bit arm trigger decisions
void ZDCTriggerSimCombLUT::doSimStage()
{
  ZDCTriggerSim::SimDataCPtr ptr = stackTopData();
  if (ptr->getNumData() != 2 || ptr->getNumBits() != 4) throw std::logic_error("Invalid input data in ZDCTriggerSimCombLUT");

  unsigned int bitsSideA = ptr->getValueTrunc(0);
  unsigned int bitsSideC = ptr->getValueTrunc(1);

  unsigned int address = (bitsSideC<<4) + bitsSideA;
  unsigned int comLUTvalue = m_combLUT.at(address);

  // ZDCTriggerSim::SimDataPtr uses shared_ptr semantics so cleanup is guaranteed
  //
  ZDCTriggerSim::SimDataPtr lutOut_p(new ZDCTriggerSim::CombLUTOutput());
  static_cast<ZDCTriggerSim::CombLUTOutput*>(lutOut_p.get())->setDatum(comLUTvalue);

  stackPush(lutOut_p);
}

// Obtain 4x2 bit output from arm energy sums
void ZDCTriggerSimAllLUTs::doSimStage()
{
  ZDCTriggerSim::SimDataCPtr ptr = stackTopData();
  if (ptr->getNumData() != 2 || ptr->getNumBits() != 12) throw std::logic_error("Invalid input data in ZDCTriggerSimAllLUTs");;

  unsigned int inputSideA = ptr->getValueTrunc(0);
  unsigned int inputSideC = ptr->getValueTrunc(1);

  unsigned int valueA = m_LUTA.at(inputSideA);
  unsigned int valueC = m_LUTC.at(inputSideC);

  // ZDCTriggerSim::SimDataPtr uses shared_ptr semantics so cleanup is guaranteed
  //
  ZDCTriggerSim::SimDataPtr inputs_p(new ZDCTriggerSim::CombLUTInputsInt);
  static_cast<ZDCTriggerSim::CombLUTInputsInt*>(inputs_p.get())->setData({valueA, valueC});

  stackPush(ZDCTriggerSim::SimDataCPtr(inputs_p));
  ZDCTriggerSimCombLUT::doSimStage();
}

// Obtain arm energy sums from Module by Module Calibrated energies
void ZDCTriggerSimModuleAmpls::doSimStage()
{
  ZDCTriggerSim::SimDataCPtr ptr = stackTopData();
  if (ptr->getNumData() != 8 || ptr->getNumBits() != 12) throw std::logic_error("Invalid input data in ZDCTriggerSimModuleAmpls");

  unsigned int sumA = 0;
  for (size_t i = 0; i < 4; i++) {
    sumA += ptr->getValueTrunc(i);
  }

  unsigned int sumC = 0;
  for (size_t i = 4; i < 8; i++) {
    sumC += ptr->getValueTrunc(i);
  }

  // The sums get divided by 4
  //
  sumA /= 4;
  sumC /= 4;

  ZDCTriggerSim::SimDataPtr inputs_p(new ZDCTriggerSim::SideLUTInputsInt);
  static_cast<ZDCTriggerSim::SideLUTInputsInt*>(inputs_p.get())->setData({sumA, sumC});

  stackPush(ZDCTriggerSim::SimDataCPtr(inputs_p));

  ZDCTriggerSimAllLUTs::doSimStage();
}
