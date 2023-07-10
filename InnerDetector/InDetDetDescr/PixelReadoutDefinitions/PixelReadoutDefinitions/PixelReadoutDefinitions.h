/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PIXELREADOUTDEFINITIONS_H
#define PIXELREADOUTDEFINITIONS_H
#include <cstddef> //for size_t

namespace InDetDD{

  enum class PixelModuleType{
    DBM,
    IBL_PLANAR,
    IBL_3D,
    PIX_BARREL,
    PIX_ENDCAP,
    NONE
  };

  enum class PixelDiodeType{
    NORMAL,
    LONG,
    GANGED,
    LARGE,
    N_DIODETYPES
  };

  enum class PixelReadoutTechnology{
    FEI3,
    FEI4,
    RD53,
    N_TECHNOLOGIES
  };
  
  ///Convert an enum class to size_t for use as an array index
  template <typename T>
  constexpr std::size_t
  enum2uint(T n){
    return static_cast<size_t>(n);
  }

}

#endif
