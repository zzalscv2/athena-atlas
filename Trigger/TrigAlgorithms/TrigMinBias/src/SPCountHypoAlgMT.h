/*
Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TRIGMINBIAS_SPCOUNTHYPOALGMT_H
#define TRIGMINBIAS_SPCOUNTHYPOALGMT_H

// Framework includes
#include "SPCountHypoTool.h"
#include "DecisionHandling/HypoBase.h"
#include "xAODTrigger/TrigCompositeContainer.h"
#include "DecisionHandling/TrigCompositeUtils.h"

// STL includes
#include <string>

class SPCountHypoAlgMT : public ::HypoBase
{

public:
  SPCountHypoAlgMT(const std::string &name, ISvcLocator *pSvcLocator);

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext &context) const override;

private:
  SPCountHypoAlgMT();
  ToolHandleArray<SPCountHypoTool> m_hypoTools{this, "HypoTools", {}, "Tools that perform actual selection"};
  SG::ReadHandleKey<xAOD::TrigCompositeContainer> m_spacePointsKey{this, "SpacePointsKey", "Undefined"};
};
#endif // TRIGMINBIAS_SPCOUNTHYPOALGMT_H
