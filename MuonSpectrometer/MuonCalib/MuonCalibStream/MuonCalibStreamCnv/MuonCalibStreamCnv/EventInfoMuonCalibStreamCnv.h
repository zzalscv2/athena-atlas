/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONCALIBSTREAMCNVSVC_EVENTINFOMUONCALIBSTREAMCNV_H
#define MUONCALIBSTREAMCNVSVC_EVENTINFOMUONCALIBSTREAMCNV_H

#include "AthenaBaseComps/AthMessaging.h"
#include "GaudiKernel/Converter.h"
#include "MuCalDecode/CalibEvent.h"
#include "xAODEventInfo/EventInfo.h"
#include "xAODEventInfo/EventAuxInfo.h"
#include "MuonCalibStreamCnvSvc/MuonCalibStreamAddress.h"


class IOpaqueAddress;
class DataObject;
class StatusCode;
class MuonCalibStreamCnvSvc;
class MuonCalibStreamDataProviderSvc;

#include <string>

// Abstract factory to create the converter
template <class TYPE> class CnvFactory;

class EventInfoMuonCalibStreamCnv : public Converter, public AthMessaging {
    friend class CnvFactory<EventInfoMuonCalibStreamCnv>;

public:
    virtual StatusCode initialize();
    virtual StatusCode createObj(IOpaqueAddress *pAddr, DataObject *&pObj);
    virtual StatusCode createRep(DataObject *pObj, IOpaqueAddress *&pAddr);

    /// Storage type and class ID
    virtual long repSvcType() const { return MuonCalibStreamAddress::storageType(); }
    static long storageType() { return MuonCalibStreamAddress::storageType(); }
    static const CLID &classID();

    EventInfoMuonCalibStreamCnv(ISvcLocator *svcloc);

private:
    MuonCalibStreamCnvSvc *m_MuonCalibStreamCnvSvc;
    MuonCalibStreamDataProviderSvc *m_dataProvider;
};
#endif
