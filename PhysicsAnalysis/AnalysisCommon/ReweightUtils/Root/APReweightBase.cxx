/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#define APReweightBase_cxx
#include "ReweightUtils/APReweightBase.h"

std::atomic<unsigned int> APReweightBase::s_NID = 0;

APReweightBase::APReweightBase()
  : m_scale(0),
    m_isTrig(0),
    m_isQuiet(0),
    m_syst_uncert_global(0),
    m_empty_weight(0)
{
  m_ID = s_NID;
  ++s_NID;
}

APReweightBase::~APReweightBase() { }

unsigned int APReweightBase::GetID() const {
  return m_ID;
}
