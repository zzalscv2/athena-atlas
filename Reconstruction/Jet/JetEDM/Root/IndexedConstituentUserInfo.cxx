/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// IndexedConstituentUserInfo.cxx

#include "JetEDM/IndexedConstituentUserInfo.h"
#include "JetEDM/LabelIndex.h"

using jet::IndexedConstituentUserInfo;
using Label = IndexedConstituentUserInfo::Label;
using Index = IndexedConstituentUserInfo::Index;
using jet::LabelIndex;
using xAOD::IParticle;

//**********************************************************************

IndexedConstituentUserInfo::IndexedConstituentUserInfo()
: m_ppar(nullptr) { }

//**********************************************************************

IndexedConstituentUserInfo::
IndexedConstituentUserInfo(const IParticle& par, Index idx, const LabelIndex* pli)
: BaseIndexedConstituentUserInfo(idx, pli), m_ppar(&par) { }


//**********************************************************************

const xAOD::IParticle* IndexedConstituentUserInfo::particle() const {
  return m_ppar;
}

//**********************************************************************
