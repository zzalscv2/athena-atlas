/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "H5FileSvc.h"
#include "H5Cpp.h"

H5FileSvc::H5FileSvc(const std::string& name, ISvcLocator* pSvcLocator):
  AthService(name, pSvcLocator)
{
}

H5FileSvc::~H5FileSvc() = default;

StatusCode H5FileSvc::initialize() {
  if (m_file_path.empty()) {
    return StatusCode::FAILURE;
  }
  m_file = std::make_unique<H5::H5File>(m_file_path, H5F_ACC_TRUNC);
  return StatusCode::SUCCESS;
}

H5::Group* H5FileSvc::group() {
  return m_file.get();
}

// interface magic, sort of copied from
// Control/AthenaBaseComps/src/AthCnvSvc.cxx
// There's another example under PerfMonComps
// https://gitlab.cern.ch/atlas/athena/-/tree/main/Control/PerformanceMonitoring/PerfMonComps
//
StatusCode H5FileSvc::queryInterface(const InterfaceID& riid,
                                     void** ppvInterface)
{
  if ( IH5GroupSvc::interfaceID().versionMatch(riid) )  {
    // This thing seems to get called when we run the code.
    //
    *ppvInterface = dynamic_cast<IH5GroupSvc*>(this);
  } else {
    // And this thing gets called when the code is being compiled
    return AthService::queryInterface(riid, ppvInterface);
  }
  addRef();
  return StatusCode::SUCCESS;
}
