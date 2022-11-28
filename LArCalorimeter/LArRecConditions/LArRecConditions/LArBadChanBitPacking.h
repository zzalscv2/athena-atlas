/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LArBadChanBitPacking_H
#define LArBadChanBitPacking_H

#include"LArRecConditions/LArBadChanBitPackingBase.h"

typedef TLArBadChanBitPackingBase<LArBadChannel::LArBadChannelSCEnum> LArBadChanSCBitPackingBase;
typedef TLArBadChanBitPackingBase<LArBadChannel::LArBadChannelEnum> LArBadChanBitPackingBase;

class LArBadChanBitPacking : public LArBadChanBitPackingBase {

  public:
     LArBadChanBitPacking();  

};

class LArBadChanSCBitPacking : public LArBadChanSCBitPackingBase {

  public:
     LArBadChanSCBitPacking();  

};

#endif

