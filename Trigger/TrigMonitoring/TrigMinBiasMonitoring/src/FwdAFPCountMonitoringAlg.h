/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGMINBIASMONITORING_FWDAFPCOUNTMONITORINGALG_H
#define TRIGMINBIASMONITORING_FWDAFPCOUNTMONITORINGALG_H

// Framework include(s):
#include <AthenaMonitoring/AthMonitorAlgorithm.h>
#include <AthenaMonitoringKernel/Monitored.h>

// xAOD
#include <xAODForward/AFPTrackContainer.h>
#include <xAODJet/JetContainer.h>

class FwdAFPCountMonitoringAlg : public AthMonitorAlgorithm {
 public:
  FwdAFPCountMonitoringAlg(const std::string& name, ISvcLocator* pSvcLocator);

  virtual StatusCode initialize() override;
  virtual StatusCode fillHistograms(const EventContext& context) const override;

 private:
  Gaudi::Property<std::vector<std::string>> m_chains{this, "chains", {}, "Chains to monitor"};
};

#endif
