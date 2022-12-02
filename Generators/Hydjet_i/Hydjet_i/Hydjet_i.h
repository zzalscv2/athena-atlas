/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// --------------------------------------------------
//
// File:  Generators/Hydjet.h
// Description:
//    This code is used to get a HYDJET Monte Carlo event.
//    genInitialize() is used to read parameters
//    callGenerator() makes the event
//    genFinalize() writes log files etc
//    fillEvt(GeneratorEvent* evt) passes the event to HepMC
//
//    The output will be stored in the transient event store so it can be
//    passed to the simulation.
//
// AuthorList:
//         Andrzej Olszewski, February 2007
//         Andrzej Olszewski, March 2008

#ifndef GENERATORMODULESHYDJET_H
#define GENERATORMODULESHYDJET_H

#include "GeneratorModules/GenModule.h"

#include "Hydjet_i/HyiPar.h"
#include "Hydjet_i/HyFlow.h"
#include "Hydjet_i/HyfPar.h"
#include "Hydjet_i/HyJets.h"
#include "Hydjet_i/HyjPar.h"
#include "Hydjet_i/LuJets.h"
#include "Hydjet_i/LuDatr.h"
#include "Hydjet_i/PyDat1.h"
#include "Hydjet_i/PyPars.h"
#include "Hydjet_i/PySubs.h"
#include "Hydjet_i/PyqPar.h"

typedef std::vector<std::string> CommandVector;

// new to store hydjet event parameters
#include "GeneratorObjects/HijingEventParams.h"

class Hydjet:public GenModule {
public:
  Hydjet(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~Hydjet() = default;

  virtual StatusCode genInitialize();
  virtual StatusCode callGenerator();
  virtual StatusCode genFinalize();
  virtual StatusCode fillEvt(HepMC::GenEvent* evt);

protected:

  // inputs to HYINIT (Hydjet event generation)
  double   m_e{0.};
  double   m_a{0.};
  int          m_ifb{0};
  double   m_bmin{0.};
  double   m_bmax{0.};
  double   m_bfix{0.};
  int          m_nh{0};

  // Random numbers seed
  std::vector<long int> m_seeds;

  // event counter
  int m_events{0};

  // Commands to setup hydjet
  CommandVector m_InitializeVector;

  // Accessor to HYDJET Options and parameters COMMONs
  HyiPar m_hyipar;
  HyjPar m_hyjpar;
  HyfPar m_hyfpar;
  HyFlow m_hyflow;

  // Accessor to PYTHIA parameters COMMON
  PyPars m_pypars;
  PySubs m_pysubs;
  PyDat1 m_pydat1;

  // Accessor to PYQUEN parameters COMMON
  PyqPar m_pyqpar;

  // Accessor to JETSET Random number seed COMMON
  LuDatr m_ludatr;

  // Accessors to Event information COMMONS
  LuJets m_lujets;
  HyJets m_hyjets;

  void        set_user_params(void);

  SG::WriteHandleKey<HijingEventParams> m_event_paramsKey{this, "HijingOutputKey", "Hijing_event_params"};
  //--------------------------------
};

#endif
