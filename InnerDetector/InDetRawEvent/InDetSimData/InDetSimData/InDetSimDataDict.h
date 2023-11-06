/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETSIMEVENT_INDETSIMDATADICT_H
#define INDETSIMEVENT_INDETSIMDATADICT_H

#include "InDetSimData/InDetSimDataCollection.h"


// Helpers for use from python.
namespace InDetSimDataHelpers {

  
std::vector<Identifier> identifiers (const InDetSimDataCollection& coll)
{
  std::vector<Identifier> v;
  v.reserve (coll.size());
  for (const auto& p : coll)
    v.push_back (p.first);
  return v;
}

const InDetSimData* getData (const InDetSimDataCollection& coll,
                             const Identifier& id)
{
  auto it = coll.find (id);
  if (it != coll.end()) {
    return &it->second;
  }
  return nullptr;
}


} // namespace InDetSimDataHelpers

 
namespace AthenaPoolTestDataDict 
{
    std::pair< HepMcParticleLink , float> d_pair;
}

 
#endif
