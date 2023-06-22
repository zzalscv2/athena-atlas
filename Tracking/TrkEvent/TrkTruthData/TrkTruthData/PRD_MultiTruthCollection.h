/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// Dear emacs, this is -*-c++-*-
#ifndef PRD_MULTITRUTHCOLLECTION_H
#define PRD_MULTITRUTHCOLLECTION_H

#include <map>

#include "Identifier/Identifier.h"
#include "GeneratorObjects/HepMcParticleLink.h"

#include "AthenaKernel/CLASS_DEF.h"

#include "AthAllocators/ArenaPoolSTLAllocator.h"

/**
 * A PRD is mapped onto all contributing particles.
 */
class PRD_MultiTruthCollection
    : public std::multimap<Identifier, HepMcParticleLink, std::less<Identifier>,
                           SG::ArenaPoolSTLAllocator<std::pair<
                               const Identifier, HepMcParticleLink>>> {};

CLASS_DEF(PRD_MultiTruthCollection, 1162521747, 1)

#endif/*PRD_MULTITRUTHCOLLECTION_H*/
