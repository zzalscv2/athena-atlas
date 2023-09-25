#include <string>
#include <stdexcept>

#include "FPGATrackSimMaps/Remappings.h"


std::vector<uint32_t> Remappings::diskIndices(const std::string& geoKey) {
    const std::string recentKey("ATLAS-P2-RUN4");
    const std::vector<uint32_t> recentMapping({0, 15, 21, 44, 50, 61, 69, 77, 86});
    if ( geoKey.compare(0, recentKey.size(), recentKey) == 0 ) {
        return recentMapping;        
    } else if ( geoKey == "ATLAS-P2-ITK-22-02-00" ) {
        return {0,17,47,58,66};
    } else if ( geoKey == "ATLAS-P2-ITK-23-00-01" ) {
        return {0,15,44,50,61,69,77,86};
    }
    throw std::invalid_argument(std::string("GeoKey ") + geoKey +" not know to remapping");

}
