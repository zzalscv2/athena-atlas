/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef AMDCSTAND_GETAMDCSIMRECVARIABLE_H
#define AMDCSTAND_GETAMDCSIMRECVARIABLE_H

#include <string>

double       GetAmdcRealVar(const std::string & VarName, int I1, int I2, int I3);
int          GetAmdcIntVar (const std::string & VarName, int I1, int I2, int I3);
std::string  GetAmdcCharVar(const std::string & VarName, int I1, int I2, int I3);

#endif
