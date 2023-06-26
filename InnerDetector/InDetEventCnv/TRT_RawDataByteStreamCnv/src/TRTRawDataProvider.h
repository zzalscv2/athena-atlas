/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// Top algorithm to decode the TRT BS
///////////////////////////////////////////////////////////////////

#ifndef TRT_RAWDATABYTESTREAMCNV_TRTRAWDATAPROVIDER_H
#define TRT_RAWDATABYTESTREAMCNV_TRTRAWDATAPROVIDER_H

// Base class
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "ByteStreamCnvSvcBase/IROBDataProviderSvc.h"
#include "InDetIdentifier/TRT_ID.h"
#include "InDetRawData/TRT_RDO_Container.h"
#include "TRT_RawDataByteStreamCnv/ITRTRawDataProviderTool.h"
#include "TRT_Cabling/ITRT_CablingSvc.h"
#include "IRegionSelector/IRegSelTool.h"
#include "TrigSteeringEvent/TrigRoiDescriptorCollection.h"

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "InDetByteStreamErrors/TRT_BSErrContainer.h"

#include <string>

class TRTRawDataProviderTool;
class TRT_CablingSvc;


class TRTRawDataProvider : public AthReentrantAlgorithm
{
public:

  //! Constructor.
  TRTRawDataProvider(const std::string &name, ISvcLocator *pSvcLocator);

  //! Initialize
  virtual StatusCode initialize();

  //! Execute
  virtual StatusCode execute(const EventContext& ctx) const;

  //! Finalize
  virtual StatusCode finalize()
    { return StatusCode::SUCCESS;}

  //! Destructor
  virtual ~TRTRawDataProvider() = default;

private:
 ToolHandle<IRegSelTool> m_regionSelector{this, "RegSelTool",
                                          "RegSelTool/RegSelTool_TRT"};
 Gaudi::Property<bool> m_useDataPoolWithCache{this, "useDataPoolWithCache",
                                              false, "use DataPool With Cache"};
 ServiceHandle<IROBDataProviderSvc> m_robDataProvider;
 ToolHandle<ITRTRawDataProviderTool> m_rawDataTool;
 ServiceHandle<ITRT_CablingSvc> m_CablingSvc;
 const TRT_ID* m_trt_id;

 bool m_roiSeeded;
 SG::ReadHandleKey<TrigRoiDescriptorCollection> m_roiCollectionKey;
 SG::WriteHandleKey<TRT_RDO_Container> m_rdoContainerKey;
 SG::WriteHandleKey<TRT_BSErrContainer> m_bsErrContKey;
 SG::UpdateHandleKey<TRT_RDO_Cache> m_rdoCacheKey;

};

#endif


