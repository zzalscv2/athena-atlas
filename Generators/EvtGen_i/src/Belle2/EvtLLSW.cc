/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Florian Bernlochner                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "EvtGenBase/EvtParticle.hh"
#include "EvtGenBase/EvtPDL.hh"
#include <string>
#include "EvtGenBase/EvtSemiLeptonicScalarAmp.hh"
#include "EvtGenBase/EvtSemiLeptonicVectorAmp.hh"
#include "EvtGenBase/EvtSemiLeptonicTensorAmp.hh"

//#include "generators/evtgen/EvtGenModelRegister.h"
#include "EvtGen_i/EvtGenExternal/Belle2/EvtLLSW.h"
#include "EvtGen_i/EvtGenExternal/Belle2/EvtLLSWFF.h"

//B2_EVTGEN_REGISTER_MODEL(EvtLLSW);

EvtLLSW::EvtLLSW():
  m_llswffmodel(0)
  , m_calcamp(0)
{}


EvtLLSW::~EvtLLSW()
{
  delete m_llswffmodel;
  m_llswffmodel = 0;
  delete m_calcamp;
  m_calcamp = 0;
}

std::string EvtLLSW::getName()
{

  return "LLSW";

}



EvtDecayBase* EvtLLSW::clone()
{

  return new EvtLLSW;

}

void EvtLLSW::decay(EvtParticle* p)
{

  p->initializePhaseSpace(getNDaug(), getDaugs());

  m_calcamp->CalcAmp(p, _amp2, m_llswffmodel);

}

void EvtLLSW::initProbMax()
{

  EvtId parnum, mesnum, lnum;

  parnum = getParentId();
  mesnum = getDaug(0);
  lnum = getDaug(1);

  // Leptons
  static const EvtId EM = EvtPDL::getId("e-");
  static const EvtId EP = EvtPDL::getId("e+");
  static const EvtId MUM = EvtPDL::getId("mu-");
  static const EvtId MUP = EvtPDL::getId("mu+");
  static const EvtId TAUM = EvtPDL::getId("tau-");
  static const EvtId TAUP = EvtPDL::getId("tau+");


  if (lnum == EP || lnum == EM || lnum == MUP || lnum == MUM) {
    setProbMax(5000.0);
    return;
  }
  if (lnum == TAUP || lnum == TAUM) {
    setProbMax(4000.0);
    return;
  }


}

void EvtLLSW::init()
{

  if (getNArg() < 2) checkNArg(2);
  checkNDaug(3);

  static const EvtId D1P1P = EvtPDL::getId("D_1+");
  static const EvtId D1P1N = EvtPDL::getId("D_1-");
  static const EvtId D1P10 = EvtPDL::getId("D_10");
  static const EvtId D1P1B = EvtPDL::getId("anti-D_10");
  static const EvtId D3P2P = EvtPDL::getId("D_2*+");
  static const EvtId D3P2N = EvtPDL::getId("D_2*-");
  static const EvtId D3P20 = EvtPDL::getId("D_2*0");
  static const EvtId D3P2B = EvtPDL::getId("anti-D_2*0");

  static const EvtId DS1P = EvtPDL::getId("D_s1+");
  static const EvtId DS1M = EvtPDL::getId("D_s1-");
  static const EvtId DS2STP = EvtPDL::getId("D_s2*+");
  static const EvtId DS2STM = EvtPDL::getId("D_s2*-");

  EvtId daughter = getDaug(0);

  bool isNarrow = false;

  if (daughter == D1P1P || daughter == D1P1N || daughter == D1P10 || daughter == D1P1B ||
      daughter == D3P2P || daughter == D3P2N || daughter == D3P20 || daughter == D3P2B ||
      daughter == DS1P || daughter == DS1M || daughter == DS2STP || daughter == DS2STM)
    isNarrow = true;

  //We expect the parent to be a scalar
  //and the daughters to be X lepton neutrino

  checkSpinParent(EvtSpinType::SCALAR);
  checkSpinDaughter(1, EvtSpinType::DIRAC);
  checkSpinDaughter(2, EvtSpinType::NEUTRINO);

  EvtSpinType::spintype mesontype = EvtPDL::getSpinType(getDaug(0));

  if (isNarrow)
    m_llswffmodel = new EvtLLSWFF(getArg(0), getArg(1), getNArg() > 2 ? getArg(2) : 0., getNArg() > 3 ? getArg(3) : 0.);
  else
    m_llswffmodel = new EvtLLSWFF(getArg(0), getArg(1), getNArg() > 2 ? getArg(2) : 0.);

  if (mesontype == EvtSpinType::SCALAR) {
    m_calcamp = new EvtSemiLeptonicScalarAmp;
  }
  if (mesontype == EvtSpinType::VECTOR) {
    m_calcamp = new EvtSemiLeptonicVectorAmp;
  }
  if (mesontype == EvtSpinType::TENSOR) {
    m_calcamp = new EvtSemiLeptonicTensorAmp;
  }

}







