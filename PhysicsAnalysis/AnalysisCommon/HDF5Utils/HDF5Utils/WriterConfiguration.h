/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef HDF5UTILS_WRITER_CONFIGURATION_H
#define HDF5UTILS_WRITER_CONFIGURATION_H

#include <optional>
#include <array>
#include <string>

#include "H5public.h"

namespace H5Utils {
  template <size_t N>
  struct WriterConfiguration
  {
    std::string name;
    std::array<hsize_t,N> extent;
    std::optional<hsize_t> batch_size{std::nullopt};
    std::optional<std::array<hsize_t,N>> chunks{std::nullopt};
    std::optional<int> deflate{std::nullopt};
  };
}
#endif
