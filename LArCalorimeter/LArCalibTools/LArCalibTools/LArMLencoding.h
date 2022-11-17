/*
 Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARMLENCODING_H //include guard
#define LARMLENCODING_H //include guard

class LArMLencoding
{
 public:
  //input Et should be MeV/12.5 unit.
  static int get_MultiLinearCode_eFEX(int Et, bool saturated = false, bool invalid = false, bool empty = false);
  static int get_MultiLinearCode_jFEX(int Et, bool invalid = false, bool empty = false);
  static int get_MultiLinearCode_gFEX(int Et, bool invalid = false, bool empty = false);


};

#endif //include guard
