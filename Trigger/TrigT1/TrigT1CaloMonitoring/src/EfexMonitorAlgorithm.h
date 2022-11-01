/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TRIGT1CALOMONITORING_EFEXMONITORALGORITHM_H
#define TRIGT1CALOMONITORING_EFEXMONITORALGORITHM_H

#include "AthenaMonitoring/AthMonitorAlgorithm.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "StoreGate/ReadHandleKey.h"
//
#include "xAODTrigger/eFexEMRoIContainer.h"
#include "xAODTrigger/eFexEMRoI.h"
#include "xAODTrigger/eFexTauRoIContainer.h"
#include "xAODTrigger/eFexTauRoI.h"

class EfexMonitorAlgorithm : public AthMonitorAlgorithm {
public:EfexMonitorAlgorithm( const std::string& name, ISvcLocator* pSvcLocator );
  virtual ~EfexMonitorAlgorithm()=default;
  virtual StatusCode initialize() override;
  virtual StatusCode fillHistograms( const EventContext& ctx ) const override;

private:

  StringProperty m_packageName{this,"PackageName","EfexMonitor","group name for histograming"};
  Gaudi::Property<float> m_lowPtCut{this,"LowPtCut",0.0,"The Et value for the low Pt cut (probably 0)"};
  Gaudi::Property<float> m_hiPtCut{this,"HiPtCut",15.0,"The Et value for the high Pt cut"};

  // container keys including this, steering parameter, default value and help description
  SG::ReadHandleKey<xAOD::eFexEMRoIContainer> m_eFexContainerKey{this,"eFexEMRoIContainer","L1_eEMxRoI","SG key of the input eFex RoI container"};
  SG::ReadHandleKey<xAOD::eFexTauRoIContainer> m_eFexTauContainerKey{this,"eFexTauRoIContainer","L1_eTauxRoI","SG key of the input eFex Tau RoI container"};

  StatusCode fillEMHistograms(const std::string& cut_name, const xAOD::eFexEMRoIContainer *emcont, const float &cut_et) const;
  StatusCode fillTauHistograms(const std::string& cut_name, const xAOD::eFexTauRoIContainer *taucont, const float &cut_et) const;

};
#endif
