/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/
#ifndef IMuonCalibStreamDataProviderSvc_H
#define IMuonCalibStreamDataProviderSvc_H
#include <inttypes.h>

#include <vector>

#include "GaudiKernel/IInterface.h"
#include "MuCalDecode/CalibEvent.h"

class IMuonCalibStreamDataProviderSvc : virtual public IInterface {
public:
    /// Retrieve interface ID
    static const InterfaceID &interfaceID() {
        // Declaration of the interface ID ( interface id, major version, minor version)

        static const InterfaceID IID_IMuonCalibStreamDataProviderSvc("IMuonCalibStreamDataProviderSvc", 1, 0);

        return IID_IMuonCalibStreamDataProviderSvc;
    }
    virtual void setNextEvent(const LVL2_MUON_CALIBRATION::CalibEvent *re) = 0;
    virtual const LVL2_MUON_CALIBRATION::CalibEvent *getEvent() = 0;

    virtual ~IMuonCalibStreamDataProviderSvc() = default;
};

#endif
