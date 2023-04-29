/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 CLASS_DEF for InDetSimData map
 ------------------------------
 ATLAS Collaboration
 ***************************************************************************/

// $Id: InDetSimDataCollection.h,v 1.3 2004-04-05 23:16:07 costanzo Exp $

#ifndef INDETSIMDATA_INDETSIMDATACOLLECTION_H
#define INDETSIMDATA_INDETSIMDATACOLLECTION_H

#include "AthenaKernel/CLASS_DEF.h"
#include "Identifier/Identifier.h"
#include "InDetSimData/InDetSimData.h"
// std includes
#include <map>
class InDetSimDataCollection
    : public std::map<Identifier, InDetSimData>{
  // empty
};
CLASS_DEF(InDetSimDataCollection, 2543, 1)

#endif  // INDETSIMDATA_INDETSIMDATACLASS_DEF_H
