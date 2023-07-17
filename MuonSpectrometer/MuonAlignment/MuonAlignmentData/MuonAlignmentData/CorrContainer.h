/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONALIGNMENTDATA_CORRCONTAINER_H
#define MUONALIGNMENTDATA_CORRCONTAINER_H

#include "AthenaKernel/CLASS_DEF.h"
#include "AthenaKernel/CondCont.h"
#include "MuonAlignmentData/ALinePar.h"
#include "MuonAlignmentData/BLinePar.h"
#include "MuonAlignmentData/CscInternalAlignmentPar.h"
#include "MuonAlignmentData/MdtAsBuiltPar.h"
#include "MuonIdHelpers/MdtIdHelper.h"

using ALineContainer = std::set<ALinePar, std::less<>>;
CLASS_DEF( ALineContainer , 1206027754 , 1 );
CONDCONT_DEF( ALineContainer , 1165315046 );

using BLineContainer =  std::set<BLinePar, std::less<>>;
CLASS_DEF( BLineContainer , 1162339375 , 1 );
CONDCONT_DEF( BLineContainer , 1122676225 );


typedef std::map<Identifier, CscInternalAlignmentPar> CscInternalAlignmentMapContainer;
CLASS_DEF(CscInternalAlignmentMapContainer, 1285567354, 1)
CLASS_DEF(CondCont<CscInternalAlignmentMapContainer>, 1227105862, 0)

typedef std::map<Identifier, MdtAsBuiltPar> MdtAsBuiltMapContainer;
CLASS_DEF(MdtAsBuiltMapContainer, 1198729422, 1)
CLASS_DEF(CondCont<MdtAsBuiltMapContainer>, 1076645826, 0)

typedef CscInternalAlignmentMapContainer::iterator iCscInternalAlignmentMap;
typedef MdtAsBuiltMapContainer::iterator iMdtAsBuiltMap;

#endif  // MUONALIGNMENTDATA_ALINEPARCONTAINER_H
