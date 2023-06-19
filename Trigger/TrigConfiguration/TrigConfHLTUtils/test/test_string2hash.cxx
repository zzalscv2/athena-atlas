/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

  Unit test for string2hash.
*/

#undef NDEBUG

#include "TrigConfHLTUtils/HLTUtils.h"
#include <cassert>

using namespace TrigConf;


int main() {
  // Basic checks
  assert( HLTUtils::string2hash("L1_EM22VH")==3462952785 );
  assert( HLTUtils::hash2string(3462952785)=="L1_EM22VH" );

  assert( HLTUtils::string2hash("L1_EM22VH","MyCat")==3462952785 );
  assert( HLTUtils::hash2string(3462952785,"MyCat")=="L1_EM22VH" );

  assert( HLTUtils::hash2string(42)=="UNKNOWN HASH ID" );
  assert( HLTUtils::hash2string(42,"NOCAT")=="UNKNOWN CATEGORY" );

  // Check exception in case of hash collision
  const std::string same_hash[] = {"FnhW7k5kKGWHoKXkOoem",
                                   "TovIi9l0PoC9dKZqN1ZU"};

  assert( HLTUtils::string2hash(same_hash[0])==1343856449 );
  bool thrown = false;
  try {
    HLTUtils::string2hash(same_hash[1]);
  }
  catch (std::domain_error& exc) {
    thrown = true;
  }
  assert(thrown);

  // But same hash in different category is allowed
  HLTUtils::string2hash(same_hash[1], "MyCat");

  // Test writing to file
  HLTUtils::hashes2file();

  // And reading back
  HLTUtils::file2hashes();

  return 0;
}
