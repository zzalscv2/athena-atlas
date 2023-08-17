/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include <MuonAlignmentDataR4/sTgcAlignmentStore.h>
#include <exception>
#include <sstream>

void sTgcAlignmentStore::cacheBLine(const Identifier& detElId, const BLinePar& bline) {
    m_blines[detElId] = &bline;
}
