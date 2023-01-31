/*
 Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "LArCalibTools/LArMLencoding.h"
#include <cmath>

//input Et should be MeV/12.5 unit.
//function to get ML code for eFEX
int LArMLencoding::get_MultiLinearCode_eFEX(int Et, bool saturated, bool invalid, bool empty){
  // for special codes
  if (empty) return 0;
  else if (invalid) return 1022;
  else if (saturated) return 1023;

  // for specific energies
  else if (Et < -60) return 1;
  else if (Et < 448) return (Et+60)/2 + 2;
  else if (Et < 1472) return (Et/4) + 144;
  else if (Et < 3520) return (Et/8) + 328;
  else if (Et < 11584) return (Et/32) + 658;
  else return 1020;
}

//function to get ML code for jFEX
int LArMLencoding::get_MultiLinearCode_jFEX(int Et,  bool invalid, bool empty){
  // for special code
  if (empty) return 0;
  else if (invalid) return 4095;

  // for specific energies
  else if (Et < -252) return 1;
  else if (Et < 512) return (Et+512)/2 + 2;
  else if (Et < 64000)
  {
    const int pow = log(Et/512)/log(4);
    return Et/(std::pow(2,pow+2))+256*std::pow(2,pow);
  }
  else return 4048;
}

//function to get ML code for gFEX
int LArMLencoding::get_MultiLinearCode_gFEX(int Et, bool invalid, bool empty){
  //for special codes
  if (empty) return 0;
  else if (invalid) return 4095;

  //for specific codes
  if (Et < -8096) return 1;
  else if (Et < -4000) return (Et+8096)/1024 + 2;
  else if (Et < -1024) return (Et+4000)/4 + 6;
  else if (Et < 1024) return (Et+1024)/2 + 750;
  else if (Et < 4096) return (Et-1024)/4 + 1774;
  else if (Et < 16000) return (Et-4096)/8 + 2542;
  else if (Et < 81536) return (Et-16000)/2048 + 4030;
  else return 4062;
}
