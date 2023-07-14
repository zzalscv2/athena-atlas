/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#ifndef HBB_TAG_CONFIG_H
#define HBB_TAG_CONFIG_H

#include <filesystem>

namespace FlavorTagDiscriminants {

  struct HbbTagConfig
  {
    HbbTagConfig(const std::filesystem::path& path);
    HbbTagConfig() = default;
    std::filesystem::path input_file_path;
    std::string subjet_link_name;
    double min_subjet_pt = 7e3;
  };
}

#endif
