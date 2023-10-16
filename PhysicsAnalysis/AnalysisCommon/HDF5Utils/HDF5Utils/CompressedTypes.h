// this is -*- C++ -*-
/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef COMPRESSED_TYPES_H
#define COMPRESSED_TYPES_H

#include "CompressionEnums.h"
#include "H5Traits.h"

#include "H5Cpp.h"

#include <stdexcept>

namespace H5Utils {

  namespace internal {

    H5::DataType halfPrecisionFloat(int ebias = 15);

    template <typename T>
    H5::DataType getCompressedType(Compression comp) {
      if constexpr (std::is_floating_point<T>::value) {
        switch (comp) {
          case Compression::STANDARD: return H5Traits<T>::type;
          case Compression::HALF_PRECISION: return halfPrecisionFloat();
          case Compression::HALF_PRECISION_LARGE: return halfPrecisionFloat(5);
          default: throw std::logic_error("unknown float compression");
        }
      }
      if (comp != Compression::STANDARD) {
        throw std::logic_error("compression not supported for this type");
      }
      return H5Traits<T>::type;
    }

  } // end internal
}   // end H5Utils

#endif
