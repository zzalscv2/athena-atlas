/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "MCTruth/TrackInformation.h"
#include "AtlasHepMC/GenEvent.h"
#include "AtlasHepMC/GenParticle.h"
#include "AtlasHepMC/GenVertex.h"

TrackInformation::TrackInformation():m_regenerationNr(0),m_theParticle(0),m_theBaseISFParticle(0),m_returnedToISF(false)
{
}

TrackInformation::TrackInformation(HepMC::GenParticlePtr p, ISF::ISFParticle* baseIsp):
    m_regenerationNr(0),
    m_theParticle(p),
    m_theBaseISFParticle(baseIsp),
    m_returnedToISF(false)
{
}

int TrackInformation::GetParticleBarcode() const
{
  return ( m_theParticle ? HepMC::barcode(m_theParticle) : 0 );
}

void TrackInformation::SetParticle(HepMC::GenParticlePtr p)
{
  m_theParticle=p;
}

void TrackInformation::SetBaseISFParticle(ISF::ISFParticle* p)
{
  m_theBaseISFParticle=p;
}
