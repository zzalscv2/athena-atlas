/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TRIGMINBIASMONITORING_FWDZDCMONITORINGALG_H
#define TRIGMINBIASMONITORING_FWDZDCMONITORINGALG_H

#include <string>

#include "AthenaMonitoring/AthMonitorAlgorithm.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "StoreGate/ReadHandleKey.h"
// Input Containers
#include "xAODForward/ZdcModuleContainer.h"
#include "TrigDecisionTool/TrigDecisionTool.h"

/**
 * @class FwdZDCMonitoringAlg
 * @brief
 **/
class FwdZDCMonitoringAlg : public AthMonitorAlgorithm
{
public:
  FwdZDCMonitoringAlg(const std::string &name, ISvcLocator *pSvcLocator);
  virtual ~FwdZDCMonitoringAlg();
  virtual StatusCode initialize() override;
  virtual StatusCode fillHistograms(const EventContext &context) const override;

private:
  Gaudi::Property<std::vector<std::string>> m_triggerList{
      this, "triggerList", {}, "Add triggers to this to be monitored"};
  SG::ReadHandleKey<xAOD::ZdcModuleContainer> m_zdcModuleContainerKey{
      this, "ZdcModuleContainerKey", "ZdcModules", "Read handle key for ZdcModuleContainer"};
};

#endif // TRIGMINBIASMONITORING_FWDZDCMONITORINGALG_H

