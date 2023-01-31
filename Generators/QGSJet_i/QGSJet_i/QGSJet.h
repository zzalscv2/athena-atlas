/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GENERATORMODULESQGSJET_H
#define GENERATORMODULESQGSJET_H

#include "GeneratorModules/GenModule.h"
#include "AtlasHepMC/HEPEVT_Wrapper.h"

/**
@class Epos
@brief This code is used to get an Epos Monte Carlo event.

  genInitialize() is used to read parameters
  callGenerator() makes the event
  genFinalize() writes log files etc
  fillEvt(GeneratorEvent* evt) passes the event to HepMC

  The output will be stored in the transient event store so it can be
  passed to the simulation.

@author Sami Kama
  Adapted from Ian Hinchliffe's code
*/

class QGSJet: public GenModule {
public:
  QGSJet(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~QGSJet() = default;

  virtual StatusCode genInitialize();
  virtual StatusCode callGenerator();
  virtual StatusCode genFinalize();
  virtual StatusCode fillEvt(HepMC::GenEvent* evt);

protected:
  // event counter
  int m_events{0}; // current event number (counted by interface)
  int m_ievent{0}; // current event number counted by QGSJet
  int m_iout{0}; //output type

  // setable properties
  DoubleProperty m_beamMomentum{this, "BeamMomentum", -3500.0};
  DoubleProperty m_targetMomentum{this, "TargetMomentum", 3500.0};
  IntegerProperty m_model{this, "Model", 7}; // 0=EPOS 1.99 LHC, 1=EPOS 1.99, 7=QGSJETII04
  IntegerProperty m_primaryParticle{this, "PrimaryParticle", 1}; // 1=p, 12=C, 120=pi+, 207=Pb
  IntegerProperty m_targetParticle{this, "TargetParticle", 1};
  StringProperty   m_paramFile{this, "ParamFile", "crmc.param"};
  StringProperty   m_lheout{this, "LheFile", "qgsjet.lhe"};
  IntegerProperty m_itab{this, "TabCreate", 0};
  IntegerProperty m_ilheout{this, "LheOutput", 0};
  IntegerProperty m_nEvents{this, "nEvents", 5500};

  //Gen_tf run args.
  IntegerProperty m_dsid{this, "Dsid", 999999};

  static const size_t kMaxParticles = HEPEVT_EntriesAllocation;
  std::vector<int>    m_partID;
  std::vector<double> m_partPx;
  std::vector<double> m_partPy;
  std::vector<double> m_partPz;
  std::vector<double> m_partEnergy;
  std::vector<double> m_partMass;
  std::vector<int>    m_partStat;

  std::vector<long int> m_seeds;
};

#endif
