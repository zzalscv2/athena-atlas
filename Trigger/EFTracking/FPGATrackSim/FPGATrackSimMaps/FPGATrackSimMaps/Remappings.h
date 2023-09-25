#ifndef FPGATrackSimMaps_Remappings_h
#define FPGATrackSimMaps_Remappings_h
#include <vector>
namespace Remappings {
    /**
    * @brief for a given geo tag return disk indices remappings
    * @warning throws an exception if the geo key is not known
    */
    std::vector<uint32_t> diskIndices(const std::string& geoKey);
}

#endif