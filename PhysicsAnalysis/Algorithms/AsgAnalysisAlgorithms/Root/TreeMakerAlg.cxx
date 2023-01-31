// Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration

// Local include(s):
#include "AsgAnalysisAlgorithms/TreeMakerAlg.h"

// ROOT include(s):
#include <TTree.h>

namespace CP {

   StatusCode TreeMakerAlg::initialize() {
       ANA_CHECK(book(TTree(m_treeName.value().c_str(), "xAOD->NTuple tree")));
       TTree *treePtr = tree(m_treeName);
       if (!treePtr)
       {
           ANA_MSG_ERROR("Failed to create output tree \"" << m_treeName.value() << "\"");
           return StatusCode::FAILURE;
       }
       treePtr->SetAutoFlush(m_treeAutoFlush);
       return StatusCode::SUCCESS;
   }

} // namespace CP
