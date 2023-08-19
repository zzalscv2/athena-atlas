/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TGCWireDoubletSB_hh
#define TGCWireDoubletSB_hh
#include "TrigT1TGC/TGCDoubletSB.h"

namespace LVL1TGCTrigger {

class TGCWireDoubletSB : public TGCDoubletSB {
public:
  TGCWireDoubletSB():TGCDoubletSB(){
    m_maxDev=7;
    m_nChAdj=4;
    m_iChBase=9;
    m_posMaxDev=7;
    m_negMaxDev=-7;
    m_SType = WIRE;
  };
  ~TGCWireDoubletSB(){};

protected:

};

} //end of namespace bracket

#endif
