/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONPREPRAWDATA_RPCPREPDATACOLLECTION_H
#define MUONPREPRAWDATA_RPCPREPDATACOLLECTION_H

#include "MuonPrepRawData/RpcPrepData.h"
#include "MuonPrepRawData/MuonPrepDataCollection.h"
#include "AthenaKernel/CLASS_DEF.h"

namespace Muon {
   
    /**Overload of << operator for MsgStream for debug output*/
    MsgStream& operator << ( MsgStream& sl, const RpcPrepDataCollection& coll);
    
    /**Overload of << operator for std::ostream for debug output*/ 
    std::ostream& operator << ( std::ostream& sl, const RpcPrepDataCollection& coll);
    
}

CLASS_DEF(Muon::RpcPrepDataCollection, 2041800033, 1)

#endif
