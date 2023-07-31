/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PixelChargeCalibUtils_h
#define PixelChargeCalibUtils_h

#include <utility> //std::pair
#include "PixelReadoutDefinitions/PixelReadoutDefinitions.h" //enum PixelReadoutTechnology

class IdentifierHash;
class PixelID;
namespace InDetDD{
  class SiDetectorElement;
}

namespace PixelChargeCalib{
  std::pair<int, int>
  getBecAndLayer(const PixelID * pPixelId, IdentifierHash hash);
  //
  std::pair<size_t, InDetDD::PixelReadoutTechnology>
  numChipsAndTechnology(const InDetDD::SiDetectorElement *element);
}

#endif
