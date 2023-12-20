/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonCalibStreamCnvSvc/MuonCalibRunLumiBlockCoolSvc.h"

// COOL includes
#include "CoolKernel/ChannelSelection.h"
#include "CoolKernel/IDatabaseSvc.h"
#include "CoolKernel/IFolder.h"
#include "CoolKernel/IObject.h"
#include "CoolKernel/IObjectIterator.h"
#include "CoraCool/CoraCoolDatabase.h"
#include "CoraCool/CoraCoolDatabaseSvc.h"
#include "CoraCool/CoraCoolDatabaseSvcFactory.h"
#include "CoraCool/CoraCoolFolder.h"
#include "CoraCool/CoraCoolObject.h"
#include "CoraCool/CoraCoolObjectIter.h"
#include "CoralBase/Blob.h"

MuonCalibRunLumiBlockCoolSvc::MuonCalibRunLumiBlockCoolSvc(const std::string &name, ISvcLocator *svc_locator) :
    AthService(name, svc_locator) {}

StatusCode MuonCalibRunLumiBlockCoolSvc::queryInterface(const InterfaceID &riid, void **ppvUnknown) {
    if (IID_IMuonCalibRunLumiBlockCoolSvc.versionMatch(riid)) {
        *ppvUnknown = (MuonCalibRunLumiBlockCoolSvc *)this;
    } else {
        return AthService::queryInterface(riid, ppvUnknown);
    }
    return StatusCode::SUCCESS;
}

MuonCalibRunLumiBlockCoolSvc::~MuonCalibRunLumiBlockCoolSvc() {}

StatusCode MuonCalibRunLumiBlockCoolSvc::initialize() { return StatusCode::SUCCESS; }

StatusCode MuonCalibRunLumiBlockCoolSvc::finalize() { return StatusCode::SUCCESS; }

StatusCode MuonCalibRunLumiBlockCoolSvc::GetRunEventNumber(unsigned int timestamp, int &run_number, int &lb_nr) {
    // comment out the implementation of get lb, set lb = 0, to be fixed later
    lb_nr = 0 ;
    ATH_MSG_DEBUG("timestamp : "<<timestamp<<" runNumber : "<<run_number<<" lb : "<<lb_nr);
    return StatusCode::SUCCESS;
}  // MuonCalibRunLumiBlockCoolSvc::GetRunEventNumber()
