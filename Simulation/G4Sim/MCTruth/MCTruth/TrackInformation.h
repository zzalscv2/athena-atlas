/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TrackInformation_H
#define TrackInformation_H

#include "VTrackInformation.h"
#include "TruthUtils/MagicNumbers.h"
#include "CxxUtils/checker_macros.h"

namespace ISF {
  class ISFParticle;
}

class TrackInformation: public VTrackInformation {
public:
  TrackInformation();
  TrackInformation(HepMC::GenParticlePtr p, ISF::ISFParticle* baseIsp=nullptr);
  virtual HepMC::ConstGenParticlePtr GetHepMCParticle() const override {return m_theParticle;}
  virtual HepMC::GenParticlePtr GetHepMCParticle() override {return m_theParticle;}
  virtual const ISF::ISFParticle *GetBaseISFParticle() const override {return m_theBaseISFParticle;}
  virtual ISF::ISFParticle *GetBaseISFParticle() override {return m_theBaseISFParticle;}
  virtual int GetParticleBarcode() const override; // TODO Drop this once UniqueID and Status are used instead
  virtual int GetParticleUniqueID() const override;
  virtual int GetParticleStatus() const override;
  virtual void SetParticle(HepMC::GenParticlePtr) override;
  virtual void SetBaseISFParticle(ISF::ISFParticle*) override;
  virtual void SetReturnedToISF(bool returned) override {m_returnedToISF=returned;}
  virtual bool GetReturnedToISF() const override {return m_returnedToISF;}
  void SetRegenerationNr(int i) {m_regenerationNr=i;}
  int GetRegenerationNr() const {return m_regenerationNr;}
private:
  int m_regenerationNr{0};
  HepMC::GenParticlePtr m_theParticle{};
  mutable int m_barcode ATLAS_THREAD_SAFE = HepMC::INVALID_PARTICLE_BARCODE; // TODO Drop this once UniqueID and Status are used instead
  mutable int m_uniqueID ATLAS_THREAD_SAFE = HepMC::INVALID_PARTICLE_BARCODE;
  ISF::ISFParticle *m_theBaseISFParticle{};
  bool m_returnedToISF{false};
};

#endif
