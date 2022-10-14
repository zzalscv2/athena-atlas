/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ZDC_BYTESTREAMLUCRODDATA_H
#define ZDC_BYTESTREAMLUCRODDATA_H

#include <stdint.h>
#include <map>
#include <string>

#include "AthenaBaseComps/AthAlgorithm.h"

#include "ByteStreamCnvSvcBase/IROBDataProviderSvc.h"

#include "ZdcByteStream/ZdcLucrodDataContainer.h"
#include "ZdcByteStream/ZdcLucrodData.h"
#include "ZdcByteStream/ZdcLucrodDecoder.h"
#include "StoreGate/WriteHandleKey.h"
#include "ZdcByteStream/ZdcDefs.h"

class StoreGateSvc;
class ZdcLucrodDecoder;

using OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment;

class ZdcByteStreamLucrodData: public AthAlgorithm {

 public:
  
  ZdcByteStreamLucrodData (const std::string &name, ISvcLocator* pSvcLocator);
  ~ZdcByteStreamLucrodData() = default;

  StatusCode initialize();
  StatusCode execute();
  StatusCode finalize();

  StatusCode fillContainer(std::vector<const ROBFragment*>, ZdcLucrodDataContainer* zdcLucrodDataContainer); 
  
private:
  
  ServiceHandle<IROBDataProviderSvc> m_robDataProvider;
  
  SG::WriteHandleKey<ZdcLucrodDataContainer> m_ZdcLucrodDataContainerKey{ this, "ZdcLucrodDataContainerLocation", ZdcDefs::ZdcLucrodDataContainerLocation, "" };
  ZdcLucrodDecoder        m_ZdcLucrodDecoder;
};

#endif
