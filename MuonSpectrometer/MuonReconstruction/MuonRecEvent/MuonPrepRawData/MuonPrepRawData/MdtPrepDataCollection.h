/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/


#ifndef MUONPREPRAWDATA_MDTPREPDATACOLLECTION_H
#define MUONPREPRAWDATA_MDTPREPDATACOLLECTION_H

#include "MuonPrepRawData/MdtPrepData.h"
#include "MuonPrepRawData/MuonPrepDataCollection.h"
#include "AthenaKernel/CLASS_DEF.h"

namespace Muon {

    /**Overload of << operator for MsgStream for debug output*/
    MsgStream& operator << ( MsgStream& sl, const MdtPrepDataCollection& coll);
    
    /**Overload of << operator for std::ostream for debug output*/ 
    std::ostream& operator << ( std::ostream& sl, const MdtPrepDataCollection& coll);
    
}

CLASS_DEF(Muon::MdtPrepDataCollection, 2061800032, 1)

#endif
