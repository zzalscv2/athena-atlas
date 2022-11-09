/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ZDC_LUCRODDECODER_H
#define ZDC_LUCRODDECODER_H

#include <inttypes.h>

#include "ByteStreamData/RawEvent.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/Bootstrap.h"
#include "AthenaBaseComps/AthMessaging.h"
#include "AthenaKernel/getMessageSvc.h"

#include "ZdcByteStream/ZdcLucrodData.h"

#define ROD_MARKER        0xee1234ee
#define ROD_HEADER_SIZE   9
#define ROD_TRAILER_SIZE  3
#define ROD_VERSION       0x03010000
#define ROD_SOURCE_ID     0x00830000
#define ROD_NSTATUS       1
//#define ROD_NDATA         165
//#define ROD_FRAGMENT_SIZE 178
#define ROD_NDATA         101
#define ROD_FRAGMENT_SIZE 114
#define ROD_STATUS_POS    1
#define ROD_NCHANNELS     8


class ZdcLucrodDecoder : public AthMessaging {
  
 public: 
  
 ZdcLucrodDecoder() :  AthMessaging(Athena::getMessageSvc(), "ZdcLucrodDecoder")  {};
  ~ZdcLucrodDecoder() {};
  
  StatusCode decode(const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment* robFragment, ZdcLucrodData* zld);
  
  MsgStream& msg(MSG::Level lvl) const { return AthMessaging::msg() << lvl; }
  
  bool msgLevel(MSG::Level lvl) const { return AthMessaging::msgLvl(lvl); }
  
 private:
  
}; 

#endif
