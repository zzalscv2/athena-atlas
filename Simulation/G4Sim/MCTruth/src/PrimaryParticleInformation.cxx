/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MCTruth/PrimaryParticleInformation.h"

PrimaryParticleInformation::PrimaryParticleInformation()
{
}

PrimaryParticleInformation::PrimaryParticleInformation(HepMC::GenParticlePtr p, ISF::ISFParticle* isp):m_theParticle(p),m_theISFParticle(isp)
{
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

int PrimaryParticleInformation::GetParticleUniqueID() const
{
  if (m_uniqueID !=  HepMC::INVALID_PARTICLE_BARCODE) return m_uniqueID;
  if (m_theParticle) {
    HepMC::ConstGenParticlePtr particle = m_theParticle;
    m_uniqueID = HepMC::uniqueID(particle);
    return m_uniqueID;
  }
  return 0;
}

void PrimaryParticleInformation::SetParticle(HepMC::GenParticlePtr p)
{
  m_theParticle=p;
  m_barcode = HepMC::INVALID_PARTICLE_BARCODE;
  m_uniqueID = HepMC::INVALID_PARTICLE_BARCODE;
}

void PrimaryParticleInformation::SetISFParticle(ISF::ISFParticle* p)
{
  m_theISFParticle=p;
}
