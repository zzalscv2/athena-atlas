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
  return m_theParticle?HepMC::barcode(m_theParticle):m_barcode;
}

void PrimaryParticleInformation::SetParticle(HepMC::GenParticlePtr p)
{
  m_theParticle=p;
}

void PrimaryParticleInformation::SetISFParticle(ISF::ISFParticle* p)
{
  m_theISFParticle=p;
}
