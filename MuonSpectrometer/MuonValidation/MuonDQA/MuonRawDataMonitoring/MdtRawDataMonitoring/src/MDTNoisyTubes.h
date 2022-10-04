/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////////////
// Tool to mask Noisy Tubes
// Nov. 2009
// Author Justin Griffiths <griffith@cern.ch>
///////////////////////////////////////////////////////////////////////////

#ifndef MDTNoisyTubes_H
#define MDTNoisyTubes_H

#include <map>
#include <set>
#include <string>

#include "MuonPrepRawData/MuonPrepDataContainer.h"

class MDTNoisyTubes {
public:
    MDTNoisyTubes(bool doMask = true);
    ~MDTNoisyTubes();

    bool isNoisy(const Muon::MdtPrepData*) const;
    std::set<Identifier> getNoiseList(IdentifierHash);

private:
    std::map<IdentifierHash, std::set<Identifier> > m_noise_map;
};

#endif
