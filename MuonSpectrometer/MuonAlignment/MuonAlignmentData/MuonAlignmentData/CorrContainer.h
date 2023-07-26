/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONALIGNMENTDATA_CORRCONTAINER_H
#define MUONALIGNMENTDATA_CORRCONTAINER_H

#include "AthenaKernel/CLASS_DEF.h"
#include "AthenaKernel/CondCont.h"
#include "MuonAlignmentData/ALinePar.h"
#include "MuonAlignmentData/BLinePar.h"
#include "MuonAlignmentData/MdtAsBuiltPar.h"

#include <set>

using ALineContainer = std::set<ALinePar, std::less<>>;
CLASS_DEF( ALineContainer , 1206027754 , 1 );
CONDCONT_DEF( ALineContainer , 1165315046 );

using BLineContainer =  std::set<BLinePar, std::less<>>;
CLASS_DEF( BLineContainer , 1162339375 , 1 );
CONDCONT_DEF( BLineContainer , 1122676225 );

using MdtAsBuiltContainer = std::set<MdtAsBuiltPar, std::less<>>;
CLASS_DEF( MdtAsBuiltContainer , 1241118903 , 1 );
CONDCONT_DEF( MdtAsBuiltContainer , 1221636643 );

#endif  // MUONALIGNMENTDATA_ALINEPARCONTAINER_H
