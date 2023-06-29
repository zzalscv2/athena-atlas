/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PrimaryParticleInformation_H
#define PrimaryParticleInformation_H

#include "G4VUserPrimaryParticleInformation.hh"
#include "AtlasHepMC/GenEvent.h"
#include "AtlasHepMC/GenParticle.h"
#include "TruthUtils/MagicNumbers.h"
#include "CxxUtils/checker_macros.h"

namespace ISF {
  class ISFParticle;
}

class PrimaryParticleInformation: public G4VUserPrimaryParticleInformation {
public:
  PrimaryParticleInformation();
  PrimaryParticleInformation(HepMC::GenParticlePtr, ISF::ISFParticle* isp=0);
  HepMC::ConstGenParticlePtr GetHepMCParticle() const { return m_theParticle; }
  HepMC::GenParticlePtr GetHepMCParticle() { return m_theParticle; }
  int GetParticleBarcode() const;
  void SuggestBarcode(int bc);
  void SetParticle(HepMC::GenParticlePtr);
  void Print() const {}
  int GetRegenerationNr() {return  m_regenerationNr;}
  void SetRegenerationNr(int i) {m_regenerationNr=i;}

  void SetISFParticle(ISF::ISFParticle* isp);
  const ISF::ISFParticle* GetISFParticle() const { return m_theISFParticle; }
  ISF::ISFParticle* GetISFParticle() { return m_theISFParticle; }

private:
  HepMC::GenParticlePtr m_theParticle{};
  ISF::ISFParticle* m_theISFParticle{};

  int m_regenerationNr{0};
  mutable int m_barcode ATLAS_THREAD_SAFE = HepMC::INVALID_PARTICLE_BARCODE;
};

#endif
