#ifndef FPGATrackSimMaps_Remappings_h
#define FPGATrackSimMaps_Remappings_h
#include <vector>
#include <string>
#include <cstdint>
namespace Remappings {
    /**
    * @brief for a given geo tag return disk indices remappings
    * the format is this: 'GeometryVersion':'ATLAS-P2-RUN4-03-00-00' - as present in the config files
    * @warning throws an exception if the geo key is not known
    */
    std::vector<uint32_t> diskIndices(const std::string& geoKey);

}

#endif
