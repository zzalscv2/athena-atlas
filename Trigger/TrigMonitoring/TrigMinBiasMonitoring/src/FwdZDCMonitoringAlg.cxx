/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "FwdZDCMonitoringAlg.h"


FwdZDCMonitoringAlg::FwdZDCMonitoringAlg(const std::string &name, ISvcLocator *pSvcLocator)
    : AthMonitorAlgorithm(name, pSvcLocator) {}

FwdZDCMonitoringAlg::~FwdZDCMonitoringAlg() {}

StatusCode FwdZDCMonitoringAlg::initialize()
{
  ATH_CHECK(m_zdcModuleContainerKey.initialize());
  return AthMonitorAlgorithm::initialize();
}

StatusCode FwdZDCMonitoringAlg::fillHistograms(const EventContext &context) const
{
  const auto &trigDecTool = getTrigDecisionTool();

  SG::ReadHandle<xAOD::ZdcModuleContainer> zdcModHandle = SG::makeHandle(m_zdcModuleContainerKey, context);

  if (!zdcModHandle.isValid())
  {
    ATH_MSG_ERROR("evtStore() does not contain Zdc Collection with name " << m_zdcModuleContainerKey);
    return StatusCode::FAILURE;
  }

  for (const auto &trig : m_triggerList)
  {
     if (!trigDecTool->isPassed(trig, TrigDefs::Physics))
    {
      continue;
    }

    ATH_MSG_DEBUG("Chain " << trig << " is passed: YES");

    auto TrigCounts = Monitored::Scalar<std::string>("TrigCounts", trig);
    fill("ZDCall", TrigCounts);

    // declare the quantities which should be monitored
    auto e_A = Monitored::Scalar<float>("e_A", 0.0);     // ZDC ADC on Side A
    auto e_C = Monitored::Scalar<float>("e_C", 0.0);     // ZDC ADC on Side C
    auto moduleEnergy = Monitored::Scalar<float>("moduleEnergy", 0.0);
    auto moduleNum = Monitored::Scalar<float>("moduleNum", 0.0);

    // read single modules
    const xAOD::ZdcModuleContainer *zdcSingleModules = zdcModHandle.cptr();
    for (const auto zdcSM : *zdcSingleModules)
    {
      if (zdcSM->zdcType() == 0)
      { // type = 0 are big modules, type = 1 the pixels
        // Side A
        if (zdcSM->zdcSide() > 0)
        {
          moduleEnergy = zdcSM->auxdataConst<float>("CalibEnergy");
          moduleNum = zdcSM->zdcModule();
          e_A += zdcSM->auxdataConst<float>("CalibEnergy");
          fill(trig + "_expert", moduleEnergy, moduleNum);
          fill("ZDCall", moduleEnergy, moduleNum);
        }
        // Side C
        if (zdcSM->zdcSide() < 0)
        {
          moduleEnergy = zdcSM->auxdataConst<float>("CalibEnergy");
          moduleNum = zdcSM->zdcModule() + 4.;
          e_C += zdcSM->auxdataConst<float>("CalibEnergy");
          fill(trig + "_expert", moduleEnergy, moduleNum);
          fill("ZDCall", moduleEnergy, moduleNum);
        }
      }
    }
    fill(trig + "_expert", e_A, e_C);
    fill("ZDCall", e_A, e_C);
    }
  return StatusCode::SUCCESS;
}
