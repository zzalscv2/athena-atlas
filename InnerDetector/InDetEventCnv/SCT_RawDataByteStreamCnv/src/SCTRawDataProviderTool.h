/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef SCT_RAWDATABYTESTREAMCNV_SCTRAWDATAPROVIDERTOOL_H
#define SCT_RAWDATABYTESTREAMCNV_SCTRAWDATAPROVIDERTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"
#include "ByteStreamData/RawEvent.h"
#include "InDetRawData/SCT_RDO_Container.h"
#include "GaudiKernel/IIncidentListener.h"
#include "SCT_RawDataByteStreamCnv/ISCTRawDataProviderTool.h"

#include <set>
#include <string>


class ISCT_RodDecoder;

class SCTRawDataProviderTool : virtual public ISCTRawDataProviderTool, 
  virtual public AthAlgTool
{

 public:
   
  //! AlgTool InterfaceID
  //  static const InterfaceID& interfaceID( ) ;
  
  //! constructor
  SCTRawDataProviderTool(const std::string& type, const std::string& name,
                         const IInterface* parent ) ;

  //! destructor 
  virtual ~SCTRawDataProviderTool() = default;

  //! initialize
  virtual StatusCode initialize() override;

  //! finalize is empty, unnecessary to override
  
  //! this is the main decoding method
  virtual StatusCode convert(std::vector<const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment*>& vecRobs,
                             ISCT_RDO_Container& rdoIdc,
                             InDetBSErrContainer* errs,
                             SCT_ByteStreamFractionContainer* bsFracCont) override;

  virtual void BeginNewEvent() override;

 private: 
  
  ToolHandle<ISCT_RodDecoder> m_decoder{this, "Decoder", "SCT_RodDecoder", "Decoder"};
  
  // bookkeeping if we have decoded a ROB already
  std::set<uint32_t> m_robIdSet;
};

#endif // SCT_RAWDATABYTESTREAMCNV_SCTRAWDATAPROVIDERTOOL_H
