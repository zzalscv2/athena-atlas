/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "DerivationFrameworkInDet/MinBiasPRDAssociation.h"

MinBiasPRDAssociation::MinBiasPRDAssociation(){
  reset();
}



void MinBiasPRDAssociation::reset(){

  nPixelUA = 0;
  nBlayerUA = 0;
  nPixelBarrelUA = 0;
  nPixelEndCapAUA = 0;
  nPixelEndCapCUA = 0;

  nSCTUA = 0;
  nSCTBarrelUA = 0;
  nSCTEndCapAUA = 0;
  nSCTEndCapCUA = 0;

  nTRTUA = 0;
  nTRTBarrelUA = 0;
  nTRTEndCapAUA = 0;
  nTRTEndCapCUA = 0;
}
