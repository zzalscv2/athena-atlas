/*
   Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
 */

#ifndef __ELRECOMFILEHELPERS__
#define __ELRECOMFILEHELPERS__
#include <string>

namespace ElRecomFileHelpers {

// Convert reco, ID, iso and trigger key values into a
// single key according to the map file key format
std::string
convertToOneKey(const std::string& recokey,
                const std::string& idkey,
                const std::string& isokey,
                const std::string& trigkey);

// Retrieves the value from the provided map file as
// associated with the provided key
std::string
getValueByKey(const std::string& mapFile, const std::string& key);

}

#endif
