/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef H5_FILE_SVC_H
#define H5_FILE_SVC_H

#include "AthenaBaseComps/AthService.h"
#include "Gaudi/Property.h"

#include "HDF5Utils/IH5GroupSvc.h"

#include <memory>

namespace H5 {
  class H5File;
}

class H5FileSvc : public IH5GroupSvc, public AthService
{
public:
  H5FileSvc(const std::string& name, ISvcLocator* pSvcLocator);
  ~H5FileSvc();
  StatusCode initialize() override;
  H5::Group* group() override;
private:
  StatusCode queryInterface(const InterfaceID& riid,
                            void** ppvInterface) override;

  std::unique_ptr<H5::H5File> m_file{nullptr};
  Gaudi::Property<std::string> m_file_path {this, "path", "", "path to file"};
};

#endif
