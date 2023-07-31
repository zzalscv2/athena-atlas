/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "PixelConditionsData/PixelChargeCalibUtils.h"
#include "InDetIdentifier/PixelID.h"
#include "Identifier/Identifier.h"
#include "PixelReadoutGeometry/PixelModuleDesign.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"


namespace PixelChargeCalib{
  std::pair<int, int>
  getBecAndLayer(const PixelID * pPixelId, IdentifierHash hash){
    const Identifier waferId = pPixelId->wafer_id(hash);
    return  {pPixelId->barrel_ec(waferId), pPixelId->layer_disk(waferId)};
  }
  
  std::pair<size_t, InDetDD::PixelReadoutTechnology> 
  numChipsAndTechnology(const InDetDD::SiDetectorElement *element){
    static constexpr int halfModuleThreshold{8};
    const InDetDD::PixelModuleDesign *pDesign = static_cast<const InDetDD::PixelModuleDesign*>(&element->design());
    const unsigned int n = (pDesign->numberOfCircuits() < halfModuleThreshold) ? pDesign->numberOfCircuits() : 2 * pDesign->numberOfCircuits();
    return {n, pDesign->getReadoutTechnology()};
  }
    
}