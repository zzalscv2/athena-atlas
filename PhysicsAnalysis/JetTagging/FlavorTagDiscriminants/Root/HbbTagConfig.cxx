#include "FlavorTagDiscriminants/HbbTagConfig.h"
#include <filesystem>

namespace FlavorTagDiscriminants {
  HbbTagConfig::HbbTagConfig(const std::filesystem::path& path):
    input_file_path(path)
  {
    namespace fs = std::filesystem;
    fs::path parent_dir_name = path.parent_path().stem();
    if (parent_dir_name.empty()) throw std::runtime_error(
      "can't figure out subjet link name from " + path.string());
    subjet_link_name = parent_dir_name.string();
  }
}
