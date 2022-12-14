/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file GeneratorFilters/TTbarPlusHeavyFlavorFilter.h
 * @author Georges Aad
 * @date Sep. 2014
 * @brief filter to select ttbar+jets/HF events
*/


#ifndef GeneratorFilters_TTbarPlusHeavyFlavorFilter_H
#define GeneratorFilters_TTbarPlusHeavyFlavorFilter_H

#include "GeneratorModules/GenFilter.h"

#include "AtlasHepMC/GenParticle_fwd.h"

class TTbarPlusHeavyFlavorFilter: public GenFilter {
public:
  TTbarPlusHeavyFlavorFilter(const std::string& fname, ISvcLocator* pSvcLocator);
  virtual ~TTbarPlusHeavyFlavorFilter();
  virtual StatusCode filterInitialize();
  virtual StatusCode filterFinalize();
  virtual StatusCode filterEvent();

private:


  /// properties
  bool m_useFinalStateHadrons;
  bool m_selectB;
  bool m_selectC;
  bool m_selectL;


  double m_bPtMinCut;
  double m_bEtaMaxCut;
  double m_cPtMinCut;
  double m_cEtaMaxCut;

  int m_bMultiCut;
  int m_cMultiCut;

  bool m_excludeBFromTop;
  bool m_excludeCFromTop;


  bool passBSelection(HepMC::ConstGenParticlePtr part) const;
  bool passCSelection(HepMC::ConstGenParticlePtr part) const;

  int hadronType(int pdgid) const;
  bool isBHadron(HepMC::ConstGenParticlePtr part) const;
  bool isCHadron(HepMC::ConstGenParticlePtr part) const;

  bool isInitialHadron(HepMC::ConstGenParticlePtr part) const;
  bool isFinalHadron(HepMC::ConstGenParticlePtr part) const;

  bool isQuarkFromHadron(HepMC::ConstGenParticlePtr part) const;
  bool isCHadronFromB(HepMC::ConstGenParticlePtr part) const;

  HepMC::ConstGenParticlePtr  findInitial(HepMC::ConstGenParticlePtr part) const;

  bool isFromTop(HepMC::ConstGenParticlePtr part) const;
  bool isDirectlyFromTop(HepMC::ConstGenParticlePtr part) const;
  bool isDirectlyFromWTop(HepMC::ConstGenParticlePtr part) const;



  

  

};

#endif /// GeneratorFilters_TTbarPlusHeavyFlavorFilter_H
