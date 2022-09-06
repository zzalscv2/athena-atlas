/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#ifndef HLTSEEDING_EMROISUNPACKINGTOOL_H
#define HLTSEEDING_EMROISUNPACKINGTOOL_H

#include "RoIsUnpackingToolBase.h"

#include "TrigT1Interfaces/RecEmTauRoI.h"
#include "TrigT1Interfaces/CPRoIDecoder.h"
#include "StoreGate/WriteHandleKey.h"

class EMRoIsUnpackingTool : public RoIsUnpackingToolBase { 
public: 

  EMRoIsUnpackingTool(const std::string& type,
                      const std::string& name, 
                      const IInterface* parent);

  using RoIsUnpackingToolBase::unpack;
  StatusCode unpack(const EventContext& ctx,
                    const ROIB::RoIBResult& roib,
                    const HLT::IDSet& activeChains) const override;
  
  virtual StatusCode initialize() override;
  virtual StatusCode start() override;
  
private: 
  SG::WriteHandleKey< DataVector<LVL1::RecEmTauRoI> > m_recRoIsKey{
    this, "OutputRecRoIs", "HLT_RecEMRoIs",
    "Name of the RoIs object produced by the unpacker"};

  Gaudi::Property<float> m_roIWidth{
    this, "RoIWidth", 0.1, "Size of RoI in eta/ phi"};

  LVL1::CPRoIDecoder m_cpDecoder;
};

#endif //> !HLTSEEDING_EMROISUNPACKINGTOOL_H
