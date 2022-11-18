/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef VTrackInformation_H
#define VTrackInformation_H

#include "G4VUserTrackInformation.hh"

enum TrackClassification { Primary, RegeneratedPrimary, RegisteredSecondary, Secondary, BarcodeOnly } ;

#include "AtlasHepMC/GenParticle_fwd.h"

namespace ISF {
  class ISFParticle;
}

class VTrackInformation: public G4VUserTrackInformation {
public:
  VTrackInformation(TrackClassification tc=Primary);
  HepMC::ConstGenParticlePtr GetPrimaryHepMCParticle() const {return m_thePrimaryParticle;}
  HepMC::GenParticlePtr GetPrimaryHepMCParticle() {return m_thePrimaryParticle;}
  void  SetPrimaryHepMCParticle(HepMC::GenParticlePtr);
  virtual HepMC::ConstGenParticlePtr GetHepMCParticle() const {return nullptr;}
  virtual HepMC::GenParticlePtr GetHepMCParticle() {return nullptr;}
  virtual const ISF::ISFParticle *GetBaseISFParticle() const {return nullptr;}
  virtual ISF::ISFParticle *GetBaseISFParticle() {return nullptr;}
  virtual bool GetReturnedToISF() const;
  virtual int  GetParticleBarcode() const =0;
  virtual void SetParticle(HepMC::GenParticlePtr);
  virtual void SetBaseISFParticle(ISF::ISFParticle*);
  virtual void SetReturnedToISF(bool) ;
  virtual void Print() const {}
  void SetClassification(TrackClassification tc) {m_classify=tc;}
  TrackClassification GetClassification() const {return m_classify;}
private:
  TrackClassification m_classify;
  HepMC::GenParticlePtr m_thePrimaryParticle{};
};

#endif
