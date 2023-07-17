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


class ZdcLucrodDecoder : public AthMessaging 
{
  enum {ROD_MARKER = 0xee1234ee,
	ROD_NCHANNELS = 8};

 public: 
  
 ZdcLucrodDecoder(unsigned int expectedSrcIDHigh, unsigned int expectedRODVersion = 0x301) :  
  AthMessaging(Athena::getMessageSvc(), "ZdcLucrodDecoder"),
    m_sourceIdHigh(expectedSrcIDHigh), m_rodVersion(expectedRODVersion)
  {};

  
  ~ZdcLucrodDecoder() {};

  StatusCode decode(const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment* robFragment, ZdcLucrodData* zld);
  
  MsgStream& msg(MSG::Level lvl) const { return AthMessaging::msg() << lvl; }
  
  bool msgLevel(MSG::Level lvl) const { return AthMessaging::msgLvl(lvl); }
  
 private:

  unsigned short m_sourceIdHigh;
  unsigned short m_rodVersion;
}; 

#endif
