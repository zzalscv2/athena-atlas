/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// RpcDigit.cxx

#include "MuonDigitContainer/RpcDigit.h"

//**********************************************************************
// Member functions.
//**********************************************************************
 
//**********************************************************************

// Full constructor from Identifier.

RpcDigit::RpcDigit(const Identifier& id, float time, float ToT)
: MuonDigit(id), m_time(time), m_ToT(ToT) { }

//**********************************************************************



