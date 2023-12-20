/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MuonCalibRunLumiBlockCoolSvc_h
#define MuonCalibRunLumiBlockCoolSvc_h

#include "AthenaBaseComps/AthService.h"
#include "GaudiKernel/IService.h"

// c - c++
#include <string>

const InterfaceID IID_IMuonCalibRunLumiBlockCoolSvc("MuonCalibRunLumiBlockCoolSvc", 1, 0);

class MuonCalibRunLumiBlockCoolSvc : public AthService {
public:
    MuonCalibRunLumiBlockCoolSvc(const std::string &name, ISvcLocator *svc_locator);
    virtual ~MuonCalibRunLumiBlockCoolSvc();
    static const InterfaceID &interfaceID() { return IID_IMuonCalibRunLumiBlockCoolSvc; }
    virtual StatusCode queryInterface(const InterfaceID &riid, void **ppvUnknown);
    virtual StatusCode initialize();
    virtual StatusCode finalize();

    // retrieve lumiblock for timestamp. If run_number<0 get run number too.
    StatusCode GetRunEventNumber(unsigned int timestamp, int &run_number, int &lb_nr);
    
};

#endif
