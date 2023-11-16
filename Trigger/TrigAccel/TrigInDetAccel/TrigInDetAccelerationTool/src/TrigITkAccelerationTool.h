/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGINDETACCELERATIONTOOL_TRIGITKACCELERATIONTOOL_H
#define TRIGINDETACCELERATIONTOOL_TRIGITKACCELERATIONTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"

#include "TrigInDetAccelerationTool/ITrigInDetAccelerationTool.h"

#include "TrigInDetAccelerationService/ITrigInDetAccelerationSvc.h"

class TrigITkAccelerationTool: public extends<AthAlgTool, ITrigInDetAccelerationTool> {

 public:
  TrigITkAccelerationTool( const std::string&, const std::string&, const IInterface* );
  virtual StatusCode initialize() override;

  size_t virtual exportSeedMakingJob(const TrigCombinatorialSettings&, const IRoiDescriptor*, const std::vector<TrigSiSpacePointBase>&, TrigAccel::DATA_EXPORT_BUFFER&) const override;
  virtual int extractTripletsFromOutput(std::shared_ptr<TrigAccel::OffloadBuffer>, const std::vector<TrigSiSpacePointBase>&, std::vector<TrigInDetTriplet>&) const override;
 private:

  ServiceHandle<ITrigInDetAccelerationSvc> m_accelSvc;
};

#endif