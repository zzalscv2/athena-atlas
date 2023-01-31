/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TRIGT1CALOMONITORING_EFEXSIMMONITORALGORITHM_H
#define TRIGT1CALOMONITORING_EFEXSIMMONITORALGORITHM_H

#include "AthenaMonitoring/AthMonitorAlgorithm.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "StoreGate/ReadHandleKey.h"
//
#include "xAODTrigger/eFexEMRoIContainer.h"
#include "xAODTrigger/eFexEMRoI.h"
#include "xAODTrigger/eFexTauRoIContainer.h"
#include "xAODTrigger/eFexTauRoI.h"

class EfexSimMonitorAlgorithm : public AthMonitorAlgorithm {
public:EfexSimMonitorAlgorithm( const std::string& name, ISvcLocator* pSvcLocator );
  virtual ~EfexSimMonitorAlgorithm()=default;
  virtual StatusCode initialize() override;
  virtual StatusCode fillHistograms( const EventContext& ctx ) const override;

private:

  StringProperty m_packageName{this,"PackageName","EfexSimMonitor","group name for histograming"};

  // container keys including this, steering parameter, default value and help description
  SG::ReadHandleKey<xAOD::eFexEMRoIContainer> m_eFexEmContainerKey{this,"eFexEMRoIContainer","L1_eEMRoI","SG key of the data eFex Em RoI container"};
  SG::ReadHandleKey<xAOD::eFexTauRoIContainer> m_eFexTauContainerKey{this,"eFexTauRoIContainer","L1_eTauRoI","SG key of the data eFex Tau RoI container"};
  SG::ReadHandleKey<xAOD::eFexEMRoIContainer> m_eFexEmSimContainerKey{this,"eFexEMRoISimContainer","L1_eEMRoISim","SG key of the simulated eFex Em RoI container"};
  SG::ReadHandleKey<xAOD::eFexTauRoIContainer> m_eFexTauSimContainerKey{this,"eFexTauSimRoIContainer","L1_eTauRoISim","SG key of the simulated eFex Tau RoI container"};

  StatusCode fillEmErrorHistos(const std::string &errName, const xAOD::eFexEMRoIContainer *emcont, const std::set<uint32_t> &simEqDataCoords) const;
  StatusCode fillTauErrorHistos(const std::string &errName, const xAOD::eFexTauRoIContainer *taucont, const std::set<uint32_t> &simEqDataCoords) const;

};
#endif
