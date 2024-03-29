/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// Top algorithm to decode the BCM BS
///////////////////////////////////////////////////////////////////

#ifndef BCM_RAWDATABYTESTREAMCNV_BCM_RAWDATAPROVIDER_H
#define BCM_RAWDATABYTESTREAMCNV_BCM_RAWDATAPROVIDER_H

#include "InDetBCM_RawData/BCM_RDO_Container.h"
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/WriteHandleKey.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "ByteStreamCnvSvcBase/IROBDataProviderSvc.h"

class BCM_RawDataProviderTool;

class BCM_RawDataProvider : public AthReentrantAlgorithm
{
public:

  //! delegate to base-class constructor.
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  //! Initialize
  virtual StatusCode initialize() override;

  //! Execute
  virtual StatusCode execute(const EventContext& ctx) const override;

  //! Finalize
  virtual StatusCode finalize() override
    { return StatusCode::SUCCESS; }

  //! Destructor
  virtual ~BCM_RawDataProvider();

private:

  ServiceHandle<IROBDataProviderSvc>   m_robDataProvider{this,"ROBDataProviderSvc","ROBDataProviderSvc"};
  ToolHandle<BCM_RawDataProviderTool>  m_rawDataTool{this,"ProviderTool","BCM_RawDataProviderTool"};
  SG::WriteHandleKey<BCM_RDO_Container> m_RDO_Key{this, "RDOKey", "BCM_RDOs", "" };
};

#endif

