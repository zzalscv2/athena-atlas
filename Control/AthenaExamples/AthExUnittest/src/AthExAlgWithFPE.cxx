//  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

#include "AthExAlgWithFPE.h"

StatusCode AthExAlgWithFPE::execute() {

  float value=42;
  float zero=0;
  float byZero=value/zero;
  ATH_MSG_INFO("Division of " << value << " by zero is " << byZero);

  return StatusCode::SUCCESS;
}
