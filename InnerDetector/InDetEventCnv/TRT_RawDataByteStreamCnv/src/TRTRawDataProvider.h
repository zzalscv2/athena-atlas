/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// Top algorithm to decode the TRT BS
///////////////////////////////////////////////////////////////////

#ifndef TRT_RAWDATABYTESTREAMCNV_TRTRAWDATAPROVIDER_H
#define TRT_RAWDATABYTESTREAMCNV_TRTRAWDATAPROVIDER_H

// Base class
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "ByteStreamCnvSvcBase/IROBDataProviderSvc.h"
#include "InDetIdentifier/TRT_ID.h"
#include "InDetRawData/TRT_RDO_Container.h"
#include "TRT_RawDataByteStreamCnv/ITRTRawDataProviderTool.h"
#include "TRT_Cabling/ITRT_CablingSvc.h"

#include "AthenaBaseComps/AthAlgorithm.h"

#include <string>

class TRTRawDataProviderTool;
class TRT_CablingSvc;


class TRTRawDataProvider : public AthAlgorithm
{
public:

  //! Constructor.
  TRTRawDataProvider(const std::string &name, ISvcLocator *pSvcLocator);

  //! Initialize
  virtual StatusCode initialize();

  //! Execute
  virtual StatusCode execute();

  //! Finalize
  virtual StatusCode finalize()
    { return StatusCode::SUCCESS;}

  //! Destructor
  ~TRTRawDataProvider()
    {   
    }

private:

  ServiceHandle<IROBDataProviderSvc>  m_robDataProvider;
  ToolHandle<ITRTRawDataProviderTool> m_rawDataTool;
  ServiceHandle<ITRT_CablingSvc>     m_CablingSvc;

  const TRT_ID*                      m_trt_id; 
  SG::WriteHandleKey<TRT_RDO_Container> m_rdoContainerKey;

};

#endif


