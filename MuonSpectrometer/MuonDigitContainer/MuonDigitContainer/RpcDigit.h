/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// RpcDigit.h

#ifndef RpcDigitUH
#define RpcDigitUH

// RPC digitization. Holds a channel ID.

#include <iosfwd>
#include "MuonDigitContainer/MuonDigit.h"
#include "MuonIdHelpers/RpcIdHelper.h"

class RpcDigit : public MuonDigit {

private:  // data

  // Time.
  float m_time{0.f};
  float m_ToT{-1.f};

public:  // functions

  // Default constructor.
  RpcDigit()=default;

  // Full constructor from Identifier.
  RpcDigit(const Identifier& id, float time, float ToT=-1.);

  // Return the Time.
  float time() const { return m_time; }

  // Return the Time over Threshold.
  float ToT() const { return m_ToT; }

};

#endif
