/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MCTruth/TrackBarcodeInfo.h"

TrackBarcodeInfo::TrackBarcodeInfo(int bc, ISF::ISFParticle* baseIsp)
  : VTrackInformation(BarcodeOnly)
  , m_theBaseISFParticle(baseIsp)
  , m_barcode(bc)
  , m_uniqueID(bc) // TODO Assign this separately
  , m_returnedToISF(false)
{
}

void TrackBarcodeInfo::SetBaseISFParticle(ISF::ISFParticle* isp)
{
  m_theBaseISFParticle=isp;
}

void TrackBarcodeInfo::SetReturnedToISF(bool returned)
{
  m_returnedToISF = returned;
}
