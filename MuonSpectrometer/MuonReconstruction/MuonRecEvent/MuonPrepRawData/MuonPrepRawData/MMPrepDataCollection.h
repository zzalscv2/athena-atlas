/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/


#ifndef MUONPREPRAWDATA_MMPREPDATACOLLECTION_H
#define MUONPREPRAWDATA_MMPREPDATACOLLECTION_H

#include "MuonPrepRawData/MMPrepData.h"
#include "MuonPrepRawData/MuonPrepDataCollection.h"
#include "AthenaKernel/CLASS_DEF.h"

namespace Muon {
   
    /**Overload of << operator for MsgStream for debug output*/
    MsgStream& operator << ( MsgStream& sl, const MMPrepDataCollection& coll);
    
    /**Overload of << operator for std::ostream for debug output*/ 
    std::ostream& operator << ( std::ostream& sl, const MMPrepDataCollection& coll);
    
}

CLASS_DEF(Muon::MMPrepDataCollection, 1297888256, 1)

#endif
