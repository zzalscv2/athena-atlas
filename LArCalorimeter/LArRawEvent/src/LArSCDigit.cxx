/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "LArRawEvent/LArSCDigit.h"

/** default constructor for persistency */
LArSCDigit::LArSCDigit()
  : LArDigit(),
    m_chan(0),m_sourceId(0)
{}
