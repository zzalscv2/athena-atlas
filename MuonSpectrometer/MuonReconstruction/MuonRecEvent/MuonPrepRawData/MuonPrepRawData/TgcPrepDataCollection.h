/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONPREPRAWDATA_TGCPREPDATACOLLECTION_H
#define MUONPREPRAWDATA_TGCPREPDATACOLLECTION_H

#include "MuonPrepRawData/TgcPrepData.h"
#include "MuonPrepRawData/MuonPrepDataCollection.h"
#include "AthenaKernel/CLASS_DEF.h"

namespace Muon {
    /**Overload of << operator for MsgStream for debug output*/
    MsgStream& operator << ( MsgStream& sl, const TgcPrepDataCollection& coll);
    
    /**Overload of << operator for std::ostream for debug output*/ 
    std::ostream& operator << ( std::ostream& sl, const TgcPrepDataCollection& coll);
    
}

CLASS_DEF(Muon::TgcPrepDataCollection, 2011800034, 1)


#endif
