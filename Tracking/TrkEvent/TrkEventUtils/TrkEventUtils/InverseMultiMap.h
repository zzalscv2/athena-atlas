/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file This file defines the Trk::InverseMultiMap template class and a function to fill it.
 *
 * @author Andrei Gaponenko <agaponenko@lbl.gov>, 2006
 */

#ifndef INVERSEMULTIMAP_H
#define INVERSEMULTIMAP_H

#include <map>
#include "AthAllocators/ArenaPoolSTLAllocator.h"

namespace Trk {
/**
 * @class InverseMultiMap
 *
 * An InverseMultiMap object built from a generic map or multimap
 * allows for quick lookup of original (multi)map's key by
 * original mapped value.
 *
 * Example of use:
 * @code
 *     #include "TrkEventUtils/InverseMultiMap.h"
 *
 *     ...
 *
 *     PRD_MultiTruthCollection *origMap = ...; // retrieve from the SG
 *
 *     Trk::InverseMultiMap<PRD_MultiTruthCollection> invMap;
 *
 *     Trk::addToInverseMultiMap(&invMap, *origMap);
 * @endcode
 *
 */

template <class OrigMap, class CmpT = std::less<typename OrigMap::mapped_type>>
class InverseMultiMap
    : public std::multimap<
          typename OrigMap::mapped_type, typename OrigMap::key_type, CmpT,
          SG::ArenaPoolSTLAllocator<
              std::pair<const typename OrigMap::mapped_type, typename OrigMap::key_type>>> {};

//----------------------------------------------------------------
// And this is the code to fill an inverse map with data
template <class OrigMap, class CmpT>
void addToInverseMultiMap(InverseMultiMap<OrigMap, CmpT>* result,
                          const OrigMap& rec2truth) {
  for (const auto& p : rec2truth) {
    result->insert(std::make_pair(p.second, p.first));
  }
}
}  // namespace Trk

#endif /*INVERSEMULTIMAP_H*/
