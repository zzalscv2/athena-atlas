/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/


#ifndef MUONPREPRAWDATA_CSCPREPDATACOLLECTION_H
#define MUONPREPRAWDATA_CSCPREPDATACOLLECTION_H

#include "MuonPrepRawData/CscPrepData.h"
#include "MuonPrepRawData/MuonPrepDataCollection.h"
#include "AthenaKernel/CLASS_DEF.h"

namespace Muon {
    /**Overload of << operator for MsgStream for debug output*/
    MsgStream& operator << ( MsgStream& sl, const CscPrepDataCollection& coll);

    /**Overload of << operator for std::ostream for debug output*/ 
    std::ostream& operator << ( std::ostream& sl, const CscPrepDataCollection& coll);
    
}


CLASS_DEF(Muon::CscPrepDataCollection, 2081800031, 1)

#endif
