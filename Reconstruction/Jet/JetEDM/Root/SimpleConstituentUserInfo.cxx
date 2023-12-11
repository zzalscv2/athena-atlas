/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// SimpleConstituentUserInfo.cxx

#include <utility>

#include "JetEDM/SimpleConstituentUserInfo.h"

using jet::SimpleConstituentUserInfo;
using Label = SimpleConstituentUserInfo::Label;
using xAOD::IParticle;

//**********************************************************************

SimpleConstituentUserInfo::SimpleConstituentUserInfo()
: m_ppar(nullptr) { }

//**********************************************************************

SimpleConstituentUserInfo::
SimpleConstituentUserInfo(const IParticle& par, Label lab)
: m_ppar(&par), m_label(std::move(lab)) { }

//**********************************************************************

const xAOD::IParticle* SimpleConstituentUserInfo::particle() const {
  return m_ppar;
}

//**********************************************************************

Label SimpleConstituentUserInfo::label() const {
  return m_label;
}

//**********************************************************************
