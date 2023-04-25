/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGMINBIASMONITORING_FWDAFPJETEFFMONITORINGALG_H
#define TRIGMINBIASMONITORING_FWDAFPJETEFFMONITORINGALG_H

// Framework include(s):
#include <AthenaMonitoring/AthMonitorAlgorithm.h>
#include <AthenaMonitoringKernel/Monitored.h>

// xAOD
#include <xAODForward/AFPTrackContainer.h>
#include <xAODJet/JetContainer.h>

class FwdAFPJetEffMonitoringAlg : public AthMonitorAlgorithm {
 public:
  FwdAFPJetEffMonitoringAlg(const std::string& name, ISvcLocator* pSvcLocator);

  virtual StatusCode initialize() override;
  virtual StatusCode fillHistograms(const EventContext& context) const override;

 private:
  Gaudi::Property<std::vector<std::string>> m_chains{this, "chains", {}, "Chains to monitor with specific reference"};
  Gaudi::Property<std::vector<std::string>> m_references{this, "references", {}, "Specific reference chains"};
  Gaudi::Property<SG::ReadHandleKey<xAOD::JetContainer>> m_jetKey{
      this, "jetContainer", "HLT_AntiKt4EMTopoJets_subjesIS", "SG key for the jet container"};
};

#endif
