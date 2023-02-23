/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "MCTruth/PrimaryParticleInformation.h"

PrimaryParticleInformation::PrimaryParticleInformation()
{
}

PrimaryParticleInformation::PrimaryParticleInformation(HepMC::GenParticlePtr p, ISF::ISFParticle* isp):m_theParticle(p),m_theISFParticle(isp)
{
}

void PrimaryParticleInformation::SuggestBarcode(int bc)
{
  m_barcode=bc;
  if (m_theParticle) {
    std::cout<<"ERROR: PrimaryParticleInformation::SuggestBarcode() should be only called if no HepMC::Particle is available"<<std::endl;
  }
}

int PrimaryParticleInformation::GetParticleBarcode() const
{
  if (m_barcode !=  HepMC::INVALID_PARTICLE_BARCODE) return m_barcode;
  if (m_theParticle) {
      m_barcode = HepMC::barcode(m_theParticle);
      return m_barcode;
  }
  return 0;
}

void PrimaryParticleInformation::SetParticle(HepMC::GenParticlePtr p)
{
  m_theParticle=p;
  m_barcode = HepMC::INVALID_PARTICLE_BARCODE;
}

void PrimaryParticleInformation::SetISFParticle(ISF::ISFParticle* p)
{
  m_theISFParticle=p;
}
