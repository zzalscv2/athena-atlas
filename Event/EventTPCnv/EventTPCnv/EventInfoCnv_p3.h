/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EVENTATHENAPOOL_EVENTINFOCNV_P3_H
#define EVENTATHENAPOOL_EVENTINFOCNV_P3_H

#include "EventTPCnv/EventInfo_p3.h"
#include "EventInfo/EventInfo.h"
#include "AthenaPoolCnvSvc/T_AthenaPoolTPConverter.h"

class MsgStream;
class EventInfoCnv_p3  : public T_AthenaPoolTPCnvBase<EventInfo, EventInfo_p3>  {
public:
  EventInfoCnv_p3() {}
  virtual void   persToTrans(const EventInfo_p3* persObj, EventInfo* transObj, MsgStream &log) override;
  virtual void   transToPers(const EventInfo* transObj, EventInfo_p3* persObj, MsgStream &log) override;

  void   persToTrans(const EventInfo_p3* persObj, EventInfo* transObj, MsgStream &log) const;
  void   transToPers(const EventInfo* transObj, EventInfo_p3* persObj, MsgStream &log) const;

  // needed to handle specific default constructor of EventInfo
  virtual EventInfo *createTransient( const EventInfo_p3* persObj, MsgStream &log) override;
  EventInfo *createTransient( const EventInfo_p3* persObj, MsgStream &log) const;
};

template<>
class T_TPCnv<EventInfo, EventInfo_p3>
  : public EventInfoCnv_p3
{
public:
};


#endif
